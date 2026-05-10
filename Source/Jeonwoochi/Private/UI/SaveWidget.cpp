// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SaveWidget.h"
#include "UI/SaveButton.h"
#include "UI/MenuButton.h"
#include "Data/Save/SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ScrollBox.h"

void USaveWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		SaveSub = GI->GetSubsystem<USaveSubsystem>();
		if (SaveSub)
		{
			SaveSub->OnGameListChanged.AddDynamic(this, &USaveWidget::RefreshSlots);
			SaveSub->RebuildGameList();
		}
	}
	// 호출해야 스타트 메뉴에서 불러오기 했을 때, 보임
	RefreshSlots();

	if (BTN_Back)
	{
		BTN_Back->OnMenuClicked.RemoveAll(this);
		BTN_Back->OnMenuClicked.AddDynamic(this, &USaveWidget::OnClicked_Back);
	}
}

void USaveWidget::NativeDestruct()
{
	if (SaveSub)
	{
		SaveSub->OnGameListChanged.RemoveDynamic(this, &USaveWidget::RefreshSlots);
	}
	Super::NativeDestruct();
}

void USaveWidget::RefreshSlots()
{
	if (!SlotLists || !SaveSub) return;
	SlotLists->ClearChildren();

	TArray<FJWGameInfo> Slots;
	SaveSub->GetGameFiles(Slots);

	UWidget* Prev = BTN_Back;

	for (const FJWGameInfo& S : Slots)
	{
		if (!SaveButtonClass) continue;
		// SaveButton 생성 (Outer를 이 위젯으로 주는 게 깔끔)
		if (USaveButton* Row = CreateWidget<USaveButton>(this, SaveButtonClass))
		{
			Row->Setup(S);
			Row->OnClicked.AddDynamic(this, &USaveWidget::OnClicked_Load);
			Row->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 5.0f));
			SlotLists->AddChild(Row);   // ScrollBox에 추가
			Prev->SetNavigationRuleExplicit(EUINavigation::Down, Row);
			Prev->SetNavigationRuleExplicit(EUINavigation::Right, Row);
			Row->SetNavigationRuleExplicit(EUINavigation::Up, Prev);
			Row->SetNavigationRuleExplicit(EUINavigation::Left, Prev);

			Prev = Row;
		}
	}


}

void USaveWidget::OnClicked_Load(const FString& GameName)
{
	if (SaveSub) 
	{ 
		SaveSub->SelectGame(GameName);
		SaveSub->LoadForContinue(GameName);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *GameName);
	} // 이어하기
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Clicked"));
	}
	this->RemoveFromParent();
}

void USaveWidget::OnClicked_Back()
{
	SetVisibility(ESlateVisibility::Collapsed);
}