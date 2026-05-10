// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/EnvQueryContext_Target.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/AIController/BaseAIController.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_Target::ProvideContext(
    FEnvQueryInstance& QueryInstance,
    FEnvQueryContextData& ContextData) const
{

    // 1) Querier로 지정된 객체(여기서는 Pawn)를 꺼내기
    UObject* QuerierObject = QueryInstance.Owner.Get();
    APawn* QuerierPawn = Cast<APawn>(QuerierObject);
    if (!QuerierPawn) return;

    // 2) AIController → BlackboardComponent 가져오기
    ABaseAIController* AICon = Cast<ABaseAIController>(QuerierPawn->GetController());
    if (!AICon) return;

    UBlackboardComponent* BBComp = AICon->GetBlackboardComponent();
    if (!BBComp) return;

    // 3) Blackboard에서 TargetActor 꺼내기
    UObject* Obj = BBComp->GetValueAsObject(TEXT("TargetActor"));
    AActor* TargetActor = Cast<AActor>(Obj);
    if (!TargetActor) return;

    // 4) Actor 컨텍스트로 추가
    UEnvQueryItemType_Actor::SetContextHelper(ContextData, TargetActor);

}