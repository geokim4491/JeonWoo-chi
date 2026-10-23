// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartMenu.generated.h"

class UMenuButton;
class UYesOrNoWidget;

/**
 * 
 */

UCLASS()
class JEONWOOCHI_API UStartMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UMenuButton* BTN_Play;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UMenuButton* BTN_Load;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UMenuButton* BTN_Quit;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UMenuButton* BTN_Option;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UYesOrNoWidget* YesOrNo;

protected:
	// BP에서 WBP_SaveWidget 지정
	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<class USaveWidget> SaveWidgetClass;

	// 재사용용 캐시
	UPROPERTY() class USaveWidget* SaveWidgetInst = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<class UOptionMenu> OptionWidgetClass;

	UPROPERTY() class UOptionMenu* OptionWidget = nullptr;
private:
	UFUNCTION()
	void OnPlayButtonClicked();

	UFUNCTION()
	void OnLoadButtonClicked();

	UFUNCTION()
	void OnQuitButtonClicked();

	UFUNCTION()
	void OnOptionButtonClicked();

	UFUNCTION()
	void OnYesClicked();

	UFUNCTION()
	void OnNoClicked();

	UFUNCTION()
	void BackToStart();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};