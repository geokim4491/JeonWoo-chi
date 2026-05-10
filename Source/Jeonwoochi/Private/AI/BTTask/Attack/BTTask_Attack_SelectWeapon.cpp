// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask/Attack/BTTask_Attack_SelectWeapon.h"

#include "AI/AIController/BaseAIController.h"
#include "Characters/AI/BaseAICharacter.h"

#include "Weapon/BaseWeaponActor.h"
#include "Components/Combat/HitDetectionComponent.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "MotionWarpingComponent.h"
#include "Components/BoxComponent.h"

UBTTask_Attack_SelectWeapon::UBTTask_Attack_SelectWeapon()
{
	NodeName = TEXT("Attack Task (Select Weapon)");
	bNotifyTick = true;
}

TSet<FName> UBTTask_Attack_SelectWeapon::NormalizeIDs(const TArray<FName>& In)
{
	TSet<FName> Out;
	for (const FName& N : In)
	{
		if (!N.IsNone())
		{
			Out.Add(N);
		}
	}
	return Out;
}

void UBTTask_Attack_SelectWeapon::CollectWeaponsAndSnapshot(ABaseAICharacter* Char, FAttackSelectWeaponMemory* Mem) const
{
	Mem->Snapshots.Reset();

	TArray<AActor*> Attached;
	Char->GetAttachedActors(Attached);

	for (AActor* A : Attached)
	{
		if (ABaseWeaponActor* W = Cast<ABaseWeaponActor>(A))
		{
			FWeaponVisSnapshot S;
			S.Weapon = W;

			// 이전 가시성 상태
			S.bPrevHidden = W->IsHidden();

			// 이전 히트박스 충돌 상태
			if (UHitDetectionComponent* HDC = W->GetHitDetectionComp())
			{
				if (HDC->HitShape)
				{
					S.PrevCollision = HDC->HitShape->GetCollisionEnabled();
				}
			}

			Mem->Snapshots.Add(S);
		}
	}
}

TArray<TWeakObjectPtr<ABaseWeaponActor>> UBTTask_Attack_SelectWeapon::PickByWeaponIDs(
	const TArray<FWeaponVisSnapshot>& Snapshots,
	const TSet<FName>& IDs) const
{
	TArray<TWeakObjectPtr<ABaseWeaponActor>> Result;
	if (IDs.Num() == 0) return Result;

	for (const FWeaponVisSnapshot& S : Snapshots)
	{
		if (ABaseWeaponActor* W = S.Weapon.Get())
		{
			if (IDs.Contains(W->WeaponID)) // 여러 ID 중 하나라도 매치되면 선택
			{
				Result.Add(W);
			}
		}
	}
	return Result;
}


void UBTTask_Attack_SelectWeapon::ApplySelection(
	const TArray<FWeaponVisSnapshot>& Snapshots,
	const TArray<TWeakObjectPtr<ABaseWeaponActor>>& SelectedSet) const
{
	// 빠른 포함 체크
	TSet<const ABaseWeaponActor*> SelectedFast;
	for (const auto& WPtr : SelectedSet)
		if (const ABaseWeaponActor* W = WPtr.Get())
			SelectedFast.Add(W);

	// 0) 전부 숨김 + 전부 히트박스 OFF
	for (const FWeaponVisSnapshot& S : Snapshots)
	{
		if (ABaseWeaponActor* W = S.Weapon.Get())
		{
			// 가시성
			W->SetActorHiddenInGame(true);
		}
	}

	// 1) 선택된 무기만 보이기 + 히트박스 ON
	for (const auto& WPtr : SelectedSet)
	{
		if (ABaseWeaponActor* W = WPtr.Get())
		{
			// 가시성
			W->SetActorHiddenInGame(false);
		}
	}
}

void UBTTask_Attack_SelectWeapon::Restore(const TArray<FWeaponVisSnapshot>& Snapshots) const
{
	/*
	for (const FWeaponVisSnapshot& S : Snapshots)
	{
		if (ABaseWeaponActor* W = S.Weapon.Get())
		{
			// 가시성 복구 -> 이전에 가렸던 애들.
			W->SetActorHiddenInGame(S.bPrevHidden);

			// 히트박스 충돌 복구
			if (UHitDetectionComponent* HDC = W->GetHitDetectionComp())
			{
				if (HDC->HitShape)
				{`
					HDC->HitShape->SetCollisionEnabled(S.PrevCollision);
				}
			}
		}
	}
	*/
}

EBTNodeResult::Type UBTTask_Attack_SelectWeapon::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	// 기본 전제 체크
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	ABaseAICharacter* Char = Cast<ABaseAICharacter>(AICon->GetPawn());
	if (!Char || !DamageInfo.Montage)
		return EBTNodeResult::Failed;

	FAttackSelectWeaponMemory* Mem = reinterpret_cast<FAttackSelectWeaponMemory*>(NodeMemory);
	Mem->ElapsedTime = 0.f;

	// 0) Motion Warping (부모 태스크 로직 그대로)
	if (bUseMotionWarping)
	{
		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		AActor* TargetActor = nullptr;
		if (BB)
		{
			UObject* Obj = BB->GetValueAsObject(TargetActorKey.SelectedKeyName);
			if (!Obj)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[BTTask_Attack_SelectWeapon] Blackboard Key '%s' 에 유효한 Actor가 없습니다."),
					*TargetActorKey.SelectedKeyName.ToString());
				return EBTNodeResult::Failed;
			}
			TargetActor = Cast<AActor>(Obj);
		}

		const FVector TargetLoc = TargetActor ? TargetActor->GetActorLocation() : Char->GetActorLocation();

		if (UMotionWarpingComponent* WarpComp = Char->FindComponentByClass<UMotionWarpingComponent>())
		{
			WarpComp->AddOrUpdateWarpTargetFromLocation(WarpTargetName, TargetLoc);
		}
	}

	// 1) 무기 수집 + 스냅샷
	CollectWeaponsAndSnapshot(Char, Mem);

	// 2) 선택
	const TSet<FName> IDs = NormalizeIDs(DesiredWeaponIDs);
	TArray<TWeakObjectPtr<ABaseWeaponActor>> Selected = PickByWeaponIDs(Mem->Snapshots, IDs);
	
	Mem->SelectedSet = Selected;

	if (Selected.Num() == 0)
	{
		// 로깅용으로 CSV 문자열 생성
		TArray<FString> Names;
		Names.Reserve(DesiredWeaponIDs.Num());
		for (const FName& N : DesiredWeaponIDs) { Names.Add(N.ToString()); }

		UE_LOG(LogTemp, Warning,
			TEXT("[BTTask_Attack_SelectWeapon] 지정한 WeaponIDs(%s) 중 일치하는 무기를 찾지 못했습니다."),
			*FString::Join(Names, TEXT(",")));

		if (bFailIfNotFound)
		{
			return EBTNodeResult::Failed;
		}
		// 못 찾았지만 실패시키지 않을 거면 그냥 진행(아무것도 보이지 않게 유지)
	}
	else
	{
		// “전부 숨김 → 선택한 ID들만 보임 + HitShape ON”
		ApplySelection(Mem->Snapshots, Selected);
	}

	// 4) Attack 실행
	Char->Attack(DamageInfo);

	// 5) InProgress → Tick에서 WaitTime 경과 후 종료/복구
	return EBTNodeResult::InProgress;
}

void UBTTask_Attack_SelectWeapon::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	FAttackSelectWeaponMemory* Mem = reinterpret_cast<FAttackSelectWeaponMemory*>(NodeMemory);
	Mem->ElapsedTime += DeltaSeconds;

	if (Mem->ElapsedTime >= WaitTime)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_Attack_SelectWeapon::AbortTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	FAttackSelectWeaponMemory* Mem = reinterpret_cast<FAttackSelectWeaponMemory*>(NodeMemory);
	return EBTNodeResult::Aborted;
}