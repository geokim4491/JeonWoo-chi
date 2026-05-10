// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Weapons_ShowByIDs.generated.h"

// 전용 스냅샷(이 태스크 내부에서만 사용)
USTRUCT()
struct FWeaponVisSnapshot_Show
{
    GENERATED_BODY()

    UPROPERTY() TWeakObjectPtr<class ABaseWeaponActor> Weapon;
    UPROPERTY() bool bPrevHidden = true;
    UPROPERTY() TEnumAsByte<ECollisionEnabled::Type> PrevCollision = ECollisionEnabled::NoCollision;
};

// 노드 메모리
USTRUCT()
struct FShowByIDsMemory
{
    GENERATED_BODY()

    UPROPERTY() TArray<FWeaponVisSnapshot_Show> Snapshots;
    UPROPERTY() TArray<TWeakObjectPtr<class ABaseWeaponActor>> SelectedSet;
};

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UBTTask_Weapons_ShowByIDs : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_Weapons_ShowByIDs();
	
    // 여러 WeaponID를 지정 (비어있으면 아무것도 표시하지 않음)
    UPROPERTY(EditAnywhere, Category = "Select")
    TArray<FName> DesiredWeaponIDs;

    // 못 찾았을 때 태스크 실패로 처리할지
    UPROPERTY(EditAnywhere, Category = "Select")
    bool bFailIfNotFound = true;

    // 태스크 종료/중단 시 가시성을 스냅샷대로 복구할지 (기본: 복구 안함=플리커 방지)
    UPROPERTY(EditAnywhere, Category = "Visibility")
    bool bRestoreVisibilityOnExit = false;

    // Abort(중단) 시 전부 숨길지(권장: true)
    UPROPERTY(EditAnywhere, Category = "Visibility")
    bool bHideAllOnAbort = true;

    // 액터 Hidden만이 아닌 컴포넌트 Visibility까지 토글할지
    UPROPERTY(VisibleAnywhere, Category = "Visibility")
    bool bAlsoToggleComponentVisibility = false;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual uint16 GetInstanceMemorySize() const override { return sizeof(FShowByIDsMemory); }

private:
    static TSet<FName> NormalizeIDs(const TArray<FName>& In);

    void CollectWeaponsAndSnapshot(APawn* OwnerPawn, FShowByIDsMemory* Mem) const;

    TArray<TWeakObjectPtr<class ABaseWeaponActor>> PickByWeaponIDs(
        const TArray<FWeaponVisSnapshot_Show>& Snapshots,
        const TSet<FName>& IDs) const;

    void ApplySelection(
        const TArray<FWeaponVisSnapshot_Show>& Snapshots,
        const TArray<TWeakObjectPtr<class ABaseWeaponActor>>& SelectedSet) const;

    void Restore(const TArray<FWeaponVisSnapshot_Show>& Snapshots) const;

    // 헬퍼
    void ForceHideWeapon(class ABaseWeaponActor* W) const;
    void ForceShowWeapon(class ABaseWeaponActor* W) const;

    class UPrimitiveComponent* GetWeaponHitShape(class ABaseWeaponActor* W) const;
};
