// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AI/Paper2D/PaperSkeletonCharacter.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Jeonwoochi/JeonwoochiPaperCharacter.h"
#include "Components/Combat/HealthComponent.h"

void APaperSkeletonCharacter::Attack1()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UGameplayStatics::PlaySound2D(World, AttackSound);

    const FVector Origin = GetActorLocation() + GetActorForwardVector() * 100.f;
    const float Radius = 300.f;

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
    ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); // Pawn만

    TArray<AActor*> Ignore; Ignore.Add(this);

    TArray<AActor*> OutActors;
    const bool bHit = UKismetSystemLibrary::SphereOverlapActors(
        World, Origin, Radius, ObjTypes, /*ClassFilter*/AJeonwoochiPaperCharacter::StaticClass(), Ignore, OutActors
    );

    if (!bHit) return;

    for (AActor* Other : OutActors)
    {
        if (!Other || Other == this) continue;
        UHealthComponent* HealthComp = Other->FindComponentByClass<UHealthComponent>();
        if (!HealthComp) return;
        HealthComp->ApplyDamage(AttackDamage);
        UGameplayStatics::PlaySound2D(World, HitSound);
    }
}

void APaperSkeletonCharacter::Attack2()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UGameplayStatics::PlaySound2D(World, AttackSound);

    const FVector Origin = GetActorLocation() + GetActorForwardVector() * 100.f;
    const float Radius = 300.f;

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
    ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); // Pawn만

    TArray<AActor*> Ignore; Ignore.Add(this);

    TArray<AActor*> OutActors;
    const bool bHit = UKismetSystemLibrary::SphereOverlapActors(
        World, Origin, Radius, ObjTypes, /*ClassFilter*/AJeonwoochiPaperCharacter::StaticClass(), Ignore, OutActors
    );

    if (!bHit) return;

    for (AActor* Other : OutActors)
    {
        if (!Other || Other == this) continue;
        UHealthComponent* HealthComp = Other->FindComponentByClass<UHealthComponent>();
        if (!HealthComp) return;
        HealthComp->ApplyDamage(AttackDamage);
        UGameplayStatics::PlaySound2D(World, HitSound);
    }
}