// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlayerSkills/PlayerSkillComponent.h"
#include "Components/PlayerSkills/illusionSkillComponent.h"
#include "Components/PlayerSkills/CloneSpawnComponent.h"

// Sets default values for this component's properties
UPlayerSkillComponent::UPlayerSkillComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	illusionComp = 
		CreateDefaultSubobject<UillusionSkillComponent>(TEXT("illusionComp"));

	CloneSpawnComp =
		CreateDefaultSubobject<UCloneSpawnComponent>(TEXT("CloneSpawnComp"));
}


// Called when the game starts
void UPlayerSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

