// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainMenu.h"
#include "UI/MenuButton.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "UI/StatWidget.h"
#include "UI/SkillMenu.h"
#include "UI/SkillButton.h"
#include "UI/OptionMenu.h"

void UMainMenu::NativePreConstruct()
{
    Super::NativePreConstruct();
}

void UMainMenu::NativeConstruct()
{
    if (BTN_Stat)
    {
        BTN_Stat->OnMenuClicked.RemoveAll(this);
        BTN_Stat->OnMenuClicked.AddDynamic(this, &UMainMenu::OnStatButtonClicked);
    }
    if (BTN_Equipment) 
    { 
        BTN_Equipment->OnMenuClicked.RemoveAll(this); 
        BTN_Equipment->OnMenuClicked.AddDynamic(this, &UMainMenu::OnEquipmentButtonClicked);
    }
    if (BTN_Inventory) 
    { 
        BTN_Inventory->OnMenuClicked.RemoveAll(this);
        BTN_Inventory->OnMenuClicked.AddDynamic(this, &UMainMenu::OnInventoryButtonClicked);
    }
    if (BTN_Skill) 
    { 
        BTN_Skill->OnMenuClicked.RemoveAll(this);     
        BTN_Skill->OnMenuClicked.AddDynamic(this, &UMainMenu::OnSkillButtonClicked);
    }
    if (BTN_Option) 
    { 
        BTN_Option->OnMenuClicked.RemoveAll(this);    
        BTN_Option->OnMenuClicked.AddDynamic(this, &UMainMenu::OnOptionButtonClicked);
    }

}

void UMainMenu::NativeDestruct()
{
    if (BTN_Stat) BTN_Stat->OnMenuClicked.RemoveAll(this);
    if (BTN_Equipment) BTN_Equipment->OnMenuClicked.RemoveAll(this);
    if (BTN_Inventory) BTN_Inventory->OnMenuClicked.RemoveAll(this);
    if (BTN_Skill)     BTN_Skill->OnMenuClicked.RemoveAll(this);
    if (BTN_Option)    BTN_Option->OnMenuClicked.RemoveAll(this);

    Super::NativeDestruct();
}

void UMainMenu::OnStatButtonClicked()
{
    if (ContentSwitcher)
    {
        ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(EMenuPage::Stat));
    }
}

void UMainMenu::OnEquipmentButtonClicked()
{
    if (ContentSwitcher)
    {
        ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(EMenuPage::Equipment));
    }
}

void UMainMenu::OnInventoryButtonClicked()
{
    if (ContentSwitcher)
    {
        ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(EMenuPage::Inventory));
    }
}

void UMainMenu::OnSkillButtonClicked()
{
    if (ContentSwitcher)
    {
        ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(EMenuPage::Skill));
        USkillMenu* SkillWidget = Cast<USkillMenu>(ContentSwitcher->GetActiveWidget());
        if (!SkillWidget) return;

        SkillWidget->BTN_Skill1->SetKeyboardFocus();
    }
}

void UMainMenu::OnOptionButtonClicked()
{
    if (ContentSwitcher)
    {
        ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(EMenuPage::Option));
        UOptionMenu* OptionWidget = Cast<UOptionMenu>(ContentSwitcher->GetActiveWidget());
        if (!OptionWidget) return;

        if (!OptionWidget->BTN_Save) return;
        OptionWidget->BTN_Save->SetKeyboardFocus();
    }
}