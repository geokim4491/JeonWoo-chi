// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BTService_RotateToTarget.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UBTService_RotateToTarget : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_RotateToTarget();

    /** TargetActor(Object) 키 셀렉터 */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetKey;

    /** 회전 보간 속도 (degrees/sec) */
    UPROPERTY(EditAnywhere, Category = "Rotation")
    float RotationSpeed = 360.f;

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
