// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "../Combat/AttackComponent.h"

#include "HitDetectionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UHitDetectionComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHitDetectionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 히트박스 활성화 */
	UFUNCTION(BlueprintCallable, Category = "HitDetection")
	void EnableDetection();

	/** 히트박스 비활성화 */
	UFUNCTION(BlueprintCallable, Category = "HitDetection")
	void DisableDetection();

	UFUNCTION()
	void StartHitWindow();

	UFUNCTION()
	void EndHitWindow();

	UFUNCTION() 
	void RefreshOwnerPointers();

public:
	/** 히트박스로 사용할 BoxShape 컴포넌트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitDetection")
	UBoxComponent* HitShape;

	/** 한 스윙당 중복 히트를 막기 위한 저장소 */
	UPROPERTY(VisibleAnywhere, Category = "HitDetection")
	TSet<AActor*> AlreadyHitActors;

		
private:
	/** Overlap 이벤트 핸들러 */
	UFUNCTION()
	void HandleOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, // 부위별 피격 판정 할거면 사용
		bool bFromSweep,
		const FHitResult& SweepResult);

	// 캐싱된 포인터
	UAttackComponent* CachedAttackComp = nullptr;
	AActor* CachedOwnerActor = nullptr;
	ABaseCharacter* CachedOwnerChar = nullptr;
};
