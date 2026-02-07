// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIDefenseStateEnum.generated.h"

UENUM(BlueprintType)
enum class EAIDefenseState : uint8
{
	None UMETA(DisplayName="None"),
	Parrying UMETA(DisplayName = "Parrying"),
	Dodging UMETA(DisplayName = "Dodging")
};