// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/ExecutionComponent.h"
#include "Characters/AI/BaseAICharacter.h"
#include "Jeonwoochi/JeonwoochiCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/Utilities/CameraActionRouterComponent.h"
#include "Components/Combat/PlayerDefensiveComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UExecutionComponent::UExecutionComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

    OwnerChar = Cast<AJeonwoochiCharacter>(GetOwner());

    
}


// Called when the game starts
void UExecutionComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


// Called every frame
void UExecutionComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UExecutionComponent::Execute()
{
    if (!ExecutionMontage || !OwnerChar || !ExecutedTarget) return;

    if (auto* Def = OwnerChar->GetPlayerDefensiveComponent())
        Def->SetDefenseState(EPlayerDefenseState::Invincible);

    CachedExecutedTarget = ExecutedTarget;

    if (ABaseAICharacter* AI = Cast<ABaseAICharacter>(CachedExecutedTarget))
    {
        AI->ShowLockMarker(false);
    }

    OwnerChar->ClearAllIMC();

    // 이동 완전 정지(루트모션만 쓰고 싶다면 DisableMovement가 깔끔)
    OwnerChar->GetCharacterMovement()->StopMovementImmediately();
    
    FTransform AlignTF;
    if (ExecutedTarget->CalcExecAlignTransform(AlignTF))
    {
        OwnerChar->SetActorTransform(AlignTF);
    }

    ExecutedTarget->Executed();
    
    if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
    {
        Anim->OnMontageEnded.AddDynamic(this, &UExecutionComponent::OnExecutionMontageEnded);

        float PlayedLen = OwnerChar->PlayAnimMontage(ExecutionMontage);
        if (PlayedLen <= 0.f)
        {
            // 재생 실패 시 즉시 복구
            OnExecutionMontageEnded(ExecutionMontage, /*bInterrupted*/true);
        }
    }

    UWorld* World = GetWorld();
    if (World && ExecutionSound)
    {
        UGameplayStatics::PlaySound2D(World, ExecutionSound);
    }
    
    if (auto* Router = OwnerChar->FindComponentByClass<UCameraActionRouterComponent>())
    {
        Router->PlayAction(TEXT("Execution"));
    }
}

void UExecutionComponent::OnExecutionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == ExecutionMontage)
    {
        if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
        {
            Anim->OnMontageEnded.RemoveDynamic(this, &UExecutionComponent::OnExecutionMontageEnded);
        }

        if (auto* Def = OwnerChar->GetPlayerDefensiveComponent())
        {
            Def->SetDefenseState(EPlayerDefenseState::None);
        }

        SetCanExecute(false);

        // 2) 입력 복구
        OwnerChar->ApplyIMC_Default();

        if (ABaseAICharacter* AI = Cast<ABaseAICharacter>(CachedExecutedTarget))
        {
            AI->ShowLockMarker(true);
        }

    }
}
