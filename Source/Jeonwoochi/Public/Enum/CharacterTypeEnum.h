// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterTypeEnum.generated.h"

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	None UMETA(DisplayName="None"),
	Player UMETA(DisplayName="Player"),
	Companion UMETA(DisplayName="Companion"),
	NormalEnemy UMETA(DisplayName="Enemy"),
	Boss UMETA(DisplayName="Boss")
};