// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack_MontageOnly.generated.h"

USTRUCT()
struct FAttackMontageOnlyTaskMemory
{
    GENERATED_BODY()
    float ElapsedTime;
};

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UBTTask_Attack_MontageOnly : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_Attack_MontageOnly();

    /** 재생할 애니메이션 몽타주 */
    UPROPERTY(EditAnywhere, Category = "Attack|Montage", meta = (AllowedClasses = "AnimMontage"))
    UAnimMontage* AttackMontage;

    /** 몽타주 재생 후 대기할 시간 (초) */
    UPROPERTY(EditAnywhere, Category = "Attack|WaitingTime", meta = (ClampMin = "0.0"))
    float WaitTime;

    /** 몽타주가 Z 방향으로 올라가면, 사용해야 함. */
    UPROPERTY(EditAnywhere, Category = "Attack|Montage")
    bool bForceFlyingRootMotion = true;

    /** Motion Warping 옵션들 */
    UPROPERTY(EditAnywhere, Category = "Attack|MotionWarp")
    bool bUseMotionWarping;

    UPROPERTY(EditAnywhere, Category = "Attack|MotionWarp", meta = (EditCondition = "bUseMotionWarping", EditConditionHides))
    FName WarpTargetName;

    UPROPERTY(EditAnywhere, Category = "Attack|Target")
    FBlackboardKeySelector TargetLocationKey;

    uint16 GetInstanceMemorySize() const override
    {
        return sizeof(FAttackMontageOnlyTaskMemory);
    }

protected:
    // 실행 노드
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;


    virtual void TickTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds
    ) override;

};
