// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UI/SlidingTipWidget.h"
#include "SlidingTipTriggerVolume.generated.h"

class UBoxComponent;
class USoundBase;

UCLASS()
class JEONWOOCHI_API ASlidingTipTriggerVolume : public AActor
{
	GENERATED_BODY()
	
public:
	ASlidingTipTriggerVolume();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 트리거 볼륨 */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* Box;

	/** 위젯 클래스 (USlidingTipWidget 파생 BP 권장) */
	UPROPERTY(EditAnywhere, Category = "Tip")
	TSubclassOf<USlidingTipWidget> TipWidgetClass;

	/** 표출할 문구 */
	UPROPERTY(EditAnywhere, Category = "Tip", meta = (MultiLine = "true"))
	FText TipMessage;

	/** 어느 쪽에서 들어올지 */
	UPROPERTY(EditAnywhere, Category = "Tip")
	ETipSlideSide Side = ETipSlideSide::Left;

	/** 애니메이션 타이밍(초) */
	UPROPERTY(EditAnywhere, Category = "Tip", meta = (ClampMin = "0"))
	float SlideIn = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Tip", meta = (ClampMin = "0"))
	float Hold = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Tip", meta = (ClampMin = "0"))
	float SlideOut = 0.25f;

	/** 스타일 */
	UPROPERTY(EditAnywhere, Category = "Tip|Style")
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Tip|Style", meta = (ClampMin = "8", UIMin = "12", UIMax = "200"))
	int32 FontSize = 28;

	UPROPERTY(EditAnywhere, Category = "Tip|Layout", meta = (ClampMin = "0"))
	float EdgePadding = 24.f;

	UPROPERTY(EditAnywhere, Category = "Tip|Layout", meta = (ClampMin = "0"))
	float SlideDistance = 350.f;

	/** 0=상단, 1=하단 (화면 비율 Y 위치) */
	UPROPERTY(EditAnywhere, Category = "Tip|Layout", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NormalizedY = 0.2f;

	/** (옵션) 사운드 */
	UPROPERTY(EditAnywhere, Category = "Tip|Sound")
	USoundBase* AppearSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Tip|Sound")
	USoundBase* DisappearSound = nullptr;

private:
	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void TryTrigger(AActor* InstigatorActor);
};
