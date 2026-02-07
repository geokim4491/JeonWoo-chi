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

    // 배치 스폰
    for (const FVector& Loc : SpawnLocations)
    {
        InternalSpawn(Loc);
    }

    // 소환 후 쿨다운 시작
    bCanSpawn = false;
    GetWorld()->GetTimerManager().SetTimer(
        SpawnCooldownTimerHandle,
        this,
        &UCloneSpawnComponent::ResetSpawnAbility,
        CloneDuration,
        false
    );

    return true;
}

void UCloneSpawnComponent::InternalSpawn(const FVector& Loc)
{
    UE_LOG(LogTemp, Display, TEXT("Spawn"));

    if (!CloneClass) return;
    UWorld* World = GetWorld();
    AActor* Owner = GetOwner();
    if (!World || !Owner) return;

    FRotator Rot = Owner->GetActorRotation();
    FActorSpawnParameters Params;
    Params.Owner = Owner;
    Params.Instigator = Cast<APawn>(Owner);

    // 2) Deferred Spawn
    FTransform SpawnTransform(Owner->GetActorRotation(), Loc);
    // SpawnActorDeferred로 인스턴스만 만들고 초기화는 FinishSpawningActor에서
    ACloneAICharacter* Clone = World->SpawnActorDeferred<ACloneAICharacter>(
        CloneClass,
        SpawnTransform,
        Owner,
        Cast<APawn>(Owner),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (!Clone) return;

    // 외형 복사
    if (ACharacter* SourceChar = Cast<ACharacter>(Owner))
    {
        // 1) 메쉬·머티리얼·ChildActor 복사
        Clone->CopyAppearanceFrom(SourceChar);
    }

    if (UAttackComponent* AC = Clone->FindComponentByClass<UAttackComponent>())
    {
        AC->SetAnimInstance(Clone->GetMesh()->GetAnimInstance());  // <- 캐시 갱신
    }

    UGameplayStatics::FinishSpawningActor(Clone, SpawnTransform);
    
    Clone->illusioned(CloneDuration);

    ActiveClones.Add(Clone);

    bCanSpawn = false;
    World->GetTimerManager().SetTimer(
        SpawnCooldownTimerHandle,
        this,
        &UCloneSpawnComponent::ResetSpawnAbility,
        CloneDuration,
        false
    );
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