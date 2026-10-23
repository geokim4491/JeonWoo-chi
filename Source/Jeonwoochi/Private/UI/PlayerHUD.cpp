// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerHUD.h"
#include "Components/ProgressBar.h"
#include "UI/BossHpBar.h"
#include"Components/InvalidationBox.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"

void UPlayerHUD::NativeConstruct()
{
    Super::NativeConstruct();
    HpShown = HpTarget = 1.f;
    ChiShown = ChiTarget = 1.f;
    if (PlayerHpBar)  PlayerHpBar->SetPercent(HpShown);
    if (PlayerChiBar) PlayerChiBar->SetPercent(ChiShown);
    if (PlayerHpTargetBar)  PlayerHpTargetBar->SetPercent(HpTarget);
    if (PlayerChiTargetBar) PlayerChiTargetBar->SetPercent(ChiTarget);
    if (Status) Status->SetCanCache(true); // 기본은 켜두기

    if (MainCanvas) MainCanvas->SetRenderOpacity(0.f);
    SetVisibility(ESlateVisibility::HitTestInvisible);

    if (OutAnimation)
    {
        FWidgetAnimationDynamicEvent OnEnded;
        OnEnded.BindDynamic(this, &UPlayerHUD::OnOutFinished); // 콜백 바인딩
        BindToAnimationFinished(OutAnimation, OnEnded);         // 애니 끝 이벤트 연결
    }

    if (BossHpBarWidget) HideBossBar();
}

void UPlayerHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    bool bAnyAnimating = false;

    if (bHpAnimating && PlayerHpBar)
    {
        HpShown = FMath::FInterpTo(HpShown, HpTarget, InDeltaTime, HpInterpSpeed);
        PlayerHpBar->SetPercent(HpShown);

        if (FMath::IsNearlyEqual(HpShown, HpTarget, StopTolerance))
        {
            HpShown = HpTarget;
            PlayerHpBar->SetPercent(HpShown);
            bHpAnimating = false;
        }
    }

    if (bChiAnimating && PlayerChiBar)
    {
        ChiShown = FMath::FInterpTo(ChiShown, ChiTarget, InDeltaTime, ChiInterpSpeed);
        PlayerChiBar->SetPercent(ChiShown);

        if (FMath::IsNearlyEqual(ChiShown, ChiTarget, StopTolerance))
        {
            ChiShown = ChiTarget;
            PlayerChiBar->SetPercent(ChiShown);
            bChiAnimating = false;
        }
    }

    bAnyAnimating = bHpAnimating || bChiAnimating;

    // 애니가 완전히 끝났을 때만 다시 캐시 ON
    if (!bAnyAnimating && Status && !Status->GetCanCache())
    {
        Status->SetCanCache(true);
    }
}


void UPlayerHUD::UpdateHealth(const float MaxHealth, const float CurrentHealth)
{
    if (!PlayerHpBar) return;

    HpTarget = (MaxHealth > 0.f) ? FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f) : 0.f;

    // 타겟 바는 즉시 최종 도착선으로 스냅
    if (PlayerHpTargetBar) PlayerHpTargetBar->SetPercent(HpTarget);

    // 보간 시작 + 캐시 OFF
    if (!FMath::IsNearlyEqual(HpShown, HpTarget, StopTolerance))
    {
        bHpAnimating = true;
        if (Status) { Status->SetCanCache(false); }
    }
    // 기존처럼 여기서 PlayerHpBar->SetPercent()는 호출하지 않음. (보간은 Tick)
}

void UPlayerHUD::UpdateChi(const float MaxChi, const float CurrentChi)
{
    if (!PlayerChiBar) return;

    ChiTarget = (MaxChi > 0.f) ? FMath::Clamp(CurrentChi / MaxChi, 0.f, 1.f) : 0.f;

    if (PlayerChiTargetBar) PlayerChiTargetBar->SetPercent(ChiTarget);

    if (!FMath::IsNearlyEqual(ChiShown, ChiTarget, StopTolerance))
    {
        bChiAnimating = true;
        if (Status) { Status->SetCanCache(false); }
    }
}

void UPlayerHUD::ShowBossBar(const FText Name, float MaxHp, float CurrentHp, float MaxPosture, float CurrentPosture)
{
    if (BossHpBarWidget)
    {
        BossHpBarWidget->TXT_BossName->SetText(Name);
        BossHpBarWidget->BossHpBar->SetPercent(CurrentHp / MaxHp);
        BossHpBarWidget->PostureBar->SetPercent(CurrentPosture / MaxPosture);
        BossHpBarWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void UPlayerHUD::HideBossBar()
{
    if (BossHpBarWidget)
    {
        BossHpBarWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPlayerHUD::UpdateBossHealth(AActor* DummyActor, ECharacterType DummyCharacterType, float CurrentHp, float MaxHp)
{
    if (BossHpBarWidget)
    {
        BossHpBarWidget->BossHpBar->SetPercent(CurrentHp / MaxHp);
    }
}

void UPlayerHUD::UpdateBossPosture(float CurPosture, float MaxPosture)
{
    if (BossHpBarWidget)
    {
        BossHpBarWidget->PostureBar->SetPercent(float(CurPosture / MaxPosture));
    }
}

void UPlayerHUD::HandleCombatStarted()
{
    if (bCombatUIVisible) return;
    bCombatUIVisible = true;

    // 애니 동안 캐시 비활성화
    if (Status) { Status->SetCanCache(false);  }
    if (Target) { Target->SetCanCache(false); }

    SetVisibility(ESlateVisibility::HitTestInvisible);
    if(MainCanvas) MainCanvas->SetRenderOpacity(1.f);

    if (InAnimation)
    {
        PlayAnimation(InAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
    }
    else
    {
        // 애니 없을 때 최소 동작
        if (MainCanvas) MainCanvas->SetRenderOpacity(1.f);
    }

    // 인 애니가 끝나면 캐시를 켜도 무방 (프레임 아끼기)
    if (Status) Status->SetCanCache(true);
    if (Target) Target->SetCanCache(true);
}

void UPlayerHUD::HandleCombatEnded()
{
    if (!bCombatUIVisible) return;
    bCombatUIVisible = false;

    if (Status) { Status->SetCanCache(false); }
    if (Target) { Target->SetCanCache(false); }

    if (OutAnimation)
    {
        PlayAnimation(OutAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
    }
    else
    {
        if (MainCanvas) MainCanvas->SetRenderOpacity(0.f);
        OnOutFinished();
    }
}

void UPlayerHUD::OnOutFinished()
{
    SetVisibility(ESlateVisibility::Collapsed);

    if (Status) { Status->SetCanCache(true); }
    if (Target) { Target->SetCanCache(true); }
}