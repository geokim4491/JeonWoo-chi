// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structure/Combat/DamageInfoStruct.h"
#include "Structure/Combat/DotDealInfoStruct.h"

#include "Interface/Combat/DamageableInterface.h"
#include "AttackComponent.generated.h"

class ABaseCharacter;
class UCharacterMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttackComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 공격력 등에 따른 설정
	void CalculateDamage();

	void PerformGrab(AActor* Other);
	
	/** 방근 공격한 공격 정보를 임시 저장 */
	UPROPERTY()
	FDamageInfoStruct PendingDamageInfo;

	TObjectPtr<UAnimInstance> AnimInst;

	virtual void ResetComboForMoveCancel();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Animation만 재생함 */
	void Attack(const FDamageInfoStruct& DamageInfo);

	/** DamageableComp 체크없이 바로 데미지 적용함에 유의! */
	UFUNCTION()
	void DotAttack(const FDotDealInfoStruct& DotInfo, AActor* Target);

	UFUNCTION()
	void HandleHit(AActor* Other, const FHitResult& Hit);

	UFUNCTION()
	bool GetGrabSuccess() const { return bGrabSuccess; }

	UFUNCTION()
	void SetGrabSuccess(bool bGrab);

	UPROPERTY()
	bool bIsAttacking;

	UFUNCTION()
	void SetAnimInstance(UAnimInstance* AnimInstance);

	UFUNCTION()
	void SetPendingDamageInfo(const FDamageInfoStruct& DamageInfo);

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void Notify_UnlockMovementEarly();    // 노티파이에서 호출

	bool TryCancelAttackByMoveInput(const FVector2D& Axis, float Threshold = 0.1f);

private:
	bool bAllowMoveCancel = false;
	bool bGrabSuccess;
	ABaseCharacter* CachedChar = nullptr;
	UAnimMontage* CachedMontage = nullptr;

	UPROPERTY() TSet<TWeakObjectPtr<UAnimMontage>> FlyingMontages;
	UPROPERTY() TWeakObjectPtr<UCharacterMovementComponent> CachedMove;
	TEnumAsByte<EMovementMode> SavedMovementMode = MOVE_Walking;

	UFUNCTION() void OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);

	void EnsureAnimDelegates(ACharacter* Character);

};
