// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimNotify/EnableAttackAnimNotify.h"
#include "Components/Combat/PlayerAttackComponent.h"

void UEnableAttackAnimNotify::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    // PlayerAttackComponent °Ë»ö
    if (UPlayerAttackComponent* AttackComp =
        Owner->FindComponentByClass<UPlayerAttackComponent>())
    {
        AttackComp->bCanAttack = true;
    }
}