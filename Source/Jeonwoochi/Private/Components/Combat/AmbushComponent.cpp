// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/AmbushComponent.h"
#include "Characters/AI/BaseAICharacter.h"
#include "Jeonwoochi/JeonwoochiCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/Utilities/CameraActionRouterComponent.h"
#include "Components/Combat/PlayerDefensiveComponent.h"
#include "Components/Utilities/LockOnComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UAmbushComponent::UAmbushComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	OwnerChar = Cast<AJeonwoochiCharacter>(GetOwner());
}


// Called when the game starts
void UAmbushComponent::BeginPlay()
{
	Super::BeginPlay();
}
void UAmbushComponent::SetCanAmbush(bool bCanAmbu)
{
    bCanAmbush = bCanAmbu; 
    
    UWorld* World = GetWorld();
    if (World && SetAmbushSound && bCanAmbu)
    {
        UGameplayStatics::PlaySound2D(World, SetAmbushSound);
    }
    
    OnCanAmbush.Broadcast(bCanAmbu);
}

void UAmbushComponent::Ambush()
{
    if (!AmbushMontage || !OwnerChar || !AmbushedTarget) return;

    if (AmbushedTarget->GetCharacterType() != ECharacterType::NormalEnemy) return;

    CachedAmbushedTarget = AmbushedTarget;

    if (ABaseAICharacter* AI = Cast<ABaseAICharacter>(CachedAmbushedTarget))
    {
        if (OwnerChar->GetLockOnComponent()->TargetActor == CachedAmbushedTarget)
            AI->ShowLockMarker(false);
    }

    if (auto* Def = OwnerChar->GetPlayerDefensiveComponent())
        Def->SetDefenseState(EPlayerDefenseState::Invincible);

    OwnerChar->ClearAllIMC();

    // 이동 완전 정지(루트모션만 쓰고 싶다면 DisableMovement가 깔끔)
    OwnerChar->GetCharacterMovement()->StopMovementImmediately();

    FTransform AlignTF;
    if (AmbushedTarget->CalcAmbushAlignTransform(AlignTF))
    {
        OwnerChar->SetActorTransform(AlignTF);
    }

    AmbushedTarget->Ambushed();
    
    if (auto* Router = OwnerChar->FindComponentByClass<UCameraActionRouterComponent>())
    {
        Router->PlayAction(TEXT("Ambush"));
    }

    if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
    {
        Anim->OnMontageEnded.AddDynamic(this, &UAmbushComponent::OnAmbushMontageEnded);

        float PlayedLen = OwnerChar->PlayAnimMontage(AmbushMontage);
        if (PlayedLen <= 0.f)
        {
            // 재생 실패 시 즉시 복구
            OnAmbushMontageEnded(AmbushMontage, /*bInterrupted*/true);
        }
    }

    UWorld* World = GetWorld();
    if (World && AmbushSound)
    {
        UGameplayStatics::PlaySound2D(World, AmbushSound);
    }
}

void UAmbushComponent::OnAmbushMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == AmbushMontage)
    {
        if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
        {
            Anim->OnMontageEnded.RemoveDynamic(this, &UAmbushComponent::OnAmbushMontageEnded);
        }

        if (auto* Def = OwnerChar->GetPlayerDefensiveComponent())
        {
            Def->SetDefenseState(EPlayerDefenseState::None);
        }
        // 2) 입력 복구
        OwnerChar->ApplyIMC_Default();

        if (ABaseAICharacter* AI = Cast<ABaseAICharacter>(CachedAmbushedTarget))
        {
            if (OwnerChar->GetLockOnComponent()->TargetActor == CachedAmbushedTarget)
                AI->ShowLockMarker(true);
        }
    }
}

