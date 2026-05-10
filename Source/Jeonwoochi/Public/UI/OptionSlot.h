// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionSlot.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UOptionSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* Slider;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BTN_Left;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BTN_Right;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnLeftClicked();

	UFUNCTION()
	void OnRightClicked();

};
