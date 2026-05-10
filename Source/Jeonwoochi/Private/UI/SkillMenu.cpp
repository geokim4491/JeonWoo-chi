// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SkillMenu.h"
#include "UI/SkillButton.h"
#include "UI/SkillInfoWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

void USkillMenu::NativeConstruct()
{
	if (BTN_Skill1)
	{
        BTN_Skill1->BTN_Skill->OnClicked.AddDynamic(this, &USkillMenu::OnSkill1ButtonClicked);
	}

    if (BTN_Skill2)
    {
        BTN_Skill2->BTN_Skill->OnClicked.AddDynamic(this, &USkillMenu::OnSkill2ButtonClicked);
    }
}

void USkillMenu::OnSkill1ButtonClicked()
{
    if (ContentSwitcher)
    {
        ContentSwitcher->SetActiveWidgetIndex(0);
        USkillInfoWidget* SkillWidget = Cast<USkillInfoWidget>(ContentSwitcher->GetActiveWidget());

        if (!SkillWidget) return;
        SkillWidget->PlayPreview();
    }
}

void USkillMenu::OnSkill2ButtonClicked()
{
    if (ContentSwitcher)
    {
        ContentSwitcher->SetActiveWidgetIndex(1);
        USkillInfoWidget* SkillWidget = Cast<USkillInfoWidget>(ContentSwitcher->GetActiveWidget());

        if (!SkillWidget) return;
        SkillWidget->PlayPreview();
    }
}
