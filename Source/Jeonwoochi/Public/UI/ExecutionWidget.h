// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ExecutionWidget.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UExecutionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void SetVisibility(ESlateVisibility InVisibility) override;

	UFUNCTION()
	void HandleCanExecution();

	UFUNCTION()
	void HandleCannotExecution();

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	class UWidgetAnimation* ExecutionAnimation;
};
