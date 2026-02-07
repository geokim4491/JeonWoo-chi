// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PlayerAttackComponent.h"
#include "Characters/BaseCharacter.h"


// Sets default values for this component's properties
UPlayerAttackComponent::UPlayerAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
    
    bCanAttack = true;
}


// Called when the game starts
void UPlayerAttackComponent::BeginPlay()
{
	Super::BeginPlay();

    
    if (auto* Character = Cast<ABaseCharacter>(GetOwner()))
    {
        if (AnimInst)
        {
            AnimInst->OnMontageEnded.AddDynamic(this, &UPlayerAttackComponent::OnAttackMontageEnded);
        }
    }	
}

FDamageInfoStruct UPlayerAttackComponent::GetCurrentDamageInfo(EPlayerAttackType AttackType)
{
    if (const FAttackInfo* Info = AttackInfos.Find(AttackType))
    {
        const TArray<FDamageInfoStruct>& Seq = Info->ComboSequence;
        if (Seq.Num() == 0) return FDamageInfoStruct{};

        // Å¸ÀÔº° ÇöÀç ÀÎµ¦½º Á¶È¸(¾øÀ¸¸é 0)
        int32 Index = 0;
        if (const int32* FoundIdx = ComboIndexByType.Find(AttackType))
        {
            Index = *FoundIdx;
        }

        // ¹üÀ§ º¸Á¤
        if (!Seq.IsValidIndex(Index))
        {
            Index = 0;
        }

        return Seq[Index];
    }

    return FDamageInfoStruct{}; // ±âº»°ª
}

void UPlayerAttackComponent::DoAttack(EPlayerAttackType AttackType)
{
    if (!JudgementCanAttack()) return;

    FAttackInfo* Info = AttackInfos.Find(AttackType);
    if (!Info || Info->ComboSequence.Num() == 0)
    {
        UAttackComponent::Attack(PendingDamageInfo);
        return;
    }

    int32& Index = ComboIndexByType.FindOrAdd(AttackType, 0);
    if (!Info->ComboSequence.IsValidIndex(Index))
    {
        Index = 0;
    }

    const FDamageInfoStruct& Damage = Info->ComboSequence[Index];

    Attack(Damage);

    Index = (Index + 1) % Info->ComboSequence.Num();
}

bool UPlayerAttackComponent::JudgementCanAttack()
{
    if (bCanAttack)
    {
        bCanAttack = false;
        return true;
    }
    else
    {
        return false;
    }
}

void UPlayerAttackComponent::ResetAllCombos()
{
    for (auto& Pair : ComboIndexByType)
    {
        Pair.Value = 0;
    }
}

void UPlayerAttackComponent::ResetComboForMoveCancel()
{
    ResetAllCombos();
    bIsAttacking = false;
}

void UPlayerAttackComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == PendingDamageInfo.Montage && bInterrupted)
    {
        GetWorld()->GetTimerManager().SetTimer(
            CooldownHandle,
            [this]() { 
                if (!bCanAttack) 
                {
                    UE_LOG(LogTemp, Display, TEXT("Timer"));
                    ResetAllCombos();
                    bIsAttacking = false;
                    bCanAttack = true;
                }
            },
            0.5f,
            false
        );
    }
}

void UPlayerAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AnimInst)
    {
        AnimInst->OnMontageEnded.RemoveDynamic(this, &UPlayerAttackComponent::OnAttackMontageEnded);
    }

    GetWorld()->GetTimerManager().ClearTimer(CooldownHandle);

    Super::EndPlay(EndPlayReason);
}