// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/ChangeMapActor.h"
#include "Components/Utilities/InteractTriggerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Jeonwoochi/JeonwoochiGameMode.h"

// Sets default values
AChangeMapActor::AChangeMapActor()
{
 	PrimaryActorTick.bCanEverTick = false;
	
	InteractTrrigerComp = 
		CreateDefaultSubobject<UInteractTriggerComponent>(TEXT("InteractTriggerComp"));
}

// Called when the game starts or when spawned
void AChangeMapActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AChangeMapActor::Interact_Implementation(AActor* Target)
{

	UWorld* World = GetWorld();
	if (!World) return;

	if (auto* GM = World->GetAuthGameMode<AJeonwoochiGameMode>())
	{
		GM->OpenLevel(DestinationLevel, TipIndex); // 로딩창과 함께 openlevel
	}
}
