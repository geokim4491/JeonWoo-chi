// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask/Attack/BTTask_Attack_MontageOnly.h"
#include "AI/AIController/BaseAIController.h"
#include "Characters/AI/BaseAICharacter.h"
#include "Animation/AnimInstance.h"
#include "MotionWarpingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/Combat/AttackComponent.h"

UBTTask_Attack_MontageOnly::UBTTask_Attack_MontageOnly()
{
	NodeName = TEXT("Montage Play(Attack) Task");
	bNotifyTick = true;    // TickTask를 활성화
	AttackMontage = nullptr;
	WaitTime = 0.f;
	bUseMotionWarping = false;
	WarpTargetName = FName("DefaultWarp");
}

EBTNodeResult::Type UBTTask_Attack_MontageOnly::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    // AIController와 Pawn 체크
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    ABaseAICharacter* Char = Cast<ABaseAICharacter>(AICon->GetPawn());
    if (!Char || !AttackMontage)
        return EBTNodeResult::Failed;

    // (옵션) Motion Warping 설정
    if (bUseMotionWarping)
    {
        UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
        FVector TargetLoc = BB
            ? BB->GetValueAsVector(TargetLocationKey.SelectedKeyName)
            : Char->GetActorLocation();

        if (UMotionWarpingComponent* WarpComp =
            Char->FindComponentByClass<UMotionWarpingComponent>())
        {
            WarpComp->AddOrUpdateWarpTargetFromLocation(
                WarpTargetName, TargetLoc);
        }
    }

    if (UAttackComponent* AC = Char->FindComponentByClass<UAttackComponent>()) {
        FDamageInfoStruct Info{};
        Info.Montage = AttackMontage;
        Info.bForceFlyingRootMotion = bForceFlyingRootMotion; // 컴포넌트가 처리
        AC->Attack(Info);                                     // ← 단일 진입점
    }
    else {
        return EBTNodeResult::Failed;
    }

    // 메모리 초기화
    FAttackMontageOnlyTaskMemory* Mem = reinterpret_cast<FAttackMontageOnlyTaskMemory*>(NodeMemory);
    Mem->ElapsedTime = 0.0f;

    // InProgress로 두고 TickTask에서 대기 시간 체크
    return EBTNodeResult::InProgress;
}

void UBTTask_Attack_MontageOnly::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    FAttackMontageOnlyTaskMemory* Mem = reinterpret_cast<FAttackMontageOnlyTaskMemory*>(NodeMemory);
    Mem->ElapsedTime += DeltaSeconds;

    // 지정된 대기 시간 경과 시
    if (Mem->ElapsedTime >= WaitTime)
    {
        // 태스크 종료 (Succeeded 또는 필요 시 Failed)
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}