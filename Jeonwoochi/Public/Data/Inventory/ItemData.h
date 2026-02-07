// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Consumable UMETA(DisplayName = "Consumable"),
	Material UMETA(DisplayName = "Material"),
	Quest UMETA(DisplayName = "Quest"),
	Equipment UMETA(DisplayName = "Equipment")
};


UENUM(BlueprintType)
enum class EEquipSlot : uint8
{
	None UMETA(DisplayName = "None"),
	Head UMETA(DisplayName = "Head"),
	Chest UMETA(DisplayName = "Chest"),
	Legs UMETA(DisplayName = "Legs"),
	Weapon UMETA(DisplayName = "Weapon")
};

UCLASS()
class JEONWOOCHI_API UItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemID = NAME_None;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	UTexture2D* Icon = nullptr;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EItemCategory Category = EItemCategory::Material;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "Category == EItemCategory::Equipment"))
	EEquipSlot EquipSlot = EEquipSlot::None;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
	int32 MaxStackSize = 99;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bUsable = false; // true: can Use(); false: cannot
};
