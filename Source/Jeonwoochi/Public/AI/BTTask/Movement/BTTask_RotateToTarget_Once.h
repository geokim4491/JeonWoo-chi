// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RotateToTarget_Once.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UBTTask_RotateToTarget_Once : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_RotateToTarget_Once();

protected:
    // 실행 노드
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;
};
