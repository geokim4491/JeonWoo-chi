// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AttributeWidget.h"
#include "Components/Button.h"

void UAttributeWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (BTN_Left)
    {
        BTN_Left->OnClicked.RemoveAll(this);
        BTN_Left->OnClicked.AddDynamic(this, &UAttributeWidget::HandleLeftClicked);
    }

    if (BTN_Right)
    {
        BTN_Right->OnClicked.RemoveAll(this);
        BTN_Right->OnClicked.AddDynamic(this, &UAttributeWidget::HandleRightClicked);
    }
}

void UAttributeWidget::NativeDestruct()
{
    if (BTN_Left)
    {
        BTN_Left->OnClicked.RemoveAll(this);
    }

    if (BTN_Right)
    {
        BTN_Right->OnClicked.RemoveAll(this);
    }

    Super::NativeDestruct();
}

void UAttributeWidget::HandleLeftClicked()
{
    OnLeftClicked.Broadcast(this);
}

void UAttributeWidget::HandleRightClicked()
{
    OnRightClicked.Broadcast(this);
}

void UAttributeWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{

}

void UAttributeWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{

}