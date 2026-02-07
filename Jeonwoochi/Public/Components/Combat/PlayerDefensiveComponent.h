// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Enum/Combat/PlayerDefenseStateEnum.h"
#include "PlayerDefensiveComponent.generated.h"

UENUM(BlueprintType)
enum class ERollDirection : uint8
{
	Forward        UMETA(DisplayName = "F"),
	ForwardRight   UMETA(DisplayName = "FR"),
	Right          UMETA(DisplayName = "R"),
	BackwardRight  UMETA(DisplayName = "BR"),
	Backward       UMETA(DisplayName = "B"),
	BackwardLeft   UMETA(DisplayName = "BL"),
	Left           UMETA(DisplayName = "L"),
	ForwardLeft    UMETA(DisplayName = "FL"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UPlayerDefensiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerDefensiveComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void Blocking();

	UFUNCTION()
	void StopBlocking();

	UFUNCTION()
	void Dodging();

	UFUNCTION()
	void StopDodging();

	UFUNCTION()
	EPlayerDefenseState GetPlayerDefenseState();

	UPROPERTY(EditAnywhere, Category = "Animation|Roll")
	TMap<ERollDirection, UAnimMontage*> RollMontageMap;

	UFUNCTION()
	void SetDefenseState(EPlayerDefenseState DefenseStat);

private:

	UPROPERTY()
	EPlayerDefenseState DefenseState;
	
	UPROPERTY()
	float ParryDuration = 0.15f; // 추후 조절

	UPROPERTY()
	float DodgeDuration = 0.25; // 추후 조절

	UPROPERTY()
	float PerfectDodgeDuration = 0.15f; // 추후 조절

	UPROPERTY()
	FTimerHandle BlockTimerHandle;

	UPROPERTY()
	FTimerHandle DodgeTimerHandle;

	UPROPERTY()
	FTimerHandle PerfectDodgeTimerHandle;


	/** Block과 Dodge를 할 수 있는지 판단하는 함수 아직 정의 안함 */
	bool CanBlocking();

	bool CanDodging();
};
