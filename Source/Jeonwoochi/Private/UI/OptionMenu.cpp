// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OptionMenu.h"
#include "UI/OptionSlot.h"
#include "Components/Slider.h"
#include "UI/MenuButton.h"
#include "Sound/SoundClass.h"
#include "Data/Save/SaveSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../JeonwoochiGameMode.h"

void UOptionMenu::NativeConstruct()
{
	Super::NativeConstruct();
		
	if (BTN_Save)
	{
		BTN_Save->OnMenuClicked.RemoveAll(this);
		BTN_Save->OnMenuClicked.AddDynamic(this, &UOptionMenu::OnSaveClicked);
	}

	// 사운드: 슬라이더 값 변경만 메뉴에서 처리
	if (Slot_Sound && Slot_Sound->Slider)
	{
		Slot_Sound->Slider->OnValueChanged.AddDynamic(this, &UOptionMenu::OnSoundSliderChanged);
	}

	if (BTN_Back)
	{
		BTN_Back->OnMenuClicked.RemoveAll(this);
		BTN_Back->OnMenuClicked.AddDynamic(this, &UOptionMenu::OnBackClicked);
	}

	if (BTN_Exit)
	{
		BTN_Exit->OnMenuClicked.RemoveAll(this);
		BTN_Exit->OnMenuClicked.AddDynamic(this, &UOptionMenu::OnExitClicked);
	}

	if (BTN_Start)
	{
		BTN_Start->OnMenuClicked.RemoveAll(this);
		BTN_Start->OnMenuClicked.AddDynamic(this, &UOptionMenu::OnStartClicked);
	}
}

void UOptionMenu::OnSaveClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>();
		if (SaveSub)
		{
			SaveSub->SaveProgress(false);
			UE_LOG(LogTemp, Warning, TEXT("Saved"));
		}
	}
}

void UOptionMenu::OnSoundSliderChanged(float Value)
{
	ApplySoundVolume(Value);
}

void UOptionMenu::ApplySoundVolume(float Normalized)
{
	if (MasterSoundClass)
	{
		MasterSoundClass->Properties.Volume = FMath::Clamp(Normalized, 0.0f, 1.0f);
		// 필요 시 SoundMix/Class Override로 즉시 반영 강화 가능
	}
}

void UOptionMenu::OnBackClicked()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UOptionMenu::OnExitClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>();
		if (SaveSub)
		{
			SaveSub->SaveProgressBeforeExit();
			UE_LOG(LogTemp, Warning, TEXT("Saved"));
		}
	}
	UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, true);
}

void UOptionMenu::OnStartClicked()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (auto* GM = World->GetAuthGameMode<AJeonwoochiGameMode>())
	{
		GM->OpenLevelHard(TEXT("/Game/Maps/Start/StartLevel"), 1); // 로딩창과 함께 openlevel
	}
}