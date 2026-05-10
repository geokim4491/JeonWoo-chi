// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatModeComponent.generated.h"

UENUM(BlueprintType)
enum class ECombatLayer : uint8
{
	None,       // 비전투
	Soft,       // 공격/스킬만 사용
	Hard       // 적이 발견 (교전)
};

// UI 페이드(true: 보이기/false: 숨김, duration초)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatUIFade, bool, bShow);
// 상태 전환
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatLayerChanged, ECombatLayer, NewLayer, ECombatLayer, OldLayer);
// 하드 모드 진입(맵/레벨 태그와 함께 → 이걸로 OST 선택)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatHardEntered, FName, RegionTag);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UCombatModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatModeComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float TimeoutSeconds = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Region")
	FName RegionTag = NAME_None; // 레벨/지역에서 세팅

	UPROPERTY(BlueprintAssignable) FOnCombatUIFade OnCombatUIFade;
	UPROPERTY(BlueprintAssignable) FOnCombatLayerChanged OnCombatLayerChanged;
	UPROPERTY(BlueprintAssignable) FOnCombatHardEntered OnCombatHardEntered;

	UFUNCTION(BlueprintPure) ECombatLayer GetLayer() const { return Layer; }

	// ===== 이벤트 진입점(최소 세트) =====
	UFUNCTION(BlueprintCallable) void NotifyPlayerAction_Soft(); // 공격/스킬/물약 → UI만 빠르게 페이드 인 + Soft 진입/유지 + 타이머 리셋
	UFUNCTION(BlueprintCallable) void NotifyEnemyTargetAcquired(AActor* Enemy); // 적이 타깃으로 설정 → Hard 진입 + 타이머 리셋
	UFUNCTION(BlueprintCallable) void NotifyEnemyLostTarget(AActor* Enemy);     // 적이 타깃 해제 → 전부 잃으면 타이머 시작/리셋

	// 지역 변경(레벨 블루프린트 등에서)
	UFUNCTION(BlueprintCallable) void SetRegionTag(FName InTag) { RegionTag = InTag; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SetLayer(ECombatLayer NewLayer);
	void ResetCountdown();          // 항상 10초로 리셋
	void ClearCountdown();
	void HandleTimeout();           // 타이머 만료 처리

	// Aggressors: Hard 유지 여부 판단 (최소 구현 – 파괴 감지는 생략)
	UPROPERTY() TSet<TWeakObjectPtr<AActor>> Aggressors;

	UPROPERTY(VisibleAnywhere)
	ECombatLayer Layer = ECombatLayer::None;

	FTimerHandle CountdownHandle;
	bool IsCountdownActive() const { return GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(CountdownHandle); }

		
};
