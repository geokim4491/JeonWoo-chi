// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/ChiComponent.h"

// Sets default values for this component's properties
UChiComponent::UChiComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...

	MaxChi = 100.f;
}


// Called when the game starts
void UChiComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentChi = MaxChi;
}


// Called every frame
void UChiComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UChiComponent::UpdateCurrentChi(float Amount)
{
	CurrentChi = FMath::Clamp(CurrentChi + Amount, 0.f, MaxChi);
	OnChiChanged.Broadcast(CurrentChi, MaxChi);
}

void UChiComponent::IncreaseMaxChi(float Amount)
{
	if (Amount <= 0) return;

	MaxChi += Amount;

	CurrentChi = MaxChi;
	OnChiChanged.Broadcast(CurrentChi, MaxChi);
}

void UChiComponent::SetMaxChi(float NewMax)
{
	MaxChi = FMath::Max(NewMax, 0.f);
	CurrentChi = FMath::Clamp(CurrentChi, 0.f, MaxChi);
	OnChiChanged.Broadcast(CurrentChi, MaxChi);
}

void UChiComponent::SetCurrentChi(float NewValue)
{
	CurrentChi = FMath::Clamp(NewValue, 0.f, MaxChi);
	OnChiChanged.Broadcast(CurrentChi, MaxChi);
}

void UChiComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnChiChanged.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

