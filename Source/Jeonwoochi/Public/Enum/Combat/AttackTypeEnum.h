// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackTypeEnum.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	NormalAttack UMETA(DisplayName="Normal"),
	JumpAttack UMETA(DisplayName = "Jump"),
	GrabAttack UMETA(DisplayName = "Grab"),
	SpawnAttack UMETA(DisplayName = "Spawn"),
	AOEAttack UMETA(DisplayName = "AOE")
};