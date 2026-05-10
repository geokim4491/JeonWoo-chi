// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enum/CharacterTypeEnum.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "../Interface/Character/CharacterTypeInterface.h"
#include "BaseCharacter.generated.h"

class UMotionWarpingComponent;

UCLASS()
class JEONWOOCHI_API ABaseCharacter : public ACharacter,
	public ICharacterTypeInterface,
	public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ECharacterType CharacterType;

	/** Stimuli Source ������Ʈ */
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	UAIPerceptionStimuliSourceComponent* StimuliSourceComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionWarping")
	UMotionWarpingComponent* MotionWarpingComp;

	UPROPERTY(EditAnywhere, Category = "TeamId")
	FGenericTeamId TeamId;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual ECharacterType GetCharacterType_Implementation() override;

	// IGenericTeamAgentInterface ����
	virtual FGenericTeamId GetGenericTeamId() const override 
	{
		return FGenericTeamId((TeamId)); 
	}
	virtual void SetGenericTeamId(const FGenericTeamId& NewID) override 
	{ 
		TeamId = NewID.GetId();
	}
	ECharacterType GetCharacterType() const
	{
		return CharacterType;
	}
};
