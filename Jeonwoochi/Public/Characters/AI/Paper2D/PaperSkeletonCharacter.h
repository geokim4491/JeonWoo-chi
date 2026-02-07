// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AI/PaperAICharacter.h"
#include "PaperSkeletonCharacter.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API APaperSkeletonCharacter : public APaperAICharacter
{
	GENERATED_BODY()
	
protected:

	virtual void Attack1() override;
	virtual void Attack2() override;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRadius = 200.f;          // 원(원판) 반지름

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 10.f;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* AttackSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* HitSound = nullptr;
};
