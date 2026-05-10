// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BTService_CalcDistance.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UBTService_CalcDistance : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_CalcDistance();

    /** Blackboard에서 읽어올 Target Actor */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    /** 계산된 거리를 쓸 Blackboard Float 키 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector DistanceKey;

protected:
    /** Service가 Tick될 때마다 호출됩니다. */
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
    virtual void OnBecomeRelevant(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
};
