// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LevelIntroWidget.h"
#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "TimerManager.h"

void ULevelIntroWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// 시작은 완전 투명
	SetRenderOpacity(0.f);

	// 만약 BP에서 TextBlock 안 만들어뒀으면 FallbackTitle을 써줌
	if (!TitleText && !FallbackTitle.IsEmpty())
	{
		// 위젯 트리 없이 전부 C++로 만들려면 여기서 WidgetTree로 생성해도 됨.
	}

	if (TitleText)
	{
		TitleText->SetColorAndOpacity(FSlateColor(TitleColor));
		FSlateFontInfo FI = TitleText->Font;
		FI.Size = FMath::Max(1, TitleFontSize);
		TitleText->SetFont(FI);
	}
}

void ULevelIntroWidget::SetTitleColor(const FLinearColor& InColor)
{
	TitleColor = InColor;
	if (TitleText)
	{
		TitleText->SetColorAndOpacity(FSlateColor(TitleColor));
	}
}

void ULevelIntroWidget::SetTitleFontSize(int32 InSize)
{
	TitleFontSize = FMath::Max(1, InSize);
	if (TitleText)
	{
		FSlateFontInfo FI = TitleText->Font; // 기존 폰트 패밀리 유지
		FI.Size = TitleFontSize;
		TitleText->SetFont(FI);
	}
}

void ULevelIntroWidget::ShowIntro(const FText& InTitle, float InFadeIn, float InHold, float InFadeOut)
{
	if (TitleText)
	{
		TitleText->SetText(InTitle);
		TitleText->SetColorAndOpacity(FSlateColor(TitleColor));
		FSlateFontInfo FI = TitleText->Font;
		FI.Size = FMath::Max(1, TitleFontSize);
		TitleText->SetFont(FI);
	}
	else
	{
		FallbackTitle = InTitle;
	}

	FadeInDur = FMath::Max(0.f, InFadeIn);
	HoldDur = FMath::Max(0.f, InHold);
	FadeOutDur = FMath::Max(0.f, InFadeOut);

	Phase = (FadeInDur > 0.f) ? EPhase::FadeIn : (HoldDur > 0.f ? EPhase::Hold : EPhase::FadeOut);
	Elapsed = 0.f;

	// 60Hz 느낌의 타이머로 간단히 보간
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(TimerHandle);
		W->GetTimerManager().SetTimer(TimerHandle, this, &ULevelIntroWidget::TickFade, 0.016f, true);
	}

	// 즉시 갱신
	TickFade();
}

void ULevelIntroWidget::TickFade()
{
	UWorld* W = GetWorld();
	if (!W) return;

	const float DT = W->GetDeltaSeconds();
	Elapsed += DT;

	float Alpha = GetRenderOpacity();

	switch (Phase)
	{
	case EPhase::FadeIn:
	{
		const float T = (FadeInDur > 0.f) ? FMath::Clamp(Elapsed / FadeInDur, 0.f, 1.f) : 1.f;
		Alpha = T;
		SetRenderOpacity(Alpha);

		if (Elapsed >= FadeInDur)
		{
			Phase = (HoldDur > 0.f) ? EPhase::Hold : EPhase::FadeOut;
			Elapsed = 0.f;
		}
		break;
	}
	case EPhase::Hold:
	{
		SetRenderOpacity(1.f);
		if (Elapsed >= HoldDur)
		{
			Phase = EPhase::FadeOut;
			Elapsed = 0.f;
		}
		break;
	}
	case EPhase::FadeOut:
	{
		const float T = (FadeOutDur > 0.f) ? FMath::Clamp(Elapsed / FadeOutDur, 0.f, 1.f) : 1.f;
		Alpha = 1.f - T;
		SetRenderOpacity(Alpha);

		if (Elapsed >= FadeOutDur)
		{
			// 종료
			W->GetTimerManager().ClearTimer(TimerHandle);
			Phase = EPhase::None;
			OnIntroFinished.Broadcast();
			RemoveFromParent();
		}
		break;
	}
	default:
		break;
	}
}