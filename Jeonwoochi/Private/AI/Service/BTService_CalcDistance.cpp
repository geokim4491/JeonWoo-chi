// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_CalcDistance.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTService_CalcDistance::UBTService_CalcDistance()
{
	NodeName = TEXT("Calculate Target Distance");
	
	// Tick Interval
	Interval = 0.5f;

    bNotifyBecomeRelevant = true;
}

void UBTService_CalcDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    // Blackboard에서 Actor 꺼내오기
    UObject* Obj = BB->GetValueAsObject(TargetActorKey.SelectedKeyName);
    AActor* Target = Cast<AActor>(Obj);
    if (!Target)
    {
        // Target이 없으면 0으로 설정하거나 처리 생략
        BB->SetValueAsFloat(DistanceKey.SelectedKeyName, 0.f);
        return;
    }

    // AI Pawn 가져오기
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return;
    APawn* AIPawn = AICon->GetPawn();
    if (!AIPawn) return;

    // 거리 계산
    float Dist = FVector::Dist(Target->GetActorLocation(),
        AIPawn->GetActorLocation());

    // Blackboard에 값 저장
    BB->SetValueAsFloat(DistanceKey.SelectedKeyName, Dist);
}

void UBTService_CalcDistance::OnBecomeRelevant(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);
    // 트리 실행 직후 한 번 호출
    TickNode(OwnerComp, NodeMemory, 0.f);
}