//#include "JeonwoochiGASPCharacter.h"
//
//// Engine & Input
//#include "InputMappingContext.h" 
//#include "InputAction.h"
//#include "EnhancedInputComponent.h"
//#include "EnhancedInputSubsystems.h"
//#include "InputActionValue.h"
//#include "Engine/LocalPlayer.h"
//#include "GameFramework/CharacterMovementComponent.h"
//#include "Components/CapsuleComponent.h"
//#include "Types/TagTypes.h"
//
//// Perception & Motion Warping
//#include "Perception/AIPerceptionStimuliSourceComponent.h"
//#include "Perception/AISense_Sight.h"
//#include "Perception/AISense_Hearing.h"
//
//// Components
//#include "Public/Components/Combat/HealthComponent.h"
//#include "Public/Components/Combat/PlayerAttackComponent.h"
//#include "Public/Components/Combat/PlayerDefensiveComponent.h"
//#include "Public/Components/Combat/PlayerDamageableComponent.h"
//#include "Public/Components/Utilities/LockOnComponent.h"
//#include "Public/Components/Utilities/WeaponManageComponent.h"
//#include "Public/Components/Utilities/PlayerUIComponent.h"
//#include "Public/Components/PlayerSkills/PlayerSkillComponent.h"
//#include "Public/Components/PlayerSkills/illusionSkillComponent.h"
//#include "Public/Components/PlayerSkills/CloneSpawnComponent.h"
//#include "Public/Components/Combat/ExecutionComponent.h"
//
//DEFINE_LOG_CATEGORY(LogJeonwoochiGASP);
//
//// -----------------------------------------------------
//// Constructor
//AJeonwoochiGASPCharacter::AJeonwoochiGASPCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
//{
//	PrimaryActorTick.bCanEverTick = false;
//
//	// ===== BaseCharacter 이식: 타입/팀/지각/모션워핑 =====
//	CharacterType = ECharacterType::Player;
//	TeamId = FGenericTeamId(static_cast<uint8>(CharacterType));
//	SetGenericTeamId(TeamId);
//
//	StimuliSourceComp = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComp"));
//	if (StimuliSourceComp)
//	{
//		StimuliSourceComp->RegisterForSense(UAISense_Sight::StaticClass());
//		StimuliSourceComp->RegisterForSense(UAISense_Hearing::StaticClass());
//		StimuliSourceComp->RegisterWithPerceptionSystem();
//	}
//
//	// 카메라 충돌 최소화
//	// GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
//	// GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
//
//	// ===== 전우치 고유 컴포넌트 =====
//	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
//	DamageableComp = CreateDefaultSubobject<UPlayerDamageableComponent>(TEXT("DamageableComp"));
//	DefensiveComp = CreateDefaultSubobject<UPlayerDefensiveComponent>(TEXT("DefensiveComp"));
//	AttackComp = CreateDefaultSubobject<UPlayerAttackComponent>(TEXT("AttackComp"));
//	SkillComp = CreateDefaultSubobject<UPlayerSkillComponent>(TEXT("SkillComp"));
//	LockOnComp = CreateDefaultSubobject<ULockOnComponent>(TEXT("LockOnComp"));
//	WeaponManageComp = CreateDefaultSubobject<UWeaponManageComponent>(TEXT("WeaponManageComp"));
//	ExecutionComp = CreateDefaultSubobject<UExecutionComponent>(TEXT("ExecutionComp"));
//
//	if (HealthComp) { HealthComp->MaxHealth = 100.f; }
//
//	// PoseMode 고정
//	PoseMode = PoseModeTags::Default;   
//	OverlayMode = OverlayModeTags::Default;
//
//	// 이동과 카메라 세팅은 GASP가 담당
//}
//
//// -----------------------------------------------------
//// lifecycle
//void AJeonwoochiGASPCharacter::BeginPlay()
//{
//	Super::BeginPlay();
//
//	ApplyIMC_Default();
//}
//
//void AJeonwoochiGASPCharacter::PossessedBy(AController* NewController)
//{
//	Super::PossessedBy(NewController);
//	ApplyIMC_Default();
//}
//
//void AJeonwoochiGASPCharacter::NotifyControllerChanged()
//{
//	Super::NotifyControllerChanged();
//	ApplyIMC_Default();
//}
//
//void AJeonwoochiGASPCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
//{
//	if (StimuliSourceComp)
//	{
//		StimuliSourceComp->UnregisterFromPerceptionSystem();
//	}
//	Super::EndPlay(EndPlayReason);
//}
//
//// -----------------------------------------------------
//// Input setup
//void AJeonwoochiGASPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
//{
//	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
//	if (!EIC)
//	{
//		UE_LOG(LogJeonwoochiGASP, Error, TEXT("EnhancedInputComponent not found on %s"), *GetName());
//		return;
//	}
//
//	// === 이동/시야/점프(GASP) ===
//	EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &ThisClass::OnJumpStarted);
//	EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ThisClass::OnJumpCompleted);
//	EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ThisClass::OnMove);
//	EIC->BindAction(IA_Move, ETriggerEvent::Completed, this, &ThisClass::OnClearMoveInput);
//	EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ThisClass::OnLook);
//
//	// === 이동 보조 ===
//	EIC->BindAction(IA_RunHold, ETriggerEvent::Started, this, &ThisClass::OnRunHoldStarted);
//	EIC->BindAction(IA_RunHold, ETriggerEvent::Completed, this, &ThisClass::OnRunHoldCompleted);
//	EIC->BindAction(IA_RunHold, ETriggerEvent::Canceled, this, &ThisClass::OnRunHoldCompleted);
//	EIC->BindAction(IA_RunToggle, ETriggerEvent::Started, this, &ThisClass::OnRunToggle);
//
//	// === 전투/방어/회피 ===
//	EIC->BindAction(IA_FastAttack, ETriggerEvent::Started, this, &ThisClass::OnFastAttack);
//	EIC->BindAction(IA_StrongAttack, ETriggerEvent::Started, this, &ThisClass::OnStrongAttack);
//	EIC->BindAction(IA_DashAttack, ETriggerEvent::Triggered, this, &ThisClass::OnDashAttack);
//	EIC->BindAction(IA_QTE, ETriggerEvent::Triggered, this, &ThisClass::OnStruggle);
//	EIC->BindAction(IA_Block, ETriggerEvent::Started, this, &ThisClass::OnStartBlock);
//	EIC->BindAction(IA_Block, ETriggerEvent::Completed, this, &ThisClass::OnStopBlock);
//	EIC->BindAction(IA_Dodge, ETriggerEvent::Triggered, this, &ThisClass::OnDodge);
//
//	// === 락온/스킬/일루전/클론 ===
//	EIC->BindAction(IA_LockOn, ETriggerEvent::Started, this, &ThisClass::OnLockOn);
//	EIC->BindAction(IA_Skill_Mode_RB, ETriggerEvent::Started, this, &ThisClass::OnSkillMode);
//	EIC->BindAction(IA_Skill_Mode_RB, ETriggerEvent::Completed, this, &ThisClass::OnStopSkillMode);
//	EIC->BindAction(IA_illusionMode, ETriggerEvent::Triggered, this, &ThisClass::OnillusionMode);
//	EIC->BindAction(IA_illusionMode, ETriggerEvent::Completed, this, &ThisClass::OnStopillusionMode);
//	EIC->BindAction(IA_SpawnClone, ETriggerEvent::Started, this, &ThisClass::OnSpawnClone);
//	EIC->BindAction(IA_Castillusion, ETriggerEvent::Started, this, &ThisClass::OnCastillusion);
//}
//
//// -----------------------------------------------------
//// GASP 래퍼(이동/시야/점프/스프린트)
//void AJeonwoochiGASPCharacter::OnJumpStarted(const FInputActionValue&) { JumpAction(true); }
//void AJeonwoochiGASPCharacter::OnJumpCompleted(const FInputActionValue&) { JumpAction(false); }
//
//void AJeonwoochiGASPCharacter::OnMove(const FInputActionValue& Value)
//{
//	const FVector2D Axis = Value.Get<FVector2D>();
//	MoveInput = Axis.GetSafeNormal();
//	Super::MoveAction(Axis);
//}
//
//void AJeonwoochiGASPCharacter::OnClearMoveInput(const FInputActionValue&)
//{
//	MoveInput = FVector2D::ZeroVector;
//	if (!bIsRunToggled)
//	{
//		// GASP가 내부 게이트로 처리 -> 별도 처리 없음
//	}
//}
//
//void AJeonwoochiGASPCharacter::OnLook(const FInputActionValue& Value)
//{
//	if (LockOnComp && LockOnComp->bLockOnEnabled) return;
//	Super::LookAction(Value.Get<FVector2D>());
//}
//
//void AJeonwoochiGASPCharacter::OnRunHoldStarted(const FInputActionValue&) { SprintAction(true); }
//void AJeonwoochiGASPCharacter::OnRunHoldCompleted(const FInputActionValue&) { if (!bIsRunToggled) SprintAction(false); }
//void AJeonwoochiGASPCharacter::OnRunToggle(const FInputActionValue&)
//{
//	bIsRunToggled = !bIsRunToggled;
//	SprintAction(bIsRunToggled);
//}
//
//// -----------------------------------------------------
//// 전투/방어/회피/락온/스킬
//void AJeonwoochiGASPCharacter::OnFastAttack(const FInputActionValue&)
//{
//	if (!AttackComp) return;
//
//	if (GetCharacterMovement()->IsFalling() || GetCharacterMovement()->Velocity.Z > 0.f)
//	{
//		AttackComp->AirAttack();
//	}
//	else
//	{
//		AttackComp->FastAttack();
//	}
//}
//
//void AJeonwoochiGASPCharacter::OnStrongAttack(const FInputActionValue&)
//{
//	if (!AttackComp) return;
//
//	if (ExecutionComp && ExecutionComp->GetCanExecute())
//	{
//		ExecutionComp->Execute();
//	}
//	else
//	{
//		UE_LOG(LogJeonwoochiGASP, Log, TEXT("Strong Attack"));
//		// AttackComp->StrongAttack(); // 필요 시 사용
//	}
//}
//
//void AJeonwoochiGASPCharacter::OnDashAttack(const FInputActionValue&)
//{
//	if (!AttackComp) return;
//	UE_LOG(LogJeonwoochiGASP, Log, TEXT("Dash Attack"));
//	AttackComp->DashAttack();
//}
//
//void AJeonwoochiGASPCharacter::OnStruggle(const FInputActionValue&)
//{
//	UE_LOG(LogJeonwoochiGASP, Log, TEXT("Struggle(QTE)"));
//}
//
//void AJeonwoochiGASPCharacter::OnStartBlock(const FInputActionValue&) { if (DefensiveComp) DefensiveComp->Blocking(); }
//void AJeonwoochiGASPCharacter::OnStopBlock(const FInputActionValue&) { if (DefensiveComp) DefensiveComp->StopBlocking(); }
//
//void AJeonwoochiGASPCharacter::OnDodge(const FInputActionValue&)
//{
//	if (!DefensiveComp || !GetMesh() || !GetMesh()->GetAnimInstance()) return;
//
//	const int32 DirIdx = Get8DirIndex(MoveInput);
//	const ERollDirection DirEnum = static_cast<ERollDirection>(DirIdx);
//
//	if (UAnimMontage* Montage = DefensiveComp->RollMontageMap.FindRef(DirEnum))
//	{
//		if (!GetMesh()->GetAnimInstance()->Montage_IsPlaying(Montage))
//		{
//			PlayAnimMontage(Montage);
//			DefensiveComp->Dodging();
//			return;
//		}
//	}
//	DefensiveComp->Dodging();
//}
//
//void AJeonwoochiGASPCharacter::OnLockOn(const FInputActionValue&)
//{
//	if (LockOnComp) LockOnComp->ToggleLockOn();
//}
//
//void AJeonwoochiGASPCharacter::OnSkillMode(const FInputActionValue&) { ApplyIMC_Skill(); }
//void AJeonwoochiGASPCharacter::OnStopSkillMode(const FInputActionValue&) { RemoveIMC(IMC_Skill); }
//
//void AJeonwoochiGASPCharacter::OnillusionMode(const FInputActionValue&)
//{
//	ApplyIMC_Illusion();
//	if (SkillComp && SkillComp->illusionComp) SkillComp->illusionComp->illusionMode();
//}
//
//void AJeonwoochiGASPCharacter::OnStopillusionMode(const FInputActionValue&)
//{
//	ApplyIMC_Default();
//	if (SkillComp && SkillComp->illusionComp) SkillComp->illusionComp->StopillusionMode();
//}
//
//void AJeonwoochiGASPCharacter::OnCastillusion(const FInputActionValue&)
//{
//	ApplyIMC_Default();
//	if (SkillComp && SkillComp->illusionComp) SkillComp->illusionComp->Castillusion();
//}
//
//void AJeonwoochiGASPCharacter::OnSpawnClone(const FInputActionValue&)
//{
//	if (!SkillComp || !SkillComp->CloneSpawnComp) return;
//
//	const FVector Origin = GetActorLocation();
//	const FRotator Rot = GetActorRotation();
//
//	TArray<FVector> Points;
//	Points.Add(Origin + Rot.RotateVector(FVector(200, 0, 150)));
//	Points.Add(Origin + Rot.RotateVector(FVector(0, -200, 150)));
//	Points.Add(Origin + Rot.RotateVector(FVector(0, 200, 150)));
//
//	SkillComp->CloneSpawnComp->SpawnClones(Points);
//}
//
//// -----------------------------------------------------
//// IMC helpers
//UEnhancedInputLocalPlayerSubsystem* AJeonwoochiGASPCharacter::GetInputSubsystem() const
//{
//	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
//	{
//		return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
//	}
//	return nullptr;
//}
//
//void AJeonwoochiGASPCharacter::ClearAllIMC()
//{
//	if (auto* Sys = GetInputSubsystem()) { Sys->ClearAllMappings(); }
//}
//
//void AJeonwoochiGASPCharacter::RemoveIMC(UInputMappingContext* Context)
//{
//	if (auto* Sys = GetInputSubsystem())
//	{
//		if (Context) Sys->RemoveMappingContext(Context);
//		UE_LOG(LogJeonwoochiGASP, Verbose, TEXT("Remove IMC: %s"), Context ? *Context->GetName() : TEXT("None"));
//	}
//}
//
//void AJeonwoochiGASPCharacter::ApplyIMC_Default()
//{
//	if (auto* Sys = GetInputSubsystem())
//	{
//		ClearAllIMC();
//		if (IMC_Default) Sys->AddMappingContext(IMC_Default, 0);
//	}
//}
//
//void AJeonwoochiGASPCharacter::ApplyIMC_Skill()
//{
//	if (auto* Sys = GetInputSubsystem())
//	{
//		if (IMC_Skill) Sys->AddMappingContext(IMC_Skill, 1);
//		UE_LOG(LogJeonwoochiGASP, Verbose, TEXT("Skill IMC applied"));
//	}
//}
//
//void AJeonwoochiGASPCharacter::ApplyIMC_Illusion()
//{
//	if (auto* Sys = GetInputSubsystem())
//	{
//		ClearAllIMC();
//		if (IMC_Illusion) Sys->AddMappingContext(IMC_Illusion, 1);
//	}
//}
//
//void AJeonwoochiGASPCharacter::ApplyIMC_UI()
//{
//	if (auto* Sys = GetInputSubsystem())
//	{
//		if (IMC_UI) Sys->AddMappingContext(IMC_UI, 10);
//	}
//}
//
//// -----------------------------------------------------
//// 인터페이스
//ECharacterType AJeonwoochiGASPCharacter::GetCharacterType_Implementation()
//{
//	return CharacterType;
//}
//
//
//void AJeonwoochiGASPCharacter::Attack()
//{
//	// if (AttackComp) { AttackComp->AttackCombo(); }
//	// else { UE_LOG(LogJeonwoochiGASP, Warning, TEXT("AttackComp missing")); }
//}
//
//
//void AJeonwoochiGASPCharacter::TakeDamage_Implementation(const FDamageInfoStruct& DamageInfo)
//{
//	if (DamageableComp) DamageableComp->HandleTakeDamage(DamageInfo);
//}
//
//float AJeonwoochiGASPCharacter::GetMaxHealth() const
//{
//	return HealthComp ? HealthComp->GetMaxHealth() : 0.f;
//}
//
//float AJeonwoochiGASPCharacter::GetCurrentHealth_Implementation() const
//{
//	return HealthComp ? HealthComp->GetCurrentHealth() : 0.f;
//}
//
//void AJeonwoochiGASPCharacter::ApplyDamage_Implementation(const float& DamageAmount)
//{
//	if (HealthComp) HealthComp->ApplyDamage(DamageAmount);
//}
//
//void AJeonwoochiGASPCharacter::ApplyDotDamage_Implementation(const FDotDealInfoStruct& DotInfo)
//{
//	if (HealthComp) HealthComp->ApplyDotDamage(DotInfo);
//}
//
//void AJeonwoochiGASPCharacter::Die_Implementation()
//{
//	UE_LOG(LogJeonwoochiGASP, Warning, TEXT("PlayerDie!"));
//}
//
//// -----------------------------------------------------
//// 내부 유틸
//int32 AJeonwoochiGASPCharacter::Get8DirIndex(const FVector2D& In) const
//{
//	if (!LockOnComp || !LockOnComp->bLockOnEnabled) return 0; // Forward
//	if (In.SizeSquared() < 0.02f) return 4;                   // Back
//
//	float Deg = FMath::Atan2(In.Y, In.X) * 180.f / PI;
//	if (Deg < 0.f) Deg += 360.f;
//
//	int32 Bucket = FMath::FloorToInt((Deg + 22.5f) / 45.f);
//	Bucket = (Bucket % 8 + 8) % 8;
//	return (2 - Bucket + 8) % 8;
//}
