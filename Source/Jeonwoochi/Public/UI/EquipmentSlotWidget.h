// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/Inventory/ItemData.h"
#include "Components/Utilities/PlayerInventoryComponent.h"
#include "EquipmentSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	EEquipSlot SlotType = EEquipSlot::None;

	UPROPERTY(meta = (BindWidget)) class UImage* IMG_Icon = nullptr;
	UPROPERTY(meta = (BindWidget)) class UButton* BTN_Click = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UPlayerInventoryComponent> InventoryComp = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void Setup(UPlayerInventoryComponent* InComp, EEquipSlot InSlot);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void Refresh();

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeo, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeo, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	UFUNCTION() void OnClicked();
	void SetVisual(const FInventorySlot& Data);
};
