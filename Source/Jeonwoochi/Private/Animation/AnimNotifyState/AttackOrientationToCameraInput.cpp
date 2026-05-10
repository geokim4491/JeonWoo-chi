#include "Animation/AnimNotifyState/AttackOrientationToCameraInput.h"
#include "../../../JeonwoochiCharacter.h"                         // 캐릭터 캐시 사용
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

static inline FVector XY0(const FVector& V) { return FVector(V.X, V.Y, 0.f); }

void UAttackOrientationToCameraInput::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Anim, TotalDuration, EventReference);
    APawn* Pawn = MeshComp ? Cast<APawn>(MeshComp->GetOwner()) : nullptr;
    if (!Pawn) return;

    BeginTime = MeshComp->GetWorld()->GetTimeSeconds();
    CachedCamYaw = Pawn->GetControlRotation().Yaw; // ★ 좌표계 고정

    if (bTemporarilyOverrideMovementRotation)
    {
        if (ACharacter* Char = Cast<ACharacter>(Pawn))
            OverrideRotationPolicyBegin(Char);
    }

    // Begin 시점 입력으로 1회 커밋
    AJeonwoochiCharacter* C = nullptr;
    FVector2D Dir2D; bool bHasInput = false;
    GetCamRelativeInputDir2D(Pawn, C, Dir2D, bHasInput);

    TargetYaw = bHasInput
        ? MakeYawFromDir2D(Dir2D)
        : (bFallbackToCameraForwardIfNoInput ? CachedCamYaw : Pawn->GetActorRotation().Yaw);

    if (bInstantSnapOnBegin)
        Pawn->SetActorRotation(FRotator(0.f, TargetYaw, 0.f));
}

void UAttackOrientationToCameraInput::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Anim, FrameDeltaTime, EventReference);
    APawn* Pawn = MeshComp ? Cast<APawn>(MeshComp->GetOwner()) : nullptr;
    if (!Pawn) return;

    const float Now = MeshComp->GetWorld()->GetTimeSeconds();
    if ((Now - BeginTime) < FollowWindowSec)
    {
        AJeonwoochiCharacter* C = nullptr;
        FVector2D Dir2D; bool bHasInput = false;
        if (GetCamRelativeInputDir2D(Pawn, C, Dir2D, bHasInput) && bHasInput)
        {
            const float NewYaw = MakeYawFromDir2D(Dir2D);
            const float DeltaDeg = FMath::Abs(FMath::FindDeltaAngleDegrees(TargetYaw, NewYaw));
            if (DeltaDeg >= AngleThresholdDeg)
                TargetYaw = NewYaw;
        }
    }

    ApplyInterpRotation(Pawn, FrameDeltaTime);
}

void UAttackOrientationToCameraInput::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Anim, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Anim, EventReference);
    if (APawn* Pawn = MeshComp ? Cast<APawn>(MeshComp->GetOwner()) : nullptr)
    {
        if (bTemporarilyOverrideMovementRotation)
        {
            if (ACharacter* Char = Cast<ACharacter>(Pawn))
                OverrideRotationPolicyEnd(Char);
        }
    }
}

/* ===== Helpers ===== */

bool UAttackOrientationToCameraInput::GetCamRelativeInputDir2D(
    APawn* Pawn, AJeonwoochiCharacter*& OutC, FVector2D& OutDir, bool& bOutHasInput) const
{
    OutC = Cast<AJeonwoochiCharacter>(Pawn);
    OutDir = FVector2D::ZeroVector;
    bOutHasInput = false;

    const float Now = Pawn->GetWorld()->GetTimeSeconds();
    const float Deadzone = OutC ? OutC->MoveInputDeadzone : 0.20f;
    const float HoldWindow = OutC ? OutC->MoveDirHoldSeconds : 0.15f;

    // 1) "현재 프레임의 월드 이동입력"으로 우선 판단
    FVector MoveW = Pawn->GetPendingMovementInputVector();
    if (MoveW.IsNearlyZero()) MoveW = Pawn->GetLastMovementInputVector();
    MoveW.Z = 0.f;

    if (MoveW.Size() >= Deadzone)
    {
        // 카메라(Yaw=CachedCamYaw) 기준으로 분해 → (Right, Forward)
        const FRotator YawOnly(0.f, CachedCamYaw, 0.f);
        const FVector CamFwd = YawOnly.Vector();
        const FVector CamRight = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);

        const FVector MoveN = MoveW.GetSafeNormal();
        const float ForwardVal = FVector::DotProduct(MoveN, CamFwd);
        const float RightVal = FVector::DotProduct(MoveN, CamRight);

        FVector2D Dir2D(RightVal, ForwardVal);
        const float Len = Dir2D.Length();
        if (Len > KINDA_SMALL_NUMBER)
        {
            OutDir = Dir2D / Len;   // 카메라 기준 "정규화" 입력 방향
            bOutHasInput = true;
        }
        return true; // 현재 입력 있었음 → 끝
    }

    // 2) 현재 입력이 없으면, 홀드 윈도우 내의 "최근 유효 방향" 사용
    if (OutC && (Now - OutC->LastNonZeroMoveTime) <= HoldWindow
        && !OutC->LastNonZeroMoveDir.IsNearlyZero())
    {
        OutDir = OutC->LastNonZeroMoveDir;   // 이미 정규화됨
        bOutHasInput = true;
    }

    return true; // 입력 없으면 호출측에서 폴백 처리
}


float UAttackOrientationToCameraInput::MakeYawFromDir2D(const FVector2D& Dir2D) const
{
    const FRotator YawOnly(0.f, CachedCamYaw, 0.f);
    const FVector CamFwd = YawOnly.Vector();
    const FVector CamRight = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);
    const FVector AttackDir = (CamFwd * Dir2D.Y + CamRight * Dir2D.X).GetSafeNormal2D();
    return AttackDir.Rotation().Yaw;
}

void UAttackOrientationToCameraInput::ApplyInterpRotation(APawn* Pawn, float DeltaSeconds) const
{
    const FRotator Cur(0.f, Pawn->GetActorRotation().Yaw, 0.f);
    const FRotator Tar(0.f, TargetYaw, 0.f);
    const FRotator NewRot = FMath::RInterpConstantTo(Cur, Tar, DeltaSeconds, TurnRateDegPerSec);
    Pawn->SetActorRotation(NewRot);
}

void UAttackOrientationToCameraInput::OverrideRotationPolicyBegin(ACharacter* Char)
{
    if (!Char) return;
    if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
    {
        bSaved_OrientRotationToMovement = Move->bOrientRotationToMovement;
        Move->bOrientRotationToMovement = false;   // 이동-기반 회전 OFF
    }
    bSaved_UseControllerRotationYaw = Char->bUseControllerRotationYaw;
    Char->bUseControllerRotationYaw = false;       // 직접 SetActorRotation 주도
    bHasSavedRotationPolicy = true;
}

void UAttackOrientationToCameraInput::OverrideRotationPolicyEnd(ACharacter* Char)
{
    if (!Char || !bHasSavedRotationPolicy) return;
    if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        Move->bOrientRotationToMovement = bSaved_OrientRotationToMovement;
    Char->bUseControllerRotationYaw = bSaved_UseControllerRotationYaw;
    bHasSavedRotationPolicy = false;
}
