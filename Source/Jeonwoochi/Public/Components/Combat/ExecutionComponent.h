// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExecutionComponent.generated.h"

class AJeonwoochiCharacter;
class ABaseAICharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UExecutionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UExecutionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ExecutionMontage;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* ExecutionSound;

	FORCEINLINE void SetCanExecute(bool bCanExec) { bCanExecute = bCanExec; OnCanExecute.Broadcast(bCanExec); }
	FORCEINLINE bool GetCanExecute() const { return bCanExecute; }
	FORCEINLINE void SetExecutedTarget(ABaseAICharacter* Char) { ExecutedTarget = Char; }
	FORCEINLINE ABaseAICharacter* GetExecutedTarget() const { return ExecutedTarget; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCanExecute, bool, bCanExec);
	FOnCanExecute OnCanExecute;

	UFUNCTION()
	void Execute();
	
private:
	// void PlayEffect(EEffectType EffectType);
	// void CinematicCamera();
	UFUNCTION()
	void OnExecutionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	bool bCanExecute = false;

	AJeonwoochiCharacter* OwnerChar;
	ABaseAICharacter* ExecutedTarget = nullptr;
	ABaseAICharacter* CachedExecutedTarget = nullptr;
};
