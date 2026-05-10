// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AI/PaperAICharacter.h"
#include "Components/2D/PaperAIAnimationComponent.h"
#include "Components/Combat/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "Components/ArrowComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

APaperAICharacter::APaperAICharacter()
{
	AnimationComponent = CreateDefaultSubobject<UPaperAIAnimationComponent>(TEXT("AnimationComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	GetCharacterMovement()->MaxWalkSpeed = 400.0f;

	FacingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingArrow"));
	FacingArrow->SetupAttachment(RootComponent);
	FacingArrow->ArrowSize = 1.5f;
	FacingArrow->bHiddenInGame = false;
}

void APaperAICharacter::BeginPlay()
{
	Super::BeginPlay();

	UPaperFlipbookComponent* SpriteRef = GetSprite();
	check(SpriteRef);

	InitialSpriteScale = SpriteRef->GetRelativeScale3D();
	AnimationComponent->Init(SpriteRef);

	FacingArrowBaseRot = FacingArrow->GetRelativeRotation();
}

void APaperAICharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const bool  bOnGround = GetCharacterMovement()->IsMovingOnGround();
	const float Speed = GetVelocity().Size2D();

	// Idle/Walk/Run 판정은 항상 호출해도 FSM 내부에서 안전 처리
	AnimationComponent->OnLocomotionUpdate(bOnGround, Speed, /*Walk*/5.f, /*Run*/250.f);

	// 좌우 반전
	const float X = GetVelocity().X;
	if (FMath::Abs(X) > 1.f)
	{
		FVector NewScale = GetSprite()->GetRelativeScale3D();
		NewScale.X = bFacingRight ? FMath::Abs(NewScale.X) : -FMath::Abs(NewScale.X);
		GetSprite()->SetRelativeScale3D(NewScale);
	}
}

void APaperAICharacter::StopBT()
{
	if (!OwnerController)
	{
		OwnerController = Cast<AAIController>(GetController());
	}
	if (UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(OwnerController->BrainComponent))
	{
		BT->StopTree(EBTStopMode::Safe);
	}
}

void APaperAICharacter::RestartBT()
{
	if (!OwnerController)
	{
		OwnerController = Cast<AAIController>(GetController());
	}

	if (UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(OwnerController->BrainComponent))
	{
		BT->StopTree(EBTStopMode::Safe);

		BT->RestartTree();
	}
}

void APaperAICharacter::SetFacing(bool bRight)
{
	bFacingRight = bRight;

	// 1) 스프라이트 좌우 반전
	UPaperFlipbookComponent* SpriteRef = GetSprite();
	FVector NewScale = SpriteRef->GetRelativeScale3D();
	NewScale.X = bRight ? FMath::Abs(InitialSpriteScale.X)
		: -FMath::Abs(InitialSpriteScale.X);
	SpriteRef->SetRelativeScale3D(NewScale);

	// 2) FacingArrow Yaw 회전 동기화 (0도=우, 180도=좌)
	FRotator NewRot = FacingArrowBaseRot;
	NewRot.Yaw += (bRight ? 0.f : 180.f);
	FacingArrow->SetRelativeRotation(NewRot);
}

ECharacterType APaperAICharacter::GetCharacterType_Implementation()
{
	return CharacterType;
}