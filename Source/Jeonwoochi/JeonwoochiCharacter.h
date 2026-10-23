// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Characters/BaseCharacter.h"

#include "Public/Interface/Combat/AttackInterface.h"
#include "Public/Interface/Combat/DamageableInterface.h"
#include "Public/Interface/Combat/HealthInterface.h"

#include "Public/Structure/Combat/DotDealInfoStruct.h"

// 입력
#include "InputAction.h"
#include "InputMappingContext.h"

#include "Public/Components/Boss/BossActivateComponent.h"
#include "Public/Components/Boss/BossPhaseComponent.h"
#include "Public/Miscellaneous/CutScene/CutSceneTriggerActor.h"

#include "JeonwoochiCharacter.generated.h"

class UChiComponent;
class UPlayerSkillComponent;
class UExecutionComponent;
class UBlinkComponent;
class UAmbushComponent;
class UPlayerInteractComponent;
class UHealthComponent;
class UPlayerAttackComponent;
class UPlayerDamageableComponent;
class ULockOnComponent;
class UWeaponManageComponent;
class UCombatModeComponent;
class UPlayerStatComponent;
class UPlayerLevelComponent;

class UPlayerUIComponent;

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UEnhancedInputLocalPlayerSubsystem;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class JEONWOOCHI_API AJeonwoochiCharacter : public ABaseCharacter,
	public IDamageableInterface, public IHealthInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** ============ MappingContext ============ */
	UPROPERTY(EditDefaultsOnly, Category = "Input|IMC")
	UInputMappingContext* IMC_Default;

	UPROPERTY(EditDefaultsOnly, Category = "Input|IMC")
	UInputMappingContext* IMC_Skill;

	UPROPERTY(EditDefaultsOnly, Category = "Input|IMC")
	UInputMappingContext* IMC_Illusion;

	UPROPERTY(EditDefaultsOnly, Category = "Input|IMC")
	UInputMappingContext* IMC_Blink;

	UPROPERTY(EditDefaultsOnly, Category = "Input|IMC")
	UInputMappingContext* IMC_UI;

	UPROPERTY(EditDefaultsOnly, Category = "Input|IMC")
	UInputMappingContext* IMC_CutScene;

	/** ============ InputAction ============ */

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	UInputAction* IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	UInputAction* IA_LockOn;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	UInputAction* IA_Dodge;

	// 컨트롤러 B키 or 키보드 Shift키
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	UInputAction* IA_RunHold;

	// 컨트롤러 L3키 or 키보드 B키
	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	UInputAction* IA_RunToggle;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Interact")
	UInputAction* IA_Interact;

	/** ============ Combat ============ */
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	UInputAction* IA_FastAttack;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	UInputAction* IA_StrongAttack;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	UInputAction* IA_DashAttack;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	UInputAction* IA_QTE;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	UInputAction* IA_Block;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	UInputAction* IA_Heal;

	/** ============ Skill ============ */
	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill")
	UInputAction* IA_Skill_Mode_RB;
	

	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill")
	UInputAction* IA_Castillusion;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill")
	UInputAction* IA_illusionMode;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill")
	UInputAction* IA_SpawnClone;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill")
	UInputAction* IA_Blink;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill")
	UInputAction* IA_Attack_Skill1;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill")
	UInputAction* IA_Attack_Skill2;

	/** ============= UI ==============*/
	UPROPERTY(EditDefaultsOnly, Category = "Input|UI")
	UInputAction* IA_ToggleMainMenu;

	UPROPERTY(EditDefaultsOnly, Category = "Input|CutScene")
	UInputAction* IA_SkipCutScene;


	/** ============ Components ============ */

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UChiComponent* ChiComp;

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UPlayerDamageableComponent* DamageableComp;

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UPlayerDefensiveComponent* DefensiveComp;

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UPlayerAttackComponent* AttackComp;

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UCombatModeComponent* CombatModeComp;

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UPlayerSkillComponent* SkillComp;

	UPROPERTY(VisibleAnywhere, Category = "Utilities", meta = (AllowPrivateAccess = "true"))
	ULockOnComponent* LockOnComp;

	UPROPERTY(VisibleAnywhere, Category = "Utilities", meta = (AllowPrivateAccess = "true"))
	UPlayerStatComponent* PlayerStatComp;

	UPROPERTY(VisibleAnywhere, Category = "Utilities", meta = (AllowPrivateAccess = "true"))
	UPlayerLevelComponent* PlayerLevelComp;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	UWeaponManageComponent* WeaponManageComp;

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UExecutionComponent* ExecutionComp;

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UBlinkComponent* BlinkComp;

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UAmbushComponent* AmbushComp;

	UPROPERTY(VisibleAnywhere, Category = "Utilities|Interact", meta = (AllowPrivateAccess = "true"))
	UPlayerInteractComponent* InteractComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	UPlayerUIComponent* UIComp;

public:
	AJeonwoochiCharacter();

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Speed")
	float WalkSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Speed")
	float RunSpeed = 700.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsRunning = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defense")
	bool bIsBlocking = false;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* HealAnim = nullptr;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* DieAnim = nullptr;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* SkillFailSound = nullptr;

	/** ===== Attack Orientation to Camera Input ===== */
	UPROPERTY(EditAnywhere, Category = "Input")
	float MoveInputDeadzone = 0.20f;           

	UPROPERTY(EditAnywhere, Category = "Input")
	float MoveDirHoldSeconds = 0.15f;             

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	FVector2D LastNonZeroMoveDir = FVector2D::ZeroVector; 

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	float LastNonZeroMoveTime = -1000.f;        
	
	/** ===== IMC 전환 ===== */
	UFUNCTION() void ApplyIMC_Default();
	UFUNCTION() void ApplyIMC_Skill();
	UFUNCTION() void ApplyIMC_Blink();
	UFUNCTION() void ApplyIMC_Illusion();
	UFUNCTION() void ApplyIMC_UI();
	UFUNCTION() void ApplyIMC_CutScene();
	UFUNCTION() void RemoveIMC(UInputMappingContext* Context);
	UFUNCTION() void ClearAllIMC();

protected:
	
	UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem() const;

	/** ===== 바인딩 핸들러 ===== */
	void OnMove(const FInputActionValue& Value);
	void OnClearMoveInput(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnLockOn(const FInputActionValue& Value);
	void OnDodge(const FInputActionValue& Value);
	void OnRunHoldStarted(const FInputActionValue& Value);
	void OnRunHoldCompleted(const FInputActionValue& Value);
	void OnRunToggle(const FInputActionValue& Value);

	void OnInteract(const FInputActionValue& Value);

	void OnFastAttack(const FInputActionValue& Value);
	void OnStrongAttack(const FInputActionValue& Value);
	void OnDashAttack(const FInputActionValue& Value);
	void OnStruggle(const FInputActionValue& Value);

	void OnStartBlock(const FInputActionValue& Value);
	void OnStopBlock(const FInputActionValue& Value);

	void OnHeal(const FInputActionValue& Value);

	void OnSkillMode(const FInputActionValue& Value);
	void OnStopSkillMode(const FInputActionValue& Value);
	void OnSpawnClone(const FInputActionValue& Value);
	void OnBlink(const FInputActionValue& Value);
	void OnAttackSkill1(const FInputActionValue& Value);
	void OnAttackSkill2(const FInputActionValue& Value);


	void OnillusionMode(const FInputActionValue& Value);
	void OnStopillusionMode(const FInputActionValue& Value);
	void OnCastillusion(const FInputActionValue& Value);

	void OnSkipCutScene(const FInputActionValue& Value);

	void OnToggleMainMenu(const FInputActionValue& Value);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void TakeDamage_Implementation(const FDamageInfoStruct& DamageInfo) override;
	virtual float GetMaxHealth() const override;
	virtual float GetCurrentHealth_Implementation() const override;
	virtual void Die_Implementation() override;
	virtual void ApplyDamage_Implementation(const float& DamageAmount) override;
	virtual void ApplyDotDamage_Implementation(const FDotDealInfoStruct& DotInfo) override;

public:
	// Stat Management
	void IncreaseMaxHP();
	void IncreaseMaxMP();
	void IncreaseMaxAP();
	void IncreaseMaxIntP();
	void IncreaseMaxSkillP();
	void IncreaseMaxMystP();
	void IncreaseMaxDefP();
	

	void ReviveFromRest();
	void ClearLockOn();
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UPlayerUIComponent* GetUIComponent() { return UIComp; }
	FORCEINLINE ULockOnComponent* GetLockOnComponent() { return LockOnComp; }
	UFUNCTION() UHealthComponent* GetHealthComponent() const;
	UFUNCTION() UPlayerDefensiveComponent* GetPlayerDefensiveComponent();

	void SetBossActiveComp(UBossActivateComponent* BossActiveComp) { CachedBAComp = BossActiveComp; }
	void SetBossPhaseComp(UBossPhaseComponent* BossPhaseComp) { CachedBPComp = BossPhaseComp; }

private:
	FVector2D MoveInput;
	int32 Get8DirIndex(const FVector2D& In) const;
	
	void SetSoftCombatMode();

	bool bIsRunToggled;
	bool bIsStrongAttackHeld = false;
	bool bCanHeal = true;

	UBossActivateComponent* CachedBAComp = nullptr;
	UBossPhaseComponent* CachedBPComp = nullptr;
	ACutSceneTriggerActor* CachedCSTActor = nullptr;

	FTimerHandle HealTimer;
};

