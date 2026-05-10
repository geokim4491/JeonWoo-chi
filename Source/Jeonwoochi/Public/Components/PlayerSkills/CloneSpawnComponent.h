// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CloneSpawnComponent.generated.h"

class ACloneAICharacter;
enum class EEffectType : uint8;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UCloneSpawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCloneSpawnComponent();

	// 분신으로 사용할 액터 클래스
	UPROPERTY(EditAnywhere, Category = "Clone")
	TSubclassOf<ACloneAICharacter> CloneClass;

	// 한번에 유지할 최대 분신 수
	UPROPERTY(EditAnywhere, Category = "Clone", meta = (ClampMin = 1))
	int32 MaxClones = 3;

	// 분신이 사라지기까지 걸리는 시간(초)
	UPROPERTY(EditAnywhere, Category = "Clone", meta = (ClampMin = 0.1f))
	float CloneDuration = 10.f;

	// 분신 생성 함수
	UFUNCTION(BlueprintCallable, Category = "Clone")
	void InternalSpawn(const FVector& Loc);

	UFUNCTION(BlueprintCallable, Category = "Clone")
	bool SpawnClones(const TArray<FVector>& SpawnLocations);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
		
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	/** 소환 가능 여부 플래그 */
	bool bCanSpawn;

	/** 쿨다운 타이머 핸들 */
	FTimerHandle SpawnCooldownTimerHandle;

	/** 쿨다운 종료 시 호출 */
	void ResetSpawnAbility();

	void PlayEffect(EEffectType EffectType, FVector Location, FRotator Rotation);

	// 현재 활성 분신
	UPROPERTY()

	TArray<ACloneAICharacter*> ActiveClones;


};
