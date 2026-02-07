// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.h"
#include "../../Structure/Combat/DamageInfoStruct.h"
#include "PlayerAttackComponent.generated.h"


UENUM(BlueprintType)
enum class EPlayerAttackType : uint8
{
	FastAttack        UMETA(DisplayName = "FastAttack"),
	StrongAttack      UMETA(DisplayName = "StrongAttack"),
	SkillAttack1        UMETA(DisplayName = "SkillAttack1"),
	SkillAttack2      UMETA(DisplayName = "SkillAttack2"),
	AirAttack         UMETA(DisplayName = "AirAttack"),
	DashAttack        UMETA(DisplayName = "DashAttack"),
	Execution         UMETA(DisplayName = "Execution")
	/* ���� ��ų �߰� */
};

USTRUCT(BlueprintType)
struct FAttackInfo
{	
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, Category = "AttackInfo")
	TArray<FDamageInfoStruct> ComboSequence;
};



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class JEONWOOCHI_API UPlayerAttackComponent : public UAttackComponent 
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerAttackComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void FastAttack() { DoAttack(EPlayerAttackType::FastAttack); }

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void StrongAttack() { DoAttack(EPlayerAttackType::StrongAttack); }

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void SkillAttack1() { DoAttack(EPlayerAttackType::SkillAttack1); }

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void SkillAttack2() { DoAttack(EPlayerAttackType::SkillAttack2); }

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void AirAttack() { DoAttack(EPlayerAttackType::AirAttack); }

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void DashAttack() { DoAttack(EPlayerAttackType::DashAttack); }

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void Execution() { DoAttack(EPlayerAttackType::Execution); }


	/** ���ݿ� ����� ���� Ÿ�Ժ� FDamageInfoStruct �迭 (�����Ϳ��� ����) */
	UPROPERTY(EditAnywhere, Category = "AttackInfo")
	TMap<EPlayerAttackType, FAttackInfo> AttackInfos;

	UPROPERTY()
	bool bCanAttack;

	UFUNCTION()
	FDamageInfoStruct GetCurrentDamageInfo(EPlayerAttackType AttackType);

	UFUNCTION()
	void ResetAllCombos();

protected:
	virtual void ResetComboForMoveCancel() override;
		
private:

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	bool JudgementCanAttack();

	void DoAttack(EPlayerAttackType AttackType);


	TMap<EPlayerAttackType, int32> ComboIndexByType;
	FTimerHandle CooldownHandle;
};
