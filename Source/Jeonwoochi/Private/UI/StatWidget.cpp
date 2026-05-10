// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/StatWidget.h"
#include "UI/AttributeWidget.h"
#include "Components/TextBlock.h"

void UStatWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    // 디자인 타임(위젯 디자이너)에서도 호출되니 널가드 필수
    auto SetName = [](UAttributeWidget* Attr, const FString& Name)
        {
            if (Attr && Attr->TXT_AttrName)
            {
                Attr->TXT_AttrName->SetText(FText::FromString(Name));
            }
        };

    SetName(AttrHealthP, TEXT("체력"));
    SetName(AttrMentalP, TEXT("기"));
    SetName(AttrAttackP, TEXT("공격력"));
    SetName(AttrIntP, TEXT("도력"));
    SetName(AttrSkillP, TEXT("신비"));
    SetName(AttrMystP, TEXT("기량"));
    SetName(AttrDefP, TEXT("방어력"));
}

void UStatWidget::NativeConstruct()
{
    Super::NativeConstruct();

    /*auto BindAttr = [&](UAttributeWidget* Attr)
        {
            if (!Attr) return;
            Attr->OnLeftClicked.RemoveAll(this);
            Attr->OnRightClicked.RemoveAll(this);
            Attr->OnLeftClicked.AddDynamic(this, &UStatWidget::OnAttrLeftClicked);
            Attr->OnRightClicked.AddDynamic(this, &UStatWidget::OnAttrRightclicked);
        };

    BindAttr(AttrHealthP);
    BindAttr(AttrMentalP);
    BindAttr(AttrAttackP);
    BindAttr(AttrIntP);
    BindAttr(AttrSkillP);
    BindAttr(AttrMystP);
    BindAttr(AttrDefP);*/
}

void UStatWidget::NativeDestruct()
{
    /*auto RemoveAttr = [&](UAttributeWidget* Attr)
        {
            if (!Attr) return;
            Attr->OnLeftClicked.RemoveAll(this);
            Attr->OnRightClicked.RemoveAll(this);
        };

    RemoveAttr(AttrHealthP);
    RemoveAttr(AttrMentalP);
    RemoveAttr(AttrAttackP);
    RemoveAttr(AttrIntP);
    RemoveAttr(AttrSkillP);
    RemoveAttr(AttrMystP);
    RemoveAttr(AttrDefP);*/

    Super::NativeDestruct();
}

void UStatWidget::OnAttrLeftClicked(UAttributeWidget* Sender)
{

}

void UStatWidget::OnAttrRightclicked(UAttributeWidget* Sender)
{

}