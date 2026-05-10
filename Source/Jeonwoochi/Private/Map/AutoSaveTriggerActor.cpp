// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/AutoSaveTriggerActor.h"
#include "Components/BoxComponent.h"
#include "Data/Save/SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAutoSaveTriggerActor::AAutoSaveTriggerActor()
{
 	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	Box->SetupAttachment(RootComponent);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AAutoSaveTriggerActor::BeginPlay()
{
	Super::BeginPlay();
	
	Box->OnComponentBeginOverlap.AddDynamic(this, &AAutoSaveTriggerActor::OnBoxBeginOverlap);
}

void AAutoSaveTriggerActor::OnBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	APawn* OverlapPawn = Cast<APawn>(OtherActor);
	if (!OverlapPawn) return;

	if (!OverlapPawn->IsPlayerControlled()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
	{
		if (USaveSubsystem* SS = GI->GetSubsystem<USaveSubsystem>())
		{
			switch (SaveType)
			{
			case EAutoSaveType::Progress:
				if (SS->SaveProgress(true))
					UE_LOG(LogTemp, Display, TEXT("Auto Save"));
				break;
			case EAutoSaveType::Rest:
				if (SS->SaveRestAtBonfire())
					UE_LOG(LogTemp, Display, TEXT("Rest Save"));
				break;
			case EAutoSaveType::All:
				if (SS->SaveProgress(true) && SS->SaveRestAtBonfire())
					UE_LOG(LogTemp, Display, TEXT("Save ALL"));

				break;
			default:
				if (SS->SaveProgress(true))
					UE_LOG(LogTemp, Display, TEXT("Auto Save"));
				break;
			}
		}
	}

	// 한번만 발동하게 설정.
	Box->OnComponentBeginOverlap.RemoveAll(this);
	Box->SetGenerateOverlapEvents(false);
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAutoSaveTriggerActor::EndPlay(EEndPlayReason::Type EndplayReason)
{
	if (Box)
	{
		Box->OnComponentBeginOverlap.RemoveDynamic(
			this, &AAutoSaveTriggerActor::OnBoxBeginOverlap);
	}

	Super::EndPlay(EndplayReason);
}