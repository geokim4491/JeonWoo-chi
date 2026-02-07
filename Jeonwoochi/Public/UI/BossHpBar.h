// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHpBar.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UBossHpBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "ProgressBar", meta = (BindWidget))
	class UProgressBar* BossHpBar;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* TXT_BossName;

	UPROPERTY(EditAnywhere, Category = "ProgressBar", meta = (BindWidget))
	class UProgressBar* PostureBar;
};
