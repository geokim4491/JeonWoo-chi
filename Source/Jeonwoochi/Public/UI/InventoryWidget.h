// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UPlayerInventoryComponent;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 디자이너에서 그리드/장비 슬롯들 바인딩
	UPROPERTY(meta = (BindWidget)) class UUniformGridPanel* GridInventory = nullptr;

	UPROPERTY(meta = (BindWidget)) class UEquipmentSlotWidget* Eq_Head = nullptr;
	UPROPERTY(meta = (BindWidget)) UEquipmentSlotWidget* Eq_Chest = nullptr;
	UPROPERTY(meta = (BindWidget)) UEquipmentSlotWidget* Eq_Weapon = nullptr;
	UPROPERTY(meta = (BindWidget)) UEquipmentSlotWidget* Eq_Legs = nullptr;
	// 필요한 만큼 더…

	// 외부에서 주입
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UPlayerInventoryComponent> InventoryComp = nullptr;

	// 슬롯 생성에 사용할 위젯 클래스 (BP로 지정 권장)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<class UInventorySlotWidget> InventorySlotClass;

	// 그리드 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 NumColumns = 6;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeWithComponent(UPlayerInventoryComponent* InComp);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RebuildGrid();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshAll();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY() TArray<TObjectPtr<UInventorySlotWidget>> SlotWidgets;

	UFUNCTION() void HandleInventoryChanged();
	UFUNCTION() void HandleEquipmentChanged();

	void SetupEquipmentWidgets();
};
