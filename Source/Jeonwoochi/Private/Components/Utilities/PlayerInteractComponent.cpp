// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Utilities/PlayerInteractComponent.h"
#include "Interface/Interact/InteractInterface.h"

// Sets default values for this component's properties
UPlayerInteractComponent::UPlayerInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}


// Called when the game starts
void UPlayerInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UPlayerInteractComponent::SetInteractTarget(AActor* Target)
{
	InteractTarget = Target;

	if (Target == nullptr)
	{
		OnInteractTargetReleased.Broadcast(CachedInteractTarget);
	}
	else
	{
		OnInteractTagetSetted.Broadcast(InteractTarget);
	}

	CachedInteractTarget = Target;
}

void UPlayerInteractComponent::Interact()
{
	if (!InteractTarget) return;

	if (InteractTarget->Implements<UInteractInterface>())
	{
		IInteractInterface::Execute_Interact(InteractTarget, GetOwner());
	}
}

