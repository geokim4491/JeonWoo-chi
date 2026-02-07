// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ToolTipComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnToolTipTextChanged, const FText& NewText);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API UToolTipComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UToolTipComponent();

	UFUNCTION(BlueprintCallable, Category = "Text")
	void SetToolTipText(const FText& NewText);

	UFUNCTION(BlueprintPure, Category = "Text")
	FText GetToolTipText() const { return ToolTipText; }

	/** 툴 팁 텍스트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	FText ToolTipText;

	FOnToolTipTextChanged OnToolTipTextChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

};
