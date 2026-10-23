// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/Inventory/ItemData.h"
#include "Components/Utilities/PlayerInventoryComponent.h"
#include "InventorySlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// 디자이너에서 바인드 (Icon, Count, ClickButton)
	UPROPERTY(meta = (BindWidget)) class UImage* IMG_Icon = nullptr;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* TXT_Count = nullptr;
	UPROPERTY(meta = (BindWidget)) class UButton* BTN_Click = nullptr;

	// 외부에서 주입
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UPlayerInventoryComponent> InventoryComp = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 SlotIndex = INDEX_NONE;

	// 설정/갱신
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void Setup(UPlayerInventoryComponent* InComp, int32 InIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void Refresh();

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeo, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeo, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeo, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	UFUNCTION() void OnClicked();

	FInventorySlot GetMySlot() const;
	void SetVisual(const FInventorySlot& Data);
};
