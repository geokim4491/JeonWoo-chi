// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API USkillInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 위젯 디자이너에서 할당(또는 BP에서 세팅)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	class UMediaPlayer* MediaPlayer = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	class UMediaSource* MediaSource = nullptr;

	// ContentSwitcher에서 이 위젯이 활성화되면 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Media")
	void PlayPreview();

};
