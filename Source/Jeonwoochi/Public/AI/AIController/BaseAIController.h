// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"

#include "Interface/Character/CharacterTypeInterface.h"

#include "BaseAIController.generated.h"

class USplineComponent;
class UHealthComponent;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ABaseAIController();

	// Possess 시 캐싱되는 순찰 지점 목록
	UPROPERTY()
	USplineComponent* PatrolSpline;

	UPROPERTY()
	int32 CurrentSplinePointIndex;

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual void SetGenericTeamId(const FGenericTeamId& NewID) override { TeamId = NewID; }

	void SetTargetActor(AActor* NewTarget);
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FGameplayTag IdleTag;
	FGameplayTag CombatTag;
	
	// Main BT
	UPROPERTY(EditAnywhere, Category = "AI|BT")
	UBehaviorTree* MainBT;

	/** 컴포넌트*/
	UPROPERTY(VisibleAnywhere, meta = (HideInDetails))
	UBehaviorTreeComponent* BehaviorComp;

	UPROPERTY(VisibleAnywhere, meta = (HideInDetails))
	UBlackboardComponent* BlackboardComp;


	/** Perception 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	class UAIPerceptionComponent* PerceptionComp;

	/** 시야 감지 설정 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	class UAISenseConfig_Sight* SightConfig;

	/** 청각 감지 설정 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	class UAISenseConfig_Hearing* HearingConfig;

	/** 데미지 감지 설정 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	class UAISenseConfig_Damage* DamageSenseConfig;

	/** 지각 대상이 업데이트될 때 호출 */
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	void OnAggroTimerExpired();

	void OnClearTarget(AActor* Target, ECharacterType CharType);
	void OnSelfDead(AActor* DeadSelf, ECharacterType);

	void StartAIBehaviorTree();

	ECharacterType OwnerType;

	/** 어그로 타이머 */
	FTimerHandle AggroTimerHandle;
	float AggroDuration;
	bool bCanSwitchTarget;

	FGenericTeamId TeamId;

	FTimerHandle SpawnMontageTimerHandle;
	FDelegateHandle DeathHandle;
	TWeakObjectPtr<UHealthComponent> BindingOtherHealthComp;
};
