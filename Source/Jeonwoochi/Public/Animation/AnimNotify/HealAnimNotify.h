// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "HealAnimNotify.generated.h"

/**
 * 배치한 타이밍에 HealthComp의 Heal 호출
 */
UCLASS()
class JEONWOOCHI_API UHealAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation) override;
};
