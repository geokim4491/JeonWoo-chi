// Copyright Epic Games, Inc. All Rights Reserved.

#include "JeonwoochiGameMode.h"
#include "JeonwoochiCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "../Plugins/AsyncLoadingScreen/Source/AsyncLoadingScreen/Public/AsyncLoadingScreenLibrary.h"

AJeonwoochiGameMode::AJeonwoochiGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AJeonwoochiGameMode::OpenLevelHard(const FName& FURL, int32 TipIndex)
{
	UAsyncLoadingScreenLibrary::SetDisplayTipTextIndex(TipIndex);
	UAsyncLoadingScreenLibrary::SetEnableLoadingScreen(true);
	UGameplayStatics::OpenLevel(this, FURL);
}

void AJeonwoochiGameMode::OpenLevel(TSoftObjectPtr<UWorld> DestinationLevel, int32 TipIndex)
{
	UAsyncLoadingScreenLibrary::SetDisplayTipTextIndex(TipIndex);
	UAsyncLoadingScreenLibrary::SetEnableLoadingScreen(true);
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), DestinationLevel);
}