// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SkillButton.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void USkillButton::NativePreConstruct()
{
    Super::NativePreConstruct();

    if (SkillIcon && BTN_Skill)
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(SkillIcon);
        Brush.ImageSize = FVector2D(250.f, 250.f); // 원하는 버튼 크기

        FButtonStyle NewStyle = BTN_Skill->GetStyle();
        NewStyle.SetNormal(Brush);
        NewStyle.SetHovered(Brush);
        NewStyle.SetPressed(Brush);

        BTN_Skill->SetStyle(NewStyle);
    }

    SetIsFocusable(true);
}

void USkillButton::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
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

void USkillButton::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnRemovedFromFocusPath(InFocusEvent);

    SetRenderOpacity(0.5f);
}