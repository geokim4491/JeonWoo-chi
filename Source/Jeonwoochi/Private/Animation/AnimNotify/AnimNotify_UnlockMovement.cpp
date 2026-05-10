// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimNotify/AnimNotify_UnlockMovement.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Jeonwoochi/JeonwoochiCharacter.h"
#include "Components/Combat/AttackComponent.h"

void UAnimNotify_UnlockMovement::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;
    if (AActor* Owner = MeshComp->GetOwner())
    {
        if (UAttackComponent* AC = Owner->FindComponentByClass<UAttackComponent>())
        {
            AC->Notify_UnlockMovementEarly();
        }
    }
}