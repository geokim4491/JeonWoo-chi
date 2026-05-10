// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SelectNextPatrolPoint.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UBTTask_SelectNextPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_SelectNextPatrolPoint();

protected:
    // ExecuteTask에서 블랙보드에 지점 지정
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;

    // 블랙보드 키 (에디터에서 바인딩)
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector NextLocationKey;
};
