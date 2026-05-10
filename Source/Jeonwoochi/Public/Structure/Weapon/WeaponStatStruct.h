// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponStatStruct.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FWeaponStatStruct
{
    GENERATED_BODY()

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Stat")
    float AttackPower;

    // 기본 생성자(필요 시)
    FWeaponStatStruct() {}
};
