// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "EffectManagerAnimNotify.generated.h"

/**
 * 
 */

enum class EEffectType : uint8;

UCLASS()
class JEONWOOCHI_API UEffectManagerAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation) override;

	UPROPERTY(EditAnywhere, Category = "Miscellaneous")
	bool bRelative = false;

	UPROPERTY(EditAnywhere, Category = "Miscellaneous")
	bool bUseHaptic = false;


	UPROPERTY(EditAnywhere, Category = "EffectType")
	EEffectType EffectType;

	UPROPERTY(EditAnywhere, Category = "Transform")
	FVector Location;
	
	UPROPERTY(EditAnywhere, Category = "Transform")
	FRotator Rotation;
};
