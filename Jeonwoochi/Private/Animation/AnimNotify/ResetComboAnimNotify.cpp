// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimNotify/ResetComboAnimNotify.h"
#include "Components/Combat/PlayerAttackComponent.h"

void UResetComboAnimNotify::Notify(
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
        AttackComp->ResetAllCombos();
        AttackComp->bIsAttacking = false;
    }
}