// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

class UMenuButton;
class UWidgetSwitcher;
class UStatWidget;

enum class EMenuPage : int32 { Stat = 0, Equipment, Inventory, Skill, Option };

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UMenuButton* BTN_Stat;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UMenuButton* BTN_Equipment;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UMenuButton* BTN_Inventory;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UMenuButton* BTN_Skill;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UMenuButton* BTN_Option;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* ContentSwitcher;

private:
	UFUNCTION()
	void OnStatButtonClicked();

	UFUNCTION()
	void OnEquipmentButtonClicked();

	UFUNCTION()
	void OnInventoryButtonClicked();

	UFUNCTION()
	void OnSkillButtonClicked();

	UFUNCTION()
	void OnOptionButtonClicked();
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
