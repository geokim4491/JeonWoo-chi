// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillMenu.generated.h"

class USkillButton;
class UWidgetSwitcher;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API USkillMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USkillButton* BTN_Skill1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USkillButton* BTN_Skill2;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* ContentSwitcher;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnSkill1ButtonClicked();

	UFUNCTION()
	void OnSkill2ButtonClicked();

};
