// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MusicSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UMusicSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable) void PlayBGM(USoundBase* NewSound, float FadeInSeconds = 1.2f, float TargetVolume = 1.f, float StartTime = 0.f, bool bPersist = false);
    UFUNCTION(BlueprintCallable) void PlayBGM_Soft(TSoftObjectPtr<USoundBase> NewSound, float FadeInSeconds = 1.2f, float TargetVolume = 1.f, float StartTime = 0.f, bool bPersist = false);
    UFUNCTION(BlueprintCallable) void StopBGM(float FadeOutSeconds = 1.0f);

private:
    UAudioComponent* AudioComp;
};
