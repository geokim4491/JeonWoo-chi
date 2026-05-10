// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SkillInfoWidget.h"
#include "MediaPlayer.h"
#include "MediaSource.h"

void USkillInfoWidget::PlayPreview()
{
    if (MediaPlayer && MediaSource)
    {
        // 열려있던 소스가 있으면 닫고 새로 연 뒤 재생
        MediaPlayer->Close();
        if (MediaPlayer->OpenSource(MediaSource))
        {
            MediaPlayer->Play();
        }
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("Why this not working"));
    }
}
