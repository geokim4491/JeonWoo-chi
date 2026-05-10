// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask/Attack/BTTask_SpawnAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/AIController/BaseAIController.h"
#include "Characters/AI/BaseAICharacter.h"
#include "Weapon/ProjectileActor.h"

UBTTask_SpawnAttack::UBTTask_SpawnAttack()
{
    NodeName = TEXT("Spawn Attack Task");
    bNotifyTick = true;    // TickTask를 활성화
    WaitTime = 0.f;   // 기본 대기 없음
    ShotTime = 0.f;
}

EBTNodeResult::Type UBTTask_SpawnAttack::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    // 캐릭터·AI 체크
    AAIController* AICon = OwnerComp.GetAIOwner();
    ABaseAICharacter* Char = AICon ? Cast<ABaseAICharacter>(AICon->GetPawn()) : nullptr;
    if (!Char) return EBTNodeResult::Failed;

    // (선택) 몽타주·공격 정보 실행
    Char->Attack(DamageInfo);

    // 경과시간 초기화
    auto* Mem = reinterpret_cast<FSpawnAttackTaskMemory*>(NodeMemory);
    Mem->ElapsedTime = 0.f;
    Mem->bProjectileShot = false;

    // ShotTime이 0이면 즉시 발사할 수도 있지만,
    // TickTask에서 조건문 하나로 처리하도록 InProgress 반환
    return EBTNodeResult::InProgress;
}

/** 매 프레임 호출 */
void UBTTask_SpawnAttack::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    auto* Mem = reinterpret_cast<FSpawnAttackTaskMemory*>(NodeMemory);
    Mem->ElapsedTime += DeltaSeconds;

    AAIController* AICon = OwnerComp.GetAIOwner();
    ABaseAICharacter* Char = AICon ? Cast<ABaseAICharacter>(AICon->GetPawn()) : nullptr;
    if (!Char)                                   // 캐릭터가 사라지면 실패 처리
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    /** 1) ShotTime 경과 && 아직 발사 전 -> 투사체 스폰 */
    if (!Mem->bProjectileShot && Mem->ElapsedTime >= ShotTime)
    {
        // 타깃 확보
        AActor* Target = Cast<AActor>(
            OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
        if (!Target)      // 타깃이 없으면 실패
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }

        // 스폰 위치·방향 계산
        const FVector SpawnLoc = Char->GetActorLocation() + Char->GetActorForwardVector() * 100.f;
        const FVector Dir = (Target->GetActorLocation() - SpawnLoc).GetSafeNormal();

        // 스폰
        FActorSpawnParameters Params;
        Params.Owner = Char;
        Params.Instigator = Char;

        if (AProjectileActor* Proj = GetWorld()->SpawnActor<AProjectileActor>(
            ProjectileClass, SpawnLoc, Dir.Rotation(), Params))
        {
            Proj->InitVelocity(Dir);
        }

        Mem->bProjectileShot = true;
    }

    /** 2) WaitTime까지 지난 뒤 태스크 종료 */
    if (Mem->ElapsedTime >= WaitTime)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}