// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileActor.generated.h"

class UProjectileMovementComponent;
class UHitDetectionComponent;

UCLASS()
class JEONWOOCHI_API AProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileActor();

	/** 투사체의 초기 방향과 속도를 설정 */
	void InitVelocity(const FVector& ShootDirection);

	UFUNCTION()
	void SetProjectileSpeed(float InitSpeed, float MaxSpeed);

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	UStaticMeshComponent* MeshComp;

	/** 이동을 담당하는 컴포넌트 */
	UPROPERTY(EditAnywhere, Category = "Projectile | Movement")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ClampMin = "0.0"))
	float LifeTime = 3.f;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UHitDetectionComponent* HitComp;
};
