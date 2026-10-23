// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIController/AIController2D.h"
#include "BehaviorTree/BlackboardComponent.h"

AAIController2D::AAIController2D()
{
}

void AAIController2D::BeginPlay()
{
    Super::BeginPlay();

    if (MainBT)
    {
        RunBehaviorTree(MainBT);
    }
}

void AAIController2D::SetTargetActor(AActor* Actor)
{
    if (auto* BB = GetBlackboardComponent())
    {
        BB->SetValueAsObject(TEXT("TargetActor"), Actor);
        UE_LOG(LogTemp, Warning, TEXT("Set Target Actor: %s"), *Actor->GetName());
    }
}