// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/ThrowAttackAnimNotify.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/Combat/AttackComponent.h"
#include "Weapon/ProjectileActor.h" // 투사체

namespace
{
	static bool ResolveBBLocation(APawn* Pawn, FVector& OutLoc)
	{
		if (!Pawn) return false;
		if (AAIController* AIC = Cast<AAIController>(Pawn->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				if (AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor"))))
				{
					OutLoc = TargetActor->GetActorLocation();
					return true;
				}
			}
		}
		return false;
	}

	static FTransform GetBaseSpawnTransform(USkeletalMeshComponent* MeshComp, FName SocketName)
	{
		if (!MeshComp) return FTransform::Identity;
		if (SocketName != NAME_None && MeshComp->DoesSocketExist(SocketName))
		{
			return MeshComp->GetSocketTransform(SocketName);
		}
		return MeshComp->GetComponentTransform();
	}
}

UThrowAttackAnimNotify::UThrowAttackAnimNotify()
{
}

void UThrowAttackAnimNotify::Notify(
	USkeletalMeshComponent* MeshComp, UAnimSequenceBase* /*Animation*/)
{
	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	AActor* Owner = MeshComp->GetOwner();
	if (!World || !Owner) return;

	APawn* AsPawn = Cast<APawn>(Owner);
	if (!ProjectileClass) return;

	// 1) 스폰 기준 트랜스폼
	const FTransform BaseTM = GetBaseSpawnTransform(MeshComp, SpawnSocketName);
	const FTransform SpawnTM = SpawnOffset * BaseTM; // 로컬 오프셋 → 기준 트랜스폼

	FVector SpawnLoc = SpawnTM.GetLocation();
	FRotator SpawnRot = SpawnTM.Rotator();
	FVector SpawnScale = SpawnTM.GetScale3D();

	// 2) 타깃 위치 해석
	FVector TargetLoc = FVector::ZeroVector;
	const bool bHasTarget = ResolveBBLocation(AsPawn, TargetLoc);

	// 3) 방향 계산
	FVector Dir = FVector::ForwardVector;
	if (bHasTarget)
	{
		Dir = (TargetLoc - SpawnLoc).GetSafeNormal();
	}
	else
	{
		// 타깃 없으면 전방
		Dir = Owner->GetActorForwardVector().GetSafeNormal();
	}

	// 4) 회전
	if (bAlignRotationToDirection)
	{
		SpawnRot = Dir.Rotation();
	}
	SpawnRot += AdditionalRotation;

	// 5) 데미지 설정
	if (auto* AC = Owner->FindComponentByClass<UAttackComponent>())
	{
		AC->SetPendingDamageInfo(DamageInfo);
	}

	// 6) 스폰
	FActorSpawnParameters Params;
	Params.Owner = Owner;
	Params.Instigator = AsPawn;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


	AProjectileActor* Proj = World->SpawnActor<AProjectileActor>
		(ProjectileClass, FTransform(SpawnRot, SpawnLoc, SpawnScale), Params);
	
	if (!Proj) return;

	// 7) 속도 설정 + 발사
	if (bOverrideProjectileSpeed)
	{
		Proj->SetProjectileSpeed(InitialSpeed, MaxSpeed);
	}
	Proj->InitVelocity(Dir);

	// 2) 회전은 별도(추가 회전 반영)
	if (Proj->ProjectileMovement)
	{
		Proj->ProjectileMovement->bRotationFollowsVelocity = false; // 회전 독립
	}
	const FRotator VisualRot = (bAlignRotationToDirection ? Dir.Rotation() : SpawnRot) + AdditionalRotation;
	Proj->SetActorRotation(VisualRot.GetNormalized());
}