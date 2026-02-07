// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/Combat/HitDetectionComponent.h"


// Sets default values
AProjectileActor::AProjectileActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    // 1) 메시 컴포넌트 (선택)
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));;
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RootComponent = MeshComp;

    // 2) HitComp
    HitComp = CreateDefaultSubobject<UHitDetectionComponent>(TEXT("HitComp"));
    HitComp->SetupAttachment(MeshComp);

    // 3) ProjectileMovement 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = MeshComp;
    ProjectileMovement->InitialSpeed = 1500.f;
    ProjectileMovement->MaxSpeed = 1500.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

}

void AProjectileActor::BeginPlay()
{
    Super::BeginPlay();

    // 충돌 감지 활성화 (HitDetectionComponent 내부의 BoxComponent 충돌을 켭니다)
    HitComp->StartHitWindow();

    // 이동 컴포넌트 활성화
    ProjectileMovement->Activate();

    if (LifeTime > 0.f)
    {
        SetLifeSpan(LifeTime);
    }
}

void AProjectileActor::InitVelocity(const FVector& ShootDirection)
{
    if (ProjectileMovement)
    {
        ProjectileMovement->Velocity = ShootDirection * ProjectileMovement->InitialSpeed;
    }
}

void AProjectileActor::SetProjectileSpeed(float InitSpeed, float MaxSpeed)
{
    ProjectileMovement->InitialSpeed = InitSpeed;
    ProjectileMovement->MaxSpeed = MaxSpeed;
}

