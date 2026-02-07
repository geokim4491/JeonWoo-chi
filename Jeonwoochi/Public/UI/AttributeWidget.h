// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeWidget.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UAttributeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BTN_Right;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BTN_Left;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonClicked, UAttributeWidget*, Sender);
	UPROPERTY(BlueprintAssignable)
	FOnButtonClicked OnRightClicked;

	UPROPERTY(BlueprintAssignable)
	FOnButtonClicked OnLeftClicked;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TXT_AttrName;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TXT_BaseAttrVal;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TXT_PendingAttrVal;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	UFUNCTION() void HandleRightClicked();
	UFUNCTION() void HandleLeftClicked();
};
