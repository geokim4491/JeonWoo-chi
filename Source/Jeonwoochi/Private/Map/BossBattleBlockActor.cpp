// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/BossBattleBlockActor.h"
#include "Characters/AI/BossCharacter.h"
#include "Components/Combat/HealthComponent.h"

// Sets default values
ABossBattleBlockActor::ABossBattleBlockActor()
{
 	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABossBattleBlockActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (BindingTarget)
	{
		if (auto* HC = BindingTarget->FindComponentByClass<UHealthComponent>())
		{
			HC->OnDeath.AddUObject(this, &ABossBattleBlockActor::DestroyActor);
		}
	}
}


void ABossBattleBlockActor::DestroyActor(AActor* DestroyActor, ECharacterType CharType)
{
	Destroy();
}