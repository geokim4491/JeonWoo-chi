// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../../Enum/CharacterTypeEnum.h"
#include "CharacterTypeInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCharacterTypeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class JEONWOOCHI_API ICharacterTypeInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent)
	ECharacterType GetCharacterType();
	virtual ECharacterType GetCharacterType_Implementation() = 0;
	
};
