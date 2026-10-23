// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Components/2D/PaperAIAnimationComponent.h"
#include "BTTask_2D_EnterState.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UBTTask_2D_EnterState : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

protected:
    // 실행 노드
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;
	

    UPROPERTY(EditAnywhere, Category = "Input")
    EAnimState State;
};
