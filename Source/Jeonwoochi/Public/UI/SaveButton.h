// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/Save/SaveSubsystem.h"
#include "SaveButton.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveButtonClicked, const FString&, GameName);


UCLASS()
class JEONWOOCHI_API USaveButton : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BTN_Menu;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TXT_GameName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TXT_SavedAt;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TXT_LevelTitle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* TXT_TotalPlayTime;

	UPROPERTY(BlueprintAssignable) FOnSaveButtonClicked OnClicked;

	UPROPERTY(EditAnywhere, Category = "Sound")
	class USoundBase* Sound;

public:
	virtual void NativeConstruct() override;
	
	void SetSaveInfo(const FJWGameInfo& Info);
	void Setup(const FJWGameInfo& Info) { CachedName = Info.GameName; SetSaveInfo(Info); Bind(); }

	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

private:
	UPROPERTY() FString CachedName;

private:
	UFUNCTION() void HandleClicked() { OnClicked.Broadcast(CachedName); }
	void Bind();
};
