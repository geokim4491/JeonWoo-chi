// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/CombatModeComponent.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UCombatModeComponent::UCombatModeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


}

void UCombatModeComponent::SetLayer(ECombatLayer NewLayer)
{
    if (Layer == NewLayer) return;
    const ECombatLayer Old = Layer;
    Layer = NewLayer;
    OnCombatLayerChanged.Broadcast(Layer, Old);

    if (Layer == ECombatLayer::None)
    {
        OnCombatUIFade.Broadcast(false); // 전투 종료 → UI 페이드 아웃
    }
    else if (Old == ECombatLayer::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("UI Called"));
        OnCombatUIFade.Broadcast(true);   // 비전투→전투 진입 첫 순간에만 페이드 인
    }
}

void UCombatModeComponent::ResetCountdown()
{
    if (!GetWorld()) return;
    auto& TM = GetWorld()->GetTimerManager();
    TM.ClearTimer(CountdownHandle);
    const float Delay = FMath::Max(0.1f, TimeoutSeconds); // 즉시 만료 방지
    TM.SetTimer(CountdownHandle, this, &UCombatModeComponent::HandleTimeout, Delay, false);
}

void UCombatModeComponent::ClearCountdown()
{
    if (!GetWorld()) return;
    GetWorld()->GetTimerManager().ClearTimer(CountdownHandle);
}

void UCombatModeComponent::HandleTimeout()
{
    // 규칙:
    // - Soft: 10초 지나면 None
    // - Hard: "모든 적이 플레이어를 놓친 상태"에서 10초 타이머가 만료되면 None
    if (Layer == ECombatLayer::Soft)
    {
        SetLayer(ECombatLayer::None);
    }
    else if (Layer == ECombatLayer::Hard)
    {
        if (Aggressors.Num() == 0)
            SetLayer(ECombatLayer::None);
        else
            ResetCountdown(); // 방어적 (적이 아직 있으면 유지)
    }
}

void UCombatModeComponent::NotifyPlayerAction_Soft()
{
    // 공격/스킬/물약: UI만 빠르게 페이드 인 + Soft
    if (Layer == ECombatLayer::None)
    {
        SetLayer(ECombatLayer::Soft);
    }
    else if (Layer == ECombatLayer::Soft) 
    {
        /* 유지 */; 
    }
    else if (Layer == ECombatLayer::Hard)
    {
        /* Hard 유지 (UI 이미 켜져 있음) */;
    }

    ResetCountdown(); // 타이머만 항상 10초로 리셋
    OnCombatUIFade.Broadcast(true); // 액션 들어오면 항상 다시 켜달라고 핑 보내기
    UE_LOG(LogTemp, Warning, TEXT("Soft Combat Mode"));
}

void UCombatModeComponent::NotifyEnemyTargetAcquired(AActor* Enemy)
{
    if (Enemy) Aggressors.Add(Enemy);
    if (Layer != ECombatLayer::Hard)
    {
        SetLayer(ECombatLayer::Hard);
        OnCombatHardEntered.Broadcast(RegionTag); // 여기서 맵/레벨 태그로 OST 선택
    }
    ResetCountdown(); // 규칙: 소프트/하드 중 타이머가 돌고 있었다면 리셋
    UE_LOG(LogTemp, Warning, TEXT("Combat Mode!"));
}

void UCombatModeComponent::NotifyEnemyLostTarget(AActor* Enemy)
{
    if (Enemy) Aggressors.Remove(Enemy);

    // 모든 적이 놓친 경우에만 10초 뒤 None으로 전환 시도
    if (Aggressors.Num() == 0)
    {
        if (Layer == ECombatLayer::Hard) 
        {
            SetLayer(ECombatLayer::Soft);
            ResetCountdown();
        }
        else if (Layer == ECombatLayer::Soft)
        {
            /* Soft는 원래 타이머 굴고 있으니 그대로 */;
        }
        UE_LOG(LogTemp, Warning, TEXT("Not Combat Mode"));
    }
    // 적이 아직 남아있으면 하드 유지(타이머는 그대로 두거나 리셋 규칙을 적용하지 않음)
    UE_LOG(LogTemp, Display, TEXT("Remain Aggressors Num: %d"), Aggressors.Num());
}


void UCombatModeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearCountdown();
    Aggressors.Empty();
    Super::EndPlay(EndPlayReason);
}