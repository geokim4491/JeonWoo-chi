// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CutSceneTriggerActor.generated.h"

class AJeonwoochiCharacter;
class ULevelSequencePlayer;
class ALevelSequenceActor;
class UBoxComponent;

UCLASS()
class JEONWOOCHI_API ACutSceneTriggerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACutSceneTriggerActor();

	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(EditInstanceOnly, Category = "Sequence")
	TObjectPtr<ALevelSequenceActor> PlacedSequenceActor = nullptr;

	/** 기존 BG Ost 멈출지 여부 */
	UPROPERTY(EditInstanceOnly, Category = "Sound")
	bool bStopBGSound = true;

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

	/** LSQ 끝나고 없앨 메시 */
	UPROPERTY(EditInstanceOnly, Category = "Mesh")
	TArray<TWeakObjectPtr<AActor>> DestroyTargets;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	UFUNCTION() void PlaySequence();
	UFUNCTION() void OnFinished();

	void FadeIn(float Duration);
	void FadeOut(float Duration);

	void DestroyMesh();

	FTimerHandle FadeTimerHandle;

	AJeonwoochiCharacter* CachedChar = nullptr;
	ULevelSequencePlayer* CachedPlayer = nullptr;

};
