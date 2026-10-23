// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChiComponent.generated.h"


DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnChiChanged, 
	float /*CurrentChi*/,
	float /*MaxChi*/);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UChiComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UChiComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE float GetMaxChi() const { return MaxChi; }
	FORCEINLINE float GetCurrentChi() const { return CurrentChi; }

	UFUNCTION(BlueprintCallable) void SetMaxChi(float NewMax);
	void SetCurrentChi(float NewValue);

	/** 기를 소모하고 싶으면, Amount를 음수로 주어야 함. */
	UFUNCTION()
	void UpdateCurrentChi(float Amount);

	UFUNCTION()
	void IncreaseMaxChi(float Amount);

	FOnChiChanged OnChiChanged;

	UPROPERTY(EditAnywhere, SaveGame)
	float MaxChi;
		
private:
	
	UPROPERTY(SaveGame)
	float CurrentChi;
};
