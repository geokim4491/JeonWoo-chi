// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/MusicSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

void UMusicSubsystem::PlayBGM(USoundBase* NewSound, float FadeInSeconds, float TargetVolume, float StartTime, bool /*bPersist*/)
{
    if (!GetWorld() || !NewSound) return;
    AudioComp = 
        UGameplayStatics::CreateSound2D(GetWorld(), NewSound, 1.f, 1.f, StartTime);

    if (!AudioComp) return;

    AudioComp->FadeIn(FMath::Max(0.f, FadeInSeconds));

    
}

void UMusicSubsystem::PlayBGM_Soft(TSoftObjectPtr<USoundBase> NewSound, float FadeInSeconds, float TargetVolume, float StartTime, bool /*bPersist*/)
{
    if (USoundBase* Loaded = NewSound.LoadSynchronous())
        PlayBGM(Loaded, FadeInSeconds, TargetVolume, StartTime);
}

void UMusicSubsystem::StopBGM(float FadeOutSeconds)
{
    if (!IsValid(AudioComp)) return;

    // 1) 로컬로 옮겨 담고, 멤버는 먼저 null 처리
    UAudioComponent* AC = AudioComp;
    AudioComp = nullptr;

    // 2) 재진입 방지: 이 객체로 묶인 델리게이트 해제
    AC->OnAudioFinished.RemoveAll(this);
    AC->OnAudioPlaybackPercent.RemoveAll(this);

    // 3) 즉시 정지로 인한 동기 콜백 방지: 최소 페이드 보장
    const float SafeFade = FMath::Max(0.01f, FadeOutSeconds);

    // 4) 자동 파괴(원하면)
    AC->bAutoDestroy = true;

    // 5) 페이드 아웃 실행 (이후 AC는 손대지 말 것)
    AC->FadeOut(SafeFade, 0.f);
}