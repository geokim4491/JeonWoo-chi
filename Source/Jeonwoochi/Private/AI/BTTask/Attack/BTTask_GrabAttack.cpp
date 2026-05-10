// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask/Attack/BTTask_GrabAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Combat/AttackComponent.h"

UBTTask_GrabAttack::UBTTask_GrabAttack()
{
	NodeName = TEXT("Grab Attack");
    bCreateNodeInstance = true;

    /** Default AttackType is Grab and Dangerous */
    GrabInfo.AttackType = EAttackType::GrabAttack;
    GrabInfo.AttackCategory = EAttackCategory::DangerousAttack;
}

EBTNodeResult::Type UBTTask_GrabAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    BlackBoardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackBoardComp)
    {
        return EBTNodeResult::Failed;
    }

    MyOwnerComp = &OwnerComp;
    // AIController와 Pawn 체크
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    EnemyChar = Cast<ABaseAICharacter>(AICon->GetPawn());
    if (!EnemyChar || !DotDamageInfo.DamageInfo.Montage || !GrabInfo.Montage)
        return EBTNodeResult::Failed;

    UAnimInstance* AnimInst = EnemyChar->GetMesh()->GetAnimInstance();
    if (!AnimInst)
        return EBTNodeResult::Failed;

    if (auto AttackComp = EnemyChar->FindComponentByClass<UAttackComponent>())
    {
        AttackComp->SetGrabSuccess(false);
    }
    
    EnemyChar->Attack(GrabInfo);

    GrabEndDelegate.BindUObject(
        this, &UBTTask_GrabAttack::OnGrabMontageEnded);
    AnimInst->Montage_SetEndDelegate(GrabEndDelegate, GrabInfo.Montage);
    
    return EBTNodeResult::InProgress;
}

void UBTTask_GrabAttack::OnGrabMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != GrabInfo.Montage) return;

    UAnimInstance* AnimInst = EnemyChar->GetMesh()->GetAnimInstance();
    if (!AnimInst) return;

    // 델리게이트 해제
    EmptyDelegate.Unbind();
    AnimInst->Montage_SetEndDelegate(EmptyDelegate, GrabInfo.Montage);

    if (auto AttackComp = EnemyChar->FindComponentByClass<UAttackComponent>())
    {
        if (AttackComp->GetGrabSuccess())
        {
            UObject* Obj = BlackBoardComp->GetValueAsObject(TargetActorKey.SelectedKeyName);
            AActor* Target = Cast<AActor>(Obj);

            DotDamageInfo.TicksRemaining = FMath::CeilToInt(
                DotDamageInfo.TotalDuration / DotDamageInfo.Interval);
            
            EnemyChar->DotAttack(DotDamageInfo, Target);

            // 공격 몽타주 끝날 때까지 기다리도록 델리게이트 설정
            AttackEndDelegate.BindUObject(this, &UBTTask_GrabAttack::OnAttackMontageEnded);
            AnimInst->Montage_SetEndDelegate(
                AttackEndDelegate, DotDamageInfo.DamageInfo.Montage);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("GrabFail"));
            // 잡기 실패 → 바로 Task 종료
            FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
        }
    }
}

void UBTTask_GrabAttack::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != DotDamageInfo.DamageInfo.Montage) return;

    // 델리게이트 해제
    UAnimInstance* AnimInst = EnemyChar->GetMesh()->GetAnimInstance();
    if (AnimInst)
    {
        EmptyDelegate.Unbind();
        AnimInst->Montage_SetEndDelegate(EmptyDelegate, DotDamageInfo.DamageInfo.Montage);
    }

    // 공격 애니 끝나면 Task 종료
    FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
}

void UBTTask_GrabAttack::OnTaskFinished(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    EBTNodeResult::Type TaskResult
)
{
    if (UAnimInstance* AnimInst = EnemyChar->GetMesh()->GetAnimInstance())
    {
        EmptyDelegate.Unbind();
        AnimInst->Montage_SetEndDelegate(EmptyDelegate, GrabInfo.Montage);
        AnimInst->Montage_SetEndDelegate(EmptyDelegate, DotDamageInfo.DamageInfo.Montage);
    }

    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}