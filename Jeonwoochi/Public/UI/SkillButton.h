// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillButton.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API USkillButton : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BTN_Skill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	UTexture2D* SkillIcon;

	UPROPERTY(EditAnywhere, Category = "Sound")
	class USoundBase* Sound;

protected:
	virtual void NativePreConstruct() override;

	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
};
