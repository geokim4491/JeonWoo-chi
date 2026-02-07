// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIActivationTrigger.generated.h"

class UBoxComponent;

UCLASS()
class JEONWOOCHI_API AAIActivationTrigger : public AActor
{
	GENERATED_BODY()
	
public:
	AAIActivationTrigger();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	TObjectPtr<UBoxComponent> Box;

	/** Want to Possess AI */
	UPROPERTY(EditInstanceOnly, Category = "AI")
	TArray<TObjectPtr<APawn>> Targets;

	/** One Shot */
	UPROPERTY(EditAnywhere, Category = "Trigger")
	bool bFireOnce = true;

	UPROPERTY(EditAnywhere, Category = "Trigger", meta = (EditCondition = "bFireOnce", EditConditionHides))
	bool bDelete = true;

	/** Triggered by Player Only */
	UPROPERTY(EditAnywhere, Category = "Trigger")
	bool bPlayerOnly = true;

	/** Force Repossess (Call OnPossess()) */
	UPROPERTY(EditAnywhere, Category = "Trigger")
	bool bForceRepossess = false;

	bool bDone = false;

	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void ActivateAIs();

};
