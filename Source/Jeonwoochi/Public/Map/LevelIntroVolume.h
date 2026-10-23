// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelIntroVolume.generated.h"

class UBoxComponent;
class USoundBase;
class ULevelIntroWidget;

UCLASS()
class JEONWOOCHI_API ALevelIntroVolume : public AActor
{
	GENERATED_BODY()
	
public:
	ALevelIntroVolume();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 한 번만 동작 (true면 첫 진입 후 더 이상 오버랩 발생 X) */
	UPROPERTY(EditAnywhere, Category = "Intro")
	bool bTriggerOnce = true;

	/** 표시할 맵 이름(지역명) */
	UPROPERTY(EditAnywhere, Category = "Intro")
	FText MapTitle;

	UPROPERTY(EditAnywhere, Category = "Intro|Style")
	FLinearColor TitleColor = FLinearColor::White;

	/** 제목 폰트 사이즈 (볼륨에서 지정) */
	UPROPERTY(EditAnywhere, Category = "Intro|Style", meta = (ClampMin = "1", UIMin = "8", UIMax = "300"))
	int32 TitleFontSize = 80;

	/** 페이드 인/홀드/아웃 시간(초) */
	UPROPERTY(EditAnywhere, Category = "Intro", meta = (ClampMin = "0.0"))
	float FadeIn = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Intro", meta = (ClampMin = "0.0"))
	float Hold = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Intro", meta = (ClampMin = "0.0"))
	float FadeOut = 1.0f;

	/** 표시할 위젯 클래스 (ULevelIntroWidget 파생 BP 권장) */
	UPROPERTY(EditAnywhere, Category = "Intro")
	TSubclassOf<ULevelIntroWidget> WidgetClass;

	/** 재생할 사운드 */
	UPROPERTY(EditAnywhere, Category = "Intro|Sound")
	USoundBase* EnterSound = nullptr;

	/** 트리거용 박스 */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* Box;

private:
	bool bTriggered = false;

	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void TriggerIntro(APawn* InstigatorPawn);

	void CheckInitialOverlap();
};

