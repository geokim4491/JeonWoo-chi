// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseAnimInstance.generated.h"

class ULockOnComponent;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UBaseAnimInstance();

	/** Update every frame */
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	/** Initialize on creation */
	virtual void NativeInitializeAnimation() override;

	/** Play a montage with optional section name */
	UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
	void PlayMontage(UAnimMontage* Montage, FName StartSection = NAME_None, float PlayRate = 1.f);

protected:
	/** Cached pawn owner */
	UPROPERTY(Transient)
	APawn* OwningPawn;

	/** Character speed for BlendSpaces */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Acceleration;

	/** Character Direction for BlendSpaces */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction;

	/** Exclusive AI Variable */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsTargetSetted;



	/** In-air flag for jump/fall states */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsRunning;

	/** LockOn Component reference */
	UPROPERTY(BlueprintReadOnly, Category = "LockOn")
	bool bLockOnEnabled = false;

	/** Defense variable */
	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	bool bIsBlocking;

	/** 플레이어에게 붙은 LockOn 컴포넌트 캐시 */
	UPROPERTY(Transient)
	ULockOnComponent* LockOnComp = nullptr;
};
