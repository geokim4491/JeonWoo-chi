// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FadeInHoldOutWidget.generated.h"

class UTextBlock;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFelledFinished);

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UFadeInHoldOutWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** BP에서 TextBlock 이름을 TitleText로 만들면 자동 바인딩 됩니다. */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TitleText;

	/** 기본 문구 (코드에서 넘기지 않으면 이 값 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Felled|Text")
	FText DefaultText = FText::FromString(TEXT("GREAT ENEMY FELLED"));

	/** 스타일 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Felled|Style")
	FLinearColor TitleColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Felled|Style", meta = (ClampMin = "8", UIMin = "24", UIMax = "300"))
	int32 TitleFontSize = 84;

	/** 사운드 (시작/끝 타이밍 1회 재생, 필요 시 지연) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Felled|Sound") USoundBase* AppearSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Felled|Sound") USoundBase* DisappearSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Felled|Sound", meta = (ClampMin = "0")) float AppearSoundDelay = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Felled|Sound", meta = (ClampMin = "0")) float DisappearSoundDelay = 0.f;

	/** 반응형 중앙 배치용 디자인 기준 해상도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Felled|Layout")
	FVector2D DesignResolution = FVector2D(1920.f, 1080.f);

	/** 끝났을 때 브로드캐스트(필요한 경우 사용) */
	UPROPERTY(BlueprintAssignable, Category = "Felled")
	FOnFelledFinished OnFelledFinished;

	/** 실행: FadeIn/Hold/FadeOut 시간과 텍스트를 넘겨주세요. (텍스트 비우면 DefaultText 사용) */
	UFUNCTION(BlueprintCallable, Category = "Felled")
	void PlayFelled(const FText& InText, float InFadeIn, float InHold, float InFadeOut);

	/** 런타임 스타일 변경(선택) */
	UFUNCTION(BlueprintCallable, Category = "Felled|Style") void SetTitleColor(FLinearColor C);
	UFUNCTION(BlueprintCallable, Category = "Felled|Style") void SetTitleFontSize(int32 Size);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	enum class EPhase : uint8 { None, FadeIn, Hold, FadeOut };

	void ApplyStyle();
	void ApplyViewportCenterScale();
	void TickFade();
	static float EaseInOut(float T);

	// 상태
	EPhase Phase = EPhase::None;
	float FadeInDur = 0.f;
	float HoldDur = 0.f;
	float FadeOutDur = 0.f;
	float Elapsed = 0.f;

	bool bPlayedAppearSfx = false;
	bool bPlayedDisappearSfx = false;

	FTimerHandle TickHandle;
};
