// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/BTTask/Attack/BTTask_Attack.h"
#include "BTTask_Attack_SelectWeapon.generated.h"

class ABaseWeaponActor;

/** 개별 무기의 이전 상태 스냅샷 */
USTRUCT()
struct FWeaponVisSnapshot
{
	GENERATED_BODY()

	/** 타겟 무기 */
	UPROPERTY()
	TWeakObjectPtr<ABaseWeaponActor> Weapon;

	/** SetActorHiddenInGame 변경 이전의 숨김 상태 */
	UPROPERTY()
	bool bPrevHidden = false;

	/** HitDetectionComp.HitShape의 이전 CollisionEnabled */
	UPROPERTY()
	TEnumAsByte<ECollisionEnabled::Type> PrevCollision = ECollisionEnabled::NoCollision;
};

/** 이 태스크 인스턴스 메모리 */
USTRUCT()
struct FAttackSelectWeaponMemory
{
	GENERATED_BODY()

	UPROPERTY()
	float ElapsedTime = 0.f;

	/** 스냅샷들 (복구용) */
	UPROPERTY()
	TArray<FWeaponVisSnapshot> Snapshots;

	/** 선택된 무기(없을 수도 있음) */
	UPROPERTY()
	TArray<TWeakObjectPtr<ABaseWeaponActor>> SelectedSet;
};

/**
 * Visible Only Selected WeaponID and Disable that's HitDetection's Collision
 * When Finish/Abort, restore Visible/Collision.
 * 선택한 WeaponID만 보이도록 하고
 */
UCLASS()
class JEONWOOCHI_API UBTTask_Attack_SelectWeapon : public UBTTask_Attack
{
	GENERATED_BODY()
	
public:
	UBTTask_Attack_SelectWeapon();

	/** Select WeaponID */
	UPROPERTY(EditAnywhere, Category = "Select")
	TArray<FName> DesiredWeaponIDs;

	/** Except Selected Weapon, Hidden in games */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bHideOtherWeapons = true;

	/** Disable Non Selected Weapon's Collision*/
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bDisableNonSelectedWeaponHitbox = true;

	/** if failed Found Weapon return failed */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bFailIfNotFound = true;

	/** size of NodeMemory */
	virtual uint16 GetInstanceMemorySize() const override
	{
		return sizeof(FAttackSelectWeaponMemory);
	}

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	/** 캐릭터에 부착된 모든 ABaseWeaponActor 수집 + 스냅샷 저장 */
	void CollectWeaponsAndSnapshot(class ABaseAICharacter* Char, FAttackSelectWeaponMemory* Mem) const;

	/** WeaponID로 선택 (여러 개 가능) */
	// 기존: ABaseWeaponActor* PickByWeaponID(const TArray<FWeaponVisSnapshot>& Snapshots) const;
	TArray<TWeakObjectPtr<ABaseWeaponActor>> PickByWeaponIDs(
		const TArray<FWeaponVisSnapshot>& Snapshots,
		const TSet<FName>& IDs) const;

	/** 선택 결과를 적용(가시성/충돌 변경) — 여러 개 처리 */
	// 기존: void ApplySelection(const TArray<FWeaponVisSnapshot>&, ABaseWeaponActor* Selected) const;
	void ApplySelection(const TArray<FWeaponVisSnapshot>& Snapshots, const TArray<TWeakObjectPtr<ABaseWeaponActor>>& SelectedSet) const;

	/** 변경사항 복구 */
	void Restore(const TArray<FWeaponVisSnapshot>& Snapshots) const;

	// 중복/None 제거한 ID 집합 만들기
	static TSet<FName> NormalizeIDs(const TArray<FName>& In);
};
