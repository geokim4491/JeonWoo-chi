// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enum/Combat/PlayerDefenseStateEnum.h"
#include "Structure/Combat/DamageInfoStruct.h"
#include "PlayerDamageableComponent.generated.h"

DECLARE_DELEGATE_RetVal(EPlayerDefenseState, FGetDefenseStateDelegate);

enum class EEffectType : uint8;
class UHealthComponent;

USTRUCT(BlueprintType)
struct FReactionMontages
{
	GENERATED_BODY()

	// 맵의 “키”
	UPROPERTY(EditDefaultsOnly, Category = "HitAnimation")
	EReactionType ReactionType;

	// 맵의 “값” (배열)
	UPROPERTY(EditDefaultsOnly, Category = "HitAnimation")
	TArray<UAnimMontage*> Montages;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UPlayerDamageableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerDamageableComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void HandleTakeDamage(const FDamageInfoStruct& Info);

	/** AfterImage 스폰 & 초기화 */
	UFUNCTION()
	void SpawnAfterImage();

public:

	EPlayerDefenseState DefenseState;
		
	// OnGetDefenseState는 이 컴포넌트 안에서 DefenseState를 받기 위해 
	// 정의한 델리게이트.
	// 타 클래스에서 사용하지 않는 델리게이트임
	FGetDefenseStateDelegate OnGetDefenseState;

	UPROPERTY(EditDefaultsOnly, Category = "HitAnimation")
	TArray<FReactionMontages> HitMontages;

	UPROPERTY(EditDefaultsOnly, Category = "ParryAnimation")
	TArray<UAnimMontage*> ParryMontages;

	UPROPERTY(EditDefaultsOnly, Category = "BlockAnimation")
	TArray<UAnimMontage*> BlockMontages;

	// 에디터에서 드래그&드롭할 머터리얼 참조
	UPROPERTY(EditDefaultsOnly, Category = "Highlight")
	UMaterialInterface* PerfectDodgeHighlightMaterial;

private:
	void HandleHit(const FDamageInfoStruct& Info, EPlayerDefenseState ResultState);
	void HandleReaction(const FDamageInfoStruct& Info, EPlayerDefenseState ResultState);
	void IncreaseChi(EPlayerDefenseState ResultState);

	void PlayEffect(EEffectType EffectType);
	void CameraFadeEffect();
	void PerfectDodgeEffect();

	void ApplyHighlightMaterial();
	void ScheduleNextAfterImage(float Delay);

	int32 ParryIdx;
	int32 BlockIdx;
	int32 HitIdx;

	USkeletalMeshComponent* Mesh;

	FTimerHandle FadeTimerHandle;
	FTimerHandle SpawnTimerHandle;
	FTimerHandle MaterialTimerHandle;

	int32 AfterImageSpawnCount = 0;

	// 런타임에만 사용할 맵
	TMap<EReactionType, TArray<UAnimMontage*>> RuntimeHitMontageMap;

	UHealthComponent* CachedHealthComp;
	
};
