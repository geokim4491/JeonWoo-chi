// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SlidingTipWidget.h"

#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "TimerManager.h"

void USlidingTipWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 포커스/입력 막지 않도록
	SetIsFocusable(false);

	// 스타일 1회 적용
	ApplyStyle();

	// 시작은 투명 X — 슬라이드는 RenderTranslation으로 처리하므로 불투명 유지
	SetRenderOpacity(1.f);
}

void USlidingTipWidget::NativeDestruct()
{
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(TickHandle);
	}
	Super::NativeDestruct();
}

void USlidingTipWidget::ApplyStyle()
{
	if (TipText)
	{
		TipText->SetColorAndOpacity(TextColor);
		FSlateFontInfo FI = TipText->Font;
		FI.Size = FontSize;
		TipText->SetFont(FI);
	}
}

void USlidingTipWidget::PlaceAtEdge()
{
	// DPI 반영된 뷰포트 크기
	FVector2D ViewSize(1920.f, 1080.f);
	float DPIScale = 1.f;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewSize);
		DPIScale = UWidgetLayoutLibrary::GetViewportScale(this);
	}

	const FVector2D Effective = ViewSize / FMath::Max(0.001f, DPIScale);

	// 앵커: 좌측이면 (0,0.5), 우측이면 (1,0.5) 정렬
	const bool bLeft = (Side == ETipSlideSide::Left);
	SetAlignmentInViewport(FVector2D(bLeft ? 0.f : 1.f, 0.5f));

	// 보이는 기준 위치(정지 위치)
	const float Y = FMath::Clamp(NormalizedY, 0.f, 1.f) * Effective.Y;
	const float X = bLeft ? EdgePadding : (Effective.X - EdgePadding);

	VisiblePos = FVector2D(X, Y);
	SetPositionInViewport(VisiblePos, false); // false: DPI 스케일 고려된 좌표로 사용

	// 슬라이드 이동량: 좌측이면 -X 방향, 우측이면 +X 방향
	const float Dir = bLeft ? -1.f : +1.f;
	StartTrans = FVector2D(Dir * SlideDistance, 0.f);
	OutTrans = FVector2D(Dir * SlideDistance, 0.f);

	// 시작 시 화면 밖(translation 적용)으로 빼두기
	SetRenderTranslation(StartTrans);
}

float USlidingTipWidget::EaseInOut(float T) const
{
	// 부드러운 가감속 (큐빅)
	T = FMath::Clamp(T, 0.f, 1.f);
	return (T < 0.5f)
		? 4.f * T * T * T
		: 1.f - FMath::Pow(-2.f * T + 2.f, 3.f) / 2.f;
}

void USlidingTipWidget::ShowTip(const FText& InText, ETipSlideSide InSide, float InSlideIn, float InHold, float InSlideOut)
{
	Side = InSide;
	SlideInDur = FMath::Max(0.f, InSlideIn);
	HoldDur = FMath::Max(0.f, InHold);
	SlideOutDur = FMath::Max(0.f, InSlideOut);
	Elapsed = 0.f;

	if (TipText)
	{
		TipText->SetText(InText);
	}

	// 위치 결정 & 오프스크린에서 대기
	PlaceAtEdge();

	// 사운드(옵션)
	if (AppearSound)
	{
		UGameplayStatics::PlaySound2D(this, AppearSound);
	}

	Phase = (SlideInDur > 0.f) ? EPhase::SlideIn : (HoldDur > 0.f ? EPhase::Hold : EPhase::SlideOut);

	// 60Hz 타이머로 틱
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(TickHandle);
		W->GetTimerManager().SetTimer(TickHandle, this, &USlidingTipWidget::TickAnim, 0.016f, true);
	}

	// 즉시 1프레임 업데이트
	TickAnim();
}

void USlidingTipWidget::TickAnim()
{
	UWorld* W = GetWorld();
	if (!W) return;

	const float DT = W->GetDeltaSeconds();
	Elapsed += DT;

	switch (Phase)
	{
	case EPhase::SlideIn:
	{
		const float T = (SlideInDur > 0.f) ? EaseInOut(Elapsed / SlideInDur) : 1.f;
		// 슬라이드 인: StartTrans -> 0
		const FVector2D Cur = FMath::Lerp(StartTrans, FVector2D::ZeroVector, T);
		SetRenderTranslation(Cur);

		if (Elapsed >= SlideInDur)
		{
			Phase = (HoldDur > 0.f) ? EPhase::Hold : EPhase::SlideOut;
			Elapsed = 0.f;
			// 완전히 정위치
			SetRenderTranslation(FVector2D::ZeroVector);
		}
		break;
	}
	case EPhase::Hold:
	{
		// 제자리 유지
		SetRenderTranslation(FVector2D::ZeroVector);
		if (Elapsed >= HoldDur)
		{
			Phase = EPhase::SlideOut;
			Elapsed = 0.f;
		}
		break;
	}
	case EPhase::SlideOut:
	{
		const float T = (SlideOutDur > 0.f) ? EaseInOut(Elapsed / SlideOutDur) : 1.f;
		// 슬라이드 아웃: 0 -> OutTrans (인과 반대 방향으로 나감)
		const FVector2D Cur = FMath::Lerp(FVector2D::ZeroVector, OutTrans, T);
		SetRenderTranslation(Cur);

		// 사운드(옵션)
		if (DisappearSound && !bIsPlayedSound)
		{
			bIsPlayedSound = true;
			UGameplayStatics::PlaySound2D(this, DisappearSound);
		}

		if (Elapsed >= SlideOutDur)
		{
			W->GetTimerManager().ClearTimer(TickHandle);
			Phase = EPhase::None;
			OnTipFinished.Broadcast();
			RemoveFromParent();
		}
		break;
	}
	default:
		break;
	}
}