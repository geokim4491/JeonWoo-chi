// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "LevelIntroWidget.generated.h"

class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIntroFinished);

/**
 * 중앙에 텍스트를 크게 띄우고, 페이드 인→홀드→페이드 아웃 수행 
 */

UCLASS()
class JEONWOOCHI_API ULevelIntroWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** 에디터에서 만든 위젯 BP에서 TextBlock를 TitleText라는 이름으로 바인딩하면 자동 연결됨 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TitleText;

	/** 전부 C++로 만들고 싶으면 이 값으로 텍스트 설정 가능 (TitleText 없을 때 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro")
	FText FallbackTitle;

	/** 끝났을 때 알림 델리게이트 (필요시 바인딩) */
	UPROPERTY(BlueprintAssignable, Category = "Intro")
	FOnIntroFinished OnIntroFinished;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Style")
	FLinearColor TitleColor = FLinearColor::White;

	/** 제목 폰트 사이즈(디자인 기준 px, DPI/ScaleBox에 의해 최종 스케일됨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro|Style", meta = (ClampMin = "1", UIMin = "8", UIMax = "300"))
	int32 TitleFontSize = 80;

	/** 런타임 폰트 사이즈 변경 */
	UFUNCTION(BlueprintCallable, Category = "Intro|Style")
	void SetTitleFontSize(int32 InSize);

	/** 런타임에 글자 색 바꾸기 */
	UFUNCTION(BlueprintCallable, Category = "Intro|Style")
	void SetTitleColor(const FLinearColor& InColor);

	/** 표시 시작 (맵 이름/지속시간 인자로 제어) */
	UFUNCTION(BlueprintCallable, Category = "Intro")
	void ShowIntro(const FText& InTitle, float InFadeIn, float InHold, float InFadeOut);

protected:
	virtual void NativeConstruct() override;

private:
	enum class EPhase : uint8 { None, FadeIn, Hold, FadeOut };

	void TickFade();

	EPhase Phase = EPhase::None;
	float FadeInDur = 0.f;
	float HoldDur = 0.f;
	float FadeOutDur = 0.f;
	float Elapsed = 0.f;

	FTimerHandle TimerHandle;
};
