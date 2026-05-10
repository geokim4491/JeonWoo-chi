// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/FadeInHoldOutWidget.h"
#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UFadeInHoldOutWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetRenderOpacity(0.f);      // 시작은 투명
	ApplyStyle();
	ApplyViewportCenterScale(); // 중앙 배치 + DPI 대응
}

void UFadeInHoldOutWidget::NativeDestruct()
{
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(TickHandle);
	}
	Super::NativeDestruct();
}

void UFadeInHoldOutWidget::ApplyStyle()
{
	if (TitleText)
	{
		TitleText->SetColorAndOpacity(TitleColor);
		FSlateFontInfo FI = TitleText->Font;
		FI.Size = TitleFontSize;
		TitleText->SetFont(FI);
		// 줄바꿈(엔터) 지원: 코드에서 "\n" 포함한 FText를 넘기면 그대로 개행됨
	}
}

void UFadeInHoldOutWidget::ApplyViewportCenterScale()
{
	FVector2D ViewportSize(1920.f, 1080.f);
	float DPIScale = 1.f;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		DPIScale = UWidgetLayoutLibrary::GetViewportScale(this);
	}

	const FVector2D Effective = ViewportSize / FMath::Max(0.001f, DPIScale);
	const float Scale = FMath::Min(
		Effective.X / FMath::Max(1.f, DesignResolution.X),
		Effective.Y / FMath::Max(1.f, DesignResolution.Y)
	);

	SetDesiredSizeInViewport(DesignResolution);
	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	SetPositionInViewport(Effective * 0.5f, false);
	SetRenderScale(FVector2D(Scale, Scale));
}

float UFadeInHoldOutWidget::EaseInOut(float T)
{
	T = FMath::Clamp(T, 0.f, 1.f);
	return (T < 0.5f) ? 4.f * T * T * T : 1.f - FMath::Pow(-2.f * T + 2.f, 3.f) / 2.f;
}

void UFadeInHoldOutWidget::PlayFelled(const FText& InText, float InFadeIn, float InHold, float InFadeOut)
{
	if (TitleText)
	{
		TitleText->SetText(InText.IsEmpty() ? DefaultText : InText);
	}

	FadeInDur = FMath::Max(0.f, InFadeIn);
	HoldDur = FMath::Max(0.f, InHold);
	FadeOutDur = FMath::Max(0.f, InFadeOut);
	Elapsed = 0.f;
	bPlayedAppearSfx = false;
	bPlayedDisappearSfx = false;

	ApplyStyle();
	ApplyViewportCenterScale();

	Phase = (FadeInDur > 0.f) ? EPhase::FadeIn : (HoldDur > 0.f ? EPhase::Hold : EPhase::FadeOut);

	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(TickHandle);
		W->GetTimerManager().SetTimer(TickHandle, this, &UFadeInHoldOutWidget::TickFade, 0.016f, true);
	}
	TickFade();
}

void UFadeInHoldOutWidget::TickFade()
{
	if (UWorld* W = GetWorld())
	{
		const float DT = W->GetDeltaSeconds();
		Elapsed += DT;

		switch (Phase)
		{
		case EPhase::FadeIn:
		{
			if (!bPlayedAppearSfx && AppearSound && Elapsed >= AppearSoundDelay)
			{
				UGameplayStatics::PlaySound2D(this, AppearSound);
				bPlayedAppearSfx = true;
			}
			const float T = (FadeInDur > 0.f) ? EaseInOut(Elapsed / FadeInDur) : 1.f;
			SetRenderOpacity(T);

			if (Elapsed >= FadeInDur)
			{
				Phase = (HoldDur > 0.f) ? EPhase::Hold : EPhase::FadeOut;
				Elapsed = 0.f;
				SetRenderOpacity(1.f);
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
			if (!bPlayedDisappearSfx && DisappearSound && Elapsed >= DisappearSoundDelay)
			{
				UGameplayStatics::PlaySound2D(this, DisappearSound);
				bPlayedDisappearSfx = true;
			}
			const float T = (FadeOutDur > 0.f) ? EaseInOut(Elapsed / FadeOutDur) : 1.f;
			SetRenderOpacity(1.f - T);

			if (Elapsed >= FadeOutDur)
			{
				W->GetTimerManager().ClearTimer(TickHandle);
				Phase = EPhase::None;
				OnFelledFinished.Broadcast();
				RemoveFromParent();
			}
			break;
		}
		default: break;
		}
	}
}

void UFadeInHoldOutWidget::SetTitleColor(FLinearColor C) { TitleColor = C; ApplyStyle(); }
void UFadeInHoldOutWidget::SetTitleFontSize(int32 Size) { TitleFontSize = FMath::Max(8, Size); ApplyStyle(); }