// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/StartMenu.h"
#include "UI/SaveWidget.h"
#include "UI/OptionMenu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/MenuButton.h"
#include "Data/Save/SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/YesOrNoWidget.h"
#include "Jeonwoochi/JeonwoochiGameMode.h"

void UStartMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (BTN_Play)
	{
		BTN_Play->OnMenuClicked.AddDynamic(this, &UStartMenu::OnPlayButtonClicked);
		BTN_Play->SetKeyboardFocus();                 // 키보드 포커스 부여
	}

	if (BTN_Load)
	{
		BTN_Load->OnMenuClicked.AddDynamic(this, &UStartMenu::OnLoadButtonClicked);
	}

	if (BTN_Quit)
	{
		BTN_Quit->OnMenuClicked.AddDynamic(this, &UStartMenu::OnQuitButtonClicked);
	}

	if (BTN_Option)
	{
		BTN_Option->OnMenuClicked.AddDynamic(this, &UStartMenu::OnOptionButtonClicked);
	}

	if (YesOrNo)
	{
		YesOrNo->BTN_Yes->OnMenuClicked.AddDynamic(this, &UStartMenu::OnYesClicked);
		YesOrNo->BTN_No->OnMenuClicked.AddDynamic(this, &UStartMenu::OnNoClicked);
	}
}

void UStartMenu::BackToStart()
{
	if (BTN_Play)
	{
		BTN_Play->SetKeyboardFocus();
	}
}

void UStartMenu::NativeDestruct()
{
	if (BTN_Play)
	{
		BTN_Play->OnMenuClicked.RemoveAll(this);
	}

	if (BTN_Load)
	{
		BTN_Load->OnMenuClicked.RemoveAll(this);
	}

	if (BTN_Quit)
	{
		BTN_Quit->OnMenuClicked.RemoveAll(this);
	}

	if (BTN_Option)
	{
		BTN_Option->OnMenuClicked.RemoveAll(this);
	}
	if (YesOrNo)
	{
		YesOrNo->BTN_Yes->OnMenuClicked.RemoveAll(this);
		YesOrNo->BTN_No->OnMenuClicked.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UStartMenu::OnPlayButtonClicked()
{
	if (!YesOrNo) return;
	YesOrNo->SetVisibility(ESlateVisibility::Visible);
	YesOrNo->BTN_Yes->SetKeyboardFocus();
}

void UStartMenu::OnLoadButtonClicked()
{
	if (!SaveWidgetClass) return;

	// 한 번만 생성해서 재사용
	if (!SaveWidgetInst)
	{
		SaveWidgetInst = CreateWidget<USaveWidget>(GetWorld(), SaveWidgetClass);
		SaveWidgetInst->BTN_Back->OnMenuClicked.AddDynamic(this, &UStartMenu::BackToStart);
	}
	if (!SaveWidgetInst) return;

	if (!SaveWidgetInst->IsInViewport())
		SaveWidgetInst->AddToViewport(/*ZOrder*/100);
	else
		SaveWidgetInst->SetVisibility(ESlateVisibility::Visible);

	SaveWidgetInst->BTN_Back->SetKeyboardFocus();
}

void UStartMenu::OnQuitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, true);
}

void UStartMenu::OnOptionButtonClicked()
{
	if (!OptionWidget)
	{
		OptionWidget = CreateWidget<UOptionMenu>(GetWorld(), OptionWidgetClass);
		OptionWidget->BTN_Back->OnMenuClicked.AddDynamic(this, &UStartMenu::BackToStart);
	}
	if (!OptionWidget) return;

	if (!OptionWidget->IsInViewport())
		OptionWidget->AddToViewport(/*ZOrder*/100);
	else
		OptionWidget->SetVisibility(ESlateVisibility::Visible);

	OptionWidget->BTN_Back->SetKeyboardFocus();
}

void UStartMenu::OnYesClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>();
		if (SaveSub)
		{
			SaveSub->RebuildGameList();
			FString GameName = SaveSub->CreateNewGame();
			SaveSub->SelectGame(GameName);
			SaveSub->LoadForContinue(GameName);
			// UGameplayStatics::OpenLevel(this, FName("Palace_Tutorial"));
		}
	}
	if (UWorld* World = GetWorld())
	{
		if (auto* GM = World->GetAuthGameMode<AJeonwoochiGameMode>())
		{
			GM->OpenLevelHard(TEXT("/Game/Maps/Palace_Tutorial"), 0); // 로딩창과 함께 openlevel
		}
	}
	this->RemoveFromParent();
}

void UStartMenu::OnNoClicked()
{
	if (!YesOrNo) return;
	YesOrNo->SetVisibility(ESlateVisibility::Collapsed);
	BTN_Play->SetKeyboardFocus();
}