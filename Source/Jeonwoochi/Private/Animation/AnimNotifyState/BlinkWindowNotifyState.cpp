// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState/BlinkWindowNotifyState.h"

FBlinkWindowDelegate UBlinkWindowNotifyState::OnBlinkWindow;

void UBlinkWindowNotifyState::Bind(const FBlinkWindowDelegate& InDelegate)
{
	// 싱글캐스트: 새 바인딩이 기존 것을 덮어씀
	OnBlinkWindow = InDelegate;
}

void UBlinkWindowNotifyState::Unbind()
{
	OnBlinkWindow.Unbind();
}

void UBlinkWindowNotifyState::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration
)
{
	if (!MeshComp) return;
	if (OnBlinkWindow.IsBound())
	{
		OnBlinkWindow.Execute(MeshComp->GetOwner(), /*bOpen*/ true);
	}
}

void UBlinkWindowNotifyState::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation
)
{
	if (!MeshComp) return;
	if (OnBlinkWindow.IsBound())
	{
		OnBlinkWindow.Execute(MeshComp->GetOwner(), /*bOpen*/ false);
	}
}