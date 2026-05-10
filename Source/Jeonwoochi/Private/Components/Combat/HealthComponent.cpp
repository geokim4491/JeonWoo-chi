// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/HealthComponent.h"
#include "Interface/Character/CharacterTypeInterface.h"
#include "Interface/Combat/HealthInterface.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	
	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CacheOwnerType();

	CurrentHealth = MaxHealth;
	bIsDead = false;
	
}

void UHealthComponent::CacheOwnerType() 
{
	OwnerActor = GetOwner();

	if (!OwnerActor) {
		return;
	}

	if (OwnerActor->Implements<UCharacterTypeInterface>())
	{
		OwnerType = ICharacterTypeInterface::Execute_GetCharacterType(OwnerActor);
	}
	else
	{
		OwnerType = ECharacterType::None;
	}
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


float UHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

void UHealthComponent::SetMaxHealth(float NewMax)
{
	MaxHealth = FMath::Max(NewMax, 0.f);
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(OwnerActor, OwnerType, CurrentHealth, MaxHealth);
}

void UHealthComponent::SetCurrentHealth(float NewValue)
{
	CurrentHealth = FMath::Clamp(NewValue, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(OwnerActor, OwnerType, CurrentHealth, MaxHealth);
}

void UHealthComponent::ApplyDamage(const float& DamageAmount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("%s Health: %f"),*GetOwner()->GetName(), CurrentHealth);
	OnHealthChanged.Broadcast(OwnerActor, OwnerType, CurrentHealth, MaxHealth);

	if (CurrentHealth == 0)
	{
		Die();
	}
}

void UHealthComponent::ApplyDotDamage(const FDotDealInfoStruct& DotInfo)
{
	if (DotInfo.Interval <= 0.f || DotInfo.TotalDuration <= 0.f)
	{
		return;
	}

	int32 NewIndex = ActiveDots.Add(DotInfo);

	// 타이머 등록
	GetWorld()->GetTimerManager().SetTimer(
		ActiveDots[NewIndex].TimerHandle,
		FTimerDelegate::CreateUObject(this, &UHealthComponent::HandleDotTick, NewIndex),
		ActiveDots[NewIndex].Interval,
		true
	);

}

void UHealthComponent::HandleDotTick(int32 DotIndex)
{
	if (!ActiveDots.IsValidIndex(DotIndex))
		return;

	FDotDealInfoStruct& Dot = ActiveDots[DotIndex];

	ApplyDamage(Dot.DamageInfo.DamageAmount);

	// 남은 틱 감소
	if (--Dot.TicksRemaining <= 0)
	{
		// 타이머 해제
		GetWorld()->GetTimerManager().ClearTimer(Dot.TimerHandle);
		// 배열에서 제거 (Swap 제거로 성능 최적화)
		ActiveDots.RemoveAtSwap(DotIndex);
	}
}

void UHealthComponent::Heal(const float& HealAmount) 
{
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(OwnerActor, OwnerType, CurrentHealth, MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("%s Health: %f"), *GetOwner()->GetName(), CurrentHealth);
}

void UHealthComponent::Die()
{
	bIsDead = true;
	if (OwnerActor->GetClass()->ImplementsInterface(UHealthInterface::StaticClass()))
	{
		IHealthInterface::Execute_Die(OwnerActor);
	}
	OnDeath.Broadcast(OwnerActor, OwnerType); // broadcast를 마지막에 해야 순서 안꼬인다.
}

void UHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	OnDeath.RemoveAll(this);
	OnHealthChanged.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}