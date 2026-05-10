// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "HitWindowAnimNotifyState.generated.h"

/**
 * 히트 윈도우 구간이 시작/종료될 때
 * UAttackComponent의 StartHitWindow()/EndHitWindow()를 호출하는 Notify State
 */
UCLASS()
class JEONWOOCHI_API UHitWindowAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
    // 지정하지 않으면 NAME_None → 전체 무기 처리
    UPROPERTY(EditAnywhere, Category = "HitWindow")
    FName TargetWeaponName = NAME_None;

    // Notify State 구간 시작 시 호출
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        float TotalDuration) override;

    // Notify State 구간 종료 시 호출
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation) override;

private:
    void ProcessHitWindow(
        USkeletalMeshComponent* MeshComp,
        bool bStart
    );
};
