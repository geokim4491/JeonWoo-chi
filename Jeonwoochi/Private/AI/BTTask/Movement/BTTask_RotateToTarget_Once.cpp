// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask/Movement/BTTask_RotateToTarget_Once.h"
#include "AI/AIController/BaseAIController.h"

UBTTask_RotateToTarget_Once::UBTTask_RotateToTarget_Once()
{
	NodeName = TEXT("Rotate To Target");
}

EBTNodeResult::Type UBTTask_RotateToTarget_Once::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory
)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
    if (!Target) return EBTNodeResult::Failed;

    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
    if (!Pawn) return EBTNodeResult::Failed;

    // 회전
    const FRotator CurrentRot = Pawn->GetActorRotation();
    const float TargetYaw = (Target->GetActorLocation() - Pawn->GetActorLocation())
        .GetSafeNormal()
        .Rotation()
        .Yaw;

    // Pitch/Roll 유지 + 목표 Yaw
    const FRotator DesiredRot(CurrentRot.Pitch, TargetYaw, CurrentRot.Roll);

    Pawn->SetActorRotation(DesiredRot);

    return EBTNodeResult::Succeeded;
}