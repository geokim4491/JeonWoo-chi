#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DimensionShiftComponent.generated.h"

class ABaseCharacter;
class UBoxComponent;
class AJeonwoochiPaperCharacter;
class AJeonwoochiCharacter;
class UNiagaraSystem;
class UParticleSystem;
class UGeometryCollection;

UENUM(BlueprintType)
enum class EComebackCondition : uint8
{
	ReachLocation,
	KillEnemies,
	All
};

USTRUCT(BlueprintType)
struct F2DTransitionCharacters
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Transition")
	TObjectPtr<ABaseCharacter> Character = nullptr;

	/** 2D 전환 후 Transform */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	FTransform BeforeTransform;

	/** 3D 복귀 후 Transform */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	FTransform AfterTransform;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCompleteComebackCondition, EComebackCondition);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class JEONWOOCHI_API UDimensionShiftComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDimensionShiftComponent();

	UFUNCTION() void SetCachedChar(AActor* Char);
	UFUNCTION() void Start2DTransition();
	UFUNCTION() void Start3DTransition();

	UFUNCTION(BlueprintCallable, Category = "Break")
	void BreakOwnerAndDestroy();

	FOnCompleteComebackCondition OnCompleteComebackCondition;

protected:
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

	/** 플레이어 2D 캐릭터 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	TSubclassOf<AJeonwoochiPaperCharacter> Player2DClass;

	/** 전환 대상 캐릭터의 2D/3D Transform */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Transition")
	TArray<F2DTransitionCharacters> TransitionCharacters;

	/** 3D 복귀 조건 */
	UPROPERTY(EditAnywhere, Category = "Transition")
	EComebackCondition CombackCondition = EComebackCondition::KillEnemies;

	/** 2D 전환 시 활성화할 AI */
	UPROPERTY(EditInstanceOnly, Category = "Transition",
		meta = (EditCondition = "CombackCondition == EComebackCondition::KillEnemies"))
	TArray<TObjectPtr<APawn>> ActiveTargets;

	/** 도달 조건 판정 위치 */
	UPROPERTY(EditAnywhere, Category = "Transition",
		meta = (EditCondition = "CombackCondition == EComebackCondition::ReachLocation"))
	TObjectPtr<UBoxComponent> ReachLocationBox = nullptr;

	UPROPERTY(EditAnywhere, Category = "Transition|VFX")
	UNiagaraSystem* NiagaraSystem = nullptr;

	UPROPERTY(EditAnywhere, Category = "Transition|VFX")
	UParticleSystem* ParticleSystem = nullptr;

	UPROPERTY(EditAnywhere, Category = "Transition|VFX")
	bool bPlayIn3DChar = true;

	UPROPERTY(EditAnywhere, Category = "Transition|VFX")
	FVector VFXScale;

	UPROPERTY(EditAnywhere, Category = "Transition|VFX")
	FVector VFXAddLocation;

	UPROPERTY(EditAnywhere, Category = "Transition|Sound")
	USoundBase* Transition2DSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Transition|Sound")
	USoundBase* Transition3DSound = nullptr;

	/** 그림 평면 기준 깊이 오프셋 */
	UPROPERTY(EditAnywhere, Category = "Transition|Location")
	float LocalYOffset = 0.f;

	UPROPERTY(EditAnywhere, Category = "Break|Asset")
	TSoftObjectPtr<UGeometryCollection> FractureAsset;

	UPROPERTY(EditAnywhere, Category = "Break|Tuning")
	float DestroyDelay = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Break|Impulse")
	float RadialImpulseStrength = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Break|Impulse")
	float RadialImpulseRadius = 200.f;

	void SpawnFractureActorAndKickSim(AActor* Owner);

private:
	UFUNCTION() void ReduceRemainEnemies(AActor* Char, ECharacterType CharType);
	UFUNCTION() void ActiveAIs();
	UFUNCTION() void SetReachLocationCollisionEnabled(bool bEnable);

	/** true: 2D 진입 사운드, false: 3D 복귀 사운드 */
	UFUNCTION() void PlayVFX(bool bTransition2DSound);

	UFUNCTION() void SetCharactersTransform(bool bIsBeforeTransform);

	int32 RemainEnemies = 0;
	int32 RemainConditions = 0;

	AJeonwoochiCharacter* CachedChar = nullptr;
	AJeonwoochiPaperCharacter* Player2DCharacter = nullptr;
};