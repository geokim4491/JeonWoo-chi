// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Structure/Combat/DamageInfoStruct.h"
#include "BTTask_Attack.generated.h"

/**
 * 
 */

USTRUCT()
struct FAttackTaskMemory
{
    GENERATED_BODY()
    float ElapsedTime;
};

UCLASS()
class JEONWOOCHI_API UBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Attack();
	
    /** 재생할 애니메이션 몽타주 */
    UPROPERTY(EditAnywhere, Category = "Attack|AttackInfo", meta = (AllowedClasses = "Info"))
    FDamageInfoStruct DamageInfo;

    /** 몽타주 재생 후 대기할 시간 (초) */
    UPROPERTY(EditAnywhere, Category = "Attack|WaitingTime", meta = (ClampMin = "0.0"))
    float WaitTime;

    /** Motion Warping 옵션들 */
    UPROPERTY(EditAnywhere, Category = "Attack|MotionWarp")
    bool bUseMotionWarping;

    UPROPERTY(EditAnywhere, Category = "Attack|MotionWarp", meta = (EditCondition = "bUseMotionWarping", EditConditionHides))
    FName WarpTargetName;

    UPROPERTY(EditAnywhere, Category = "Attack|Target")
    FBlackboardKeySelector TargetActorKey;

    uint16 GetInstanceMemorySize() const override
    {
        return sizeof(FAttackTaskMemory);
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
