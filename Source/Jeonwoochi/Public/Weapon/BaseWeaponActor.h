// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/** Component */
#include "../Components/Combat/HitDetectionComponent.h"

/** Struct */
#include "../Structure/Weapon/WeaponStatStruct.h"


#include "BaseWeaponActor.generated.h"

UCLASS()
class JEONWOOCHI_API ABaseWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeaponActor();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision|Hitbox", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHitDetectionComponent> HitDetectionComp;

	UPROPERTY(EditAnywhere, Category = "Weapon|Ability")
	FWeaponStatStruct WeaponStat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Mesh")
	UStaticMeshComponent* WeaponStaticMesh;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	TObjectPtr<UHitDetectionComponent> GetHitDetectionComp() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName WeaponID;
};
