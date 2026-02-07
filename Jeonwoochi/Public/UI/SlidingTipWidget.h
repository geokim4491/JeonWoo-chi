// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlidingTipWidget.generated.h"

class UTextBlock;
class USoundBase;

UENUM(BlueprintType)
enum class ETipSlideSide : uint8
{
	Left,
	Right
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTipFinished);

/**
 * 화면 좌/우에서 슬라이드 인 → 홀드 → 반대방향 슬라이드 아웃
 */
UCLASS()
class JEONWOOCHI_API USlidingTipWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 바인딩: 위젯 BP에서 TextBlock의 이름을 TipText로 두면 자동 연결
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TipText;

	/** 텍스트/스타일(볼륨 등 외부에서 세팅 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tip|Style")
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tip|Style", meta = (ClampMin = "8", UIMin = "12", UIMax = "200"))
	int32 FontSize = 28;

	/** 레이아웃 옵션 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tip|Layout", meta = (ClampMin = "0"))
	float EdgePadding = 24.f;        // 화면 모서리로부터 여백(px, 디자인 기준)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tip|Layout", meta = (ClampMin = "0"))
	float SlideDistance = 350.f;     // 슬라이드 이동 거리(px)

	/** 0.0=화면 최상단, 1.0=최하단 (중앙=0.5). DPI 고려 후 비율로 위치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tip|Layout", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NormalizedY = 0.2f;

	/** 사운드(옵션) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tip|Sound")
	USoundBase* AppearSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tip|Sound")
	USoundBase* DisappearSound = nullptr;

	/** 종료 알림 */
	UPROPERTY(BlueprintAssignable, Category = "Tip")
	FOnTipFinished OnTipFinished;

	/** 보여주기: 슬라이드 인/홀드/아웃 시간 지정 */
	UFUNCTION(BlueprintCallable, Category = "Tip")
	void ShowTip(const FText& InText, ETipSlideSide InSide, float InSlideIn, float InHold, float InSlideOut);

	/** 런타임 스타일 변경 */
	UFUNCTION(BlueprintCallable, Category = "Tip|Style")
	void SetTextColor(const FLinearColor& InColor) { TextColor = InColor; ApplyStyle(); }

	UFUNCTION(BlueprintCallable, Category = "Tip|Style")
	void SetFontSize(int32 InSize) { FontSize = FMath::Max(8, InSize); ApplyStyle(); }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	enum class EPhase : uint8 { None, SlideIn, Hold, SlideOut };

	void ApplyStyle();
	void PlaceAtEdge();     // 화면 크기/DPI 반영해서 ‘보이는 위치’ 고정
	void TickAnim();        // 60Hz 타이머로 애니메이션
	float EaseInOut(float T) const; // 부드러운 가감속

	// 상태
	EPhase Phase = EPhase::None;
	ETipSlideSide Side = ETipSlideSide::Left;

	float SlideInDur = 0.f;
	float HoldDur = 0.f;
	float SlideOutDur = 0.f;
	float Elapsed = 0.f;

	// 위치/이동
	FVector2D VisiblePos = FVector2D::ZeroVector;   // 화면에서 최종 보이는 좌표
	FVector2D StartTrans = FVector2D::ZeroVector;   // 슬라이드 인 시작 시 RenderTranslation
	FVector2D OutTrans = FVector2D::ZeroVector;   // 슬라이드 아웃 목표 Translation

	FTimerHandle TickHandle;

	bool bIsPlayedSound = false;
};
