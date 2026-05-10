// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_RotateToTarget.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

UBTService_RotateToTarget::UBTService_RotateToTarget()
{
    NodeName = TEXT("Rotate To Target");
    Interval = 0.f;
    bNotifyTick = true;
    RandomDeviation = 0.f;
}

void UBTService_RotateToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!Target) return;

    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
    if (!Pawn) return;

    // 회전
    const FRotator CurrentRot = Pawn->GetActorRotation();
    const float TargetYaw = (Target->GetActorLocation() - Pawn->GetActorLocation())
        .GetSafeNormal()
        .Rotation()
        .Yaw;

    // Pitch/Roll 유지 + 목표 Yaw
    const FRotator DesiredRot(CurrentRot.Pitch, TargetYaw, CurrentRot.Roll);

    Pawn->SetActorRotation(DesiredRot);
    
}