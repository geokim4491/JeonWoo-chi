// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enum/CharacterTypeEnum.h"
#include "Structure/Combat/DamageInfoStruct.h"
#include "Structure/Combat/DotDealInfoStruct.h"
#include "Characters/BaseCharacter.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDeath, AActor*, ECharacterType);
DECLARE_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, AActor*, ECharacterType, float /*CurrentHealth*/, float /*MaxHealth*/);

#include "HealthComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//
	// variable
	//

	UPROPERTY(EditAnywhere, SaveGame, Category="Health")
	float MaxHealth;

	FOnDeath OnDeath;
	FOnHealthChanged OnHealthChanged;
	
	//
	// Function
	//

	float GetMaxHealth() const;
	float GetCurrentHealth() const;
	UFUNCTION(BlueprintCallable) void SetMaxHealth(float NewMax);
	void SetCurrentHealth(float NewValue);
	void Heal(const float& HealAmount);
	void ApplyDamage(const float& DamageAmount);
	void ApplyDotDamage(const FDotDealInfoStruct& DotInfo);
	
	UFUNCTION(BlueprintCallable)
	virtual void Die();

	bool IsDead() { return bIsDead; }

private:
	UPROPERTY(SaveGame)
	float CurrentHealth;

	bool bIsDead;

	TObjectPtr<AActor> OwnerActor;

	ECharacterType OwnerType; // 델리게이트에 전달해서 어떤 Type인지 확인.

	void CacheOwnerType(); // Player인지 Enemy인지 Base를 받아와서 확인. 델리게이트에 OwnerType 전달.

	// 활성 DoT 리스트
	TArray<FDotDealInfoStruct> ActiveDots;

	// 틱 처리 콜백
	void HandleDotTick(int32 DotIndex);
	
};
