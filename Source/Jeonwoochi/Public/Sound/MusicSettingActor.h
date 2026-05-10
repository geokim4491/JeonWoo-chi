// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enum/CharacterTypeEnum.h"
#include "MusicSettingActor.generated.h"

class USoundBase;
class UMusicSubsystem;
class ABossCharacter;

UCLASS()
class JEONWOOCHI_API AMusicSettingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMusicSettingActor();

    UPROPERTY(EditInstanceOnly, Category = "BGM")
    bool bAutoPlayOnBeginPlay = true;

    UPROPERTY(EditInstanceOnly, Category = "BGM")
    TSoftObjectPtr<USoundBase> DefaultBGM;

    UPROPERTY(EditInstanceOnly, Category = "BGM", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float DefaultVolume = 1.f;

    UPROPERTY(EditInstanceOnly, Category = "BGM", meta = (ClampMin = "0.0"))
    float DefaultFadeInSeconds = 1.5f;

    UPROPERTY(EditInstanceOnly, Category = "BGM", meta = (ClampMin = "0.0"))
    float DefaultFadeOutSeconds = 1.0f;

    UPROPERTY(EditInstanceOnly, Category = "BGM")
    bool bPersistAcrossLevelTransition = false;

    /** 설정한 보스기 죽은 이후에 원래 설정했던 OST로 복구 */
    UPROPERTY(EditInstanceOnly, Category = "BGM")
    ABossCharacter* BindingTarget;

    /** 바꿀 OST, 기존 OSt FadeOutTime, 바꿀 OST FadeInTime */
    UFUNCTION()
    void ChangeBGM(TSoftObjectPtr<USoundBase> ChangeBGM, float FadeOutSeconds, float FadeInSeconds);

protected:
    virtual void BeginPlay() override;

    void PlayOriginBGM(AActor* a, ECharacterType e);

    UMusicSubsystem* MusicSystem;
};
