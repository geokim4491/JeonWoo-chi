// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockOnComponent.generated.h"

enum class ECharacterType : uint8;
class UHealthComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLockOnTarget, AActor*)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API ULockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULockOnComponent();

    // 플레이어에서 호출할 함수
    UFUNCTION(BlueprintCallable, Category = "LockOn")
    void ToggleLockOn();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction
    ) override;

    UFUNCTION()
    void LockOn(AActor* NewTarget);

    /** 락온 해제 */
    UFUNCTION()
    void ClearLockOn();

    UFUNCTION()
    void OnTargetDeath(AActor* DeadActor, ECharacterType DeadCharType);

    /** Sphere Trace Radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn", meta = (ClampMin = "0.0"))
    float TraceRadius;

    /** 라인 트레이스 거리 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn", meta = (ClampMin = "0.0"))
    float TraceDistance;

    /** 바라볼 대상 액터 */
    UPROPERTY()
    AActor* TargetActor;

    /** 회전 속도 (Interpolation Speed) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn", meta = (ClampMin = "0.0"))
    float RotationInterpSpeed;

    /** 락온 활성화 여부 (TargetActor != nullptr 시 자동 true) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
    bool bLockOnEnabled;

    /** 락온 최대 거리 (이 거리 넘으면 자동 해제) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
    float MaxLockDistance;

public:
    /** 거리별 카메라 조절 */
    UPROPERTY(EditAnywhere, Category = "LockOn|Distance")
    float MinZOffset = 20.f;       // 가까울 때 뺄 Z

    UPROPERTY(EditAnywhere, Category = "LockOn|Distance")
    float MaxZOffset = 100.f;      // 멀때 뺄 Z

    UPROPERTY(EditAnywhere, Category = "LockOn|Distance")
    float MinOffsetDist = 300.f;   // MinZOffset부터 시작할 거리

    UPROPERTY(EditAnywhere, Category = "LockOn|Distance")
    float MaxOffsetDist = 2000.f;  // MaxZOffset까지 늘어날 거리

    // 몇 도 이상 회전하면 타겟 교체 시도할지 임계값
    UPROPERTY(EditAnywhere, Category = "LockOn|Switch")
    float MouseDeltaThreshold = 0.3f;

    // 몇 도 이상 회전하면 타겟 교체 시도할지 임계값
    UPROPERTY(EditAnywhere, Category = "LockOn|Switch")
    float StickDeadZone = 0.25f;

    FOnLockOnTarget OnLockOnTarget;

private:
    /** 전방 라인 트레이스로 적 찾기 */
    AActor* TryAutoTarget(bool bChangeLockOn = false);

    void ChangeLockOn(AActor* NewTarget);

    APlayerController* PC;

    // 바인딩할 때 저장해두는 포인터
    TWeakObjectPtr<UHealthComponent> BoundHealthComp;
    FDelegateHandle DeathHandle;
};
