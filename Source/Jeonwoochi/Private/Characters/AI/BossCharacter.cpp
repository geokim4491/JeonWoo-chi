// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AI/BossCharacter.h"
#include "Data/Progression/ProgressionSubsystem.h"
#include "Data/Save/SaveTargetComponent.h"
#include "Data/Save/SaveSubsystem.h"
#include "Components/Boss/BossPhaseComponent.h"

ABossCharacter::ABossCharacter()
{
	CharacterType = ECharacterType::Boss;
	SetIsBoss(true);

	BossPhaseComp = 
		CreateDefaultSubobject<UBossPhaseComponent>(TEXT("BossPhaseComp"));

	RagdollTime = 2.f;

	SaveComp->bIsEnemy = false;
	SaveComp->bIsBoss = true;
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		CachedBB = AIC->GetBlackboardComponent();
	}

	SetHPBarComponent(nullptr);
}

void ABossCharacter::Die_Implementation()
{
	Super::Die_Implementation();
	if (auto* Prog = GetGameInstance()->GetSubsystem<UProgressionSubsystem>())
	{
		if (const USaveTargetComponent* ST = FindComponentByClass<USaveTargetComponent>())
		{
			Prog->Set(ST->BossDefeatedTag, true);  // boss.<id>.defeated ON
		}
	}
	if (auto* Save = GetGameInstance()->GetSubsystem<USaveSubsystem>())
	{
		Save->SaveProgress(true);
	}
}

void ABossCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ABossCharacter::ChangePhase_Implementation(int32 CurrentPhase)
{
	UBlackboardComponent* BB = CachedBB.Get();
	if (!BB)
	{
		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			BB = AIC->GetBlackboardComponent();
			CachedBB = BB;
		}
	}
	if (BB)
	{
		// Int 키라면:
		BB->SetValueAsInt(TEXT("BossPhase"), CurrentPhase);
	}
}