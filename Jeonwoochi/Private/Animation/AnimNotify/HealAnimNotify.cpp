// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/HealAnimNotify.h"
#include "Components/Combat/HealthComponent.h"

void UHealAnimNotify::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation)
{
	if(!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	if (auto* HC = Owner->FindComponentByClass<UHealthComponent>())
	{
		HC->Heal(HC->GetMaxHealth() * 0.4f);
	}
}