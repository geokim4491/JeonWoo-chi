// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PaperAnimationComponent.h"
#include "PaperAIAnimationComponent.generated.h"


class UPaperFlipbook;
class UPaperFlipbookComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class JEONWOOCHI_API UPaperAIAnimationComponent : public UPaperAnimationComponent
{
	GENERATED_BODY()
public:
	EAnimState BTCallingState;

protected:
	virtual void BeginPlay() override;

	class APaperAICharacter* OwnerCharacter;

public:
	virtual void Enter(EAnimState Next) override;
	// 이벤트 처리
	virtual void Dispatch(EAnimEvent Event, bool bIsOnGround = false, float Speed = 0.f, float WalkT = 0.f, float RunT = 0.f) override;

};
