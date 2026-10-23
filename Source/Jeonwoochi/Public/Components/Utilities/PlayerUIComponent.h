// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Combat/HealthComponent.h"
#include "Components/SlateWrapperTypes.h"
#include "PlayerUIComponent.generated.h"

class UPlayerHUD;
class UAmbushWidget;
class UExecutionWidget;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UPlayerUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerUIComponent();

	// 컷신 동안 UMG 전부 가리기/복구 (레벨 시퀀서 이벤트에서 호출)
	UFUNCTION(BlueprintCallable, Category = "UI|Cinematic")
	void SetCinematicUIHidden(bool bHidden);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Widget", meta = (AllowPrivateAccess = "true"))
	UPlayerHUD* PlayerHUD;

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<UPlayerHUD> UPlayerHUDClass;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UpdateHealth(AActor* Actor, ECharacterType CharacterType, float CurrentHealth, float MaxHealth);

	void UpdateChi(float CurrentChi, float MaxChi);

	void CreateHUD();

	UFUNCTION()
	void HandleHUD(bool bShow);

	UFUNCTION(BlueprintCallable)
	void HandleBoss(AActor* Actor, FText Text);

	UFUNCTION()
	void HandleBossDeath(AActor* Actor, ECharacterType CharacterType);

private:
	// 컷신 강제 숨김 상태
	bool bCinematicHidden = false;

	// 복구용 이전 가시성 저장
	ESlateVisibility PrevHUDVis = ESlateVisibility::Collapsed;
};
