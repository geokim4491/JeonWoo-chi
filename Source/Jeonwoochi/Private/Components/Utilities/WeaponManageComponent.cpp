// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Utilities/WeaponManageComponent.h"

// Sets default values for this component's properties
UWeaponManageComponent::UWeaponManageComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UWeaponManageComponent::BeginPlay()
{
	Super::BeginPlay();

    // 1) 소유자(캐릭터)의 모든 ChildActorComponent 를 가져와서…
    TArray<UChildActorComponent*> ChildComps;
    GetOwner()->GetComponents(ChildComps);

    for (UChildActorComponent* CAC : ChildComps)
    {
        // 2) ChildActor 가 AWeaponActor 인지 검사
        if (ABaseWeaponActor* Weapon = Cast<ABaseWeaponActor>(CAC->GetChildActor()))
        {
            // 3) 리스트에 추가
            EquippedWeapons.Add(Weapon);
        }
    }
	
}


const TArray<ABaseWeaponActor*>& UWeaponManageComponent::GetEquippedWeapons() const 
{ 
	return EquippedWeapons; 
}

void UWeaponManageComponent::EquipWeapon(ABaseWeaponActor* W)
{
	if (W && !EquippedWeapons.Contains(W))
		EquippedWeapons.Add(W);
}

void UWeaponManageComponent::UnequipWeapon(ABaseWeaponActor* W)
{
	EquippedWeapons.Remove(W);
}