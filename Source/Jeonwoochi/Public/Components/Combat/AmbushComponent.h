// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AmbushComponent.generated.h"

class AJeonwoochiCharacter;
class ABaseAICharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UAmbushComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAmbushComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AmbushMontage;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* SetAmbushSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* AmbushSound;

	UFUNCTION()
	void SetCanAmbush(bool bCanAmbu);

	FORCEINLINE bool GetCanAmbush() const { return bCanAmbush; }
	FORCEINLINE void SetAmbushedTarget(ABaseAICharacter* Char) { AmbushedTarget = Char; }
	FORCEINLINE ABaseAICharacter* GetAmbushedTarget() const { return AmbushedTarget; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCanAmbush, bool, bCanAmbu);
	FOnCanAmbush OnCanAmbush;

	UFUNCTION()
	void Ambush();

private:
	// void PlayEffect(EEffectType EffectType);
	// void CinematicCamera();
	UFUNCTION()
	void OnAmbushMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	bool bCanAmbush = false;

	AJeonwoochiCharacter* OwnerChar;
	ABaseAICharacter* AmbushedTarget = nullptr;
	ABaseAICharacter* CachedAmbushedTarget = nullptr;
};
