// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structure/Combat/DamageInfoStruct.h"
#include "Enum/Combat/AIDefenseStateEnum.h"
#include "AIDamageableComponent.generated.h"

enum class EEffectType : uint8;

USTRUCT(BlueprintType)
struct FReactionEnemyMontages
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
class JEONWOOCHI_API UAIDamageableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAIDamageableComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UFUNCTION()
	void HandleTakeDamage(const FDamageInfoStruct& Info);

	UPROPERTY(EditDefaultsOnly, Category = "HitAnimation")
	TArray<FReactionEnemyMontages> HitMontages;

	/** true로 설정 시, 공격 중일 때에도 Hit Anim 재생 */
	UPROPERTY(EditAnywhere, Category = "HitAnimation")
	bool bForceHitAnim = false;

private:

	void HandleHit(const FDamageInfoStruct& Info);

	void PlayEffect(EEffectType EffectType, bool bUseCameraShake = true, bool bUseHaptic = true);

	void HandleReaction(const FDamageInfoStruct& Info);

	// 런타임에만 사용할 맵
	TMap<EReactionType, TArray<UAnimMontage*>> RuntimeHitMontageMap;

	int32 HitIdx;
};
