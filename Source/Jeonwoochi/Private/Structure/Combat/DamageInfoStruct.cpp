// Fill out your copyright notice in the Description page of Project Settings.


#include "Structure/Combat/DamageInfoStruct.h"

FDamageInfoStruct::FDamageInfoStruct()
{
	Attacker = nullptr;

    DamageAmount = 0.f;

    AttackType = EAttackType::NormalAttack;

    DamageTpye = EDamageType::Normal;

    StunAmount = 0.f;

    ReactionType = EReactionType::None;

    Montage = nullptr;
}