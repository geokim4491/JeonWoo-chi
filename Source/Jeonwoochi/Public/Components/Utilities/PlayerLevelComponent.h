#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enum/CharacterTypeEnum.h"
#include "Delegates/DelegateCombinations.h"
#include "PlayerLevelComponent.generated.h"

class UHealthComponent;
class UPlayerStatComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32 /*NewLevel*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnExpChanged, int32 /*CurExp*/, int32 /*ExpToNext*/);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class JEONWOOCHI_API UPlayerLevelComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPlayerLevelComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void BindAllExistingEnemies();
    void OnActorSpawned(AActor* Spawned);
    void TryBindHealth(AActor* OwnerActor);

    // HealthComponent::OnDeath 콜백
    void OnAnyActorDied(AActor* DeadActor, ECharacterType OwnerType);

    void LevelUp();
    int32 GetExpToNext(int32 Level) const;

public:
    UFUNCTION(BlueprintCallable, Category = "Level")
    void AddExperience(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Level")
    int32 GetLevel() const { return CurrentLevel; }

    UFUNCTION(BlueprintCallable, Category = "Level")
    int32 GetCurrentExp() const { return CurrentExp; }

    UFUNCTION(BlueprintCallable, Category = "Level")
    int32 GetExpToNextLevel() const { return GetExpToNext(CurrentLevel); }

public:
    UPROPERTY(EditAnywhere, Category = "Level")
    int32 StartingLevel = 1;

    UPROPERTY(EditAnywhere, Category = "Level")
    int32 BaseExpToNext = 100;

    UPROPERTY(EditAnywhere, Category = "Level")
    float ExpGrowth = 1.25f;  // 요구 경험치 성장률

    UPROPERTY(EditAnywhere, Category = "Level")
    TSet<ECharacterType> CountAsEnemyTypes = { ECharacterType::NormalEnemy, ECharacterType::Boss };

    FOnLevelUp OnLevelUp;
    FOnExpChanged OnExpChanged;

private:
    UPROPERTY(SaveGame)
    int32 CurrentLevel = 1;

    UPROPERTY(SaveGame)
    int32 CurrentExp = 0;

    UPROPERTY()
    UPlayerStatComponent* StatComp = nullptr;

    FDelegateHandle SpawnHandle;
};
