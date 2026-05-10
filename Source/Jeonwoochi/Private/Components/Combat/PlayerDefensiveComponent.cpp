// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PlayerDefensiveComponent.h"

// Sets default values for this component's properties
UPlayerDefensiveComponent::UPlayerDefensiveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UPlayerDefensiveComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPlayerDefensiveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerDefensiveComponent::SetDefenseState(EPlayerDefenseState DefenseStat)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
		DefenseState = DefenseStat;
		UE_LOG(LogTemp, Display, TEXT("State: %s"), *UEnum::GetValueAsString(DefenseState));
	}
}

void UPlayerDefensiveComponent::Blocking()
{
	if (!CanBlocking()) return;

	DefenseState = EPlayerDefenseState::Parrying;

	GetWorld()->GetTimerManager().ClearTimer(BlockTimerHandle);
	
	GetWorld()->GetTimerManager().SetTimer(
		BlockTimerHandle,
		FTimerDelegate::CreateLambda([this]()
		{
			DefenseState = EPlayerDefenseState::Blocking;
		}),
		ParryDuration,
		false
	);
}

void UPlayerDefensiveComponent::Dodging()
{
	if (!CanDodging()) return;

	DefenseState = EPlayerDefenseState::PerfectDodging;

	GetWorld()->GetTimerManager().ClearTimer(DodgeTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(PerfectDodgeTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		PerfectDodgeTimerHandle,
		FTimerDelegate::CreateLambda([this]()
		{
			DefenseState = EPlayerDefenseState::Dodging;
		}),
		PerfectDodgeDuration,
		false
	);

	GetWorld()->GetTimerManager().SetTimer(
		DodgeTimerHandle,
		FTimerDelegate::CreateLambda([this]()
		{
			DefenseState = EPlayerDefenseState::None;
		}),
		DodgeDuration,
		false
	);
}

void UPlayerDefensiveComponent::StopBlocking()
{
	GetWorld()->GetTimerManager().ClearTimer(BlockTimerHandle);

	DefenseState = EPlayerDefenseState::None;
}

void UPlayerDefensiveComponent::StopDodging()
{
	GetWorld()->GetTimerManager().ClearTimer(DodgeTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(PerfectDodgeTimerHandle);

	DefenseState = EPlayerDefenseState::None;
}

EPlayerDefenseState UPlayerDefensiveComponent::GetPlayerDefenseState()
{
	return DefenseState;
}

bool UPlayerDefensiveComponent::CanBlocking()
{
	return true;
}

bool UPlayerDefensiveComponent::CanDodging()
{
	return true;
}

void UPlayerDefensiveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
}