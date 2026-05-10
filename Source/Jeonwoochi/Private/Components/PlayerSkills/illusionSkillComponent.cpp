// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlayerSkills/illusionSkillComponent.h"
#include "Characters/AI/BaseAICharacter.h"
#include "../../../JeonwoochiCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Utilities/LockOnComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values for this component's properties
UillusionSkillComponent::UillusionSkillComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UillusionSkillComponent::BeginPlay()
{
	Super::BeginPlay();

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    AController* Ctrl = OwnerPawn->GetController();

    PC = Cast<APlayerController>(Ctrl);

    BoundLockOnComp = GetOwner()->FindComponentByClass<ULockOnComponent>();

    if (auto* Ch = Cast<AJeonwoochiCharacter>(GetOwner()))
    {
        CameraBoom = Ch->GetCameraBoom();   // 프로젝트마다 함수명 다름
    }
}

bool UillusionSkillComponent::IsValidillusionTarget()
{
    return IsValid(IllusionedTarget);
}

void UillusionSkillComponent::illusionMode()
{
    if (IsValid(IllusionedTarget)) return;

    if (!billusionModeActive)
    {
        billusionModeActive = true;
        if (BoundLockOnComp)
        {
            BoundLockOnComp->ClearLockOn();
        }
      
        CameraFadeEffect(0.f, 0.5f, 0.1f, true);
        
        UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.5f);

        if (CameraBoom)
        {
            // 최초 한 번만 기본값 저장
            if (!bCamDefaultCached)
            {
                bCamDefaultCached = true;
                DefaultArmLength = CameraBoom->TargetArmLength;
                DefaultSocketOffset = CameraBoom->SocketOffset;
                DefaultRelRot = CameraBoom->GetRelativeRotation();
            }

            const float   AimArmLen = 100.f;                         // 더 가까이
            const FVector AimOffset = FVector(+30.f, +40.f, +10.f);  // 뒤·오른쪽·위
            const FRotator AimRot = FRotator(-5.f, 0.f, 0.f);    // 살짝 회전
            const float   BlendTime = 0.15f;

            // 길이는 즉시 변경(끊김 거의 없음)
            CameraBoom->TargetArmLength = AimArmLen;

            // 위치·회전은 보간
            FLatentActionInfo Info;
            Info.CallbackTarget = this;
            UKismetSystemLibrary::MoveComponentTo(
                CameraBoom,
                AimOffset,
                AimRot,
                false, false,
                BlendTime,
                false,
                EMoveComponentAction::Move,
                Info
            );
        }

    }

	UpdateTarget();
}

void UillusionSkillComponent::StopillusionMode()
{
    if (IsValid(IllusionedTarget)) return;

    // 조준 중이던 대상이 환술 대상이 아니라면 끔
    if (CurrentTarget && CurrentTarget != IllusionedTarget)
    {
        Highlight(CurrentTarget, false);
    }

    billusionModeActive = false;
    CameraFadeEffect(0.5f, 0.f, 0.5f, false);
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);

    /* --- CameraBoom 원위치 --- */
    if (CameraBoom && bCamDefaultCached)
    {
        const float BlendTime = 0.25f;

        // 길이 즉시 복귀
        CameraBoom->TargetArmLength = DefaultArmLength;

        // 위치·회전 보간 복귀
        FLatentActionInfo Info;
        Info.CallbackTarget = this;
        UKismetSystemLibrary::MoveComponentTo(
            CameraBoom,
            DefaultSocketOffset,
            DefaultRelRot,
            false, false,
            BlendTime,
            false,
            EMoveComponentAction::Move,
            Info
        );
    }

    CurrentTarget = nullptr;
}


void UillusionSkillComponent::UpdateTarget()
{
    if (!PC) return;

    FVector  CamLoc;
    FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);

    const FVector TraceEnd = CamLoc + CamRot.Vector() * TraceDistance;

    // 2) 라인트레이스
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(Illusion), /*bTraceComplex=*/false, GetOwner());
    Params.bReturnPhysicalMaterial = false;        // 필요 없으면 false

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, CamLoc, TraceEnd, ECC_GameTraceChannel1, Params);


    /*
    // 3) 실제 그릴 끝점(맞으면 Hit.Location, 아니면 최대 거리)
    const FVector DebugEnd = bHit ? Hit.Location : TraceEnd;

    DrawDebugLine(
        GetWorld(),
        CamLoc,
        DebugEnd,
        bHit ? FColor::Green : FColor::Blue,  // 맞으면 초록, 없으면 파랑
        false,                                // 한 프레임만
        0.1f,
        0,
        2.0f);
    */

    // 4) 타깃 처리 ----------------------------------------------------------
    if (bHit && Hit.GetActor())
    {
        if (const auto* AI = Cast<ABaseAICharacter>(Hit.GetActor()))
        {
            if (AI->GetCharacterType() == ECharacterType::Companion ||
                AI->GetCharacterType() == ECharacterType::Boss) return;
            const float Dot =
                FVector::DotProduct(CamRot.Vector(),
                    (AI->GetActorLocation() - CamLoc).GetSafeNormal());

            if (Dot >= AimToleranceDot)
            {
                if (AI != CurrentTarget)
                {
                    Highlight(CurrentTarget, false);
                    CurrentTarget = const_cast<ABaseAICharacter*>(AI);
                    Highlight(CurrentTarget, true);
                }
                return;
            }
        }
    }

    Highlight(CurrentTarget, false);
    CurrentTarget = nullptr;
}

void UillusionSkillComponent::Highlight(ABaseAICharacter* Target, bool bEnable)
{
    if (!Target || !HighlightMaterial) return;

    USkeletalMeshComponent* Mesh = Target->GetMesh();
    if (!Mesh) return;

    if (bEnable)
    {
        Mesh->SetOverlayMaterial(HighlightMaterial);
    }
    else
    {
        Mesh->SetOverlayMaterial(nullptr);
    }
}

void UillusionSkillComponent::CameraFadeEffect(float FromAlpha, float ToAlpha, float FadeTime, bool bHold)
{
    if (PC)
    {
        auto CamMgr = PC->PlayerCameraManager;

        if (!CamMgr) return;

            // 화면을 페이드 인
        CamMgr->StartCameraFade(
            /* FromAlpha */ FromAlpha,
            /* ToAlpha   */ ToAlpha,
            /* Duration  */ FadeTime,
            /* Color     */ FLinearColor::Black,
            /* bFadeAudio*/ false,
            /* bHold     */ bHold
        );
        
    }
}

bool UillusionSkillComponent::Castillusion()
{
	if (CurrentTarget)
	{
		CurrentTarget->illusioned(illusionDuration);

        // Stopillusion을 호출 못해서 따로 작성.
        billusionModeActive = false;
        CameraFadeEffect(0.5f, 0.f, 0.5f, false);
        UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
        /* --- CameraBoom 원위치 --- */
        if (CameraBoom && bCamDefaultCached)
        {
            const float BlendTime = 0.25f;

            // 길이 즉시 복귀
            CameraBoom->TargetArmLength = DefaultArmLength;

            // 위치·회전 보간 복귀
            FLatentActionInfo Info;
            Info.CallbackTarget = this;
            UKismetSystemLibrary::MoveComponentTo(
                CameraBoom,
                DefaultSocketOffset,
                DefaultRelRot,
                false, false,
                BlendTime,
                false,
                EMoveComponentAction::Move,
                Info
            );
        }

        // 2) 현재 대상을 환술 대상으로 고정하고 highlight 유지
        IllusionedTarget = CurrentTarget;
        CurrentTarget = nullptr;

        GetWorld()->GetTimerManager().SetTimer(
            illusionResetHandle,
            this,
            &UillusionSkillComponent::Clearillusion,
            illusionDuration,
            false);

        return true;
	}
    return false;
}

void UillusionSkillComponent::Clearillusion()
{
    if (IsValid(IllusionedTarget))
    {
        Highlight(IllusionedTarget, false);   // highlight 해제
    }
    IllusionedTarget = nullptr;
}

void UillusionSkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

    GetWorld()->GetTimerManager().ClearTimer(illusionResetHandle);
	Super::EndPlay(EndPlayReason);
}