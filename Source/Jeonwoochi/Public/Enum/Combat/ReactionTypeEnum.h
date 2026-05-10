// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReactionTypeEnum.generated.h"

UENUM(BlueprintType)
enum class EReactionType : uint8
{
	None UMETA(DisplayName="None"),
	HeavyStagger UMETA(DisplayName = "HeavyStagger"), 
	Knockback UMETA(DisplayName = "Knockback"),
	Knockdown UMETA(DisplayName = "Knockdown"),
	Stun UMETA(DisplayName = "Stun") 
};