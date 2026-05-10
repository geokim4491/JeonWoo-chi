// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Utilities/PlayerStatComponent.h"
#include "../JeonwoochiCharacter.h"


UPlayerStatComponent::UPlayerStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	for (EPlayerStatType Type : TEnumRange<EPlayerStatType>())
	{
		StatLevelMap.Add(Type, 1);
	}
}


void UPlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();

	JeonwoochiCharacter = Cast<AJeonwoochiCharacter>(GetOwner());
}

bool UPlayerStatComponent::AllocateStat(EPlayerStatType StatType)
{
	return UpgradeStatLevel(StatType);
}

int32 UPlayerStatComponent::GetStatLevel(EPlayerStatType StatType) const
{
	return StatLevelMap[StatType];
}

int32 UPlayerStatComponent::GetRemainingStatPoints() const
{
	return RemainingStatPoints;
}

void UPlayerStatComponent::IncreaseStatPoints()
{
	RemainingStatPoints += 2;
}

bool UPlayerStatComponent::UpgradeStatLevel(EPlayerStatType StatType)
{
	int32 CurrentStatLevel = StatLevelMap[StatType];
	if (CurrentStatLevel >= MaxStatLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("The Stat is already at max level."));
		return false;
	}
	if (RemainingStatPoints <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough stat points to allocate."));
		return false;
	}

	// 스탯 레벨 업 및 남은 스탯 포인트 감소
	StatLevelMap[StatType]++;
	RemainingStatPoints--;

	// 스탯 업그레이드 로직 처리
	HandleUpgradeStat(StatType);

	UE_LOG(LogTemp, Log, TEXT("Stat is upgraded to level %d. Remaining StatPoints: %d"), StatLevelMap[StatType], RemainingStatPoints);
	return true;
}

void UPlayerStatComponent::HandleUpgradeStat(EPlayerStatType StatType)
{
	if (!JeonwoochiCharacter) return;
	switch (StatType)
	{
	case EPlayerStatType::HP:
		JeonwoochiCharacter->IncreaseMaxHP();
		break;
	case EPlayerStatType::MP:
		JeonwoochiCharacter->IncreaseMaxMP();
		break;
	case EPlayerStatType::AP:
		JeonwoochiCharacter->IncreaseMaxAP();
		break;
	case EPlayerStatType::IntP:
		JeonwoochiCharacter->IncreaseMaxIntP();
		break;
	case EPlayerStatType::SkillP:
		JeonwoochiCharacter->IncreaseMaxSkillP();
		break;
	case EPlayerStatType::MystP:
		JeonwoochiCharacter->IncreaseMaxMystP();
		break;
	case EPlayerStatType::DefP:
		JeonwoochiCharacter->IncreaseMaxDefP();
		break;
	default:
		break;
	}
}


