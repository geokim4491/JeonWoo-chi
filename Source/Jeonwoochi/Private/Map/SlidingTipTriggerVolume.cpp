// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/SlidingTipTriggerVolume.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

#include "Engine/World.h"
#include "TimerManager.h"

ASlidingTipTriggerVolume::ASlidingTipTriggerVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComponent);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 기본: Pawn과만 겹침
	Box->SetGenerateOverlapEvents(true);

	Box->OnComponentBeginOverlap.AddDynamic(this, &ASlidingTipTriggerVolume::OnBoxBeginOverlap);
}

void ASlidingTipTriggerVolume::BeginPlay()
{
	Super::BeginPlay();

}

void ASlidingTipTriggerVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

	Super::EndPlay(EndPlayReason);
}

void ASlidingTipTriggerVolume::OnBoxBeginOverlap(
	UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/,
	const FHitResult& /*SweepResult*/)
{
	if (!OtherActor) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	if (!Pawn->IsPlayerControlled()) return;

	TryTrigger(OtherActor);

	Box->SetGenerateOverlapEvents(false);
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void ASlidingTipTriggerVolume::TryTrigger(AActor* InstigatorActor)
{
	// 위젯 생성
	if (TipWidgetClass)
	{
		if (UWorld* W = GetWorld())
		{
			if (USlidingTipWidget* Tip = CreateWidget<USlidingTipWidget>(W, TipWidgetClass))
			{
				Tip->AddToViewport(12000);

				// 스타일/레이아웃 주입
				Tip->SetTextColor(TextColor);
				Tip->SetFontSize(FontSize);
				Tip->EdgePadding = EdgePadding;
				Tip->SlideDistance = SlideDistance;
				Tip->NormalizedY = NormalizedY;
				Tip->AppearSound = AppearSound;
				Tip->DisappearSound = DisappearSound;

				// 표출
				const FText Msg = TipMessage.IsEmpty()
					? FText::FromString(TEXT("Tip"))
					: TipMessage;

				Tip->ShowTip(Msg, Side, SlideIn, Hold, SlideOut);
			}
		}
	}
}
