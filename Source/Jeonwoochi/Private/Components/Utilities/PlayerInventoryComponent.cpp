// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Utilities/PlayerInventoryComponent.h"
#include "GameFramework/Actor.h"

UPlayerInventoryComponent::UPlayerInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerInventoryComponent::BeginPlay()
{
	Super::BeginPlay();


	if (Slots.Num() != Capacity)
	{
		Slots.SetNum(Capacity);
	}

}

FInventorySlot UPlayerInventoryComponent::GetEquipmentAt(EEquipSlot Slot) const
{
	if (const FInventorySlot* Found = Equipment.Find(Slot))
	{
		return *Found;
	}
	return FInventorySlot{};
}

int32 UPlayerInventoryComponent::GetMaxStack(const UItemData* Item) const
{
	return Item ? FMath::Max(1, Item->MaxStackSize) : 1;
}

bool UPlayerInventoryComponent::CanStack(const FInventorySlot& A, const FInventorySlot& B) const
{
	return A.Item && B.Item && A.Item == B.Item && A.Item->Category != EItemCategory::Equipment;
}

int32 UPlayerInventoryComponent::FindFirstFreeSlot() const
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].IsEmpty()) return i;
	}
	return INDEX_NONE;
}

bool UPlayerInventoryComponent::AddItem(UItemData* ItemDef, int32 Count)
{
	if (!ItemDef || Count <= 0) return false;


	// First, fill existing stacks
	if (ItemDef->Category != EItemCategory::Equipment)
	{
		for (FInventorySlot& Slot : Slots)
		{
			if (Slot.Item == ItemDef && Slot.Count < GetMaxStack(ItemDef))
			{
				const int32 Space = GetMaxStack(ItemDef) - Slot.Count;
				const int32 ToAdd = FMath::Min(Space, Count);
				Slot.Count += ToAdd;
				Count -= ToAdd;
				if (Count <= 0)
				{
					OnInventoryChanged.Broadcast();
					return true;
				}
			}
		}
	}


	// Then, put into empty slots
	bool bAnyPlaced = false;
	while (Count > 0)
	{
		const int32 FreeIdx = FindFirstFreeSlot();
		if (FreeIdx == INDEX_NONE) break;


		const int32 Put = (ItemDef->Category == EItemCategory::Equipment) ? 1 : FMath::Min(GetMaxStack(ItemDef), Count);
		Slots[FreeIdx].Item = ItemDef;
		Slots[FreeIdx].Count = Put;
		Count -= Put;
		bAnyPlaced = true;
	}


	if (bAnyPlaced)
	{
		OnInventoryChanged.Broadcast();
	}
	return bAnyPlaced && Count == 0; // false if ran out of space
}

bool UPlayerInventoryComponent::RemoveAtIndex(int32 Index, int32 Count)
{
	if (!Slots.IsValidIndex(Index) || Count <= 0) return false;
	FInventorySlot& Slot = Slots[Index];
	if (Slot.IsEmpty()) return false;


	Slot.Count -= Count;
	if (Slot.Count <= 0) { Slot = FInventorySlot{}; }


	OnInventoryChanged.Broadcast();
	return true;
}

bool UPlayerInventoryComponent::UseItem(int32 Index)
{
	if (!Slots.IsValidIndex(Index)) return false;
	FInventorySlot& Slot = Slots[Index];
	if (Slot.IsEmpty() || !Slot.Item) return false;


	if (!Slot.Item->bUsable)
	{
		return false; // not usable
	}


	// Game-specific effects handled in Blueprint
	BP_OnUseItem(Slot.Item);


	// Default: consume one
	Slot.Count -= 1;
	if (Slot.Count <= 0) Slot = FInventorySlot{};


	OnInventoryChanged.Broadcast();
	return true;
}

bool UPlayerInventoryComponent::EquipFromIndex(int32 Index)
{
	if (!Slots.IsValidIndex(Index)) return false;
	FInventorySlot& Slot = Slots[Index];
	if (Slot.IsEmpty() || !Slot.Item) return false;
	if (Slot.Item->Category != EItemCategory::Equipment) return false;


	const EEquipSlot TargetSlot = Slot.Item->EquipSlot;
	if (TargetSlot == EEquipSlot::None) return false;


	// If something is already equipped, try to return it to inventory (or swap)
	if (FInventorySlot* Already = Equipment.Find(TargetSlot))
	{
		if (!Already->IsEmpty())
		{
			if (!AddItem(Already->Item, 1))
			{
				// No space: swap with inventory slot
				UItemData* Temp = Already->Item;
				Already->Item = Slot.Item;
				Already->Count = 1;
				Slot.Item = Temp;
				Slot.Count = 1;


				BP_OnEquipped(TargetSlot, Already->Item);
				OnEquipmentChanged.Broadcast();
				OnInventoryChanged.Broadcast();
				return true;
			}
			else
			{
				*Already = FInventorySlot{}; // cleared to make room
			}
		}
	}


	// Equip one (equipment count is always 1)
	FInventorySlot& EqSlot = Equipment.FindOrAdd(TargetSlot);
	EqSlot.Item = Slot.Item;
	EqSlot.Count = 1;


	// remove one from inventory slot
	Slot.Count -= 1;
	if (Slot.Count <= 0) Slot = FInventorySlot{};


	BP_OnEquipped(TargetSlot, EqSlot.Item);
	OnEquipmentChanged.Broadcast();
	OnInventoryChanged.Broadcast();
	return true;
}

bool UPlayerInventoryComponent::Unequip(EEquipSlot SlotType)
{
	FInventorySlot* EqSlot = Equipment.Find(SlotType);
	if (!EqSlot || EqSlot->IsEmpty()) return false;


	if (AddItem(EqSlot->Item, 1))
	{
		BP_OnUnequipped(SlotType, EqSlot->Item);
		*EqSlot = FInventorySlot{};
		OnEquipmentChanged.Broadcast();
		return true;
	}


	return false; // no space
}


bool UPlayerInventoryComponent::MoveItem(int32 FromIndex, int32 ToIndex, int32 Amount)
{
	if (!Slots.IsValidIndex(FromIndex) || !Slots.IsValidIndex(ToIndex) || Amount <= 0) return false;
	if (FromIndex == ToIndex) return false;


	FInventorySlot& From = Slots[FromIndex];
	FInventorySlot& To = Slots[ToIndex];
	if (From.IsEmpty()) return false;


	// Destination empty: move partial/all
	if (To.IsEmpty())
	{
		const int32 MoveCount = FMath::Min(Amount, From.Count);
		To.Item = From.Item;
		To.Count = MoveCount;
		From.Count -= MoveCount;
		if (From.Count <= 0) From = FInventorySlot{};
		OnInventoryChanged.Broadcast();
		return true;
	}


	// Same item & stackable: merge
	if (CanStack(From, To))
	{
		const int32 Max = GetMaxStack(To.Item);
		const int32 Space = Max - To.Count;
		if (Space <= 0) return false;
		const int32 MoveCount = FMath::Min3(Amount, From.Count, Space);
		To.Count += MoveCount;
		From.Count -= MoveCount;
		if (From.Count <= 0) From = FInventorySlot{};
		OnInventoryChanged.Broadcast();
		return true;
	}


	// Otherwise swap
	Swap(From, To);
	OnInventoryChanged.Broadcast();
	return true;
}