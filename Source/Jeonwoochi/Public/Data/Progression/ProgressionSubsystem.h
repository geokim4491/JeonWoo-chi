// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "ProgressionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    UPROPERTY() FGameplayTagContainer Flags;   // boss.*.defeated 등 전역 플래그

    UFUNCTION(BlueprintCallable) bool Has(FGameplayTag Tag) const { return Flags.HasTag(Tag); }

    // UI/세이브 반영을 위해 변경 알림을 주고 싶으면 Delegate 추가해서 Broadcast 해도 됨
    UFUNCTION(BlueprintCallable) void Set(FGameplayTag Tag, bool bEnable)
    {
        if (!Tag.IsValid()) return;
        if (bEnable) Flags.AddTag(Tag); else Flags.RemoveTag(Tag);
    }
};
