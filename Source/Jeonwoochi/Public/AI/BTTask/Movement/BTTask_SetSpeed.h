// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetSpeed.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UBTTask_SetSpeed : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Speed")
	float Speed = 300.f;

protected:
    // 실행 노드
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;
};
