// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AI/BaseAICharacter.h"
#include "CloneAICharacter.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API ACloneAICharacter : public ABaseAICharacter
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void CopyAppearanceFrom(ACharacter* Source);

	ACloneAICharacter();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Clone|Anim")
	TSubclassOf<UAnimInstance> ClothRetargetABP;
};
