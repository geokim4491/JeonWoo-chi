// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "illusionSkillComponent.generated.h"

class ABaseAICharacter;
class ULockOnComponent;
class USpringArmComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UillusionSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UillusionSkillComponent();

	// 에디터에서 드래그&드롭할 머터리얼 참조
	UPROPERTY(EditDefaultsOnly, Category = "Highlight")
	UMaterialInterface* HighlightMaterial;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	// === 설정값 ===
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float TraceDistance = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float AimToleranceDot = 0.97f;   // 1=정중앙
	
	UPROPERTY() 
	USpringArmComponent* CameraBoom = nullptr;

	UPROPERTY()
	float illusionDuration = 20.f;

	UPROPERTY()
	ABaseAICharacter* IllusionedTarget = nullptr;

	// === 내부 상태 ===
	UPROPERTY()
	ABaseAICharacter* CurrentTarget = nullptr;

	// === 내부 함수 ===
	void UpdateTarget();
	void Highlight(ABaseAICharacter* Target, bool bEnable);
	void Clearillusion();
	void CameraFadeEffect(float FromAlpha, float ToAlpha, float FadeTime, bool bHold);

public:	
	UFUNCTION()
	void illusionMode();

	UFUNCTION()
	void StopillusionMode();

	UFUNCTION()
	bool Castillusion();

	UFUNCTION()
	bool IsValidillusionTarget();

private:
	FTimerHandle illusionResetHandle;
	APlayerController* PC;
	ULockOnComponent* BoundLockOnComp;
	bool billusionModeActive = false;

	bool   bCamDefaultCached = false;
	float  DefaultArmLength = 0.f;
	FVector DefaultSocketOffset{};
	FRotator DefaultRelRot{};

};
