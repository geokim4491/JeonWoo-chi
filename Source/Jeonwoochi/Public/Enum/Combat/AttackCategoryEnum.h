// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackCategoryEnum.generated.h"

UENUM(BlueprintType)
enum class EAttackCategory : uint8
{
	NormalAttack UMETA(DisplayName="Normal"),
	DangerousAttack UMETA(DisplayName = "Dangerous"),
	SpecialAttack UMETA(DisplayName = "Special")
};