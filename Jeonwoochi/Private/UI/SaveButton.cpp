// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SaveButton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void USaveButton::NativeConstruct()
{
    Super::NativeConstruct();

    SetIsFocusable(true);

    SetRenderOpacity(0.5f);
}

void USaveButton::SetSaveInfo(const FJWGameInfo& Info)
{
    if (TXT_GameName)
        TXT_GameName->SetText(FText::FromString(Info.GameName));

    if (TXT_SavedAt)
        TXT_SavedAt->SetText(FText::FromString(Info.SavedAt.ToString()));

    if (TXT_LevelTitle)
        TXT_LevelTitle->SetText(Info.LevelTitle.IsEmpty() ? FText::FromName(Info.LevelName)
            : Info.LevelTitle);

    if (TXT_TotalPlayTime)
    {
        int32 Hours = Info.TotalPlaySeconds / 3600;
        int32 Minutes = (Info.TotalPlaySeconds % 3600) / 60;
        int32 Seconds = Info.TotalPlaySeconds % 60;
        FString TimeStr = FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);
        TXT_TotalPlayTime->SetText(FText::FromString(TimeStr));
    }
}

void USaveButton::Bind()
{
    if (BTN_Menu)
    {
        BTN_Menu->OnClicked.RemoveAll(this);
        BTN_Menu->OnClicked.AddDynamic(this, &USaveButton::HandleClicked);
    }
}

void USaveButton::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnAddedToFocusPath(InFocusEvent);

    SetRenderOpacity(1.0f);

    if (Sound)
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            UGameplayStatics::PlaySound2D(
                PC,            // World 대신 PC를 컨텍스트로
                Sound,
                1.f,           // Volume
                1.f            // Pitch
            );
        }
    }
}

void USaveButton::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnRemovedFromFocusPath(InFocusEvent);

    SetRenderOpacity(0.5f);
}