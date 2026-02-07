// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventoryWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/TextBlock.h"
#include "UI/InventorySlotWidget.h"
#include "UI/EquipmentSlotWidget.h"
#include "Components/Utilities/PlayerInventoryComponent.h"
#include "GameFramework/Character.h"

void UInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

void UInventoryWidget::InitializeWithComponent(UPlayerInventoryComponent* InComp)
{
	if (!InComp || !GridInventory || !InventorySlotClass) return;

	InventoryComp = InComp;

	// 브로드캐스트 바인딩
	InventoryComp->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::HandleInventoryChanged);
	InventoryComp->OnEquipmentChanged.AddDynamic(this, &UInventoryWidget::HandleEquipmentChanged);

	RebuildGrid();
	SetupEquipmentWidgets();
	RefreshAll();
}

void UInventoryWidget::RebuildGrid()
{
	if (!GridInventory || !InventoryComp || !InventorySlotClass) return;

	GridInventory->ClearChildren();
	SlotWidgets.Reset();

	const int32 Count = InventoryComp->GetSlots().Num();

	for (int32 i = 0; i < Count; ++i)
	{
		UInventorySlotWidget* W = CreateWidget<UInventorySlotWidget>(this, InventorySlotClass);
		W->Setup(InventoryComp, i);

		const int32 Row = i / FMath::Max(1, NumColumns);
		const int32 Col = i % FMath::Max(1, NumColumns);
		if (UUniformGridSlot* GridSlot = GridInventory->AddChildToUniformGrid(W, Row, Col))
		{
			// 필요 시 Padding/Alignment 설정
		}
		SlotWidgets.Add(W);
	}
}

void UInventoryWidget::SetupEquipmentWidgets()
{
	if (!InventoryComp) return;

	auto SetupIf = [&](UEquipmentSlotWidget* W, EEquipSlot Type)
		{
			if (W) W->Setup(InventoryComp, Type);
		};

	SetupIf(Eq_Head, EEquipSlot::Head);
	SetupIf(Eq_Chest, EEquipSlot::Chest);
	SetupIf(Eq_Weapon, EEquipSlot::Weapon);
	SetupIf(Eq_Legs, EEquipSlot::Legs);
	// 추가 부위 설정…
}

void UInventoryWidget::RefreshAll()
{
	// 인벤토리
	for (const TObjectPtr<UInventorySlotWidget>& W : SlotWidgets)
	{
		if (W) W->Refresh();
	}
	// 장비
	SetupEquipmentWidgets(); // 간단히 재셋업하여 Refresh
}

void UInventoryWidget::HandleInventoryChanged()
{
	// 변경된 슬롯만 갱신하고 싶다면 컴포넌트에서 인덱스를 브로드캐스트 하도록 확장
	for (const TObjectPtr<UInventorySlotWidget>& W : SlotWidgets)
	{
		if (W) W->Refresh();
	}
}

void UInventoryWidget::HandleEquipmentChanged()
{
	SetupEquipmentWidgets();
}
