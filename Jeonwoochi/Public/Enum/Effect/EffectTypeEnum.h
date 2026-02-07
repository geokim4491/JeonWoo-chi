// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EffectTypeEnum.generated.h"

UENUM(BlueprintType)
enum class EEffectType : uint8
{
	Parry UMETA(DisplayName="Parry"),
	Block UMETA(DisplayName = "Block"),
	Dodge UMETA(DisplayName = "Dodge"),
	PerfectDodge UMETA(DisplayName = "PerfectDodge"),
	PlayerHit UMETA(DisplayName = "PlayerHit"),
	EnemyHit UMETA(DisplayName = "EnemyHit"),
	Execution UMETA(DisplayName = "Execution"),
	Ambush UMETA(DisplayName = "Ambush"),
	Blink UMETA(DisplayName = "Blink"),
	SpawnClone UMETA(DisplayName = "SpawnClone"),
	Bonfire UMETA(DisplayName = "Bonfire"),
	PlayerDie UMETA(DisplayName = "PlayerDie"),
};