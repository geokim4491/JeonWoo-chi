// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlayerSkills/BlinkComponent.h"
#include "Animation/AnimNotifyState/BlinkWindowNotifyState.h"
#include "Engine/StaticMeshActor.h"
#include "../../../JeonwoochiCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/Combat/AttackComponent.h"
#include "MotionWarpingComponent.h"
#include "Characters/AI/BaseAICharacter.h"
#include "Components/Utilities/CameraActionRouterComponent.h"
#include "Components/Combat/PlayerDefensiveComponent.h"

// Sets default values for this component's properties
UBlinkComponent::UBlinkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UBlinkComponent::BeginPlay()
{
	Super::BeginPlay();

	UBlinkWindowNotifyState::Bind(
		FBlinkWindowDelegate::CreateUObject(this, &UBlinkComponent::SetBlinkMode)
	);
	
	OwnerChar = Cast<AJeonwoochiCharacter>(GetOwner());
	CachedAttackComp = OwnerChar->FindComponentByClass<UAttackComponent>();
	WarpComp = OwnerChar->FindComponentByClass<UMotionWarpingComponent>();
}

void UBlinkComponent::Blink()
{
	if (!bCanBlink || !BlinkTarget || !OwnerChar) return;

	bIsBlink = true;

	if (auto* Def = OwnerChar->GetPlayerDefensiveComponent())
		Def->SetDefenseState(EPlayerDefenseState::Invincible);
	
	OwnerChar->ClearAllIMC();

	// 이동 완전 정지(루트모션만 쓰고 싶다면 DisableMovement가 깔끔)
	OwnerChar->GetCharacterMovement()->StopMovementImmediately();

	FVector WarpLoc = BlinkTarget->GetActorLocation();
	FVector ForLoc = BlinkTarget->GetActorForwardVector();
	WarpLoc += ForLoc * 300.f;

	// 연출을 위해 살짝 뒤로 보냄
	OwnerChar->SetActorLocation(WarpLoc);

	SpawnWoodLog();

	// 블링크는 카메라 액션 안쓴게 오히려 괜찮음.
	/*
	if (auto* Router = OwnerChar->FindComponentByClass<UCameraActionRouterComponent>())
	{
		Router->PlayAction(TEXT("Blink"));
	}
	*/

	WarpComp->AddOrUpdateWarpTargetFromLocation(
		TEXT("Target"),
		WarpLoc - ForLoc*100.f
	);

	if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
	{
		Anim->OnMontageEnded.AddDynamic(this, &UBlinkComponent::OnBlinkMontageEnded);
		float PlayedLen = OwnerChar->PlayAnimMontage(BlinkMontage);
		CachedAttackComp->Attack(DamageInfo);
		if (PlayedLen <= 0.f)
		{
			// 재생 실패 시 즉시 복구
			OnBlinkMontageEnded(BlinkMontage, /*bInterrupted*/true);
		}
	}

	if (ABaseAICharacter* EnemyChar = Cast<ABaseAICharacter>(BlinkTarget))
	{
		EnemyChar->FindComponentByClass<UAttackComponent>()->bIsAttacking = false;
		EnemyChar->StopBT();
	}
}

void UBlinkComponent::SetBlinkMode(AActor* Attacker, bool bOpen)
{
	bCanBlink = bOpen;
	

	if (bOpen)
	{
		BlinkTarget = Attacker;
		OwnerChar->ApplyIMC_Blink();
	}
	else
	{
		if(!bIsBlink)
			OwnerChar->ApplyIMC_Default();
	}
}

void UBlinkComponent::SpawnWoodLog()
{
	if (!SpawnMesh) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FTransform SpawnTF = GetOwner()->GetActorTransform();
	SpawnTF.SetScale3D(FVector(1.5f, 1.5f, 1.5f));
	FQuat RollQuat = FQuat(FVector::ForwardVector, FMath::DegreesToRadians(90.f));
	SpawnTF.SetRotation(RollQuat * SpawnTF.GetRotation());

	AStaticMeshActor* SMA = GetWorld()->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(), SpawnTF, Params);

	if (!SMA) return;

	UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent();
	SMC->SetMobility(EComponentMobility::Movable);
	SMC->SetStaticMesh(SpawnMesh);

	SMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SMC->SetSimulatePhysics(true);
	SMC->SetEnableGravity(true);

	SMC->SetCollisionResponseToAllChannels(ECR_Ignore);
	SMC->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SMC->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

	SMC->WakeAllRigidBodies();

	SMA->SetLifeSpan(2.f);
}

void UBlinkComponent::OnBlinkMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == BlinkMontage)
	{
		if (UAnimInstance* Anim = OwnerChar->GetMesh()->GetAnimInstance())
		{
			Anim->OnMontageEnded.RemoveDynamic(this, &UBlinkComponent::OnBlinkMontageEnded);
		}

		if (auto* Def = OwnerChar->GetPlayerDefensiveComponent())
		{
			Def->SetDefenseState(EPlayerDefenseState::None);
		}
		// 2) 입력 복구
		OwnerChar->ApplyIMC_Default();
		bIsBlink = false;

		if (ABaseAICharacter* EnemyChar = Cast<ABaseAICharacter>(BlinkTarget))
		{
			EnemyChar->RestartBT();
		}
	}
}

void UBlinkComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UBlinkWindowNotifyState::Unbind();

	Super::EndPlay(EndPlayReason);
}