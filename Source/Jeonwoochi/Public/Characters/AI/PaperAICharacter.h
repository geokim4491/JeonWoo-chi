// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "Interface/Character/CharacterTypeInterface.h"
#include "PaperAICharacter.generated.h"

class UPaperAIAnimationComponent;
class UHealthComponent;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API APaperAICharacter : public APaperCharacter, 
	public ICharacterTypeInterface
{
	GENERATED_BODY()

public:
	APaperAICharacter();

	void SetFacing(bool bRight);

	void StopBT();
	void RestartBT();

	virtual void Attack1() {}
	virtual void Attack2() {}

	virtual ECharacterType GetCharacterType_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ECharacterType CharacterType = ECharacterType::NormalEnemy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UArrowComponent* FacingArrow;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	FVector InitialSpriteScale;

protected:

	bool bFacingRight = true;
	FRotator FacingArrowBaseRot;

	UPROPERTY(VisibleAnywhere, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UPaperAIAnimationComponent* AnimationComponent;
	
	

	

	class AAIController* OwnerController;

};
