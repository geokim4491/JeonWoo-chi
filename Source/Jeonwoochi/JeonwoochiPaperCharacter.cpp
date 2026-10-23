// Fill out your copyright notice in the Description page of Project Settings.


#include "JeonwoochiPaperCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/2D/PaperAnimationComponent.h"
#include "Components/Combat/HealthComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h" 
#include "InputMappingContext.h"
#include "PaperFlipbookComponent.h"
#include "Components/ArrowComponent.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/AI/PaperAICharacter.h"

#include "Manager/EffectManager.h"

AJeonwoochiPaperCharacter::AJeonwoochiPaperCharacter()
{
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a Animation Component
	AnimComponent = CreateDefaultSubobject<UPaperAnimationComponent>(TEXT("AnimFSM"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	FacingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FacingArrow"));
	FacingArrow->SetupAttachment(RootComponent);
	FacingArrow->ArrowSize = 1.5f;
	FacingArrow->bHiddenInGame = false;

}

void AJeonwoochiPaperCharacter::BeginPlay()
{
	Super::BeginPlay();

	UPaperFlipbookComponent* SpriteRef = GetSprite();
	check(SpriteRef);

	InitialSpriteScale = SpriteRef->GetRelativeScale3D();
	AnimComponent->Init(SpriteRef);

	FacingArrowBaseRot = FacingArrow->GetRelativeRotation();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(IMC_2D, 0);
		}
	}
}

void AJeonwoochiPaperCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);


	const bool  bOnGround = GetCharacterMovement()->IsMovingOnGround();
	const float Speed = GetVelocity().Size2D();

	// Idle/Walk/Run 판정은 항상 호출해도 FSM 내부에서 안전 처리
	AnimComponent->OnLocomotionUpdate(bOnGround, Speed, /*Walk*/5.f, /*Run*/250.f);

	// 좌우 반전
	const float X = GetVelocity().X;
	if (FMath::Abs(X) > 1.f)
	{
		FVector NewScale = GetSprite()->GetRelativeScale3D();
		NewScale.X = bFacingRight ? FMath::Abs(NewScale.X) : -FMath::Abs(NewScale.X);
		GetSprite()->SetRelativeScale3D(NewScale);
	}
}

void AJeonwoochiPaperCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}
	// Movement
	EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AJeonwoochiPaperCharacter::OnMove);

	EnhancedInputComponent->BindAction(IA_Attack, ETriggerEvent::Started, this, &AJeonwoochiPaperCharacter::OnAttack);

	EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &AJeonwoochiPaperCharacter::OnJump);
	EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
}

void AJeonwoochiPaperCharacter::OnMove(const FInputActionValue& Value)
{
	if (AnimComponent->GetState() == EAnimState::Attack1 || AnimComponent->GetState() == EAnimState::Attack2) return;
	float Axis = Value.Get<float>();
	if (FMath::Abs(Axis) > 0.01f)
	{
		SetFacing(Axis > 0.f); // ★ 입력 부호로 좌/우 결정(Arrow도 함께 회전)

		// Arrow가 가리키는 '정면'으로 전진(아날로그 입력 크기 반영)
		AddMovementInput(FacingArrow->GetForwardVector(), FMath::Abs(Axis));
	}
}

void AJeonwoochiPaperCharacter::OnAttack(const FInputActionValue& Value)
{
	AnimComponent->OnAttackPressed();
}

void AJeonwoochiPaperCharacter::OnJump()
{
	Jump();                    // 실제 점프
	AnimComponent->OnJumpPressed();  // Jump Flipbook 1회 재생
}

void AJeonwoochiPaperCharacter::ApplyIMC()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(IMC_2D, 0);
		}
	}
}
void AJeonwoochiPaperCharacter::SetFacing(bool bRight)
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

ECharacterType AJeonwoochiPaperCharacter::GetCharacterType_Implementation()
{
	return CharacterType;
}

void AJeonwoochiPaperCharacter::Attack1()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::PlaySound2D(World, AttackSound);

	const FVector Origin = GetActorLocation() + GetActorForwardVector() * 100.f;
	const float Radius = 300.f;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); // Pawn만

	TArray<AActor*> Ignore; Ignore.Add(this);

	TArray<AActor*> OutActors;
	const bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		World, Origin, Radius, ObjTypes, /*ClassFilter*/APaperAICharacter::StaticClass(), Ignore, OutActors
	);

	if (!bHit) return;

	for (AActor* Other : OutActors)
	{
		if (!Other || Other == this) continue;
		UHealthComponent* HealthComp = Other->FindComponentByClass<UHealthComponent>();
		if (!HealthComp) return;
		HealthComp->ApplyDamage(AttackDamage);
		// UGameplayStatics::PlaySound2D(World, HitSound);
		PlayEffect(EEffectType::EnemyHit);
	}
}

void AJeonwoochiPaperCharacter::Attack2()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::PlaySound2D(World, AttackSound);

	const FVector Origin = GetActorLocation() + GetActorForwardVector() * 100.f;
	const float Radius = 300.f;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); // Pawn만

	TArray<AActor*> Ignore; Ignore.Add(this);

	TArray<AActor*> OutActors;
	const bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		World, Origin, Radius, ObjTypes, /*ClassFilter*/APaperAICharacter::StaticClass(), Ignore, OutActors
	);

	if (!bHit) return;

	for (AActor* Other : OutActors)
	{
		if (!Other || Other == this) continue;
		UHealthComponent* HealthComp = Other->FindComponentByClass<UHealthComponent>();
		if (!HealthComp) return;
		HealthComp->ApplyDamage(AttackDamage);
		// UGameplayStatics::PlaySound2D(World, HitSound);
		PlayEffect(EEffectType::EnemyHit);
	}
}

void AJeonwoochiPaperCharacter::PlayEffect(EEffectType EffectType)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
	{
		if (UEffectManager* EM = GI->GetSubsystem<UEffectManager>())
		{
			EM->PlayEffectOnActor(
				EffectType,
				GetOwner()
			);
		}
	}
}