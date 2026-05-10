// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveSkill.generated.h"

/**
 * 
 */
USTRUCT()
struct FMoveTaskMemory
{
    GENERATED_BODY()
    float ElapsedTime;
};

UCLASS()
class JEONWOOCHI_API UBTTask_MoveSkill : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_MoveSkill();

    /** 재생할 애니메이션 몽타주 */
    UPROPERTY(EditAnywhere, Category = "Movement|Montage", meta = (AllowedClasses = "AnimMontage"))
    UAnimMontage* MoveMontage;

    /** 몽타주 재생 후 대기할 시간 (초) */
    UPROPERTY(EditAnywhere, Category = "Movement|WaitingTime", meta = (ClampMin = "0.0"))
    float WaitTime;

    /** Motion Warping 옵션들 */
    UPROPERTY(EditAnywhere, Category = "Movement|MotionWarp")
    bool bUseMotionWarping;

    UPROPERTY(EditAnywhere, Category = "Movement|MotionWarp", meta = (EditCondition = "bUseMotionWarping", EditConditionHides))
    FName WarpTargetName;
	
    UPROPERTY(EditAnywhere, Category = "Movement|Target")
    FBlackboardKeySelector TargetLocationKey;

    uint16 GetInstanceMemorySize() const override
    {
        return sizeof(FMoveTaskMemory);
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
