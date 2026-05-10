// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventorySlotWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "UI/Operation/USlotDragPayload.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


void UInventorySlotWidget::Setup(UPlayerInventoryComponent* InComp, int32 InIndex)
{
	InventoryComp = InComp;
	SlotIndex = InIndex;
	Refresh();
}

void UInventorySlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (BTN_Click)
	{
		BTN_Click->OnClicked.RemoveAll(this);
		BTN_Click->OnClicked.AddDynamic(this, &UInventorySlotWidget::OnClicked);
	}
}

FInventorySlot UInventorySlotWidget::GetMySlot() const
{
	if (!InventoryComp || !InventoryComp->GetSlots().IsValidIndex(SlotIndex)) return FInventorySlot{};
	return InventoryComp->GetSlots()[SlotIndex];
}

void UInventorySlotWidget::SetVisual(const FInventorySlot& Data)
{
	if (IMG_Icon)
	{
		IMG_Icon->SetVisibility(Data.IsEmpty() ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
		if (!Data.IsEmpty() && Data.Item && Data.Item->Icon)
		{
			IMG_Icon->SetBrushFromTexture(Data.Item->Icon);
		}
	}
	if (TXT_Count)
	{
		if (Data.IsEmpty() || Data.Count <= 1) { TXT_Count->SetText(FText::GetEmpty()); }
		else { TXT_Count->SetText(FText::AsNumber(Data.Count)); }
	}
}

void UInventorySlotWidget::Refresh()
{
	SetVisual(GetMySlot());
}

void UInventorySlotWidget::OnClicked()
{
	if (!InventoryComp || SlotIndex == INDEX_NONE) return;
	FInventorySlot Data = GetMySlot();
	if (Data.IsEmpty() || !Data.Item) return;

	// 요구사항: 클릭 시 "사용 아이템은 사용", "장비 아이템은 장착"
	if (Data.Item->Category == EItemCategory::Equipment)
	{
		InventoryComp->EquipFromIndex(SlotIndex);
	}
	else if (Data.Item->bUsable)
	{
		InventoryComp->UseItem(SlotIndex);
	}

	Refresh();
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeo, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeo, InMouseEvent);
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeo, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!InventoryComp) return;

	UUSlotDragPayload* Payload = NewObject<UUSlotDragPayload>(this);
	Payload->bFromEquipment = false;
	Payload->FromIndex = SlotIndex;

	UDragDropOperation* Op = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
	Op->Payload = Payload;
	Op->DefaultDragVisual = this; // 필요시 미러 위젯 사용
	OutOperation = Op;
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (!InventoryComp || !InOperation || SlotIndex == INDEX_NONE) return false;

	if (UUSlotDragPayload* Payload = Cast<UUSlotDragPayload>(InOperation->Payload))
	{
		if (!Payload->bFromEquipment)
		{
			// 인벤토리→인벤토리: Move
			if (Payload->FromIndex != INDEX_NONE && Payload->FromIndex != SlotIndex)
			{
				// Amount: 전체 이동(스택 병합/스왑은 MoveItem 내부 처리)
				return InventoryComp->MoveItem(Payload->FromIndex, SlotIndex, TNumericLimits<int32>::Max());
			}
		}
		else
		{
			// 장비→인벤토리 드롭: 해제 시도 (AddItem은 컴포넌트 내부에서 처리됨)
			if (Payload->FromEquipSlot != EEquipSlot::None)
			{
				// 특정 칸으로 떨어뜨려도 결과는 동일: 성공 시 인벤토리 변경 브로드캐스트
				return InventoryComp->Unequip(Payload->FromEquipSlot);
			}
		}
	}
	return false;
}


