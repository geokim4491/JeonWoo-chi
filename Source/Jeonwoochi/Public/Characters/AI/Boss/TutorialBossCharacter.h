// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AI/BossCharacter.h"
#include "TutorialBossCharacter.generated.h"

UENUM(BlueprintType)
enum class ETutoCombatType : uint8
{
	Wand	UMETA(DisplayName = "Mage"),
	Dual	UMETA(DisplayName = "Dual"),
	Katana	UMETA(DisplayName = "Katana"),
	Spear	UMETA(DisplayName = "Spear"),
	Halberd UMETA(DisplayName = "Halberd"),
	Bow		UMETA(DisplayName = "Bow"),

};

USTRUCT(BlueprintType)
struct FTutoCombatType
{
	GENERATED_BODY()

	/** 전투 타입 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	ETutoCombatType CombatType;

	/** 타입별 몇 초 동안 지속하고 다음 타입으로 넘어갈지 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	float TypeDuration = 10.f;
};

class UDimensionShiftComponent;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API ATutorialBossCharacter : public ABossCharacter
{
	GENERATED_BODY()
	
public:
	ATutorialBossCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<FTutoCombatType> CombatTypes;

	// BB에 현재 전투 타입을 넣어줄 키 (Enum(ECombatType)로 설정!)
	UPROPERTY(EditAnywhere, Category = "AI|Blackboard")
	FBlackboardKeySelector CombatTypeBBKey;

	// “다른 것이 2번 나오기 전엔 재선택 금지” → 최근 히스토리 길이
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "1", ClampMax = "5"))
	int32 RecentMemory = 2;

	// 전투 타입 루프 시작(전투 개시/페이즈 전환 시 호출)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartCombatTypeLoop();

	UFUNCTION()
	void RestartBossBT(EComebackCondition CombackCondition);

	FORCEINLINE void SetCachedDimensionComp(UDimensionShiftComponent* DimensionComp)
	{
		CachedDimensionComp = DimensionComp;
	}

protected:
	virtual void ChangePhase_Implementation(int32 CurrentPhase) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Die_Implementation() override;

private:
	// 현재 선택된 인덱스(CombatTypes 기준)
	int32 CurrentIndex = INDEX_NONE;

	// 최근에 선택된 타입 히스토리(가장 오래된 → 최신)
	UPROPERTY(VisibleInstanceOnly, Category = "Combat")
	TArray<ETutoCombatType> RecentHistory;

	FTimerHandle TypeTimerHandle;

	void PickAndApplyNextCombatType(bool bFirstPick = false);
	void OnTypeDurationElapsed();

	float GetDurationForIndex(int32 Index) const;

	UDimensionShiftComponent* CachedDimensionComp;
};
