// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatWidget.generated.h"

class UAttributeWidget;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UStatWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UAttributeWidget* AttrHealthP;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UAttributeWidget* AttrMentalP;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UAttributeWidget* AttrAttackP;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UAttributeWidget* AttrIntP;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UAttributeWidget* AttrSkillP;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UAttributeWidget* AttrMystP;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UAttributeWidget* AttrDefP;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnAttrLeftClicked(UAttributeWidget* Sender);
	void OnAttrRightclicked(UAttributeWidget* Sender);
};
