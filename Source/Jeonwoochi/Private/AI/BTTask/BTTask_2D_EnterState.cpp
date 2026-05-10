// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask/BTTask_2D_EnterState.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/AI/PaperAICharacter.h"

EBTNodeResult::Type UBTTask_2D_EnterState::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory
)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    // AIController와 Pawn 체크
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    APaperAICharacter* Character = Cast<APaperAICharacter>(AICon->GetPawn());
    if (!Character) return EBTNodeResult::Failed;

    UPaperAIAnimationComponent* AnimComp = Character->FindComponentByClass<UPaperAIAnimationComponent>();
    if (!AnimComp) return EBTNodeResult::Failed;


    AnimComp->BTCallingState = State;

    AnimComp->Dispatch(EAnimEvent::BTCalled);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(BlackboardKey.SelectedKeyName));
    if (!TargetActor || !IsValid(TargetActor))
        return EBTNodeResult::Failed;

    FVector CharLoc = Character->GetActorLocation();
    FVector TargetLoc = TargetActor->GetActorLocation();

    CharLoc.Z = TargetLoc.Z;

    const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(CharLoc, TargetLoc);

    Character->SetActorRotation(LookAt);

    return EBTNodeResult::Succeeded;
}