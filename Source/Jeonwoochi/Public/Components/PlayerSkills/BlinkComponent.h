// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structure/Combat/DamageInfoStruct.h"
#include "../../Enum/Combat/PlayerDefenseStateEnum.h"
#include "BlinkComponent.generated.h"

class AJeonwoochiCharacter;
class UAttackComponent;
class UMotionWarpingComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UBlinkComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBlinkComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	UFUNCTION()
	void Blink();

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* SpawnMesh;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* BlinkMontage;

	UPROPERTY(EditAnywhere, Category = "Attack")
	FDamageInfoStruct DamageInfo;

	FORCEINLINE bool GetCanBlink() { return bCanBlink; }
	
private:
	UFUNCTION() void SpawnWoodLog();
	UFUNCTION() void SetBlinkMode(AActor* Attacker, bool bOpen);
	UFUNCTION() void OnBlinkMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	bool bCanBlink = false;
	bool bIsBlink = false;
	AActor* BlinkTarget = nullptr;
	AJeonwoochiCharacter* OwnerChar;
	UAttackComponent* CachedAttackComp;
	UMotionWarpingComponent* WarpComp;
};
