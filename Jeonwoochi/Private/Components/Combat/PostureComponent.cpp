// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PostureComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Characters/AI/BaseAICharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Interface/Combat/HealthInterface.h"

// Sets default values for this component's properties
UPostureComponent::UPostureComponent():
	MaxPostureCount(2), StunDuration(6.f), bIsStunned(false), DamageAmount(100.f)
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UPostureComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentPostureCount = MaxPostureCount;
}


void UPostureComponent::ReducePostureCount(int32 Amount)
{
	if (CurrentPostureCount - Amount > 0)
	{
		CurrentPostureCount -= Amount;
	}

	else
	{
		CurrentPostureCount = 0;
		Stun();
	}

	UE_LOG(LogTemp, Warning, TEXT("Posture: %d"), CurrentPostureCount);
	OnPostureUpdated.Broadcast(static_cast<float>(CurrentPostureCount), static_cast<float>(MaxPostureCount));

}

void UPostureComponent::ResetCurrentPosture()
{
	CurrentPostureCount = MaxPostureCount;
	OnPostureUpdated.Broadcast(CurrentPostureCount, MaxPostureCount);
}

void UPostureComponent::SetMaxPosture(int32 Posture)
{
	MaxPostureCount = Posture;
	CurrentPostureCount = MaxPostureCount;
	OnPostureUpdated.Broadcast(CurrentPostureCount, MaxPostureCount);
}

void UPostureComponent::Stun()
{
	if (!StunMontage) return;

	bIsStunned = true;

	// 1) AI 로직 정지
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
		Char->GetCharacterMovement()->DisableMovement();
		Char->PlayAnimMontage(StunMontage);
		Char->SetExecZoneActive(true);
	}

	GetWorld()->GetTimerManager().SetTimer(
		StunTimerHandle,
		this, &UPostureComponent::EndStun,
		StunDuration, false
	);
}

void UPostureComponent::EndStun()
{
	if (!bIsStunned)
		return;

	bIsStunned = false;

	if (ABaseAICharacter* Char = Cast<ABaseAICharacter>(GetOwner()))
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
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

	CurrentPostureCount = MaxPostureCount;
	OnPostureUpdated.Broadcast(CurrentPostureCount, MaxPostureCount);

}

void UPostureComponent::FinishExecution()
{
	if (!bIsStunned || !ExecutedMontage)
		return;

	GetWorld()->GetTimerManager().ClearTimer(StunTimerHandle);

	if (ABaseAICharacter* Char = Cast<ABaseAICharacter>(GetOwner()))
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		float MontageTime = Char->PlayAnimMontage(ExecutedMontage);
		
		Char->SetExecZoneActive(false);

		GetWorld()->GetTimerManager().SetTimer(
			ExecutedTimerHandle,
			this, &UPostureComponent::GetUp,
			MontageTime, false
		);
	}
}

void UPostureComponent::GetUp()
{
	EndStun(); 
	
	// 데미지 적용을 나중에 해야 StopBT 관련 로직이 꼬이지 않음
	if (GetOwner()->Implements<UHealthInterface>())
	{
		IHealthInterface::Execute_ApplyDamage(GetOwner(), DamageAmount);
	}
}

void UPostureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 1) 남아 있는 타이머 해제
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	// 2) 델리게이트 해제
	//    OnPostureUpdated를 바인딩한 쪽에서 RemoveDynamic을 호출해 주어야 하지만,
	//    안전을 위해 컴포넌트 자신도 RemoveAll으로 한 번 더 클리어할 수 있습니다.
	OnPostureUpdated.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}