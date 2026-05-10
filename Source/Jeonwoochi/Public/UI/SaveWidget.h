// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveWidget.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API USaveWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION() void RefreshSlots();

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UScrollBox* SlotLists;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UMenuButton* BTN_Back;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnClicked_Load(const FString& GameName);

	UFUNCTION()
	void OnClicked_Back();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<class USaveButton> SaveButtonClass;


private:
	UPROPERTY() class USaveSubsystem* SaveSub;

};
