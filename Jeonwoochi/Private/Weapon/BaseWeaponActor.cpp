// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BaseWeaponActor.h"

// Sets default values
ABaseWeaponActor::ABaseWeaponActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponStaticMesh"));
	WeaponStaticMesh->SetupAttachment(RootComponent);
	WeaponStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponStaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WeaponStaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponStaticMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	WeaponStaticMesh->SetGenerateOverlapEvents(false);

	HitDetectionComp = CreateDefaultSubobject<UHitDetectionComponent>(TEXT("HitDetectionComp"));
	HitDetectionComp->SetupAttachment(WeaponStaticMesh);

	WeaponID = NAME_None;
}

// Called when the game starts or when spawned
void ABaseWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	

}

// Called every frame
void ABaseWeaponActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TObjectPtr<UHitDetectionComponent> ABaseWeaponActor::GetHitDetectionComp() const
{
	return HitDetectionComp;
}
