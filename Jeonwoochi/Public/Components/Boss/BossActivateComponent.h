// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossActivateComponent.generated.h"

class ULevelSequence;
class ULevelSequencePlayer;
class ALevelSequenceActor;
class AJeonwoochiCharacter;
class ABossCharacter;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBossActivated, AActor*, FText);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UBossActivateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBossActivateComponent();

	UFUNCTION()
	void ActivateBoss();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Option")
	bool bActiveLSQ = true;

	UPROPERTY(EditInstanceOnly, Category = "Sequence", meta = (EditCondition = "bActiveLSQ", EditConditionHides))
	TObjectPtr<ALevelSequenceActor> PlacedSequenceActor = nullptr;

	/** Want to Possess AI */
	UPROPERTY(EditInstanceOnly, Category = "AI")
	TArray<TObjectPtr<APawn>> Targets;

	/** 기존 BG Ost 멈출지 여부 */
	UPROPERTY(EditInstanceOnly, Category = "Sound")
	bool bStopBGSound = true;

	/** 컷 신 스킵 시, 재생될 ost 직접 지정. */
	UPROPERTY(EditInstanceOnly, Category = "Sound")
	USoundBase* BossOst;

	UPROPERTY(EditInstanceOnly, Category = "FadeInOut")
	bool bActiveFadeIn = false;

	UPROPERTY(EditInstanceOnly, Category = "FadeInOut")
	bool bActiveFadeOut = false;

	UPROPERTY(EditInstanceOnly, Category = "FadeInOut", meta = (EditCondition = "bActiveFadeIn"))
	float FadeInSec = 1.f;

	UPROPERTY(EditInstanceOnly, Category = "FadeInOut", meta = (EditCondition = "bActiveFadeOut"))
	float FadeOutSec = 0.5f;

	UPROPERTY(EditInstanceOnly, Category = "Transform")
	FTransform BeforePlayerTransform;

	UPROPERTY(EditInstanceOnly, Category = "Transform")
	FTransform AfterPlayerTransform;

	UPROPERTY(EditInstanceOnly, Category = "Transform")
	FTransform BeforeBossTransform;

	UPROPERTY(EditInstanceOnly, Category = "Transform")
	FTransform AfterBossTransform;

	/** LSQ 끝나고 없앨 메시 */
	UPROPERTY(EditInstanceOnly, Category = "Mesh")
	TArray<TWeakObjectPtr<AActor>> DestroyTargets;

	UFUNCTION()
	void SetCachedChar(AActor* Char);

	UFUNCTION(BlueprintCallable)
	void SkipCutScene();

	FOnBossActivated OnBossActivated;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	
private:
	AJeonwoochiCharacter* CachedChar;
	ABossCharacter* CachedBoss;
	ULevelSequencePlayer* CachedPlayer = nullptr;

	FTimerHandle FadeTimerHandle;

	UFUNCTION() void PlaySequence();
	UFUNCTION() void OnFinished();
	void ActivateAI();
	void DestroyMesh();

	void FadeIn(float Duration);
	void FadeOut(float Duration);
};
