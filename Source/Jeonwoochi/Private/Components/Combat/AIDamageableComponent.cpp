// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/AIDamageableComponent.h"
#include "Components/Combat/AttackComponent.h"
#include "Interface/Combat/HealthInterface.h"
#include "Perception/AISense_Damage.h"
#include "Characters/BaseCharacter.h"

#include "Manager/EffectManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UAIDamageableComponent::UAIDamageableComponent() : HitIdx(0)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UAIDamageableComponent::BeginPlay()
{
	Super::BeginPlay();

	for (const FReactionEnemyMontages& Elem : HitMontages)
	{
		RuntimeHitMontageMap.Add(Elem.ReactionType, Elem.Montages);
	}
	
}

void UAIDamageableComponent::HandleTakeDamage(const FDamageInfoStruct& Info)
{
	FVector MyLocation = GetOwner()->GetActorLocation();
	FRotator MyRotation = GetOwner()->GetActorRotation();
	
	// 패링일 때

	// Dodging일 때

	// Hit 일 때
	HandleHit(Info);
	if (Info.Attacker->Implements<UCharacterTypeInterface>())
	{
		ECharacterType AttackerType = ICharacterTypeInterface::Execute_GetCharacterType(Info.Attacker);

		if (AttackerType == ECharacterType::Player) 
		{
			PlayEffect(EEffectType::EnemyHit);
		}
		else if (AttackerType == ECharacterType::Companion)
		{
			PlayEffect(EEffectType::EnemyHit, false, false);
		}
		else if (AttackerType == ECharacterType::NormalEnemy)
		{
			PlayEffect(EEffectType::PlayerHit, false, false);
		}
	}
	

}

void UAIDamageableComponent::HandleHit(const FDamageInfoStruct& Info)
{
	AActor* Char = GetOwner();

	float Damage = Info.DamageAmount;

	if (Char->Implements<UHealthInterface>())
	{
		IHealthInterface::Execute_ApplyDamage(Char, Damage);

		if (IHealthInterface::Execute_GetCurrentHealth(Char) > 0)
		{
			HandleReaction(Info);
		}
	}

	if (!Info.Attacker) return;

	UAISense_Damage::ReportDamageEvent(
		GetWorld(),
		Char,
		Info.Attacker,
		Info.DamageAmount,
		Info.Attacker->GetActorLocation(),
		Char->GetActorLocation()
	);
}

void UAIDamageableComponent::HandleReaction(const FDamageInfoStruct& Info)
{
	if (RuntimeHitMontageMap.Contains(Info.ReactionType))
	{

		HitIdx = (HitIdx + 1) % RuntimeHitMontageMap[Info.ReactionType].Num();
		UAnimMontage* HitMontage = RuntimeHitMontageMap[Info.ReactionType][HitIdx];

		if (ABaseCharacter* Char = Cast<ABaseCharacter>(GetOwner()))
		{
			if (Char->FindComponentByClass<UAttackComponent>()->bIsAttacking && !bForceHitAnim)
				return;

			Char->PlayAnimMontage(HitMontage);
		}
	}
}

void UAIDamageableComponent::PlayEffect(EEffectType EffectType, bool bUseCameraShake, bool bUseHaptic)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
	{
		if (UEffectManager* EM = GI->GetSubsystem<UEffectManager>())
		{
			EM->PlayEffectOnActor(
				EffectType,
				GetOwner(),
				bUseCameraShake,
				bUseHaptic
			);
		}
	}
}