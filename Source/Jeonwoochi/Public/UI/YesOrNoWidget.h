// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YesOrNoWidget.generated.h"

class UMenuButton;
/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UYesOrNoWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* BTN_Yes;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* BTN_No;
};
