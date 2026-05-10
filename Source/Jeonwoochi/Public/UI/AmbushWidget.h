// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmbushWidget.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UAmbushWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class USizeBox* Main;

};
