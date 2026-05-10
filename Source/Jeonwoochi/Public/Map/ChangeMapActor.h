// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Interact/InteractInterface.h"
#include "ChangeMapActor.generated.h"

class UInteractTriggerComponent;

UCLASS()
class JEONWOOCHI_API AChangeMapActor : public AActor,
	public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChangeMapActor();

	virtual void Interact_Implementation(AActor* Target) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> DestinationLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TipIndex;

	UPROPERTY(EditAnywhere, Category = "BoxTrriger")
	UInteractTriggerComponent* InteractTrrigerComp = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};