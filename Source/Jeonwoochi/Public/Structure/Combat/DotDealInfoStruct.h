// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageInfoStruct.h"
#include "DotDealInfoStruct.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FDotDealInfoStruct
{
    GENERATED_BODY()

    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DotDealInfo")
    FDamageInfoStruct DamageInfo;

    /** Damage Interval(sec) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DotDealInfo")
    float Interval;

    /** Dot Duration Time(sec) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DotDealInfo")
    float TotalDuration;

    /** TotalDuration / Interval */
    UPROPERTY(VisibleAnywhere)
    int32 TicksRemaining;
    
    UPROPERTY()
    FTimerHandle TimerHandle;

    // 기본 생성자(필요 시)
    FDotDealInfoStruct();
};
