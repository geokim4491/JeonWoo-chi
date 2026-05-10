// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "AI/AIController/BaseAIController.h"
#include "Data/AI/AIConfigData.h"
#include "AI/Miscellaneous/PatrolRoute.h"

/*Components*/
#include "Components/Combat/HealthComponent.h"
#include "Components/Combat/AIDamageableComponent.h"
#include "Components/Combat/AttackComponent.h"
#include "Components/Combat/PostureComponent.h"
#include "Components/Utilities/WeaponManageComponent.h"

/*Interface*/
#include "Interface/Combat/DamageableInterface.h"
#include "Interface/Combat/HealthInterface.h"

/*Struct*/
#include "Structure/Combat/DotDealInfoStruct.h"

#include "BaseAICharacter.generated.h"

/**
 * 
 */

class UAmbushedComponent;
class USaveTargetComponent;
class UWidgetComponent;
class UUserWidget;

UCLASS()
class JEONWOOCHI_API ABaseAICharacter : public ABaseCharacter,
	public IDamageableInterface, public IHealthInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UAIDamageableComponent* DamageableComp;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	UAttackComponent* AttackComp;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	UWeaponManageComponent* WeaponManageComp;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UPostureComponent* PostureComp;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UAmbushedComponent* AmbushedComp;

	UPROPERTY(VisibleAnywhere, Category = "UI|HP", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* HPBarComp = nullptr;

	UPROPERTY(VisibleAnywhere, Category ="UI", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* AmbushWidgetComp;

	UPROPERTY(VisibleAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* ExecutionWidgetComp;

	UPROPERTY(EditAnywhere, Category = "UI|HP")
	float HPBarAutoHideDelay = 2.0f;

	UPROPERTY(VisibleAnywhere, Category = "LockOn")
	UWidgetComponent* LockMarkerComp;
	
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	TSubclassOf<UUserWidget> LockMarkerWidgetClass;

	/** 중앙 기준 위치 보정값(원하면 살짝 띄우기) */
	UPROPERTY(EditAnywhere, Category = "LockOn")
	FVector CenterOffset = FVector(0.f, 0.f, 30.f);

	/** 메쉬 바운드 중앙을 쓸지(=true) 캡슐/액터 위치를 쓸지(=false) */
	UPROPERTY(EditAnywhere, Category = "LockOn")
	bool bUseMeshBoundsCenter = false;

	/** 카메라 쪽으로 빌보드 회전 */
	UPROPERTY(EditAnywhere, Category = "LockOn")
	bool bFacePlayerCamera = true;

	UFUNCTION()
	void OnExecZoneBeginOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Hit);

	UFUNCTION()
	void OnExecZoneEndOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 BodyIndex);

	UFUNCTION()
	void OnAmbushZoneBeginOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Hit);

	UFUNCTION()
	void OnAmbushZoneEndOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 BodyIndex);

public:
	ABaseAICharacter();
#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaTime) override;

	FORCEINLINE void SetHPBarComponent(UWidgetComponent* NewHPBarComp) { HPBarComp = NewHPBarComp; }
	
	UPROPERTY(EditAnywhere, Category = "Save", meta = (AllowPrivateAccess = "true"))
	USaveTargetComponent* SaveComp;

	float RagdollTime = 8.f;

public:
	void FaceCamera();
	void FaceCamera(class UWidgetComponent* Comp);

	void Attack(const FDamageInfoStruct& DamageInfo);

	void DotAttack(const FDotDealInfoStruct& DotInfo, AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowLockMarker(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "UI|HP")
	void ShowHPBar(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "UI|HP")
	void ShowHPBarTimed(float Seconds);           // Seconds 후 자동 숨김(락온 중이면 무시)

	UFUNCTION()
	void HideHPBarIfNotPinned();                  // 타이머 콜백

	UFUNCTION(BlueprintCallable, Category = "UI|HP")
	void SetHPBarPinnedByLockOn(bool bPinned, float HideTime = 2.f);    // 락온/해제 시 호출

	UFUNCTION(BlueprintCallable, Category = "UI|HP")
	void FadeOutHPBar(float Duration = 0.4f);     // 사망 페이드

	UFUNCTION()
	void illusioned(float Duration, bool bDeath = true);

	UFUNCTION()
	AActor* FindClosestEnemy(float Radius = 2000.f) const;

	UFUNCTION()
	void SetTargetToClosetEnemy();

	// ExecZone 앞 정렬용 트랜스폼 계산
	UFUNCTION()
	bool CalcExecAlignTransform(FTransform& OutTransform) const;

	UFUNCTION()
	void SetExecZoneActive(bool bActive);

	UFUNCTION()
	void Executed();

	UFUNCTION()
	bool CalcAmbushAlignTransform(FTransform& OutTransform) const;

	UFUNCTION()
	void SetAmbushZoneActive(bool bActice);

	UFUNCTION()
	void Ambushed();

	UFUNCTION()
	void StopBT();

	UFUNCTION()
	void RestartBT();

	void SetIsBoss(bool Set);

public:
	/* Interface 상속 함수 */

	virtual void TakeDamage_Implementation(const FDamageInfoStruct& DamageInfo) override;

	
	virtual float GetMaxHealth() const override;
	virtual float GetCurrentHealth_Implementation() const override;
	virtual void Die_Implementation() override;
	virtual void ApplyDamage_Implementation(const float& DamageAmount) override;
	virtual void ApplyDotDamage_Implementation(const FDotDealInfoStruct& DotInfo) override;

public:

	UPROPERTY(EditAnywhere, Instanced, Category = "AI|BT", meta = (ShowOnlyInnerProperties))
	UAIConfigData* ConfigData;

	/** 에디터에서 편집 */
	UPROPERTY(EditInstanceOnly, Category = "Patrol")
	APatrolRoute* AssignedRoute;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* SpawnMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* ExecutionZone = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	FVector ExecZoneOffset = FVector(0.f, 90.f, 70.f); // 앞·가운데·약간 위

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	FVector ExecZoneExtent = FVector(50.f, 50.f, 60.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* AmbushZone = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambush")
	FVector AmbushZoneOffset = FVector(0.f, -60.f, 70.f); // 뒤·가운데·약간 위

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambush")
	FVector AmbushZoneExtent = FVector(50.f, 50.f, 60.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FVector HPBarOffset = FVector(0.f, 0.f, 140.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FVector2D HPBarSize = FVector2D(200.f, 40.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	int32 ExpReward = 50;


private:
	void NotifyPlayersInZone_On();
	void NotifyPlayersInZone_Off();

	UFUNCTION() void HandleIllusionExpire();

	void UpdateLockMarkerTransform(); // 위치/회전 보정
	void Ragdoll();

	bool billusioned = false;
	bool bHPBarPinnedByLockOn = false; // 락온 중 강제표시
	bool bIsFadingHPBar = false;

	AAIController* OwnerController;
	
	FTimerHandle  illusionTimer;
	FTimerHandle TH_HPAutoHide;
	FTimerHandle TH_HPFade;
};
