// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Boss/BossActivateComponent.h"
#include "Jeonwoochi/JeonwoochiCharacter.h"
#include "Characters/AI/BossCharacter.h"
#include "AI/AIController/BaseAIController.h"
#include "Components/Utilities/PlayerUIComponent.h"

#include "MovieSceneSequencePlaybackSettings.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Sound/MusicSubsystem.h"

#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UBossActivateComponent::UBossActivateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UBossActivateComponent::BeginPlay()
{
	Super::BeginPlay();

	for (APawn* P : Targets)
	{
		CachedBoss = Cast<ABossCharacter>(P);
		BeforeBossTransform.SetScale3D(CachedBoss->GetActorScale());
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;
	APawn* Player = PC->GetPawn();
	if (!Player) return;
	UPlayerUIComponent* UIComp = Player->FindComponentByClass<UPlayerUIComponent>();
	if (!UIComp) return;
	OnBossActivated.AddUObject(UIComp, &UPlayerUIComponent::HandleBoss);
}

void UBossActivateComponent::SetCachedChar(AActor* Char)
{
	CachedChar = Cast<AJeonwoochiCharacter>(Char);
}

void UBossActivateComponent::ActivateBoss()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (bStopBGSound)
		World->GetSubsystem<UMusicSubsystem>()->StopBGM(0.1f);

	if (bActiveLSQ && PlacedSequenceActor)
	{
		// player IMC 비활성화
		if (!CachedChar) return;
		CachedChar->ClearAllIMC();

		if (bActiveFadeIn)
		{
			FadeIn(FadeInSec);

			World->GetTimerManager().SetTimer(
				FadeTimerHandle, this, &UBossActivateComponent::PlaySequence, FadeInSec, false);
		}
		else PlaySequence();
	}
	else
	{
		OnFinished();
	}

	if (!CachedBoss) return;
	OnBossActivated.Broadcast(CachedBoss, CachedBoss->BossName);
}


void UBossActivateComponent::PlaySequence()
{
	if (!PlacedSequenceActor) return;

	CachedChar->ApplyIMC_CutScene();

	if (CachedBoss && CachedChar)
	{
		CachedChar->SetActorTransform(BeforePlayerTransform);
		CachedBoss->SetActorTransform(BeforeBossTransform);
	}

	// 나중에 구조 바꾸기
	CachedChar->SetBossActiveComp(this);

	PlacedSequenceActor->PlaybackSettings.bDisableCameraCuts = false;
	PlacedSequenceActor->PlaybackSettings.bPauseAtEnd = true;

	CachedPlayer = PlacedSequenceActor->GetSequencePlayer();
	if (!CachedPlayer) return;

	CachedPlayer->OnFinished.RemoveAll(this);
	CachedPlayer->OnFinished.AddDynamic(this, &UBossActivateComponent::OnFinished);
	CachedPlayer->Play();
	CachedChar->GetUIComponent()->SetCinematicUIHidden(true);
}

void UBossActivateComponent::SkipCutScene()
{
	UE_LOG(LogTemp, Warning, TEXT("1"));

	if (!PlacedSequenceActor || !bActiveLSQ) return;

	UE_LOG(LogTemp, Warning, TEXT("2"));

	CachedPlayer->Stop();

	if(BossOst)
		GetWorld()->GetSubsystem<UMusicSubsystem>()->PlayBGM(BossOst, 0.2f);

	OnFinished();
}

void UBossActivateComponent::OnFinished()
{
	if (bActiveLSQ && PlacedSequenceActor)
	{
		if(bActiveFadeOut) FadeOut(FadeOutSec);
		CachedChar->ClearAllIMC();
		CachedChar->ApplyIMC_Default();
		DestroyMesh();

		if (CachedBoss && CachedChar)
		{
			CachedChar->SetActorTransform(AfterPlayerTransform);
			CachedBoss->SetActorTransform(AfterBossTransform);
		}

		// 나중에 구조 바꾸기
		CachedChar->SetBossActiveComp(nullptr);
		CachedChar->GetUIComponent()->SetCinematicUIHidden(false);
	}
	else
	{
		if (BossOst)
			GetWorld()->GetSubsystem<UMusicSubsystem>()->PlayBGM(BossOst, 0.2f);
	}
	ActivateAI();
}

void UBossActivateComponent::ActivateAI()
{
	for (APawn* P : Targets)
	{
		if (!IsValid(P)) continue;

		// 이미 컨트롤러가 있으면 옵션에 따라 처리
		if (AController* Existing = P->GetController())
		{
			// 강제 재빙의(=OnPossess 재호출)
			if (ABaseAIController* AIC = Cast<ABaseAIController>(Existing))
			{
				AIC->UnPossess();
				AIC->Possess(P);         // 여기서 OnPossess 자동 호출
				AIC->SetTargetActor(CachedChar);
			}
			else
			{
				// AIController가 아닌 경우 안전하게 떼어내고 새 컨트롤러 스폰
				P->DetachFromControllerPendingDestroy();
				P->SpawnDefaultController(); // 여기서 OnPossess 자동 호출
			}
			continue;
		}

		// 핵심: Pawn의 AIControllerClass 기준으로 컨트롤러 스폰 + 자동 Possess
		P->SpawnDefaultController(); // OnPossess 자동 호출
		if (ABaseAIController* AIC = Cast<ABaseAIController>(P->GetController()))
		{
			AIC->SetTargetActor(CachedChar);
		}
	}
}

void UBossActivateComponent::DestroyMesh()
{
	for (auto& Weak : DestroyTargets)
	{
		if (AActor* A = Weak.Get())
		{
			A->Destroy();
		}
	}
}

void UBossActivateComponent::FadeIn(float Duration)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->PlayerCameraManager->StartCameraFade(
			/*From*/0.f, /*To*/1.f, Duration, FLinearColor::Black,
			/*bShouldFadeAudio*/ true, /*bHoldWhenFinished*/ true);
	}
}

void UBossActivateComponent::FadeOut(float Duration)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->PlayerCameraManager->StartCameraFade(
			/*From*/1.f, /*To*/0.f, Duration, FLinearColor::Black,
			/*bShouldFadeAudio*/ false, /*bHoldWhenFinished*/ false);
	}
}

void UBossActivateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!GetWorld()) return;
	GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
	Super::EndPlay(EndPlayReason);
}