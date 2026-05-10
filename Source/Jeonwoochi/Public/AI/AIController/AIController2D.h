// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIController2D.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API AAIController2D : public AAIController
{
	GENERATED_BODY()

	AAIController2D();
public:
	virtual void SetTargetActor(AActor* Actor);

protected:
	virtual void BeginPlay() override;

protected:

	// Main BT
	UPROPERTY(EditAnywhere, Category = "AI|BT")
	UBehaviorTree* MainBT;
	
};
