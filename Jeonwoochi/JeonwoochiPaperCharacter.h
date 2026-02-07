// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "Interface/Character/CharacterTypeInterface.h"
#include "JeonwoochiPaperCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UPaperAnimationComponent;
class UHealthComponent;

class UPaperFlipbook;

class UInputMappingContext;
class UInputAction;

struct FInputActionValue;

enum class EEffectType : uint8;

UCLASS()
class JEONWOOCHI_API AJeonwoochiPaperCharacter : public APaperCharacter, public ICharacterTypeInterface
{
	GENERATED_BODY()
	
public:
	AJeonwoochiPaperCharacter();
private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;


	/** ============ MappingContext ============ */
	UPROPERTY(EditDefaultsOnly, Category = "Input|IMC")
	UInputMappingContext* IMC_2D;

	/** ============ InputAction ============ */

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	UInputAction* IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Movement")
	UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Combat")
	UInputAction* IA_Attack;

	FVector InitialSpriteScale;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ECharacterType CharacterType = ECharacterType::Player;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paper2D")
	UPaperAnimationComponent* AnimComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UArrowComponent* FacingArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Paper2D")
	UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* AttackSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* HitSound = nullptr;


	bool bFacingRight = true;
	FRotator FacingArrowBaseRot;

	void SetFacing(bool bRight);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION()
	void ApplyIMC();

	UFUNCTION()
	ECharacterType GetCharacterType_Implementation();

	void Attack1();
	void Attack2();
	void PlayEffect(EEffectType EffectType);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void OnMove(const FInputActionValue& Value);
	void OnAttack(const FInputActionValue& Value);
	void OnJump();

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRadius = 200.f;          // 원(원판) 반지름

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 10.f;
};
