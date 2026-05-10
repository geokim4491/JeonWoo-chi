// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "JeonwoochiPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API AJeonwoochiPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	// Transition
	UFUNCTION(BlueprintCallable)
	void Transition(APawn* SwitchingActor);

	const float BlendTime = 1.5f;

	FTimerHandle UnpossessTimerHandle;

	void UnpossessControlledPawn(APawn* SwitchingActor);

public:
	UPROPERTY()
	class UHealthComponent* BoundHealthComp = nullptr;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:
	void BindToPawn(APawn* InPawn);
	void UnbindFromPawn();

	// UI
public:
	UFUNCTION(BlueprintCallable) void ToggleMainMenu();
	UFUNCTION(BlueprintCallable) void OpenMainMenu();
	UFUNCTION(BlueprintCallable) void CloseMainMenu();

	UFUNCTION() void HandleDeath();

	UFUNCTION() void ShowYouDiedUI();

	UFUNCTION() void ShowEndCredits();

protected:
	UPROPERTY(EditDefaultsOnly) 
	TSubclassOf<class UMainMenu> MainMenuClass;

	UPROPERTY() 
	UMainMenu* MainMenu = nullptr;

	bool bMenuOpen = false;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UDeadWidget> DeadWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UFadeInHoldOutWidget> YouDiedWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UEndCreditsWidget> EndCreditsWidgetClass;

	UPROPERTY()
	UDeadWidget* DeadWidget = nullptr;

	UPROPERTY()
	UFadeInHoldOutWidget* YouDiedWidget = nullptr;

protected:
	virtual void BeginPlay() override; // [TUTORIAL_UI] 추가

	UFUNCTION() // [TUTORIAL_UI] TriggerBox 브로드캐스트 수신 콜백
		void OnTutorialTriggered(AActor* PlayerActor, FName TutorialId);

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial") // [TUTORIAL_UI] 에디터에서 클래스타입 지정
		TSubclassOf<class UTutorialHintWidget> TutorialHintWidgetClass = nullptr;

	UPROPERTY() // [TUTORIAL_UI] 위젯 인스턴스(한 번 생성 후 재사용)
		class UTutorialHintWidget* TutorialHintWidget = nullptr;

	void BindAllTutorialTriggers();

};
