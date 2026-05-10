// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_UnlockMovement.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Unlock Movement (Early)"))
class JEONWOOCHI_API UAnimNotify_UnlockMovement : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
