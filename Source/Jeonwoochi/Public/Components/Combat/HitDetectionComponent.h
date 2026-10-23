// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "../Combat/AttackComponent.h"

#include "HitDetectionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UHitDetectionComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHitDetectionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "HitDetection")
	void EnableDetection();

	UFUNCTION(BlueprintCallable, Category = "HitDetection")
	void DisableDetection();

	UFUNCTION()
	void StartHitWindow();

	UFUNCTION()
	void EndHitWindow();

	UFUNCTION() 
	void RefreshOwnerPointers();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitDetection")
	UBoxComponent* HitShape;

	UPROPERTY(VisibleAnywhere, Category = "HitDetection")
	TSet<AActor*> AlreadyHitActors;

		
private:
	UFUNCTION()
	void HandleOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, 
		bool bFromSweep,
		const FHitResult& SweepResult);

	UAttackComponent* CachedAttackComp = nullptr;
	AActor* CachedOwnerActor = nullptr;
	ABaseCharacter* CachedOwnerChar = nullptr;
};
