// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/EquipmentSlotWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "UI/Operation/USlotDragPayload.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UEquipmentSlotWidget::Setup(UPlayerInventoryComponent* InComp, EEquipSlot InSlot)
{
	InventoryComp = InComp;
	SlotType = InSlot;
	Refresh();
}

void UEquipmentSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (BTN_Click)
	{
		BTN_Click->OnClicked.RemoveAll(this);
		BTN_Click->OnClicked.AddDynamic(this, &UEquipmentSlotWidget::OnClicked);
	}
}

void UEquipmentSlotWidget::SetVisual(const FInventorySlot& Data)
{
	if (IMG_Icon)
	{
		IMG_Icon->SetVisibility(Data.IsEmpty() ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
		if (!Data.IsEmpty() && Data.Item && Data.Item->Icon)
		{
			IMG_Icon->SetBrushFromTexture(Data.Item->Icon);
		}
	}
}

void UEquipmentSlotWidget::Refresh()
{
	if (!InventoryComp) { SetVisual(FInventorySlot{}); return; }
	SetVisual(InventoryComp->GetEquipmentAt(SlotType));
}

void UEquipmentSlotWidget::OnClicked()
{
	if (!InventoryComp || SlotType == EEquipSlot::None) return;
	// 클릭으로 해제
	InventoryComp->Unequip(SlotType);
	Refresh();
}

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeo, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeo, InMouseEvent);
}

void UEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeo, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!InventoryComp) return;

	// 장비 → 드래그 시작 (인벤토리 칸에 드롭 시 해제/이동 용)
	UUSlotDragPayload* Payload = NewObject<UUSlotDragPayload>(this);
	Payload->bFromEquipment = true;
	Payload->FromEquipSlot = SlotType;

	UDragDropOperation* Op = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
	Op->Payload = Payload;
	Op->DefaultDragVisual = this;
	OutOperation = Op;
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 인벤토리 → 장비 드롭 (장착)
	if (!InventoryComp || !InOperation) return false;

	if (UUSlotDragPayload* Payload = Cast<UUSlotDragPayload>(InOperation->Payload))
	{
		if (!Payload->bFromEquipment && Payload->FromIndex != INDEX_NONE)
		{
			// UPlayerInventoryComponent::EquipFromIndex 는 아이템의 EquipSlot을 내부에서 체크
			// 슬롯 타입이 다르면 AddItem/Swap 로직이 작동하거나 false가 나옴
			return InventoryComp->EquipFromIndex(Payload->FromIndex);
		}
	}
	return false;
}
