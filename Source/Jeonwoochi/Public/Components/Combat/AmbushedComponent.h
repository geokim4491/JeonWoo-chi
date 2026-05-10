// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AmbushedComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UAmbushedComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAmbushedComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	UPROPERTY(EditAnywhere, Category = "Ambushed")
	float DamageAmount = 100;

	UPROPERTY(EditAnywhere, Category = "Ambushed")
	UAnimMontage* AmbushedMontage;

	UFUNCTION()
	void OnAmbushed();
		
private:
	void GetUp();
	FTimerHandle AmbushedTimerHandle;
};
