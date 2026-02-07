// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Utilities/ToolTipComponent.h"

// Sets default values for this component's properties
UToolTipComponent::UToolTipComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UToolTipComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UToolTipComponent::SetToolTipText(const FText& NewText)
{
	ToolTipText = NewText;
	OnToolTipTextChanged.Broadcast(ToolTipText);
}