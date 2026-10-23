// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PostureComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPostureCountUpdated, 
	float /*CurrentPostureCount*/,
	float /*MaxPostureCount*/);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UPostureComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPostureComponent();

	UPROPERTY(EditAnywhere, Category= "Posture")
	int32 MaxPostureCount;

	UPROPERTY(EditAnywhere, Category = "Posture")
	float StunDuration;

	UPROPERTY(EditAnywhere, Category = "Execution")
	float DamageAmount;

	UPROPERTY(EditAnywhere, Category = "Execution")
	UAnimMontage* StunMontage;

	UPROPERTY(EditAnywhere, Category = "Execution")
	UAnimMontage* ExecutedMontage;

	UFUNCTION()
	void ReducePostureCount(int32 Amount);

	UFUNCTION()
	void ResetCurrentPosture();

	UFUNCTION()
	void SetMaxPosture(int32 Posture);

	UFUNCTION()
	void FinishExecution();

	UFUNCTION()
	bool IsStunned() const { return bIsStunned; }

	FORCEINLINE int32 GetMaxPosture() const { return MaxPostureCount; }
	FORCEINLINE int32 GetCurPosture() const { return CurrentPostureCount; }

	FOnPostureCountUpdated OnPostureUpdated;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	int32 CurrentPostureCount;
	bool bIsStunned;
	FTimerHandle StunTimerHandle;
	FTimerHandle ExecutedTimerHandle;
	FTimerHandle GetUpTimerHandle;
	
	void Stun();
	void EndStun();
	void GetUp();
};
