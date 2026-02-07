// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DimensionShiftComponent.generated.h"

class ABaseCharacter;
class UBoxComponent;
class APaperCharacter;
class AJeonwoochiPaperCharacter;
class AJeonwoochiCharacter;
class UNiagaraSystem;
class UParticleSystem;

class UGeometryCollection;                 // Chaos Destruction 플러그인
class AGeometryCollectionActor;

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

	/** 타깃 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Transition")
	TObjectPtr<ABaseCharacter> Character = nullptr;

	/** 2D 전환 후 임시로 캐릭터를 어디에 배치해둘지 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	FTransform BeforeTransform;

	/** 다시 3D 전환 후 캐릭터의 위치 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	FTransform AfterTransform;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCompleteComebackCondition, EComebackCondition);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UDimensionShiftComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDimensionShiftComponent();

	UFUNCTION() void SetCachedChar(AActor* Char);
	UFUNCTION() void Start2DTransition();
	UFUNCTION() void Start3DTransition();

	UFUNCTION(BlueprintCallable, Category = "Break")
	void BreakOwnerAndDestroy();           // ← 이 함수를 호출하면 "부서짐 → Destroy"

	FOnCompleteComebackCondition OnCompleteComebackCondition;

protected:
	// Called when the game starts
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

	/** 보스나 캐릭터의 2D <-> 3D 전환 전,후 위치 설정 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Transition")
	TArray<F2DTransitionCharacters> TransitionCharacters;

	/** Transition 조건 */
	UPROPERTY(EditAnywhere, Category = "Transition")
	EComebackCondition CombackCondition = EComebackCondition::KillEnemies;

	/** 활성화 시킬 2D AI 캐릭터 */
	UPROPERTY(EditInstanceOnly, Category = "Transition", meta = (EditCondition = "CombackCondition == EComebackCondition::KillEnemies"))
	TArray<TObjectPtr<APawn>> ActiveTargets;

	/** 도달해야 할 위치 */
	UPROPERTY(EditAnywhere, Category = "Transition", meta = (EditCondition = "CombackCondition == EComebackCondition::ReachLocation"))
	TObjectPtr<UBoxComponent> ReachLocationBox = nullptr;

	/** Transition시 재생할 VFX */
	UPROPERTY(EditAnywhere, Category = "Transition|VFX")
	UNiagaraSystem* NiagaraSystem = nullptr;

	/** Transition시 재생할 VFX */
	UPROPERTY(EditAnywhere, Category = "Transition|VFX")
	UParticleSystem* ParticleSystem = nullptr;

	/** false면, 2D 캐릭터에서 스폰함 */
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

	UPROPERTY(EditAnywhere, Category = "Transition|Location")
	float LocalYOffset = 0.f;


	UPROPERTY(EditAnywhere, Category = "Break|Asset")
	TSoftObjectPtr<UGeometryCollection> FractureAsset; // 부서질 Geometry Collection

	UPROPERTY(EditAnywhere, Category = "Break|Tuning")
	float DestroyDelay = 5.0f;             // 부서지는 연출 후 원본을 없애는 시간

	UPROPERTY(EditAnywhere, Category = "Break|Impulse")
	float RadialImpulseStrength = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Break|Impulse")
	float RadialImpulseRadius = 200.f;

	// 내부 헬퍼
	void SpawnFractureActorAndKickSim(class AActor* Owner);

private:
	
	UFUNCTION() void ReduceRemainEnemies(AActor* Char, ECharacterType CharType);
	UFUNCTION() void ActiveAIs();

	UFUNCTION() void SetReachLocationCollisionEnabled(bool bEnable);

	/* true이면, 2D 진입 Sound, false이면 3D 진입 Sound. 
	VFX는 2D, 3D 진입 전부 동일.
	*/
	UFUNCTION() void PlayVFX(bool bTransition2DSound);

	/** true인 경우 before transform으로 설정. false이면 after transform으로 설정. */
	UFUNCTION() void SetCharactersTransform(bool bIsBeforeTransform);

	int32 RemainEnemies = 0;
	int32 RemainConditions = 0;
	TArray<TObjectPtr<ABaseCharacter>> CachedTargets;

	AJeonwoochiCharacter* CachedChar;
	AJeonwoochiPaperCharacter* Player2DCharacter = nullptr;
};
