// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/MusicSettingActor.h"
#include "Sound/MusicSubsystem.h"
#include "Engine/World.h"
#include "Characters/AI/BossCharacter.h"
#include "Components/Combat/HealthComponent.h"

AMusicSettingActor::AMusicSettingActor()
{
    PrimaryActorTick.bCanEverTick = false;
    SetActorEnableCollision(false);
    SetHidden(true);
}

void AMusicSettingActor::BeginPlay()
{
    Super::BeginPlay();

    if (!bAutoPlayOnBeginPlay) return;
    if (!DefaultBGM.ToSoftObjectPath().IsValid()) return;

    if (UWorld* World = GetWorld())
    {
        MusicSystem = World->GetSubsystem<UMusicSubsystem>();
        if (MusicSystem)
        {
            MusicSystem->PlayBGM_Soft(DefaultBGM, DefaultFadeInSeconds, DefaultVolume, /*StartTime*/0.f, bPersistAcrossLevelTransition);
        }
    }

    if (BindingTarget)
    {
        if (auto* HC = BindingTarget->FindComponentByClass<UHealthComponent>())
        {
            HC->OnDeath.AddUObject(this, &AMusicSettingActor::PlayOriginBGM);
        }
    }
}

void AMusicSettingActor::ChangeBGM(TSoftObjectPtr<USoundBase> ChangeBGM, float FadeOutSeconds, float FadeInSeconds)
{
    DefaultBGM = ChangeBGM;
    // MusicSubsystem에 만들기
}

void AMusicSettingActor::PlayOriginBGM(AActor* a, ECharacterType e)
{
    if (MusicSystem)
    {
        MusicSystem->StopBGM();
        MusicSystem->PlayBGM_Soft(DefaultBGM, DefaultFadeInSeconds, DefaultVolume, /*StartTime*/0.f, bPersistAcrossLevelTransition);
    }
}