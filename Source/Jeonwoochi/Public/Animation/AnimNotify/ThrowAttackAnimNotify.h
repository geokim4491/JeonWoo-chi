// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Structure/Combat/DamageInfoStruct.h"
#include "ThrowAttackAnimNotify.generated.h"


class AProjectileActor;

/**
 * 
 */
UCLASS(meta = (DisplayName = "Throw Projectile"))
class JEONWOOCHI_API UThrowAttackAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UThrowAttackAnimNotify();

	/** 투척할 ProjectileBP */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Projectile")
	TSubclassOf<AProjectileActor> ProjectileClass;

	/** 데미지 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Damage")
	FDamageInfoStruct DamageInfo;

	/** 스폰 기준: 메시 소켓명(비우면 이 노티파이를 쏜 스켈레탈 메시 컴포넌트의 위치 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Spawn")
	FName SpawnSocketName;

	/** 스폰 오프셋(로컬 기준). 소켓/컴포넌트 트랜스폼 위에 더해짐 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Spawn")
	FTransform SpawnOffset;

	/** 발사 방향으로 회전 정렬 여부 (투사체의 ProjectileMovement.bRotationFollowsVelocity도 함께 반영) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Direction")
	bool bAlignRotationToDirection = true;

	/** 최종 회전에 추가할 오프셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Direction")
	FRotator AdditionalRotation = FRotator::ZeroRotator;

	/** 투사체 속도 덮어쓰기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Speed")
	bool bOverrideProjectileSpeed = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Speed", meta = (EditCondition = "bOverrideProjectileSpeed"))
	float InitialSpeed = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Speed", meta = (EditCondition = "bOverrideProjectileSpeed"))
	float MaxSpeed = 1500.f;

	// UAnimNotify
	virtual void Notify(
		USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	virtual FString GetNotifyName_Implementation() const override { return TEXT("Throw Projectile"); }
};
