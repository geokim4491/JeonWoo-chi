// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Structure/Combat/DotDealInfoStruct.h"

#include "HealthInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHealthInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class JEONWOOCHI_API IHealthInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual float GetMaxHealth() const = 0;

	UFUNCTION(BlueprintNativeEvent)
	float GetCurrentHealth();
	virtual float GetCurrentHealth_Implementation() const = 0;

	UFUNCTION(BlueprintNativeEvent)
	void Die();
	virtual void Die_Implementation() = 0;

	UFUNCTION(BlueprintNativeEvent)
	void ApplyDamage(const float& DamageAmount);
	virtual void ApplyDamage_Implementation(const float& DamageAmount) = 0;

	
	
	UFUNCTION(BlueprintNativeEvent)
	void ApplyDotDamage(const FDotDealInfoStruct& DotInfo);
	virtual void ApplyDotDamage_Implementation(const FDotDealInfoStruct& DotInfo) = 0;
	
	/* Heal은 아직 타입 못정함
	UFUNCTION(BlueprintNativeEvent)
	void Heal(const float& HealAmount);
	virtual void Heal_Implementation() = 0;
	*/
};
