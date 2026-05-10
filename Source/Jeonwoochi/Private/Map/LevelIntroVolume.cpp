// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/LevelIntroVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "UI/LevelIntroWidget.h"
#include "Data/Save/SaveSubsystem.h"

ALevelIntroVolume::ALevelIntroVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComponent);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);

	Box->OnComponentBeginOverlap.AddDynamic(this, &ALevelIntroVolume::OnBoxBeginOverlap);
}

void ALevelIntroVolume::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimerForNextTick(this, &ALevelIntroVolume::CheckInitialOverlap);


}

void ALevelIntroVolume::OnBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	if (!Pawn->IsPlayerControlled()) return;

	TriggerIntro(Pawn);
}

void ALevelIntroVolume::TriggerIntro(APawn* InstigatorPawn)
{
	if (bTriggered && bTriggerOnce) return;

	// 다시는 겹치지 않도록 처리
	if (bTriggerOnce && Box)
	{
		Box->SetGenerateOverlapEvents(false);
		Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	bTriggered = true;

	// 사운드 재생
	if (EnterSound)
	{
		UGameplayStatics::PlaySound2D(this, EnterSound);
	}

	// UI 생성 + 페이드 실행
	if (WidgetClass)
	{
		if (UWorld* W = GetWorld())
		{
			if (ULevelIntroWidget* Wdg = CreateWidget<ULevelIntroWidget>(W, WidgetClass))
			{
				Wdg->AddToViewport(10000 /*높은 ZOrder*/);
				Wdg->SetTitleColor(TitleColor);
				Wdg->SetTitleFontSize(TitleFontSize);
				const FText Title = MapTitle.IsEmpty() ? FText::FromString(TEXT("Unknown Area")) : MapTitle;
				Wdg->ShowIntro(Title, FadeIn, Hold, FadeOut);
			}
		}
	}

	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
	{
		if (USaveSubsystem* SS = GI->GetSubsystem<USaveSubsystem>())
		{
			SS->SetLevelTitle(MapTitle);
		}
	}
}

void ALevelIntroVolume::CheckInitialOverlap()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn && Box && Box->IsOverlappingActor(PlayerPawn))
	{
		// 시작하자마자 안에 있었다면 즉시 발동
		TriggerIntro(PlayerPawn);
		return;
	}
}

void ALevelIntroVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}