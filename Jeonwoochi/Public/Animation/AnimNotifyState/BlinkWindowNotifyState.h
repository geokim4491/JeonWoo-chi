// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "BlinkWindowNotifyState.generated.h"

/**
 * 
 */

DECLARE_DELEGATE_TwoParams(FBlinkWindowDelegate, AActor* /*Attacker*/, bool /*bOpen*/);

UCLASS()
class JEONWOOCHI_API UBlinkWindowNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	static void Bind(const FBlinkWindowDelegate& InDelegate);
	static void Unbind();

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp, 
		UAnimSequenceBase* Animation, 
		float TotalDuration
	) override;
	
	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp, 
		UAnimSequenceBase* Animation
	) override;
	
private:
	static FBlinkWindowDelegate OnBlinkWindow;
};
