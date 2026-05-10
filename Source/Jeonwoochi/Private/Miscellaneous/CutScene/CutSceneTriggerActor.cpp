// Fill out your copyright notice in the Description page of Project Settings.


#include "Miscellaneous/CutScene/CutSceneTriggerActor.h"
#include "Components/BoxComponent.h"
#include "Jeonwoochi/JeonwoochiCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Sound/MusicSubsystem.h"
#include "Components/Utilities/PlayerUIComponent.h"

// Sets default values
ACutSceneTriggerActor::ACutSceneTriggerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);

	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldDynamic);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);

	Box->OnComponentBeginOverlap.AddDynamic(this, &ACutSceneTriggerActor::OnBoxBeginOverlap);
}

// Called when the game starts or when spawned
void ACutSceneTriggerActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACutSceneTriggerActor::OnBoxBeginOverlap(
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

	CachedChar = Cast<AJeonwoochiCharacter>(OtherActor);

	if (bActiveFadeIn)
	{
		FadeIn(FadeInSec);

		GetWorld()->GetTimerManager().SetTimer(
			FadeTimerHandle, this, &ACutSceneTriggerActor::PlaySequence, FadeInSec, false);
	}
	else PlaySequence();

	// 한번만 발동하게 설정.
	Box->OnComponentBeginOverlap.RemoveAll(this);
	Box->SetGenerateOverlapEvents(false);
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACutSceneTriggerActor::PlaySequence()
{
	if (!PlacedSequenceActor) return;

	CachedChar->ApplyIMC_CutScene();

	if (CachedChar)
	{
		CachedChar->SetActorTransform(BeforePlayerTransform);
	}

	PlacedSequenceActor->PlaybackSettings.bDisableCameraCuts = false;
	PlacedSequenceActor->PlaybackSettings.bPauseAtEnd = true;

	CachedPlayer = PlacedSequenceActor->GetSequencePlayer();
	if (!CachedPlayer) return;

	CachedPlayer->OnFinished.RemoveAll(this);
	CachedPlayer->OnFinished.AddDynamic(this, &ACutSceneTriggerActor::OnFinished);
	CachedPlayer->Play();
	CachedChar->GetUIComponent()->SetCinematicUIHidden(true);
}

void ACutSceneTriggerActor::OnFinished()
{

	if (bActiveFadeOut) FadeOut(FadeOutSec);
	CachedChar->ClearAllIMC();
	CachedChar->ApplyIMC_Default();

	if (CachedChar)
	{
		CachedChar->SetActorTransform(AfterPlayerTransform);
	}
	CachedChar->GetUIComponent()->SetCinematicUIHidden(false);
}

void ACutSceneTriggerActor::FadeIn(float Duration)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->PlayerCameraManager->StartCameraFade(
			/*From*/0.f, /*To*/1.f, Duration, FLinearColor::Black,
			/*bShouldFadeAudio*/ true, /*bHoldWhenFinished*/ true);
	}
}

void ACutSceneTriggerActor::FadeOut(float Duration)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->PlayerCameraManager->StartCameraFade(
			/*From*/1.f, /*To*/0.f, Duration, FLinearColor::Black,
			/*bShouldFadeAudio*/ false, /*bHoldWhenFinished*/ false);
	}
}

void ACutSceneTriggerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!GetWorld()) return;
	GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
	Super::EndPlay(EndPlayReason);
}
