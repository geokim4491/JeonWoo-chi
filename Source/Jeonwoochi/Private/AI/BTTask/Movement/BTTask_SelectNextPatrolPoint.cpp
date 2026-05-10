// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask/Movement/BTTask_SelectNextPatrolPoint.h"
#include "AI/AIController/BaseAIController.h"
#include "Components/SplineComponent.h"

UBTTask_SelectNextPatrolPoint::UBTTask_SelectNextPatrolPoint()
{
    NodeName = TEXT("Select Next Patrol Point");
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_SelectNextPatrolPoint::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    auto* AICon = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AICon || !AICon->PatrolSpline)
        return EBTNodeResult::Failed;

    USplineComponent* Spline = AICon->PatrolSpline;
    int32 NumPoints = Spline->GetNumberOfSplinePoints();
    if (NumPoints == 0)
        return EBTNodeResult::Failed;

    // 현재 인덱스에 해당하는 위치 월드 좌표로 가져오기
    FVector NextLoc = Spline->GetLocationAtSplinePoint(
        AICon->CurrentSplinePointIndex,
        ESplineCoordinateSpace::World);

    // 블랙보드에 설정 (Vector)
    OwnerComp.GetBlackboardComponent()
        ->SetValueAsVector(NextLocationKey.SelectedKeyName, NextLoc);

    // 인덱스 순환
    AICon->CurrentSplinePointIndex =
        (AICon->CurrentSplinePointIndex + 1) % NumPoints;

    return EBTNodeResult::Succeeded;
}