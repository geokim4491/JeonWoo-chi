// Copyright Epic Games, Inc. All Rights Reserved.

#include "JeonwoochiCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Public/Enum/CharacterTypeEnum.h"
#include "Components/PlayerSkills/PlayerSkillComponent.h"
#include "Components/PlayerSkills/illusionSkillComponent.h"
#include "Components/PlayerSkills/CloneSpawnComponent.h"
#include "Components/PlayerSkills/BlinkComponent.h"
#include "Components/Combat/ExecutionComponent.h"
#include "Components/Combat/AmbushComponent.h"
#include "Components/Combat/ChiComponent.h"
#include "Components/Combat/HealthComponent.h"
#include "Components/Combat/PlayerAttackComponent.h"
#include "Components/Combat/PlayerDefensiveComponent.h"
#include "Components/Combat/PlayerDamageableComponent.h"
#include "Components/Combat/CombatModeComponent.h"
#include "Components/Utilities/LockOnComponent.h"
#include "Components/Utilities/WeaponManageComponent.h"
#include "Components/Utilities/PlayerInteractComponent.h"
#include "Components/Utilities/PlayerStatComponent.h"
#include "Components/Utilities/PlayerLevelComponent.h"

#include "Components/Utilities/PlayerUIComponent.h"
#include "Controller/JeonwoochiPlayerController.h"

#include "Manager/EffectManager.h"
#include "Sound/MusicSubsystem.h"
#include "Kismet/GameplayStatics.h"

#include "UI/FadeInHoldOutWidget.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AJeonwoochiCharacter

AJeonwoochiCharacter::AJeonwoochiCharacter()
{
	CharacterType = ECharacterType::Player;
	TeamId = FGenericTeamId(static_cast<uint8>(CharacterType));
	bIsRunToggled = false;

	SetGenericTeamId(TeamId);
	StimuliSourceComp->RegisterWithPerceptionSystem();

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 800.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Components
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	HealthComp->MaxHealth = 100.0f;

	ChiComp = CreateDefaultSubobject<UChiComponent>(TEXT("ChiComp"));
	ChiComp->SetMaxChi(100.f);
	ChiComp->SetCurrentChi(100.f);

	DamageableComp = CreateDefaultSubobject<UPlayerDamageableComponent>(TEXT("DamageableComp"));

	DefensiveComp = CreateDefaultSubobject<UPlayerDefensiveComponent>(TEXT("DefensiveComp"));

	AttackComp = CreateDefaultSubobject<UPlayerAttackComponent>(TEXT("AttackComp"));

	CombatModeComp = CreateDefaultSubobject<UCombatModeComponent>(TEXT("CombatModeComp"));

	SkillComp = CreateDefaultSubobject<UPlayerSkillComponent>(TEXT("SkillComp"));

	LockOnComp = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComp"));

	PlayerStatComp = CreateDefaultSubobject<UPlayerStatComponent>(TEXT("PlayerStatComp"));

	PlayerLevelComp = CreateDefaultSubobject<UPlayerLevelComponent>(TEXT("PlayerLevelComp"));

	WeaponManageComp = CreateDefaultSubobject<UWeaponManageComponent>(TEXT("WeaponManageComp"));

	ExecutionComp = CreateDefaultSubobject<UExecutionComponent>(TEXT("ExecutionComp"));

	BlinkComp = CreateDefaultSubobject<UBlinkComponent>(TEXT("BlinkComp"));

	AmbushComp = CreateDefaultSubobject<UAmbushComponent>(TEXT("AmbushComp"));

	InteractComp = CreateDefaultSubobject<UPlayerInteractComponent>(TEXT("InteractComp"));

	UIComp = CreateDefaultSubobject<UPlayerUIComponent>(TEXT("UIComp"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AJeonwoochiCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyIMC_Default();
}

void AJeonwoochiCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ApplyIMC_Default();
}

//////////////////////////////////////////////////////////////////////////
// Input
UEnhancedInputLocalPlayerSubsystem* AJeonwoochiCharacter::GetInputSubsystem() const
{
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	}
	return nullptr;
}

void AJeonwoochiCharacter::ClearAllIMC()
{
	if (auto* Sys = GetInputSubsystem())
	{
		Sys->ClearAllMappings();
	}
}

void AJeonwoochiCharacter::RemoveIMC(UInputMappingContext* Context)
{
	if (auto* Sys = GetInputSubsystem())
	{
		if (Context) Sys->RemoveMappingContext(Context);
		UE_LOG(LogTemp, Display, TEXT("Stop: %s"), *Context->GetName());
	}
}

void AJeonwoochiCharacter::ApplyIMC_Default()
{
	if (auto* Sys = GetInputSubsystem())
	{
		ClearAllIMC();
		if (IMC_Default) Sys->AddMappingContext(IMC_Default, 0);
	}
}

void AJeonwoochiCharacter::ApplyIMC_Skill()
{
	if (auto* Sys = GetInputSubsystem())
	{
		if (IMC_Skill)	Sys->AddMappingContext(IMC_Skill, 1);
		UE_LOG(LogTemp, Display, TEXT("SkillMode"));
	}
}

void AJeonwoochiCharacter::ApplyIMC_Blink()
{
	if (auto* Sys = GetInputSubsystem())
	{
		if (IMC_Blink)	Sys->AddMappingContext(IMC_Blink, 2);
	}
}

void AJeonwoochiCharacter::ApplyIMC_Illusion()
{
	if (auto* Sys = GetInputSubsystem())
	{
		ClearAllIMC();
		if (IMC_Illusion) Sys->AddMappingContext(IMC_Illusion, 3);
	}
}

void AJeonwoochiCharacter::ApplyIMC_UI()
{
	if (auto* Sys = GetInputSubsystem())
	{
		ClearAllIMC();
		if (IMC_UI) Sys->AddMappingContext(IMC_UI, 10);
		UE_LOG(LogTemp, Display, TEXT("UI"));
	}
}

void AJeonwoochiCharacter::ApplyIMC_CutScene()
{
	if (auto* Sys = GetInputSubsystem())
	{
		if (IMC_CutScene) Sys->AddMappingContext(IMC_CutScene, 100);
	}
}

void AJeonwoochiCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	ApplyIMC_Default();
}

void AJeonwoochiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("Enhanced Input component�� �����ϴ�!"));
		return;
	}

	// ===== Default IMC�� ���ε��� �׼ǵ� =====
	// Movement
	// EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
	// EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AJeonwoochiCharacter::OnMove);
	EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Completed, this, &AJeonwoochiCharacter::OnClearMoveInput);
	EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AJeonwoochiCharacter::OnLook);
	EnhancedInputComponent->BindAction(IA_LockOn, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnLockOn);
	EnhancedInputComponent->BindAction(IA_Dodge, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnDodge);
	EnhancedInputComponent->BindAction(IA_RunHold, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnRunHoldStarted);
	EnhancedInputComponent->BindAction(IA_RunHold, ETriggerEvent::Completed, this, &AJeonwoochiCharacter::OnRunHoldCompleted);
	EnhancedInputComponent->BindAction(IA_RunHold, ETriggerEvent::Canceled, this, &AJeonwoochiCharacter::OnRunHoldCompleted);
	EnhancedInputComponent->BindAction(IA_RunToggle, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnRunToggle);

	// Interact
	EnhancedInputComponent->BindAction(IA_Interact, ETriggerEvent::Triggered, this, &AJeonwoochiCharacter::OnInteract);
	EnhancedInputComponent->BindAction(IA_SkipCutScene, ETriggerEvent::Triggered, this, &AJeonwoochiCharacter::OnSkipCutScene);
	// Combat
	EnhancedInputComponent->BindAction(IA_FastAttack, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnFastAttack);
	EnhancedInputComponent->BindAction(IA_StrongAttack, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnStrongAttack);
	EnhancedInputComponent->BindAction(IA_DashAttack, ETriggerEvent::Completed, this, &AJeonwoochiCharacter::OnDashAttack);
	EnhancedInputComponent->BindAction(IA_QTE, ETriggerEvent::Triggered, this, &AJeonwoochiCharacter::OnStruggle);
	EnhancedInputComponent->BindAction(IA_Block, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnStartBlock);
	EnhancedInputComponent->BindAction(IA_Block, ETriggerEvent::Completed, this, &AJeonwoochiCharacter::OnStopBlock);
	EnhancedInputComponent->BindAction(IA_Heal, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnHeal);

	// Skill & Illusion
	EnhancedInputComponent->BindAction(IA_Skill_Mode_RB, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnSkillMode);
	EnhancedInputComponent->BindAction(IA_Skill_Mode_RB, ETriggerEvent::Completed, this, &AJeonwoochiCharacter::OnStopSkillMode);
	EnhancedInputComponent->BindAction(IA_illusionMode, ETriggerEvent::Triggered, this, &AJeonwoochiCharacter::OnillusionMode);
	EnhancedInputComponent->BindAction(IA_illusionMode, ETriggerEvent::Completed, this, &AJeonwoochiCharacter::OnStopillusionMode);
	EnhancedInputComponent->BindAction(IA_SpawnClone, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnSpawnClone);
	EnhancedInputComponent->BindAction(IA_Blink, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnBlink);
	EnhancedInputComponent->BindAction(IA_Attack_Skill1, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnAttackSkill1);
	EnhancedInputComponent->BindAction(IA_Attack_Skill2, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnAttackSkill2);


	// UI
	EnhancedInputComponent->BindAction(IA_ToggleMainMenu, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnToggleMainMenu);

	// ===== Illusion IMC =====
	EnhancedInputComponent->BindAction(IA_Castillusion, ETriggerEvent::Started, this, &AJeonwoochiCharacter::OnCastillusion);
}

void AJeonwoochiCharacter::OnMove(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	MoveInput = MovementVector.GetSafeNormal();

	if (Controller != nullptr)
	{
		if (MovementVector.SizeSquared() <= KINDA_SMALL_NUMBER) return;
		if (AttackComp) AttackComp->TryCancelAttackByMoveInput(MovementVector);

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		if (MovementVector.Length() >= MoveInputDeadzone)
		{
			LastNonZeroMoveDir = MoveInput;                 
			LastNonZeroMoveTime = GetWorld()->GetTimeSeconds();
		}
	}
}

void AJeonwoochiCharacter::OnClearMoveInput(const FInputActionValue& Value)
{
	MoveInput = FVector2D::ZeroVector;
	// bIsRunning = false;
	
	if (bIsRunToggled)
	{
		bIsRunToggled = false;
		bIsRunning = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void AJeonwoochiCharacter::OnRunHoldStarted(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	bIsRunning = true;
}

void AJeonwoochiCharacter::OnRunHoldCompleted(const FInputActionValue& Value)
{
	if (!bIsRunToggled)
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		bIsRunning = false;
	}
}

void AJeonwoochiCharacter::OnRunToggle(const FInputActionValue& Value)
{
	bIsRunToggled = !bIsRunToggled;
	GetCharacterMovement()->MaxWalkSpeed = bIsRunToggled ? RunSpeed : WalkSpeed;
	bIsRunning = bIsRunToggled;
}

void AJeonwoochiCharacter::OnLook(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (LockOnComp && LockOnComp->bLockOnEnabled)
	{
		return;
	}

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AJeonwoochiCharacter::OnLockOn(const FInputActionValue& Value)
{
	LockOnComp->ToggleLockOn();
}

void AJeonwoochiCharacter::OnDodge(const FInputActionValue& Value)
{
	const int32 DirIdx = Get8DirIndex(MoveInput);
	const ERollDirection DirEnum = static_cast<ERollDirection>(DirIdx);

	if (UAnimMontage* Montage = DefensiveComp->RollMontageMap.FindRef(DirEnum))
	{
		if (!GetMesh()->GetAnimInstance()->Montage_IsPlaying(Montage))
		{
			PlayAnimMontage(Montage);
			DefensiveComp->Dodging();
		}
	}
	DefensiveComp->Dodging();
}

void AJeonwoochiCharacter::OnInteract(const FInputActionValue& Value)
{
	if(InteractComp)
		InteractComp->Interact();
}

void AJeonwoochiCharacter::OnFastAttack(const FInputActionValue& Value)
{
	if (!AttackComp) return;

	if (AmbushComp->GetCanAmbush())
	{
		AmbushComp->Ambush();
	}
	else if (GetCharacterMovement()->IsFalling() || GetCharacterMovement()->Velocity.Z > 0.f)
	{
		// No AirAttack for now
		// AttackComp->AirAttack();
	}
	else
	{
		AttackComp->FastAttack();
	}
	SetSoftCombatMode();
}

void AJeonwoochiCharacter::OnDashAttack(const FInputActionValue& Value)
{
	if (!AttackComp) return;

	UE_LOG(LogTemplateCharacter, Log, TEXT("Dash Attack (Y Hold)"));
	// if(LockOnComp && LockOnComp->bLockOnEnabled) { // 모션워핑 }
	AttackComp->DashAttack();

	SetSoftCombatMode();
}

void AJeonwoochiCharacter::OnStrongAttack(const FInputActionValue& Value)
{
	if (!AttackComp) return;
	if (GetCharacterMovement()->IsFalling() || GetCharacterMovement()->Velocity.Z > 0.f) return;

	if (ExecutionComp->GetCanExecute())
	{
		ExecutionComp->Execute();
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("Strong Attack (Y)"));
		AttackComp->StrongAttack();
	}
	SetSoftCombatMode();
}

void AJeonwoochiCharacter::OnStruggle(const FInputActionValue& Value)
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("Struggle Action! (X Mash)"));
	// QTE
}

void AJeonwoochiCharacter::OnStartBlock(const FInputActionValue& Value)
{
	bIsBlocking = true;
	DefensiveComp->Blocking();
	SetSoftCombatMode();
}

void AJeonwoochiCharacter::OnStopBlock(const FInputActionValue& Value)
{
	bIsBlocking = false;
	DefensiveComp->StopBlocking();
}

void AJeonwoochiCharacter::OnHeal(const FInputActionValue& Value)
{
	if (ChiComp->GetCurrentChi() >= 20.f && bCanHeal)
	{
		float len = PlayAnimMontage(HealAnim);
		ChiComp->UpdateCurrentChi(-30.f);
		SetSoftCombatMode();
		bCanHeal = false;

		GetWorld()->GetTimerManager().SetTimer(
			HealTimer,
			[this]() { bCanHeal = true; },
			len, false
			);
	}
	else
	{
		UGameplayStatics::PlaySound2D(GetWorld(), SkillFailSound);
	}
}

void AJeonwoochiCharacter::OnSkillMode(const FInputActionValue& Value)
{
	ApplyIMC_Skill();
	SetSoftCombatMode();
}

void AJeonwoochiCharacter::OnStopSkillMode(const FInputActionValue& Value)
{
	RemoveIMC(IMC_Skill);
}

void AJeonwoochiCharacter::OnSpawnClone(const FInputActionValue& Value)
{
	TArray<FVector> Points;
	FVector Origin = GetActorLocation();
	FVector Forward = GetActorForwardVector();
	FRotator Rot = GetActorRotation();
	Points.Add(Origin + Rot.RotateVector(FVector(200, 0, 150)));
	Points.Add(Origin + Rot.RotateVector(FVector(0, -200, 150)));
	Points.Add(Origin + Rot.RotateVector(FVector(0, 200, 150)));

	if (SkillComp->CloneChiConsumption > ChiComp->GetCurrentChi())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), SkillFailSound);
			return;
	}

	if (SkillComp->CloneSpawnComp->SpawnClones(Points))
	{
		ChiComp->UpdateCurrentChi(-SkillComp->CloneChiConsumption);
	}
}

void AJeonwoochiCharacter::OnAttackSkill1(const FInputActionValue& Value)
{
	AttackComp->SkillAttack1();
}
void AJeonwoochiCharacter::OnAttackSkill2(const FInputActionValue& Value)
{
	AttackComp->SkillAttack2();
}

void AJeonwoochiCharacter::OnBlink(const FInputActionValue& Value)
{
	constexpr float ForwardThreshold = 0.5f;
	const bool bForward = (MoveInput.Y > ForwardThreshold);
	
	if (bForward && BlinkComp->GetCanBlink())
	{
		BlinkComp->Blink();
	}
}

void AJeonwoochiCharacter::OnillusionMode(const FInputActionValue& Value)
{
	SkillComp->illusionComp->illusionMode();
	if (!SkillComp->illusionComp->IsValidillusionTarget())
	{
		ApplyIMC_Illusion();
	}
}

void AJeonwoochiCharacter::OnStopillusionMode(const FInputActionValue& Value)
{
	ApplyIMC_Default();
	SkillComp->illusionComp->StopillusionMode();
}

void AJeonwoochiCharacter::OnCastillusion(const FInputActionValue& Value)
{
	ApplyIMC_Default();
	
	if (SkillComp->illusionChiConsumption > ChiComp->GetCurrentChi())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), SkillFailSound);
	}

	if (SkillComp->illusionComp->Castillusion())
	{
		ChiComp->UpdateCurrentChi(-SkillComp->illusionChiConsumption);
	}
}

void AJeonwoochiCharacter::OnSkipCutScene(const FInputActionValue& Value)
{
	if (CachedBAComp)
	{
		CachedBAComp->SkipCutScene();
	}	
	else if (CachedBPComp)
		CachedBPComp->SkipCutScene();
}

void AJeonwoochiCharacter::OnToggleMainMenu(const FInputActionValue& Value)
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("Toggle Main"));
	if (auto* PC = Cast<AJeonwoochiPlayerController>(GetController()))
	{
		PC->ToggleMainMenu();
	}
}

//////////////////////////////////////////////////////////////////////////
// 내부함수
 
int32 AJeonwoochiCharacter::Get8DirIndex(const FVector2D& In) const
{
	if (!LockOnComp->bLockOnEnabled) // Lock On
		return 0; //

	if (In.SizeSquared() < 0.02f)
		return 4; // 

	float Deg = FMath::Atan2(In.Y, In.X) * 180.f / PI;      // -180~180
	if (Deg < 0.f) Deg += 360.f;

	int32 Bucket = FMath::FloorToInt((Deg + 22.5f) / 45.f);     // -1~8
	Bucket = (Bucket % 8 + 8) % 8;                              // 0~7

	int32 Index = (2 - Bucket + 8) % 8;                         // 0~7

	return Index;
}

void AJeonwoochiCharacter::SetSoftCombatMode()
{
	CombatModeComp->NotifyPlayerAction_Soft();
}

UHealthComponent* AJeonwoochiCharacter::GetHealthComponent() const
{
	return HealthComp;
}

UPlayerDefensiveComponent* AJeonwoochiCharacter::GetPlayerDefensiveComponent()
{
	return DefensiveComp;
}

//////////////////////////////////////////////////////////////////////////
// Interface

void AJeonwoochiCharacter::TakeDamage_Implementation(const FDamageInfoStruct& DamageInfo)
{
	DamageableComp->HandleTakeDamage(DamageInfo);
}

float AJeonwoochiCharacter::GetMaxHealth() const
{
	return HealthComp->GetMaxHealth();
}

float AJeonwoochiCharacter::GetCurrentHealth_Implementation() const
{
	return HealthComp->GetCurrentHealth();
}

void AJeonwoochiCharacter::ApplyDamage_Implementation(const float& DamageAmount)
{
	HealthComp->ApplyDamage(DamageAmount);
}

void AJeonwoochiCharacter::ApplyDotDamage_Implementation(const FDotDealInfoStruct& DotInfo)
{
	HealthComp->ApplyDotDamage(DotInfo);
}

void AJeonwoochiCharacter::IncreaseMaxHP()
{
}

void AJeonwoochiCharacter::IncreaseMaxMP()
{
}

void AJeonwoochiCharacter::IncreaseMaxAP()
{
}

void AJeonwoochiCharacter::IncreaseMaxIntP()
{
}

void AJeonwoochiCharacter::IncreaseMaxSkillP()
{
}

void AJeonwoochiCharacter::IncreaseMaxMystP()
{
}

void AJeonwoochiCharacter::IncreaseMaxDefP()
{
}

void AJeonwoochiCharacter::Die_Implementation()
{
	ClearAllIMC();

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
		Move->StopMovementImmediately();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if(LockOnComp) LockOnComp->ClearLockOn();
	
	PlayAnimMontage(DieAnim);

	if (StimuliSourceComp) StimuliSourceComp->UnregisterFromPerceptionSystem();

	// Sound 및 VFX
	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
	{
		if (UEffectManager* EM = GI->GetSubsystem<UEffectManager>())
		{
			EM->PlayEffectOnActor(
				EEffectType::PlayerDie,
				GetOwner()
			);
		}

		if (UMusicSubsystem* MM = World->GetSubsystem<UMusicSubsystem>())
		{
			MM->StopBGM(0.f);
		}
	}

	// UI 띄우기
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (AJeonwoochiPlayerController* JPC = Cast<AJeonwoochiPlayerController>(PC))
		{
			JPC->ShowYouDiedUI();
		}
	}
}

void AJeonwoochiCharacter::ReviveFromRest()
{
	// 1) 모션/애님
	if (DieAnim && GetMesh() && GetMesh()->GetAnimInstance())
	{
		if (GetMesh()->GetAnimInstance()->Montage_IsPlaying(DieAnim))
			StopAnimMontage(DieAnim);
	}
	// (선택) 일어나기 모션이 있으면 여기서 PlayAnimMontage(GetUpMontage);

	// 2) 이동/충돌 되살리기
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetMovementMode(MOVE_Walking);
		Move->StopMovementImmediately();
	}
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 프로젝트 기본값으로
		// 필요 시 채널 응답 원래값으로 복구
	}

	// 3) IMC/입력/락온
	ApplyIMC_Default();            // Die 때 ClearAllIMC() 했으니 기본 IMC 다시 추가
	if (LockOnComp) LockOnComp->ClearLockOn();

	// 4) Perception 자극원 재등록
	if (StimuliSourceComp) StimuliSourceComp->RegisterWithPerceptionSystem();

}

void AJeonwoochiCharacter::ClearLockOn()
{
	if (LockOnComp)
	{
		LockOnComp->ClearLockOn();
	}
}

//////////////////////////////////////////////////////////////////////////
// EndPlay

void AJeonwoochiCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(HealTimer);
	Super::EndPlay(EndPlayReason);
}