// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Weapon/BaseWeaponActor.h"
#include "WeaponManageComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UWeaponManageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponManageComponent();

	const TArray<ABaseWeaponActor*>& GetEquippedWeapons() const;

	void EquipWeapon(ABaseWeaponActor* W);

	void UnequipWeapon(ABaseWeaponActor* W);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<ABaseWeaponActor*> EquippedWeapons;

};
