// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionMenu.generated.h"

class UOptionSlot;
class UMenuButton;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UOptionMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UOptionSlot* Slot_Sound;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UMenuButton* BTN_Save;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UMenuButton* BTN_Back;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UMenuButton* BTN_Exit;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UMenuButton* BTN_Start;

	// 오디오 적용 대상 (콘텐츠 브라우저에서 할당)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Options|Audio")
	class USoundClass* MasterSoundClass = nullptr;

protected:
	virtual void NativeConstruct() override;
	UFUNCTION() void OnSaveClicked();

	UFUNCTION() void OnSoundSliderChanged(float Value);
	UFUNCTION() void OnBackClicked();
	UFUNCTION() void OnExitClicked();
	UFUNCTION() void OnStartClicked();

	void ApplySoundVolume(float Normalized);     // 0~1
};
