//#pragma once
//
//#include "CoreMinimal.h"
//#include "GASPCharacterExample.h"                                   // Parent Class
//#include "Public/Enum/CharacterTypeEnum.h"                          // UENUM(ECharacterType) needs UHT, include is necessary
//#include "Public/Interface/Character/CharacterTypeInterface.h"      // Interfaces due to inheritance
//#include "Public/Interface/Combat/DamageableInterface.h"
//#include "Public/Interface/Combat/HealthInterface.h"
//#include "Public/Interface/Combat/AttackInterface.h"
//#include "GenericTeamAgentInterface.h"								// IGenericTeamAgentInterface, FGenericTeamId
//#include "JeonwoochiGASPCharacter.generated.h"
//
//class UInputMappingContext; class UInputAction; struct FInputActionValue;
//class UEnhancedInputLocalPlayerSubsystem;
//
//class UHealthComponent; class UPlayerDamageableComponent; class UPlayerDefensiveComponent;
//class UPlayerAttackComponent; class UPlayerSkillComponent; class ULockOnComponent;
//class UWeaponManageComponent; class UPlayerUIComponent; class UExecutionComponent;
//
//class UAIPerceptionStimuliSourceComponent; class UMotionWarpingComponent;
//
//// Combat Structures
//struct FDamageInfoStruct; struct FDotDealInfoStruct;
//
//// Jeonwoochi(GASP) Log Category
//DECLARE_LOG_CATEGORY_EXTERN(LogJeonwoochiGASP, Log, All);
//
///*
// * - Based on GASP
// * - 이동/시야/점프/스프린트: GASP 액션에 위임
// * - BaseCharacter의 팀/Perception/모션워핑/타입 등을 이식
// * - 기존 전우치 전투/방어/락온/스킬/일루전/클론 로직 이식
// * - 입력 UPROPERTY는 모두 private + AllowPrivateAccess로 통일
// */
//
//UCLASS(config = Game)
//class JEONWOOCHI_API AJeonwoochiGASPCharacter
//	: public AGASPCharacterExample
//	, public ICharacterTypeInterface
//	, public IGenericTeamAgentInterface
//	, public IDamageableInterface
//	, public IHealthInterface
//	, public IAttackInterface
//{
//	GENERATED_BODY()
//
//public:
//	AJeonwoochiGASPCharacter(const FObjectInitializer& ObjectInitializer);
//
//	// ======================== 수명주기 ========================
//protected:
//	virtual void BeginPlay() override;
//	virtual void PossessedBy(AController* NewController) override;
//	virtual void NotifyControllerChanged() override;
//	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
//	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
//
//	// ======================== 입력 핸들러 ========================
//protected:
//	// GASP 이동/시야/점프 래퍼
//	void OnJumpStarted(const FInputActionValue& Value);
//	void OnJumpCompleted(const FInputActionValue& Value);
//	void OnMove(const FInputActionValue& Value);
//	void OnClearMoveInput(const FInputActionValue& Value);
//	void OnLook(const FInputActionValue& Value);
//
//	// 이동 보조
//	void OnRunHoldStarted(const FInputActionValue& Value);
//	void OnRunHoldCompleted(const FInputActionValue& Value);
//	void OnRunToggle(const FInputActionValue& Value);
//
//	// 전투/방어/회피
//	void OnFastAttack(const FInputActionValue& Value);
//	void OnStrongAttack(const FInputActionValue& Value);
//	void OnDashAttack(const FInputActionValue& Value);
//	void OnStruggle(const FInputActionValue& Value);
//	void OnStartBlock(const FInputActionValue& Value);
//	void OnStopBlock(const FInputActionValue& Value);
//	void OnDodge(const FInputActionValue& Value);
//
//	// 락온/스킬/일루전/클론
//	void OnLockOn(const FInputActionValue& Value);
//	void OnSkillMode(const FInputActionValue& Value);
//	void OnStopSkillMode(const FInputActionValue& Value);
//	void OnillusionMode(const FInputActionValue& Value);
//	void OnStopillusionMode(const FInputActionValue& Value);
//	void OnCastillusion(const FInputActionValue& Value);
//	void OnSpawnClone(const FInputActionValue& Value);
//
//	// ======================== IMC 유틸 ========================
//protected:
//	UFUNCTION() void ApplyIMC_Default();
//	UFUNCTION() void ApplyIMC_Skill();
//	UFUNCTION() void ApplyIMC_Illusion();
//	UFUNCTION() void ApplyIMC_UI();
//	UFUNCTION() void RemoveIMC(UInputMappingContext* Context);
//	UFUNCTION() void ClearAllIMC();
//
//	UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem() const;
//
//	// ======================== 인터페이스 구현 ========================
//public:
//	// CharacterTypeInterface
//	virtual ECharacterType GetCharacterType_Implementation() override;
//
//	// CharacterType Getter
//	ECharacterType GetCharacterType() const { return CharacterType; }
//
//	// Team Agent
//	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId(TeamId); }
//	virtual void SetGenericTeamId(const FGenericTeamId& NewID) override { TeamId = NewID.GetId(); }
//
//	// Combat / HP
//	virtual void TakeDamage_Implementation(const FDamageInfoStruct& DamageInfo) override;
//	virtual float GetMaxHealth() const override;
//	virtual float GetCurrentHealth_Implementation() const override;
//	virtual void ApplyDamage_Implementation(const float& DamageAmount) override;
//	virtual void ApplyDotDamage_Implementation(const FDotDealInfoStruct& DotInfo) override;
//	virtual void Die_Implementation() override;
//
//	// IAttackInterface
//	virtual void Attack() override;
//
//	// ======================== 에디트 가능한 값 ========================
//public:
//	/* (레거시 유지용) 설계상 GASP가 게이트를 관리하지만 힌트값으로 일단 남겨둠, 필요 없을 시 삭제 예정 */
//	UPROPERTY(EditDefaultsOnly, Category = "Movement|Speed")
//	float WalkSpeed = 300.f;
//
//	UPROPERTY(EditDefaultsOnly, Category = "Movement|Speed")
//	float RunSpeed = 500.f;
//
//private:
//	// ======================== IMC / 입력 (private + AllowPrivateAccess) ========================
//	UPROPERTY(EditDefaultsOnly, Category = "Input|IMC", meta = (AllowPrivateAccess = "true"))
//	UInputMappingContext* IMC_Default = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|IMC", meta = (AllowPrivateAccess = "true"))
//	UInputMappingContext* IMC_Skill = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|IMC", meta = (AllowPrivateAccess = "true"))
//	UInputMappingContext* IMC_Illusion = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|IMC", meta = (AllowPrivateAccess = "true"))
//	UInputMappingContext* IMC_UI = nullptr;
//
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_Jump = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_Move = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_Look = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_LockOn = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_Dodge = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_RunHold = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_RunToggle = nullptr;
//
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_FastAttack = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_StrongAttack = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_DashAttack = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_QTE = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_Block = nullptr;
//
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_Skill_Mode_RB = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_Castillusion = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_illusionMode = nullptr;
//	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill", meta = (AllowPrivateAccess = "true"))
//	UInputAction* IA_SpawnClone = nullptr;
//
//	// ======================== 게임플레이 컴포넌트 ========================
//	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
//	UHealthComponent* HealthComp = nullptr;
//	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
//	UPlayerDamageableComponent* DamageableComp = nullptr;
//	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
//	UPlayerDefensiveComponent* DefensiveComp = nullptr;
//	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
//	UPlayerAttackComponent* AttackComp = nullptr;
//	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
//	UPlayerSkillComponent* SkillComp = nullptr;
//	UPROPERTY(VisibleAnywhere, Category = "Utilities", meta = (AllowPrivateAccess = "true"))
//	ULockOnComponent* LockOnComp = nullptr;
//	UPROPERTY(meta = (AllowPrivateAccess = "true"))
//	UWeaponManageComponent* WeaponManageComp = nullptr;
//	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
//	UExecutionComponent* ExecutionComp = nullptr;
//
//	// BaseCharacter 이식: 지각/모션워핑/팀
//	// 모션워핑은 GASPCharacter에서 이미 생성됨
//	UPROPERTY(VisibleAnywhere, Category = "TeamId|Perception", meta = (AllowPrivateAccess = "true"))
//	UAIPerceptionStimuliSourceComponent* StimuliSourceComp = nullptr;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
//	ECharacterType CharacterType = ECharacterType::None;
//	UPROPERTY(EditAnywhere, Category = "TeamId", meta = (AllowPrivateAccess = "true"))
//	FGenericTeamId TeamId = FGenericTeamId(0);
//
//private:
//	// ======================== 내부 상태 ========================
//	FVector2D MoveInput = FVector2D::ZeroVector;
//	bool bIsRunToggled = false;
//	int32 Get8DirIndex(const FVector2D& In) const;
//};
