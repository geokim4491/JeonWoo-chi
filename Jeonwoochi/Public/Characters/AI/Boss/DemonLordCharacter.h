// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AI/BossCharacter.h"
#include "DemonLordCharacter.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API ADemonLordCharacter : public ABossCharacter
{
	GENERATED_BODY()
	
protected:
	virtual void ChangePhase_Implementation(int32 CurrentPhase) override;
	virtual void Die_Implementation() override;
};
