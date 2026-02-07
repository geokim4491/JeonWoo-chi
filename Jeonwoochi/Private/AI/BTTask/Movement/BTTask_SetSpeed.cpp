// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask/Movement/BTTask_SetSpeed.h"
#include "../../../../Public/AI/AIController/BaseAIController.h"
#include "../../../../Public/Characters/AI/BaseAICharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

EBTNodeResult::Type UBTTask_SetSpeed::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory
)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    // AIController¿Í Pawn Ã¼Å©
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    ABaseAICharacter* Char = Cast<ABaseAICharacter>(AICon->GetPawn());
    if (!Char) return EBTNodeResult::Failed;

    Char->GetCharacterMovement()->MaxWalkSpeed = Speed;

    return EBTNodeResult::Succeeded;
}