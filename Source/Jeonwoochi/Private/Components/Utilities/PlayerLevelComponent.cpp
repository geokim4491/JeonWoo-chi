#include "Components/Utilities/PlayerLevelComponent.h"
#include "EngineUtils.h"
#include "Characters/AI/BaseAICharacter.h"
#include "Components/Combat/HealthComponent.h"
#include "Components/Utilities/PlayerStatComponent.h" // 실제 경로에 맞게 수정

UPlayerLevelComponent::UPlayerLevelComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerLevelComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentLevel = FMath::Max(StartingLevel, 1);
    CurrentExp = 0;

    // 같은 액터에 붙은 StatComponent 캐싱
    StatComp = GetOwner() ? GetOwner()->FindComponentByClass<UPlayerStatComponent>() : nullptr;

    // 1) 이미 존재하는 적들 바인딩
    BindAllExistingEnemies();

    // 2) 이후 스폰되는 액터도 바인딩
    if (UWorld* World = GetWorld())
    {
        SpawnHandle = World->AddOnActorSpawnedHandler(
            FOnActorSpawned::FDelegate::CreateUObject(this, &UPlayerLevelComponent::OnActorSpawned));
    }
}

void UPlayerLevelComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->RemoveOnActorSpawnedHandler(SpawnHandle);
    }
    Super::EndPlay(EndPlayReason);
}

void UPlayerLevelComponent::BindAllExistingEnemies()
{
    if (!GetWorld()) return;

    for (TActorIterator<ABaseAICharacter> It(GetWorld()); It; ++It)
    {
        TryBindHealth(*It);
    }
}

void UPlayerLevelComponent::OnActorSpawned(AActor* Spawned)
{
    TryBindHealth(Spawned);
}

void UPlayerLevelComponent::TryBindHealth(AActor* OwnerActor)
{
    if (!OwnerActor) return;

    // 타입이 적인지 확인
    ECharacterType Type = ECharacterType::None;
    if (OwnerActor->Implements<UCharacterTypeInterface>())
    {
        Type = ICharacterTypeInterface::Execute_GetCharacterType(OwnerActor);
    }
    if (!CountAsEnemyTypes.Contains(Type)) return;

    if (UHealthComponent* HC = OwnerActor->FindComponentByClass<UHealthComponent>())
    {
        // 중복 바인딩 방지는 필요 시 별도 관리 가능
        HC->OnDeath.AddUObject(this, &UPlayerLevelComponent::OnAnyActorDied);
    }
}

void UPlayerLevelComponent::OnAnyActorDied(AActor* DeadActor, ECharacterType OwnerType)
{
    // 적만 카운트
    if (!CountAsEnemyTypes.Contains(OwnerType) || !DeadActor)
        return;

    int32 Reward = 0;

    if (const ABaseAICharacter* AI = Cast<ABaseAICharacter>(DeadActor))
    {
        Reward = FMath::Max(AI->ExpReward, 0);
    }
    else
    {
        Reward = 5; // 비전형 케이스 기본값
    }

    AddExperience(Reward);
}

void UPlayerLevelComponent::AddExperience(int32 Amount)
{
    if (Amount <= 0) return;

    CurrentExp += Amount;
    OnExpChanged.Broadcast(CurrentExp, GetExpToNextLevel());

    while (CurrentExp >= GetExpToNext(CurrentLevel))
    {
        CurrentExp -= GetExpToNext(CurrentLevel);
        LevelUp();
        OnExpChanged.Broadcast(CurrentExp, GetExpToNextLevel());
    }
}

void UPlayerLevelComponent::LevelUp()
{
    ++CurrentLevel;

    // 스탯 포인트 증가(무파라미터)
    if (StatComp)
    {
        StatComp->IncreaseStatPoints();
    }

    OnLevelUp.Broadcast(CurrentLevel);
}

int32 UPlayerLevelComponent::GetExpToNext(int32 Level) const
{
    const float Need = static_cast<float>(BaseExpToNext) * FMath::Pow(ExpGrowth, FMath::Max(Level - 1, 0));
    return FMath::Max(1, FMath::RoundToInt(Need));
}
