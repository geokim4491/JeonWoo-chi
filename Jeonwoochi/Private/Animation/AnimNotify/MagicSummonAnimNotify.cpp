// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/MagicSummonAnimNotify.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Weapon/MagicSummonActor.h"
#include "Components/Combat/AttackComponent.h"

namespace
{
	static bool ResolveBBLocation(APawn* Pawn, FTransform& OutLoc)
	{
		if (!Pawn) return false;
		if (AAIController* AIC = Cast<AAIController>(Pawn->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				if (AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor"))))
				{
					OutLoc = TargetActor->GetActorTransform();
					return true;
				}
			}
		}
		return false;
	}

	static FTransform GetSelfAnchorTM(USkeletalMeshComponent* MeshComp, FName SocketName)
	{
		if (!MeshComp) return FTransform::Identity;
		if (SocketName != NAME_None && MeshComp->DoesSocketExist(SocketName))
		{
			return MeshComp->GetSocketTransform(SocketName);
		}
		return MeshComp->GetComponentTransform();
	}
}

UMagicSummonAnimNotify::UMagicSummonAnimNotify()
{

}

void UMagicSummonAnimNotify::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	AActor* Owner = MeshComp->GetOwner();
	if (!World || !Owner) return;

	APawn* AsPawn = Cast<APawn>(Owner);

	// 1) 앵커 트랜스폼
	FTransform AnchorTM = FTransform::Identity;
	switch (Anchor)
	{
	case ESummonAnchor::Self:
		AnchorTM = GetSelfAnchorTM(MeshComp, SelfSocketName);
		break;
	case ESummonAnchor::Target:
		if (!ResolveBBLocation(AsPawn, AnchorTM))
		{
			// 타깃 없으면 실패 처리(필요 시 Self로 폴백해도 됨)
			return;
		}
		break;
	default:
		break;
	}

	// 2) 최종 스폰 트랜스폼 = LocalOffset * AnchorTM (로컬 오프셋 후 기준 트랜스폼)
	const FTransform FinalTM = LocalOffset * AnchorTM;

	// 3) 데미지 설정
	if (auto* AC = Owner->FindComponentByClass<UAttackComponent>())
	{
		AC->SetPendingDamageInfo(DamageInfo);
	}

	// 3) 스폰
	FActorSpawnParameters Params;
	Params.Owner = Owner;
	Params.Instigator = AsPawn;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Spawned = World->SpawnActor<AActor>(SummonClass, FinalTM, Params);
	if (!Spawned) return;

}