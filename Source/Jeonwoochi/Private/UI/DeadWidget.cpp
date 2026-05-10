// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DeadWidget.h"
#include "UI/MenuButton.h"
#include "Data/Save/SaveSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

void UDeadWidget::NativeConstruct()
{
	if (BTN_Restart)
	{
		BTN_Restart->OnMenuClicked.AddDynamic(this, &UDeadWidget::OnRestartButtonClicked);
	}

	if (BTN_QuitStart)
	{
		BTN_QuitStart->OnMenuClicked.AddDynamic(this, &UDeadWidget::OnQuitStartButtonClicked);
	}

	if (BTN_Quit)
	{
		BTN_Quit->OnMenuClicked.AddDynamic(this, &UDeadWidget::OnQuitButtonClicked);
	}
}

void UDeadWidget::NativeDestruct()
{
	if (BTN_Restart)
	{
		BTN_Restart->OnMenuClicked.RemoveAll(this);
	}

	if (BTN_QuitStart)
	{
		BTN_QuitStart->OnMenuClicked.RemoveAll(this);
	}

	if (BTN_Quit)
	{
		BTN_Quit->OnMenuClicked.RemoveAll(this);
	}
}

void UDeadWidget::OnRestartButtonClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>();
		if (SaveSub)
		{
			SaveSub->LoadFromRest();
		}
	}
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;
	PC->SetPause(false);
	FInputModeGameOnly Mode;
	PC->SetInputMode(Mode);
}

void UDeadWidget::OnQuitStartButtonClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>();
		if (SaveSub)
		{
			SaveSub->UpdateTotalPlaySeconds();
		}
	}
	UGameplayStatics::OpenLevel(GetWorld(), FName("StartLevel"));
}

void UDeadWidget::OnQuitButtonClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>();
		if (SaveSub)
		{
			SaveSub->UpdateTotalPlaySeconds();
		}
	}
	UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, true);
}