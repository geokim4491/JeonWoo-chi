// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/MagicSummonActor.h"
#include "Components/Combat/HitDetectionComponent.h"


// Sets default values
AMagicSummonActor::AMagicSummonActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	HitComp = CreateDefaultSubobject<UHitDetectionComponent>(TEXT("HitComp"));
	HitComp->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AMagicSummonActor::BeginPlay()
{
	Super::BeginPlay();
	
	// detection 활성화.
	if (HitDetectActivateTime <= 0.f)
	{
		ActivateHitWindow();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(
			ActivateTimerHandle,
			this,
			&AMagicSummonActor::ActivateHitWindow,
			HitDetectActivateTime,
			false  // one-shot
		);
	}

	if (LifeTime > 0.f)
	{
		SetLifeSpan(LifeTime);
	}
}

void AMagicSummonActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(ActivateTimerHandle);
	Super::EndPlay(EndPlayReason);
}


void AMagicSummonActor::ActivateHitWindow()
{
	if (IsValid(HitComp))
	{
		HitComp->StartHitWindow();
	}
}