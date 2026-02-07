#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AttackOrientationToCameraInput.generated.h"


UCLASS(meta = (DisplayName = "Attack: Orientation To Camera+Input"))
class JEONWOOCHI_API UAttackOrientationToCameraInput : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Facing", meta = (ClampMin = "0"))
    float TurnRateDegPerSec = 900.f;           // 보간 회전 속도(도/초)

    UPROPERTY(EditAnywhere, Category = "Facing", meta = (ClampMin = "0"))
    float FollowWindowSec = 0.10f;             // 입력 반영 창(0.06~0.15 권장)

    UPROPERTY(EditAnywhere, Category = "Facing", meta = (ClampMin = "0"))
    float AngleThresholdDeg = 10.f;            // 미세 떨림 억제

    UPROPERTY(EditAnywhere, Category = "Facing")
    bool bInstantSnapOnBegin = true;           // Begin에서 1회 스냅

    UPROPERTY(EditAnywhere, Category = "Facing")
    bool bFallbackToCameraForwardIfNoInput = false; // 입력 없으면 카메라 전방

    UPROPERTY(EditAnywhere, Category = "Facing")
    bool bTemporarilyOverrideMovementRotation = true; // 구간 동안 이동-기반 회전 OFF

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;


private:
    float BeginTime = 0.f;
    float CachedCamYaw = 0.f;   // Begin 시점 카메라 Yaw(좌표계 고정)
    float TargetYaw = 0.f;

    // 회전정책 복구용
    bool bSaved_OrientRotationToMovement = false;
    bool bSaved_UseControllerRotationYaw = false;
    bool bHasSavedRotationPolicy = false;

    // 캐릭터 캐시를 우선 사용하여 '카메라 기준 입력 방향(2D)' 획득
    bool GetCamRelativeInputDir2D(class APawn* Pawn, class AJeonwoochiCharacter*& OutC, FVector2D& OutDir, bool& bOutHasInput) const;

    // 입력(2D) + CachedCamYaw → TargetYaw
    float MakeYawFromDir2D(const FVector2D& Dir2D) const;

    // 보간 회전
    void ApplyInterpRotation(APawn* Pawn, float DeltaSeconds) const;

    // 회전 정책 임시 변경/복구
    void OverrideRotationPolicyBegin(class ACharacter* Char);
    void OverrideRotationPolicyEnd(class ACharacter* Char);
};
