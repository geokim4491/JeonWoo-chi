// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/CharacterTypeEnum.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UCanvasPanel* MainCanvas;

	UPROPERTY(EditAnywhere, Category = "ProgressBar", meta = (BindWidget))
	class UProgressBar* PlayerHpBar;

	UPROPERTY(EditAnywhere, Category = "ProgressBar", meta = (BindWidget))
	class UProgressBar* PlayerChiBar;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UBossHpBar* BossHpBarWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UInvalidationBox* Status;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UInvalidationBox* Target;

	UPROPERTY(EditAnywhere, meta = (BindWidget)) 
	class UProgressBar* PlayerHpTargetBar;

	UPROPERTY(EditAnywhere, meta = (BindWidget)) 
	class UProgressBar* PlayerChiTargetBar;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	class UWidgetAnimation* InAnimation;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	class UWidgetAnimation* OutAnimation;

	bool bCombatUIVisible = false;

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void UpdateHealth(const float MaxHealth, const float CurrentHealth);

	UFUNCTION(BlueprintCallable)
	void UpdateChi(const float MaxChi, const float CurrentChi);

	UFUNCTION(BlueprintCallable)
	void ShowBossBar(const FText Name, float MaxHp, float CurrentHp, float MaxPosture, float CurrentPosture);

	UFUNCTION(BlueprintCallable)
	void HideBossBar();

	UFUNCTION(BlueprintCallable)
	void UpdateBossHealth(AActor* DummyActor, ECharacterType DummyCharacterType, float CurrentHp, float MaxHp);

	UFUNCTION(BlueprintCallable)
	void UpdateBossPosture(float CurPosture, float MaxPosture);

	// 델리게이트 진입점
	UFUNCTION(BlueprintCallable) 
	void HandleCombatStarted(); // 인
	UFUNCTION(BlueprintCallable) 
	void HandleCombatEnded();   // 아웃

private:
	float HpShown = 1.f, HpTarget = 1.f;
	float ChiShown = 1.f, ChiTarget = 1.f;

	bool  bHpAnimating = false;
	bool  bChiAnimating = false;

	UPROPERTY(EditAnywhere, Category = "ProgressBar|Smoothing")
	float HpInterpSpeed = 2.f;

	UPROPERTY(EditAnywhere, Category = "ProgressBar|Smoothing")
	float ChiInterpSpeed = 2.f;

	UPROPERTY(EditAnywhere, Category = "ProgressBar|Smoothing")
	float StopTolerance = 0.002f;

	UFUNCTION() void OnOutFinished();
};
