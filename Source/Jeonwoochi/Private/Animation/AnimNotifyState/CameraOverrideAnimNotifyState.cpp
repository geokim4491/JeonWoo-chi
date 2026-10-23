// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState/CameraOverrideAnimNotifyState.h"
#include "GameFramework/Character.h"
#include "../../../JeonwoochiCharacter.h"

void UCameraOverrideAnimNotifyState::NotifyBegin(
    USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    if (ACharacter* Char = MeshComp ? Cast<ACharacter>(MeshComp->GetOwner()) : nullptr)
    {
        if (AJeonwoochiCharacter* My = Cast<AJeonwoochiCharacter>(Char))
        {
            
        }
    }
}

void UCameraOverrideAnimNotifyState::NotifyEnd(
    USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    if (ACharacter* Char = MeshComp ? Cast<ACharacter>(MeshComp->GetOwner()) : nullptr)
    {
        if (AJeonwoochiCharacter* My = Cast<AJeonwoochiCharacter>(Char))
        {
            
        }
    }
}