// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIController/FinalBossAIController.h"
#include "Characters/AI/Boss/FinalBossCharacter.h"

void AFinalBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

    if (auto* Boss = Cast<AFinalBossCharacter>(InPawn))
    {
        Boss->StartCombatTypeLoop(); // BB 준비 완료 상태에서 시작
    }

}