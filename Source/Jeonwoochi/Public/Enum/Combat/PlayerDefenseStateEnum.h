// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerDefenseStateEnum.generated.h"

UENUM(BlueprintType)
enum class EPlayerDefenseState : uint8
{
	None UMETA(DisplayName="None"),
	Blocking UMETA(DisplayName = "Blocking"),
	Parrying UMETA(DisplayName = "Parrying"),
	Dodging UMETA(DisplayName = "Dodging"),
	PerfectDodging UMETA(DisplayName = "PerfectDodging"),
	Blink UMETA(DisplayName = "Blink"),
	Invincible UMETA(DisplayName = "Invincible")
	//.. 이후에 블링크 등 추가
};