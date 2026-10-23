// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Combat/HealthComponent.h"

#include "BossPhaseComponent.generated.h"

class ALevelSequenceActor;
class ULevelSequencePlayer;
class UHealthComponent;
class AJeonwoochiCharacter;
class ABossCharacter;

USTRUCT(BlueprintType)
struct FBeforeAfterTransform
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, Category = "Transform")
	FTransform BeforePlayerTransform;

	UPROPERTY(EditInstanceOnly, Category = "Transform")
	FTransform AfterPlayerTransform;

	UPROPERTY(EditInstanceOnly, Category = "Transform")
	FTransform BeforeBossTransform;

	UPROPERTY(EditInstanceOnly, Category = "Transform")
	FTransform AfterBossTransform;
};

USTRUCT(BlueprintType)
struct FBossPhaseStruct
{
	GENERATED_BODY()

	/** 보스 페이즈(자동 설정되는 값) */ 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Phase")
	int32 Page = 1;

	/** 이 체력 이하로 떨어지면, 페이즈 전환
	 0 이라면, 다시 풀 피로 만드는 로직 사용. (max health는 변경 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (ClampMin = "0.0"))
	float TriggerAtHealth = 0.f;

	/** 다음 페이지에 체력을 새로 설정하는 경우에만 사용함 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (ClampMin = "0.0"))
	float NewMaxHealth;

	/** 다음 페이지에 Posture 초기화 할지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (ClampMin = "0.0"))
	bool bResetPosture = true;

	
	/** 전환 시 LSQ 재생할지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	bool bPlayLevelSequence = false;

	/** 재생할 LSQ 에셋 (bPlayLevelSequence=true일 때만 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (EditCondition = "bPlayLevelSequence"))
	TObjectPtr<ALevelSequenceActor> PlacedSequenceActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Transform", meta = (EditCondition = "bPlayLevelSequence"))
	FBeforeAfterTransform CutSceneTransform;

	UPROPERTY(EditInstanceOnly, Category = "FadeInOut")
	bool bActiveFadeIn = false;

	UPROPERTY(EditInstanceOnly, Category = "FadeInOut")
	bool bActiveFadeOut = false;

	UPROPERTY(EditInstanceOnly, Category = "FadeInOut", meta = (EditCondition = "bActiveFadeIn"))
	float FadeInSec = 1.f;

	UPROPERTY(EditInstanceOnly, Category = "FadeInOut", meta = (EditCondition = "bActiveFadeOut"))
	float FadeOutSec = 0.5f;

	/** LSQ 끝나고 없앨 메시 */
	UPROPERTY(EditInstanceOnly, Category = "Mesh")
	TArray<TWeakObjectPtr<AActor>> DestroyTargets;
};


/**
 * 보스 페이즈 전환 전용 컴포넌트
 * - 체력 임계 도달 → (옵션) LSQ 재생 → 보스 쪽 상태 전환 함수 호출
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UBossPhaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBossPhaseComponent();

	/** true로 설정할 경우 반드시 배열 element를 만들어야 함. */
	UPROPERTY(EditAnywhere, Category = "Phase")
	bool bEnablePhase = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase", meta = (EditCondition = "bEnablePhase"))
	TArray<FBossPhaseStruct> Phases;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Phase", meta = (EditCondition = "bEnablePhase"))
	int32 CurrentPage = 0; // 0부터 1페이즈

	/** 보스의 페이즈로 2면, 1, 2페이즈 존재 */
	UPROPERTY(EditAnywhere, Category = "Phase", meta = (EditCondition = "bEnablePhase"))
	int32 LastPhase = 2;


	UFUNCTION(BlueprintCallable)
	void SkipCutScene();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UPROPERTY() UHealthComponent* BossHealthComp = nullptr;

	UPROPERTY() AJeonwoochiCharacter* CachedChar = nullptr;
	UPROPERTY() ABossCharacter* CachedOwner = nullptr;
	UPROPERTY() ULevelSequencePlayer* CachedPlayer = nullptr;

	UFUNCTION() void PlaySequence();
	UFUNCTION() void OnFinished();

	/** 내부: 페이지 번호 정합성 유지 */
	void RefreshPageIndices();
	
	void ChangeBossPhase(
		AActor* Boss, ECharacterType CharType,
		float CurrentHealth, float MaxHealth
	);

	void FadeIn(float Duration);
	void FadeOut(float Duration);

	void DestroyMesh();

	FTimerHandle FadeTimerHandle;
};
