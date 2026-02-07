// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "InteractTriggerComponent.generated.h"

/**
 * 
 */


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class JEONWOOCHI_API UInteractTriggerComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UInteractTriggerComponent();

protected:
	// 등록 시점에 바인딩(컴포넌트가 월드에 붙을 때)
	virtual void OnRegister() override;

	// BeginOverlap 시그니처 (AddDynamic에 맞춰 UFUNCTION 필요)
	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
