// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/Inventory/ItemData.h"
#include "USlotDragPayload.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UUSlotDragPayload : public UObject
{
	GENERATED_BODY()

public:
	// 드래그 시작 출처
	UPROPERTY() bool bFromEquipment = false;

	// 인벤토리에서 시작했으면 Index 사용
	UPROPERTY() int32 FromIndex = INDEX_NONE;

	// 장비에서 시작했으면 EquipSlot 사용
	UPROPERTY() EEquipSlot FromEquipSlot = EEquipSlot::None;
};
