// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/AttackComponent.h"

#include "Enum/Combat/AttackTypeEnum.h"
#include "Characters/BaseCharacter.h"
#include "Enum/CharacterTypeEnum.h"
#include "Interface/Combat/HealthInterface.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UAttackComponent::UAttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bIsAttacking = false;
	CachedChar = Cast<ABaseCharacter>(GetOwner());
}


// Called when the game starts
void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	if (CachedChar) 
	{
		AnimInst = CachedChar->GetMesh()->GetAnimInstance();
		CachedMove = CachedChar->GetCharacterMovement();

		if (AnimInst) 
		{
			// 멀티캐스트 델리게이트에 한 번만 연결
			AnimInst->OnMontageBlendingOut.AddDynamic(
				this, &UAttackComponent::OnMontageBlendOut);
			AnimInst->OnMontageEnded.AddDynamic(
				this, &UAttackComponent::OnMontageBlendOut);
		}
	}
	
}

void UAttackComponent::SetAnimInstance(UAnimInstance* AnimInstance)
{
	if (AnimInstance)
	{
		AnimInst = AnimInstance;
	}
}

void UAttackComponent::SetPendingDamageInfo(const FDamageInfoStruct& DamageInfo)
{
	PendingDamageInfo = DamageInfo;
	PendingDamageInfo.Attacker = GetOwner();
}

void UAttackComponent::Notify_UnlockMovementEarly()
{
	bAllowMoveCancel = true;
}

bool UAttackComponent::TryCancelAttackByMoveInput(const FVector2D& Axis, float Threshold)
{
	if (!bAllowMoveCancel) return false;

	// 실제로 이동 의도가 있는지(데드존 체크)
	if (Axis.Size() < Threshold) return false;

	// 지금 AttackComponent는 AnimInst/CachedMontage를 이미 보관/델리게이트 바인딩 중임
	// → 현재 재생 중인 공격 몽타주만 짧게 블렌드아웃
	if (AnimInst)
	{
		// 특정 몽타주만 끊고 싶으면 CachedMontage 넘김, 아니면 전부 끊기(둘 중 하나 선택)
		if (CachedMontage)
		{
			AnimInst->Montage_Stop(0.15f, CachedMontage);  // 부드럽게 0.15s 블렌드아웃
		}
		else
		{
			AnimInst->Montage_Stop(0.15f);                 // 안전망: 현재 재생 몽타주들 정지
		}
	}

	ResetComboForMoveCancel();

	bAllowMoveCancel = false; // 중복 방지
	return true;
}

// Called every frame
void UAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UAttackComponent::Attack(const FDamageInfoStruct& DamageInfo)
{
	PendingDamageInfo = DamageInfo;
	PendingDamageInfo.Attacker = GetOwner();

	if (!AnimInst || !DamageInfo.Montage) return;

	EnsureAnimDelegates(CachedChar); // 항상 최신 인스턴스에 바인딩 보장

	const bool bIsAirborne = CachedMove.IsValid() ? CachedMove->IsFalling() : false;
	// const bool bUseFlying = DamageInfo.bForceFlyingRootMotion && !bIsAirborne;
	const bool bUseFlying = DamageInfo.bForceFlyingRootMotion;

	// 첫 진입에서만 모드 저장/전환
	if (bUseFlying && FlyingMontages.Num() == 0 && CachedMove.IsValid()) {
		SavedMovementMode = CachedMove->MovementMode;
		CachedMove->SetMovementMode(MOVE_Flying);
	}

	const float Len = AnimInst->Montage_Play(DamageInfo.Montage);
	CachedMontage = DamageInfo.Montage;
	if (CachedChar->GetCharacterType() != ECharacterType::Player)
	{
		bIsAttacking = true;
		UE_LOG(LogTemp, Warning, TEXT("%s's State: %s"), *GetNameSafe(CachedChar), bIsAttacking ? TEXT("Attacking") : TEXT("Not Attacking"));
	}
	if (Len <= 0.f) {
		// 재생 실패시 복구 시도
		if (bUseFlying && FlyingMontages.Num() == 0 && CachedMove.IsValid()) {
			CachedMove->SetMovementMode(SavedMovementMode);
		}
		return;
	}

	// 강제 Flying에 기여한 몽타주를 기록
	if (bUseFlying) {
		FlyingMontages.Add(DamageInfo.Montage);
	}
}

void UAttackComponent::OnMontageBlendOut(UAnimMontage* Montage, bool /*bInterrupted*/)
{
	if (Montage != CachedMontage) return;

	if (FlyingMontages.Remove(Montage) && FlyingMontages.Num() == 0) {
		if (CachedMove.IsValid()) {
			// 원래 모드가 Walking이었고, 현재 착지하지 않은 상태라면 Falling으로 전환
			if (SavedMovementMode == MOVE_Walking && !CachedMove->IsMovingOnGround()) {
				CachedMove->SetMovementMode(MOVE_Falling);
			}
			else {
				CachedMove->SetMovementMode(SavedMovementMode);
			}
		}
	}
	if (CachedChar->GetCharacterType() != ECharacterType::Player)
	{
		bIsAttacking = false; UE_LOG(LogTemp, Warning, TEXT("%s's State: %s"), *GetNameSafe(CachedChar), bIsAttacking ? TEXT("Attacking") : TEXT("Not Attacking"));
	}
}

void UAttackComponent::DotAttack(const FDotDealInfoStruct& DotInfo, AActor* Target)
{
	if (!Target) return;

	PendingDamageInfo = DotInfo.DamageInfo;

	PendingDamageInfo.Attacker = GetOwner();

	if (PendingDamageInfo.Montage)
	{
		AnimInst->Montage_Play(PendingDamageInfo.Montage);
	}
	
	CalculateDamage();

	if (Target->Implements<UHealthInterface>())
	{
		IHealthInterface::Execute_ApplyDotDamage(Target, DotInfo);
	}

}

void UAttackComponent::CalculateDamage() 
{
	// 공격력에 따른 설정
	float& FinalDamage = PendingDamageInfo.DamageAmount;

	FinalDamage = FinalDamage * 1.f;
}

void UAttackComponent::PerformGrab(AActor* Other)
{
	ECharacterType OtherCharType = ECharacterType::None;
	if (Other->Implements<UCharacterTypeInterface>())
	{
		OtherCharType =	ICharacterTypeInterface::Execute_GetCharacterType(Other);
	}

	switch (OtherCharType)
	{
	case ECharacterType::Player:
		
		/** first player input disable */
		// 미구현 플레이어 캐릭터에 입력 불가 함수 만들고 호출할 예정.

		/** bGrabSuccess */
		bGrabSuccess = true;

		break;
	case ECharacterType::Companion:
		break;
	case ECharacterType::NormalEnemy:
		break;
	case ECharacterType::Boss:
		break;
	}
}

void UAttackComponent::ResetComboForMoveCancel()
{
	// PlayerAttackCompo 에서 오버라이드
}

void UAttackComponent::HandleHit(AActor* Other, const FHitResult& Hit)
{
	switch (PendingDamageInfo.AttackType)
	{
	case EAttackType::NormalAttack:
		CalculateDamage();
		if (Other->Implements<UDamageableInterface>())
		{
			IDamageableInterface::Execute_TakeDamage(Other, PendingDamageInfo);
		}
		break;

	case EAttackType::GrabAttack:
		PerformGrab(Other);

	default:
		CalculateDamage();
		if (Other->Implements<UDamageableInterface>())
		{
			IDamageableInterface::Execute_TakeDamage(Other, PendingDamageInfo);
		}
		break;
	}
}

void UAttackComponent::SetGrabSuccess(bool bGrab) 
{
	bGrabSuccess = bGrab;
}

void UAttackComponent::EnsureAnimDelegates(ACharacter* Character)
{
	if (!Character || !Character->GetMesh()) return;
	UAnimInstance* NewAnim = Character->GetMesh()->GetAnimInstance();
	if (AnimInst == NewAnim) return;

	// 이전 인스턴스에서 제거
	if (AnimInst) {
		AnimInst->OnMontageBlendingOut.RemoveDynamic(this, &UAttackComponent::OnMontageBlendOut);
		AnimInst->OnMontageEnded.RemoveDynamic(this, &UAttackComponent::OnMontageBlendOut);
	}
	// 교체 + 재바인딩
	AnimInst = NewAnim;
	if (AnimInst) {
		AnimInst->OnMontageBlendingOut.AddDynamic(this, &UAttackComponent::OnMontageBlendOut);
		AnimInst->OnMontageEnded.AddDynamic(this, &UAttackComponent::OnMontageBlendOut);
	}
}