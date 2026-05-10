// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInteractComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractTargetSetted, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractTargetReleased, AActor*);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UPlayerInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerInteractComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UFUNCTION()
	void Interact();
	
	FOnInteractTargetSetted OnInteractTagetSetted;
	FOnInteractTargetReleased OnInteractTargetReleased;

	void SetInteractTarget(AActor* Target);
private:
	AActor* InteractTarget = nullptr;
	AActor* CachedInteractTarget = nullptr;
};
