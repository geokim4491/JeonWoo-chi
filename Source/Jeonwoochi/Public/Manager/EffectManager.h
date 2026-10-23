// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NiagaraSystem.h"
#include "Enum/Effect/EffectTypeEnum.h"
#include "EffectManager.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FTimeDilationParams
{
    GENERATED_BODY()

    // 3: SlowMotion 배수 (0.0~1.0)
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Time")
    float DilationFactor = 1.f;

    // SlowMotion 지속 시간 (초), 0이면 비활성
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Time")
    float Duration = 0.f;
};

UENUM(BlueprintType)
enum class EEffectSpawnSelection : uint8
{
    All        UMETA(DisplayName = "All"),
    RandomOne  UMETA(DisplayName = "Random One"),
};

USTRUCT(BlueprintType)
struct FEffectSpawnTuning
{
    GENERATED_BODY()

    /** bUse Context Actor (if true, apply actor's transform ) */
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Transform")
    bool bUseContextActor = true;

    /** Actor Local: +X=Forward */
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Transform", meta = (EditCondition = "bUseContextActor", EditConditionHides))
    float Forward = 0.f;

    /** Actor Local: +Y=Right*/
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Transform", meta = (EditCondition = "bUseContextActor", EditConditionHides))
    float Right = 0.f;

    /** Actor Local: +Z=Up */
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Transform", meta = (EditCondition = "bUseContextActor", EditConditionHides))
    float Up = 0.f;

    /** Actor Local: +(Yaw/Pitch/Roll) */
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Transform", meta = (EditCondition = "bUseContextActor", EditConditionHides))
    FRotator RotationOffset = FRotator::ZeroRotator;

    /** Scale */
    UPROPERTY(EditAnywhere, Category = "Transform")
    FVector Scale = FVector(1.f);

    /** is attach to Component */
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Transform", meta = (EditCondition = "bUseContextActor", EditConditionHides))
    bool bAttachToContext = false;

    /** SocketName (if null, attach to root) */
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Transform", meta = (EditCondition = "bAttachToContext", EditConditionHides))
    FName AttachSocketName = NAME_None;
};

USTRUCT(BlueprintType)
struct FNiagaraEffectEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Niagara")
    UNiagaraSystem* System = nullptr;

    UPROPERTY(EditAnywhere, Category = "Niagara")
    FEffectSpawnTuning Spawn;
};

USTRUCT(BlueprintType)
struct FCascadeEffectEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Cascade")
    UParticleSystem* System = nullptr;

    UPROPERTY(EditAnywhere, Category = "Cascade")
    FEffectSpawnTuning Spawn;
};

USTRUCT(BlueprintType)
struct FEffectSettings
{
    GENERATED_BODY()

    // 1: Sound
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Sound")
    USoundBase* Sound = nullptr;

    // 2a: Niagara VFX
    UPROPERTY(EditDefaultsOnly, Category = "Effects|VFX")
    TArray<FNiagaraEffectEntry> NiagaraEffects;

    // 2b: Cascade Particle VFX
    UPROPERTY(EditDefaultsOnly, Category = "Effects|VFX")
    TArray<FCascadeEffectEntry> CascadeEffects;

    /** if true, chose one */
    UPROPERTY(EditAnywhere, Category = "Niagara")
    bool bNiagaraRandomOneCandidate = false;

    /** if true, chose one */
    UPROPERTY(EditAnywhere, Category = "Niagara")
    bool bCascadeRandomOneCandidate = false;

    /** Default Destroy: if true, auto destroy. else if false, should call StopEffect() */
    UPROPERTY(EditAnywhere, Category = "Effects|General")
    bool bAutoDestroyOnFinish = true;

    // 3: Camera Shake
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Camera")
    bool bEnableCameraShake = false;

    UPROPERTY(EditDefaultsOnly, Category = "Effects|Camera", meta = (EditCondition = "bEnableCameraShake", EditConditionHides))
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    // camera shake intensity
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Camera", meta = (EditCondition = "bEnableCameraShake", EditConditionHides))
    float CameraShakeScale = 1.f;

    // 4: Haptic Feedback
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Haptic")
    bool bEnableHaptic = false;

    UPROPERTY(EditDefaultsOnly, Category = "Effects|Haptic", meta = (EditCondition = "bEnableHaptic", EditConditionHides))
    UForceFeedbackEffect* ForceFeedbackEffect = nullptr;

    // 5: SlowMotion
    UPROPERTY(EditDefaultsOnly, Category = "Effects|Time")
    FTimeDilationParams TimeDilation;

};

UCLASS(Config = Game, DefaultConfig)
class JEONWOOCHI_API UEffectManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    /* DeveloperSettings → EffectsMap 복사 */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    virtual void Deinitialize() override;

    // 모든 이펙트 재생 (Sound, VFX, SlowMotion 순서)
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void PlayEffect(EEffectType Type, const FVector& Location, const FRotator& Rotation, bool bUseHaptic = true);

    /** 기준 Actor를 넘기면, 에디터에서 설정한 Forward/Right/Up + RotationOffset을 적용해 스폰/부착 */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void PlayEffectOnActor(EEffectType Type, AActor* ContextActor, bool bUseCameraShake = true, bool bUseHaptic = true);

    // 남아있는 VFX들 찾아서 전부 지움
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void StopEffect(EEffectType Type);

private:
    /* 복사된 데이터 보관 */
    TMap<EEffectType, FEffectSettings> EffectsMap;

    TMap<EEffectType, TArray<UNiagaraComponent*>> ActiveNiagaras;
    TMap<EEffectType, TArray<UParticleSystemComponent*>> ActiveCascades;

    FTimerHandle TimeDilationTimerHandle;

    /** 기준 Actor/컴포넌트와 설정값으로 실제 스폰 위치/회전을 계산 */
    void ComputeFromBase(const FTransform& Base, const FEffectSpawnTuning& S, FVector& OutLoc, FRotator& OutRot) const;

    /** 부착 대상 컴포넌트(없으면 기준 Actor 루트) */
    USceneComponent* ResolveAttachComponent(AActor* ContextActor) const;
};
