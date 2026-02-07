// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuButton.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UMenuButton : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BTN_Menu;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TXT_MenuName;

	UPROPERTY(EditAnywhere, Category = "Sound")
	class USoundBase* Sound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Name")
	FString MenuName;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuClicked);
	UPROPERTY(BlueprintAssignable) 
	FOnMenuClicked OnMenuClicked;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	UFUNCTION() void HandleClicked();
	void ApplyFocusAsHover(bool bEnable);

	bool bStyleCached = false;
	FButtonStyle CachedStyle;
};
