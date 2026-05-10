// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enum/Combat/AttackTypeEnum.h"
#include "Enum/Combat/DamageTypeEnum.h"
#include "Enum/Combat/ReactionTypeEnum.h"
#include "Enum/Combat/AttackCategoryEnum.h"
#include "DamageInfoStruct.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FDamageInfoStruct
{
    GENERATED_BODY()

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DamageInfo")
    AActor* Attacker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    float DamageAmount;

    /** ex) JumpAttack, GrabAttack.. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    EAttackType AttackType;

    /** ex) NormalAttack, SpecialAttack.. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    EAttackCategory AttackCategory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    EDamageType DamageTpye;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    float StunAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    EReactionType ReactionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    UAnimMontage* Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
    bool bForceFlyingRootMotion = true;

    // 기본 생성자(필요 시)
    FDamageInfoStruct();
};
