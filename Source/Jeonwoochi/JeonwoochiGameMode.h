// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JeonwoochiGameMode.generated.h"

UCLASS(minimalapi)
class AJeonwoochiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJeonwoochiGameMode();

	UFUNCTION(BlueprintCallable)
	void OpenLevel(TSoftObjectPtr<UWorld> DestinationLevel, int32 TipIndex = 0);

	UFUNCTION(BlueprintCallable)
	void OpenLevelHard(const FName& FURL, int32 TipIndex = 0);
};

