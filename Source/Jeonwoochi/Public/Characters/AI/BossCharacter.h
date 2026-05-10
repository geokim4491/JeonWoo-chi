// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AI/BaseAICharacter.h"

#include "Interface/Boss/BossChangePhaseInterface.h"

#include "BossCharacter.generated.h"

class UBossPhaseComponent;
class UFadeInHoldOutWidget;

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API ABossCharacter : public ABaseAICharacter,
	public IBossChangePhaseInterface
{
	GENERATED_BODY()
	
public:
	ABossCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Name")
	FText BossName = FText::FromString("Boss");

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UFadeInHoldOutWidget> GreatFelledWidgetClass;

protected:
	UPROPERTY(EditAnywhere, Category = "Phase")
	UBossPhaseComponent* BossPhaseComp;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Die_Implementation() override;
	virtual void ChangePhase_Implementation(int32 CurrentPhase) override;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<class UBlackboardComponent> CachedBB;
};
