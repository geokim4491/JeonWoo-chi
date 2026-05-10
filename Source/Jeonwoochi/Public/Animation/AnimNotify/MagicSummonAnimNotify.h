// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Structure/Combat/DamageInfoStruct.h"
#include "MagicSummonAnimNotify.generated.h"

UENUM(BlueprintType)
enum class ESummonAnchor : uint8
{
	Self,
	Target
};

class AMagicSummonActor;

/**
 * 
 */
UCLASS(meta = (DisplayName = "Magic: Summon (AOE)"))
class JEONWOOCHI_API UMagicSummonAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UMagicSummonAnimNotify();

	/** 소환할 액터(BP) – 데미지/이펙트/수명 등은 이 액터에서 처리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Summon")
	TSubclassOf<AMagicSummonActor> SummonClass;

	/** 소환 위치: Self(시전자) / Target(타깃) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Summon")
	ESummonAnchor Anchor = ESummonAnchor::Target;

	//** 데미지 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Damage")
	FDamageInfoStruct DamageInfo;

	/** Self 앵커일 때 사용할 소켓(비우면 메시 컴포넌트 트랜스폼) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Summon", meta = (EditCondition = "Anchor==ESummonAnchor::Self"))
	FName SelfSocketName;

	/** 앵커 기준 로컬 오프셋(위치/회전/스케일) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Summon")
	FTransform LocalOffset;

	virtual void Notify(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation) override;
};
