// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/AIController/BaseAIController.h"
#include "FinalBossAIController.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API AFinalBossAIController : public ABaseAIController
{
	GENERATED_BODY()
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
};
