// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Utilities/PlayerUIComponent.h"

#include "Jeonwoochi/JeonwoochiCharacter.h"
#include "Components/Combat/ChiComponent.h"
#include "Components/Combat/CombatModeComponent.h"
#include "Components/Combat/AmbushComponent.h"
#include "Components/Combat/ExecutionComponent.h"
#include "Components/Combat/PostureComponent.h"

#include "UI/PlayerHUD.h"
#include "UI/AmbushWidget.h"
#include "UI/ExecutionWidget.h"

#include "Blueprint/UserWidget.h"

// Sets default values for this component's properties
UPlayerUIComponent::UPlayerUIComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerUIComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	CreateHUD();

	ACharacter* PlayerCharacter = Cast<ACharacter>(GetOwner());
	if (!PlayerCharacter) return;

	if (UCombatModeComponent* Combat = PlayerCharacter->FindComponentByClass<UCombatModeComponent>())
	{
		Combat->OnCombatUIFade.AddDynamic(this, &UPlayerUIComponent::HandleHUD);
		// 현재 상태 즉시 반영
		HandleHUD(Combat->GetLayer() != ECombatLayer::None);
	}
}


// Called every frame
void UPlayerUIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerUIComponent::UpdateHealth(AActor* Actor, ECharacterType CharacterType, float CurrentHealth, float MaxHealth)
{
	PlayerHUD->UpdateHealth(MaxHealth, CurrentHealth);
}

void UPlayerUIComponent::UpdateChi(float CurrentChi, float MaxChi)
{
	PlayerHUD->UpdateChi(MaxChi, CurrentChi);
}

void UPlayerUIComponent::CreateHUD()
{
	if (!GetWorld() || !UPlayerHUDClass) return;

	ACharacter* PlayerCharacter = Cast<ACharacter>(GetOwner());

	if (!PlayerCharacter) return;

	PlayerHUD = CreateWidget<UPlayerHUD>(GetWorld(), UPlayerHUDClass);
	if (!PlayerHUD) return;

	PlayerHUD->AddToViewport();

	UHealthComponent* HealthComponent = PlayerCharacter->FindComponentByClass<UHealthComponent>();
	UChiComponent* ChiComponent = PlayerCharacter->FindComponentByClass<UChiComponent>();

	PlayerHUD->UpdateHealth(HealthComponent->GetMaxHealth(), HealthComponent->GetCurrentHealth());
	PlayerHUD->UpdateChi(ChiComponent->GetMaxChi(), ChiComponent->GetCurrentChi());

	HealthComponent->OnHealthChanged.AddUObject(this, &UPlayerUIComponent::UpdateHealth);
	ChiComponent->OnChiChanged.AddUObject(this, &UPlayerUIComponent::UpdateChi);
}

void UPlayerUIComponent::HandleHUD(bool bShow)
{
	if (bCinematicHidden || !PlayerHUD) return;

	UE_LOG(LogTemp, Warning, TEXT("HandleHUD called"));
	if (bShow)
	{
		PlayerHUD->HandleCombatStarted();
	}
	else
	{
		PlayerHUD->HandleCombatEnded();
	}

}

void UPlayerUIComponent::HandleBoss(AActor* Actor, FText Text)
{
	UHealthComponent* HealthComponent = Actor->FindComponentByClass<UHealthComponent>();
	if (!HealthComponent) return;
	UPostureComponent* PostureComponent = Actor->FindComponentByClass<UPostureComponent>();
	if (!PostureComponent) return;

	PlayerHUD->ShowBossBar(Text, HealthComponent->GetMaxHealth(), HealthComponent->GetCurrentHealth(), PostureComponent->GetMaxPosture(), PostureComponent->GetCurPosture());
	HealthComponent->OnDeath.AddUObject(this, &UPlayerUIComponent::HandleBossDeath);
	HealthComponent->OnHealthChanged.AddUObject(PlayerHUD, &UPlayerHUD::UpdateBossHealth);
	PostureComponent->OnPostureUpdated.AddUObject(PlayerHUD, &UPlayerHUD::UpdateBossPosture);
}

void UPlayerUIComponent::HandleBossDeath(AActor* Actor, ECharacterType CharacterType)
{
	if (CharacterType != ECharacterType::Boss) return;

	PlayerHUD->HideBossBar();

	UHealthComponent* HealthComponent = Actor->FindComponentByClass<UHealthComponent>();
	if (!HealthComponent) return;
	UPostureComponent* PostureComponent = Actor->FindComponentByClass<UPostureComponent>();
	if (!PostureComponent) return;

	HealthComponent->OnDeath.RemoveAll(this);
	HealthComponent->OnHealthChanged.RemoveAll(PlayerHUD);
	PostureComponent->OnPostureUpdated.RemoveAll(PlayerHUD);
}

void UPlayerUIComponent::SetCinematicUIHidden(bool bHidden)
{
	if (bHidden)
	{
		if (bCinematicHidden) return;
		bCinematicHidden = true;

		if (PlayerHUD)
		{
			PrevHUDVis = PlayerHUD->GetVisibility();
			PlayerHUD->StopAllAnimations();
			PlayerHUD->SetVisibility(ESlateVisibility::Collapsed);

			PlayerHUD->bCombatUIVisible = false;
		}
	}
	else
	{
		if (!bCinematicHidden) return;
		bCinematicHidden = false;

		if (PlayerHUD) 
		{ 
			PlayerHUD->SetVisibility(PrevHUDVis); 
		}

		// 현재 Combat 상태로 다시 맞추기
		if (ACharacter* PC = Cast<ACharacter>(GetOwner()))
		{
			if (auto* Combat = PC->FindComponentByClass<UCombatModeComponent>())
			{
				HandleHUD(Combat->GetLayer() != ECombatLayer::None);
			}
		}
	}
}