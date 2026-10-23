// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BehaviorTree/BehaviorTree.h"
#include "../../Structure/Combat/DamageInfoStruct.h"
#include "AIConfigData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class JEONWOOCHI_API UAIConfigData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// Idle/Combat Subtree
	UPROPERTY(EditAnywhere, Category = "BT")
	UBehaviorTree* IdleSubtree;

	UPROPERTY(EditAnywhere, Category = "BT")
	UBehaviorTree* CombatSubtree;

};
