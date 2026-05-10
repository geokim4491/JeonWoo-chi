// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OptionSlot.h"
#include "Components/Slider.h"
#include "Components/Button.h"

void UOptionSlot::NativeConstruct()
{
    Super::NativeConstruct();

    if (BTN_Left)
    {
        BTN_Left->OnClicked.AddDynamic(this, &UOptionSlot::OnLeftClicked);
    }

    if (BTN_Right)
    {
        BTN_Right->OnClicked.AddDynamic(this, &UOptionSlot::OnRightClicked);
    }

}


void UOptionSlot::OnLeftClicked()
{
    if (Slider)
    {
        float NewValue = FMath::Clamp(Slider->GetValue() - 0.01f, 0.0f, 1.0f);
        Slider->SetValue(NewValue);
    }
}

void UOptionSlot::OnRightClicked()
{
    if (Slider)
    {
        float NewValue = FMath::Clamp(Slider->GetValue() + 0.01f, 0.0f, 1.0f);
        Slider->SetValue(NewValue);
    }
}

