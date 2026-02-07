// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PaperAnimationComponent.generated.h"

class UPaperFlipbook;
class UPaperFlipbookComponent;

UENUM(BlueprintType)
enum class EAnimState : uint8
{
	Idle,
	Walk,
	Run,
	SideJump,
	Jump,
	Attack1,
	Attack2,
	Hit,
	Falling,
	FallEnd,
	Dead
};

UENUM(BlueprintType)
enum class EAnimEvent : uint8
{
	LocomotionUpdate,   // 파라미터 동봉(지상/속도) — 내부에서만 사용
	AttackPressed,
	JumpPressed,
	HitReceived,
	Death,
	FlipbookFinished,
	BTCalled
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UPaperAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPaperAnimationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Flipbooks")
	UPaperFlipbook* FB_Idle;

	UPROPERTY(EditAnywhere, Category = "Flipbooks")
	UPaperFlipbook* FB_Walk;

	UPROPERTY(EditAnywhere, Category = "Flipbooks")
	UPaperFlipbook* FB_Run;

	UPROPERTY(EditAnywhere, Category = "Flipbooks")
	UPaperFlipbook* FB_Jump;

	UPROPERTY(EditAnywhere, Category = "Flipbooks")
	UPaperFlipbook* FB_SideJump;

	UPROPERTY(EditAnywhere, Category = "Flipbooks")
	UPaperFlipbook* FB_Attack1;

	UPROPERTY(EditAnywhere, Category = "Flipbooks")
	UPaperFlipbook* FB_Attack2;

	UPROPERTY(EditAnywhere, Category = "Flipbooks")
	UPaperFlipbook* FB_Hit;

	UPROPERTY(EditAnywhere, Category = "Flipbooks")
	UPaperFlipbook* FB_FallStart;

	UPROPERTY(EditAnywhere, Category = "Flipbooks")
	UPaperFlipbook* FB_FallEnd;

	UPROPERTY(EditAnywhere, Category = "Flipbooks")
	UPaperFlipbook* FB_Dead;

	UPROPERTY(VisibleAnywhere, Category = "Flipbooks")
	EAnimState State;
	bool bQueuedNext;
	EAnimState QueuedNextState;

	UPROPERTY() 
	UPaperFlipbookComponent* FlipbookComp;

	UPROPERTY()
	class AJeonwoochiPaperCharacter* Character = nullptr;
protected:
	virtual void Enter(EAnimState Next);
	void Exit(EAnimState Prev);
	void PlayLoop(UPaperFlipbook* FB);
	void PlayOnce(UPaperFlipbook* FB);

	// Flipbook 종료 콜백
	UFUNCTION() void HandleFlipbookFinished();

	// 이벤트 처리
	virtual void Dispatch(EAnimEvent Event, bool bIsOnGround = false, float Speed = 0.f, float WalkT = 0.f, float RunT = 0.f);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void Init(UPaperFlipbookComponent* InFlipbookComp);

	UFUNCTION(BlueprintCallable) 
	void OnAttackPressed();
	UFUNCTION(BlueprintCallable) 
	void OnJumpPressed();
	UFUNCTION(BlueprintCallable) 
	void OnHitReceived();

	UFUNCTION(BlueprintCallable)
	void OnLocomotionUpdate(bool bIsOnGround, float Speed, float WalkThreshold, float RunThreshold);

	UFUNCTION(BlueprintPure) 
	EAnimState GetState() const { return State; }


};
