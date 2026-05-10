// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MenuButton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"
#include "Input/Events.h"
#include "Kismet/GameplayStatics.h"

void UMenuButton::NativeConstruct()
{
    Super::NativeConstruct();
    if (BTN_Menu)
    {
        BTN_Menu->OnClicked.RemoveAll(this);  
        BTN_Menu->OnClicked.AddDynamic(this, &UMenuButton::HandleClicked);
        // 최초 1회 스타일 캐시
        if (!bStyleCached)
        {
            CachedStyle = BTN_Menu->GetStyle();
            bStyleCached = true;
        }

    }
    if (!MenuName.IsEmpty())
    {
        TXT_MenuName->SetText(FText::FromString(MenuName));
    }

    SetIsFocusable(true);
}

void UMenuButton::NativeDestruct()
{
    if (BTN_Menu)
    {
        BTN_Menu->OnClicked.RemoveAll(this);  
    }
    Super::NativeDestruct();
}

void UMenuButton::HandleClicked()
{
    OnMenuClicked.Broadcast();
}

void UMenuButton::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnAddedToFocusPath(InFocusEvent);
    ApplyFocusAsHover(true);
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

void UMenuButton::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnRemovedFromFocusPath(InFocusEvent);
    ApplyFocusAsHover(false);
}

void UMenuButton::ApplyFocusAsHover(bool bEnable)
{
    if (!BTN_Menu || !bStyleCached) return;

    FButtonStyle FocusStyle = CachedStyle;

    if (bEnable)
    {
        // Hovered 시각효과 적용
        FocusStyle.SetNormal(FocusStyle.Hovered);
    }
    else
    {
        // 원래 스타일 복원
        FocusStyle.SetNormal(CachedStyle.Normal);
    }

    BTN_Menu->SetStyle(FocusStyle);
}

FReply UMenuButton::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept || Key == EKeys::SpaceBar)
    {
        if (BTN_Menu)  // 내부 UButton
        {
            // 눌렀을 때 클릭 이벤트 트리거
            BTN_Menu->OnClicked.Broadcast();
            return FReply::Handled();
        }
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}