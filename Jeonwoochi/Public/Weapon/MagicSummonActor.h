// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicSummonActor.generated.h"

class UHitDetectionComponent;

UCLASS()
class JEONWOOCHI_API AMagicSummonActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagicSummonActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY()	UHitDetectionComponent* HitComp;
	
	UPROPERTY(EditAnywhere, Category = "Activation")
	float HitDetectActivateTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Life")
	float LifeTime = 3.f;

private:
	FTimerHandle ActivateTimerHandle;

	UFUNCTION() void ActivateHitWindow();
};
