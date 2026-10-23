// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageTypeEnum.generated.h"

UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Normal UMETA(DisplayName="Normal"),
	DotDeal UMETA(DisplayName = "DotDeal")
};