// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AutoSaveTriggerActor.generated.h"

class UBoxComponent;

UENUM(BlueprintType)
enum class EAutoSaveType : uint8
{
	Progress, // 일반 진행(자동 저장/종료 전 수동 저장)
	Rest, // 화톳불 체크포인트(사망 시 이 시점으로 복원)
	All // 둘 다 적용
};

UCLASS()
class JEONWOOCHI_API AAutoSaveTriggerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAutoSaveTriggerActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndplayReason) override;

	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "Trigger")
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(EditAnywhere, Category="Save")
	EAutoSaveType SaveType = EAutoSaveType::Progress;
};
