// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AIHpBar.generated.h"

class UProgressBar;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UAIHpBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* HpBar;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* PostureBar;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* FadeOutAnim = nullptr;

	UFUNCTION(BlueprintCallable)
	void PlayFadeOutAndHide(float Duration = 0.35f);

	UFUNCTION()
	void OnFadeOutFinished();

	UFUNCTION()
	void UpdateHp(AActor* DummyActor, ECharacterType DummyCharacterType, float CurHP, float MaxHP);

	UFUNCTION()
	void UpdatePosture(float CurPosture, float MaxPosture);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;
protected:
	float CurrentHP = 1.0f;  
	float TargetHP = 1.0f;
	float InterpSpeed = 3.0f;

	float CurrentPosture = 1.0f;
	float TargetPosture = 1.0f;
};
