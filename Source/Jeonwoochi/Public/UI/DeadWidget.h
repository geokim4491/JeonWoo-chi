// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeadWidget.generated.h"

class UMenuButton;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UDeadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UMenuButton* BTN_Restart;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UMenuButton* BTN_QuitStart;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UMenuButton* BTN_Quit;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnRestartButtonClicked();

	UFUNCTION()
	void OnQuitStartButtonClicked();

	UFUNCTION()
	void OnQuitButtonClicked();
};
