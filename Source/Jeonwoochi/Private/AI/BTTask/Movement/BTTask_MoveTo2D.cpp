// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask/Movement/BTTask_MoveTo2D.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "AITypes.h"             
#include "Components/ArrowComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Characters/AI/PaperAICharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

EBTNodeResult::Type UBTTask_MoveTo2D::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    APaperAICharacter* Character = Cast<APaperAICharacter>(AICon->GetPawn());
    if (!Character) return EBTNodeResult::Failed;

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


    AAIController* AI = Cast<AAIController>(Character->GetController());
    if (AI)
    {
        FAIMoveRequest Req;
        Req.SetGoalActor(TargetActor);
        Req.SetAcceptanceRadius(50.f);      // 도착 판정
        Req.SetUsePathfinding(true);
        Req.SetAllowPartialPath(true);
        FNavPathSharedPtr OutPath;
        AI->MoveTo(Req, &OutPath);
    }


    return EBTNodeResult::Succeeded;
}