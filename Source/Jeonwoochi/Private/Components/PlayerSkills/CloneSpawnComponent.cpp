// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlayerSkills/CloneSpawnComponent.h"

#include "Characters/AI/CloneAICharacter.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Manager/EffectManager.h"

#include "Components/Combat/AttackComponent.h"
#include "Kismet/GameplayStatics.h"  

// Sets default values for this component's properties
UCloneSpawnComponent::UCloneSpawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

    bCanSpawn = true;
	// ...
}


// Called when the game starts
void UCloneSpawnComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool UCloneSpawnComponent::SpawnClones(const TArray<FVector>& SpawnLocations)
{
    if (!bCanSpawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnClones: CoolTime."));
        return false;
    }

    UWorld* World = GetWorld();
    if (!World || SpawnLocations.IsEmpty())
        return false;

    // 실제 생성된 분신이 있을 때만 스킬 성공으로 처리
    int32 SpawnedCount = 0;
    for (const FVector& Loc : SpawnLocations)
    {
        if (TrySpawnClone(Loc))
            ++SpawnedCount;
    }

    if (SpawnedCount == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnClones: No clones were spawned."));
        return false;
    }

    // 쿨다운 타이머는 소환 요청당 한 번만 설정
    bCanSpawn = false;
    World->GetTimerManager().SetTimer(
        SpawnCooldownTimerHandle,
        this,
        &UCloneSpawnComponent::ResetSpawnAbility,
        CloneDuration,
        false
    );

    return true;
}

// Blueprint에서 직접 호출해도 같은 쿨다운/성공 판정을 적용
void UCloneSpawnComponent::InternalSpawn(const FVector& Loc)
{
    TArray<FVector> Locations;
    Locations.Add(Loc);
    SpawnClones(Locations);
}

bool UCloneSpawnComponent::TrySpawnClone(const FVector& Loc)
{
    UWorld* World = GetWorld();
    ACharacter* SourceChar = Cast<ACharacter>(GetOwner());
    if (!CloneClass || !World || !SourceChar)
        return false;

    const FTransform SpawnTransform(SourceChar->GetActorRotation(), Loc);
    ACloneAICharacter* Clone = World->SpawnActorDeferred<ACloneAICharacter>(
        CloneClass,
        SpawnTransform,
        SourceChar,
        SourceChar,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (!IsValid(Clone))
        return false;

    Clone->CopyAppearanceFrom(SourceChar);
    UGameplayStatics::FinishSpawningActor(Clone, SpawnTransform);

    // Construction Script 등에서 분신이 무효화된 경우 실패 처리
    if (!IsValid(Clone))
        return false;

    Clone->illusioned(CloneDuration);
    ActiveClones.Add(Clone);
    return true;
}

void UCloneSpawnComponent::ResetSpawnAbility()
{
    bCanSpawn = true;
}

void UCloneSpawnComponent::PlayEffect(EEffectType EffectType, FVector Location, FRotator Rotation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
    {
        if (UEffectManager* EM = GI->GetSubsystem<UEffectManager>())
        {
            EM->PlayEffect(
                EffectType,
                Location,
                Rotation
            );
        }
    }
}

void UCloneSpawnComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(SpawnCooldownTimerHandle);

    Super::EndPlay(EndPlayReason);
}