// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/AmbushedComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Characters/AI/BaseAICharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Interface/Combat/HealthInterface.h"

// Sets default values for this component's properties
UAmbushedComponent::UAmbushedComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UAmbushedComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UAmbushedComponent::OnAmbushed()
{
	if (!AmbushedMontage)
		return;

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (AAIController* AICon = Cast<AAIController>(Pawn->GetController()))
		{
			if (UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(AICon->BrainComponent))
			{
				BT->StopTree(EBTStopMode::Safe);   // ← Service까지 완전 정지
			}
		}
	}


	if (ABaseAICharacter* Char = Cast<ABaseAICharacter>(GetOwner()))
	{
		float MontageTime = Char->PlayAnimMontage(AmbushedMontage);

		GetWorld()->GetTimerManager().SetTimer(
			AmbushedTimerHandle,
			this, &UAmbushedComponent::GetUp,
			MontageTime, false
		);
	}
}

void UAmbushedComponent::GetUp()
{
	if (ABaseAICharacter* Char = Cast<ABaseAICharacter>(GetOwner()))
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		Char->SetAmbushZoneActive(false);
		Char->SetTargetToClosetEnemy();
	}

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (AAIController* AICon = Cast<AAIController>(Pawn->GetController()))
		{
			if (UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(AICon->BrainComponent))
			{
				// 1) 정지
				BT->StopTree(EBTStopMode::Safe);

				// 2) 재시작
				BT->RestartTree();

				// 3) 서브트리 재주입
				if (const auto* Config = Cast<ABaseAICharacter>(Pawn)->ConfigData)
				{
					BT->SetDynamicSubtree(FGameplayTag::RequestGameplayTag(TEXT("AI.IdleVariant")),
						Config->IdleSubtree);
					BT->SetDynamicSubtree(FGameplayTag::RequestGameplayTag(TEXT("AI.CombatVariant")),
						Config->CombatSubtree);
				}


			}
		}
	}
	if (GetOwner()->Implements<UHealthInterface>())
	{
		IHealthInterface::Execute_ApplyDamage(GetOwner(), DamageAmount);
	}
}

void UAmbushedComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 1) 남아 있는 타이머 해제
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
}