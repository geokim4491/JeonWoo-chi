// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AIHpBar.h"
#include "Components/WidgetComponent.h"
#include "Components/Combat/HealthComponent.h"
#include "Components/Combat/PostureComponent.h"
#include "Components/ProgressBar.h"
#include "Animation/WidgetAnimation.h"

void UAIHpBar::NativeConstruct()
{
    Super::NativeConstruct();
    UWidgetComponent* WidgetComp = Cast<UWidgetComponent>(GetOuter());
    if (!WidgetComp) return;

    UHealthComponent* HealthComp = WidgetComp->GetOwner()->FindComponentByClass<UHealthComponent>();
    if (!HealthComp) return;

    HealthComp->OnHealthChanged.AddUObject(this, &UAIHpBar::UpdateHp);

    UPostureComponent* PostureComp = WidgetComp->GetOwner()->FindComponentByClass<UPostureComponent>();
    if (!PostureComp) return;

    PostureComp->OnPostureUpdated.AddUObject(this, &UAIHpBar::UpdatePosture);
}

void UAIHpBar::NativeDestruct()
{
    UWidgetComponent* WidgetComp = Cast<UWidgetComponent>(GetOuter());
    if (!WidgetComp) return;

    UHealthComponent* HealthComp = WidgetComp->GetOwner()->FindComponentByClass<UHealthComponent>();
    if (!HealthComp) return;

    HealthComp->OnHealthChanged.RemoveAll(this);

    UPostureComponent* PostureComp = WidgetComp->GetOwner()->FindComponentByClass<UPostureComponent>();
    if (!PostureComp) return;

    PostureComp->OnPostureUpdated.RemoveAll(this);

    Super::NativeDestruct();
}

void UAIHpBar::UpdateHp(AActor* DummyActor, ECharacterType DummyCharacterType, float CurHP, float MaxHP)
{
	TargetHP = (MaxHP > 0.f) ? FMath::Clamp(CurHP / MaxHP, 0.f, 1.f) : 0.f;
}

void UAIHpBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    CurrentHP = FMath::FInterpTo(CurrentHP, TargetHP, InDeltaTime, InterpSpeed);
    CurrentPosture = FMath::FInterpTo(CurrentPosture, TargetPosture, InDeltaTime, InterpSpeed);

    if (HpBar)
    {
        HpBar->SetPercent(CurrentHP);
    }

    if (PostureBar)
    {
        PostureBar->SetPercent(CurrentPosture);
    }
}

void UAIHpBar::UpdatePosture(float CurPosture, float MaxPosture)
{
    TargetPosture = (MaxPosture > 0.f) ? FMath::Clamp(CurPosture / MaxPosture, 0.f, 1.f) : 0.f;
}

void UAIHpBar::PlayFadeOutAndHide(float Duration)
{
    if (!FadeOutAnim)
    {
        // 애니 없으면 그냥 숨김
        if (UWidgetComponent* WC = GetTypedOuter<UWidgetComponent>())
        {
            WC->SetVisibility(false);
            WC->SetHiddenInGame(true);
        }
        return;
    }

    // 재생 시간 맞추기: PlayAnimation은 '재생 속도'를 받음
    const float AnimLen = FadeOutAnim->GetEndTime();
    const float PlayRate = (AnimLen > 0.f && Duration > 0.f) ? (AnimLen / Duration) : 1.f;

    FWidgetAnimationDynamicEvent OnEnd;
    OnEnd.BindDynamic(this, &UAIHpBar::OnFadeOutFinished);
    BindToAnimationFinished(FadeOutAnim, OnEnd);

    PlayAnimation(FadeOutAnim, 0.f, 1, EUMGSequencePlayMode::Forward, PlayRate);
}

void UAIHpBar::OnFadeOutFinished()
{
    if (UWidgetComponent* WC = GetTypedOuter<UWidgetComponent>())
    {
        WC->SetVisibility(false);
        WC->SetHiddenInGame(true);
    }
    SetRenderOpacity(1.f); // 다음에 다시 쓸 수 있도록 복원
}