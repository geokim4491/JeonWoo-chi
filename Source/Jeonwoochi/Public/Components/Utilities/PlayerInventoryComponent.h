// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Inventory/ItemData.h"
#include "PlayerInventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UItemData> Item = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 0;


	bool IsEmpty() const { return Item == nullptr || Count <= 0; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UPlayerInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerInventoryComponent();

	// === Config ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 Capacity = 24; // number of slots

	// === State ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory")
	TArray<FInventorySlot> Slots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Inventory")
	TMap<EEquipSlot, FInventorySlot> Equipment; // count is always 1 for equipment

	// === UI Signals ===
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnEquipmentChanged OnEquipmentChanged;
public:
	// === Blueprint Events (hook game-specific logic) ===
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Events")
	void BP_OnUseItem(UItemData* ItemDef);



	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Events")
	void BP_OnEquipped(EEquipSlot Slot, UItemData* ItemDef);


	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Events")
	void BP_OnUnequipped(EEquipSlot Slot, UItemData* ItemDef);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FInventorySlot>& GetSlots() const { return Slots; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FInventorySlot GetEquipmentAt(EEquipSlot Slot) const;

	// Core
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UItemData* ItemDef, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveAtIndex(int32 Index, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool EquipFromIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool Unequip(EEquipSlot Slot);

	// === UI helper ops ===
/** Move for Inventory UI: move Amount from FromIndex to ToIndex.
* Handles stacking, swapping, and partial moves. Returns true if anything changed.
*/
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	bool MoveItem(int32 FromIndex, int32 ToIndex, int32 Amount);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	int32 FindFirstFreeSlot() const;
	bool CanStack(const FInventorySlot& A, const FInventorySlot& B) const;
	int32 GetMaxStack(const UItemData* Item) const;

		
};
