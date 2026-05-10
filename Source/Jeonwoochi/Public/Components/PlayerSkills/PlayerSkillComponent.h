// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "PlayerSkillComponent.generated.h"

class UillusionSkillComponent;

class UCloneSpawnComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UPlayerSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerSkillComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;	

public:	
	UPROPERTY(EditDefaultsOnly, Category = "Skills")
	UillusionSkillComponent* illusionComp;

	UPROPERTY(EditDefaultsOnly, Category = "Skills")
	UCloneSpawnComponent* CloneSpawnComp;

	UPROPERTY(EditAnywhere, Category = "Skills|Amount")
	float illusionChiConsumption = 40.f;

	UPROPERTY(EditAnywhere, Category = "Skills|Amount")
	float CloneChiConsumption = 30.f;
		
};
