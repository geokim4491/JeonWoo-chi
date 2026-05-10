// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask/Attack/BTTask_Attack.h"
#include "AI/AIController/BaseAIController.h"
#include "Characters/AI/BaseAICharacter.h"
#include "MotionWarpingComponent.h"

#include "Interface/Combat/AttackInterface.h"

UBTTask_Attack::UBTTask_Attack()
{
    NodeName = TEXT("Attack Task");
    bNotifyTick = true;    // TickTask를 활성화
    WaitTime = 0.f;   // 기본 대기 없음
    bUseMotionWarping = false; // MotionWarp 사용시 true로 설정
    WarpTargetName = FName("DefaultWarp");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    // AIController와 Pawn 체크
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    ABaseAICharacter* Char = Cast<ABaseAICharacter>(AICon->GetPawn());
    if (!Char || !DamageInfo.Montage)
        return EBTNodeResult::Failed;

    // (옵션) Motion Warping 설정
    if (bUseMotionWarping)
    {
        UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
        AActor* TargetActor = nullptr;
        if (BB)
        {
            UObject* Obj = BB->GetValueAsObject(TargetActorKey.SelectedKeyName);
            if (!Obj)
            {
                UE_LOG(
                    LogTemp, Warning,
                    TEXT("[BTTask] Blackboard Key '%s' 에 유효한 Actor가 없습니다."),
                    *TargetActorKey.SelectedKeyName.ToString());
                return EBTNodeResult::Failed;

            }
            TargetActor = Cast<AActor>(Obj);
        }

        // 2) Actor 위치가 유효하면 그 위치, 아니면 자기 위치
        FVector TargetLoc = TargetActor
            ? TargetActor->GetActorLocation()
            : Char->GetActorLocation();

        if (UMotionWarpingComponent* WarpComp =
            Char->FindComponentByClass<UMotionWarpingComponent>())
        {
            WarpComp->AddOrUpdateWarpTargetFromLocation(
                WarpTargetName,
                TargetLoc
            );
        }

    }

    // Attack
    Char->Attack(DamageInfo);

    // 메모리 초기화
    FAttackTaskMemory* Mem = reinterpret_cast<FAttackTaskMemory*>(NodeMemory);
    Mem->ElapsedTime = 0.0f;

    // InProgress로 두고 TickTask에서 대기 시간 체크
    return EBTNodeResult::InProgress;
}

void UBTTask_Attack::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    FAttackTaskMemory* Mem = reinterpret_cast<FAttackTaskMemory*>(NodeMemory);
    Mem->ElapsedTime += DeltaSeconds;

    // 지정된 대기 시간 경과 시
    if (Mem->ElapsedTime >= WaitTime)
    {
        // 태스크 종료 (Succeeded 또는 필요 시 Failed)
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}