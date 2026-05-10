// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/EffectManagerAnimNotify.h"
#include "Manager/EffectManager.h"
#include "Kismet/GameplayStatics.h"

void UEffectManagerAnimNotify::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
    {
        if (UEffectManager* EM = GI->GetSubsystem<UEffectManager>())
        {
            if (bRelative)
            {
                EM->PlayEffectOnActor(
                    EffectType,
                    MeshComp->GetOwner(),
                    bUseHaptic);
            }
            else
            {
                EM->PlayEffect(
                    EffectType,
                    Location,
                    Rotation,
                    bUseHaptic
                );
            }
        }
    }
}