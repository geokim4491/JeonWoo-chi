// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/NoExportTypes.h"
#include "PlayerStatComponent.generated.h"

class AJeonwoochiCharacter;


UENUM(BlueprintType)
enum class EPlayerStatType : uint8
{
	HP  UMETA(DisplayName = "HP"),
	MP    UMETA(DisplayName = "MP"),
	AP   UMETA(DisplayName = "AP"),
	IntP  UMETA(DisplayName = "IntP"),
	SkillP UMETA(DisplayName = "SkillP"),
	MystP  UMETA(DisplayName = "MystP"),
	DefP UMETA(DisplayName = "DefP"),

	Max		UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EPlayerStatType, EPlayerStatType::Max)



UCLASS(Blueprintable)
class JEONWOOCHI_API UPlayerStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerStatComponent();

protected:
	virtual void BeginPlay() override;

public:	
	// 스탯 분배 함수
	UFUNCTION(BlueprintCallable, Category = "Stat Management")
	bool AllocateStat(EPlayerStatType StatType);

	// 스탯 레벨 반환 함수
	UFUNCTION(BlueprintCallable, Category = "Stat Management")
	int32 GetStatLevel(EPlayerStatType StatType) const;

	// 남은 스탯 포인트 반환
	UFUNCTION(BlueprintCallable, Category = "Stat Management")
	int32 GetRemainingStatPoints() const;

	// 레벨업 시 스탯 포인트 +2 증가
	UFUNCTION(BlueprintCallable, Category = "Stat Management")
	void IncreaseStatPoints();
	
private:
	// 개별 스탯 레벨 업 함수
	bool UpgradeStatLevel(EPlayerStatType StatType);
	void HandleUpgradeStat(EPlayerStatType StatType);

private:
	UPROPERTY()
	AJeonwoochiCharacter* JeonwoochiCharacter;

	UPROPERTY(VisibleDefaultsOnly)
	TMap<EPlayerStatType, int32> StatLevelMap;

	UPROPERTY(VisibleDefaultsOnly)
	int32 RemainingStatPoints = 0;

	UPROPERTY(VisibleDefaultsOnly)
	int32 MaxStatLevel = 10;
		
};
