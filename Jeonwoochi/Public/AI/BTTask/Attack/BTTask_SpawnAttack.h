// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "Structure/Combat/DamageInfoStruct.h"

#include "BTTask_SpawnAttack.generated.h"

/**
 * 
 */

class AProjectileActor;

USTRUCT()
struct FSpawnAttackTaskMemory
{
    GENERATED_BODY()
    float ElapsedTime;
    bool  bProjectileShot = false;
};

UCLASS()
class JEONWOOCHI_API UBTTask_SpawnAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:

    UBTTask_SpawnAttack();
    
    UPROPERTY(EditAnywhere, Category = "Projectile")
    TSubclassOf<AProjectileActor> ProjectileClass;

    /** Spawn Actor Info */
    UPROPERTY(EditAnywhere, Category = "Attack|AttackInfo", meta = (AllowedClasses = "Info"))
    FDamageInfoStruct DamageInfo;

    /** 몽타주 재생 후 대기할 시간 (초) */
    UPROPERTY(EditAnywhere, Category = "Attack|WaitingTime", meta = (ClampMin = "0.0"))
    float WaitTime;

    UPROPERTY(EditAnywhere, Category = "Attack|Timing", meta = (ClampMin = "0.0"))
    float ShotTime = 0.f;

    uint16 GetInstanceMemorySize() const override
    {
        return sizeof(FSpawnAttackTaskMemory);
    }

protected:
    // 실행 노드
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;


    virtual void TickTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds
    ) override;
	
};
