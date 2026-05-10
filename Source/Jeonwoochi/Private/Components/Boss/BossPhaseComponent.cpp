// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Boss/BossPhaseComponent.h"
#include "Components/Combat/HealthComponent.h"
#include "Components/Utilities/PlayerUIComponent.h"

#include "MovieSceneSequencePlaybackSettings.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"

#include "Jeonwoochi/JeonwoochiCharacter.h"
#include "Characters/AI/BossCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Combat/PostureComponent.h"

// Sets default values for this component's properties
UBossPhaseComponent::UBossPhaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UBossPhaseComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsTemplate()) return; // CDO 방지

	CachedOwner = Cast<ABossCharacter>(GetOwner());
	if (!ensureAlwaysMsgf(CachedOwner, TEXT("%s: Owner null"), *GetName())) return;

	BossHealthComp = CachedOwner->FindComponentByClass<UHealthComponent>();
	if (!ensureAlwaysMsgf(BossHealthComp, TEXT("%s: UHealthComponent missing on %s"),
		*GetName(), *CachedOwner->GetName()))
		return;

	// 월드 접근도 이 타이밍에
	CachedChar = Cast<AJeonwoochiCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	BossHealthComp->OnHealthChanged.AddUObject(this, &UBossPhaseComponent::ChangeBossPhase);
}

#if WITH_EDITOR
void UBossPhaseComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshPageIndices();
}
#endif

void UBossPhaseComponent::RefreshPageIndices()
{
	for (int32 i = 0; i < Phases.Num(); ++i)
	{
		Phases[i].Page = i + 1;
	}
}

void UBossPhaseComponent::ChangeBossPhase(AActor* Boss, ECharacterType CharType, float CurrentHealth, float MaxHealth)
{
	if (LastPhase == CurrentPage + 1) return;

	if (!CachedChar)
	{
		CachedChar = Cast<AJeonwoochiCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}

	if (!BossHealthComp || !CachedChar || !CachedOwner || !bEnablePhase) return;

	// 1) 페이즈 데이터 미설정 방어
	if (Phases.IsEmpty())
	{
		ensureMsgf(false, TEXT("BossPhase: Phases is EMPTY"));
		return;
	}

	// 2) 현재 페이지 유효성 체크
	if (!Phases.IsValidIndex(CurrentPage))
	{
		ensureMsgf(false, TEXT("BossPhase: Invalid CurrentPage=%d (Num=%d)"),
			CurrentPage, Phases.Num());
		CurrentPage = FMath::Clamp(CurrentPage, 0, Phases.Num() - 1);
		if (!Phases.IsValidIndex(CurrentPage)) return;
	}


	auto& Phase = Phases[CurrentPage];

	if (CurrentHealth <= Phase.TriggerAtHealth)
	{
		if (Phase.bPlayLevelSequence)
		{
			CachedChar->ClearAllIMC();
			CachedOwner->StopBT();
			CachedChar->SetActorTransform(Phase.CutSceneTransform.BeforePlayerTransform);
			CachedOwner->SetActorTransform(Phase.CutSceneTransform.BeforeBossTransform);
			if (Phase.bActiveFadeIn)
			{
				FadeIn(Phase.FadeInSec);

				GetWorld()->GetTimerManager().SetTimer(
					FadeTimerHandle, this, &UBossPhaseComponent::PlaySequence, Phase.FadeInSec, false);
			}
			else PlaySequence();
		}

		else	OnFinished();
	}
}

void UBossPhaseComponent::PlaySequence()
{
	auto& Phase = Phases[CurrentPage];

	if (!Phase.PlacedSequenceActor) return;

	Phase.PlacedSequenceActor->PlaybackSettings.bDisableCameraCuts = false;
	Phase.PlacedSequenceActor->PlaybackSettings.bPauseAtEnd = true;
	Phase.PlacedSequenceActor->PlaybackSettings.FinishCompletionStateOverride =
		EMovieSceneCompletionModeOverride::ForceKeepState;

	CachedPlayer = Phase.PlacedSequenceActor->GetSequencePlayer();
	if (!CachedPlayer) return;

	CachedPlayer->SetCompletionModeOverride(EMovieSceneCompletionModeOverride::ForceKeepState);

	CachedPlayer->OnFinished.RemoveAll(this);
	CachedPlayer->OnFinished.AddDynamic(this, &UBossPhaseComponent::OnFinished);
	CachedPlayer->Play();
	CachedChar->GetUIComponent()->SetCinematicUIHidden(true);

	CachedChar->ClearLockOn();
}

void UBossPhaseComponent::OnFinished()
{
	auto& Phase = Phases[CurrentPage];

	//if (Phase.TriggerAtHealth == 0.f)
	//{
	//	Phase.NewMaxHealth = Phase.NewMaxHealth == 0.f ?
	//		BossHealthComp->GetMaxHealth() : Phase.NewMaxHealth;

	//	BossHealthComp->SetMaxHealth(Phase.NewMaxHealth);
	//}

	//BossHealthComp->SetCurrentHealth(Phase.TriggerAtHealth - 0.1f); // 그대로 맞추면, 무한재귀

	if (Phases[CurrentPage].bPlayLevelSequence)
	{
		CachedChar->ApplyIMC_Default();
		CachedOwner->RestartBT();
		CachedChar->SetActorTransform(Phase.CutSceneTransform.AfterPlayerTransform);
		CachedOwner->SetActorTransform(Phase.CutSceneTransform.AfterBossTransform);

		DestroyMesh();

		if (Phase.bActiveFadeOut) FadeOut(Phase.FadeOutSec);

		CachedChar->GetUIComponent()->SetCinematicUIHidden(false);
	}

	if (Phase.bResetPosture)
	{
		if (auto* Posture = CachedOwner->FindComponentByClass<UPostureComponent>())
			Posture->ResetCurrentPosture();
	}

	CurrentPage++;

	// 보스별 행동 지정.
	if (CachedOwner->Implements<UBossChangePhaseInterface>())
	{
		IBossChangePhaseInterface::Execute_ChangePhase(CachedOwner, CurrentPage + 1);
	}
	
}

void UBossPhaseComponent::SkipCutScene()
{
	
}

void UBossPhaseComponent::FadeIn(float Duration)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->PlayerCameraManager->StartCameraFade(
			/*From*/0.f, /*To*/1.f, Duration, FLinearColor::Black,
			/*bShouldFadeAudio*/ true, /*bHoldWhenFinished*/ true);
	}
}

void UBossPhaseComponent::FadeOut(float Duration)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->PlayerCameraManager->StartCameraFade(
			/*From*/1.f, /*To*/0.f, Duration, FLinearColor::Black,
			/*bShouldFadeAudio*/ false, /*bHoldWhenFinished*/ false);
	}
}

void UBossPhaseComponent::DestroyMesh()
{
	auto& Phase = Phases[CurrentPage];

	for (auto& Weak : Phase.DestroyTargets)
	{
		if (AActor* A = Weak.Get())
		{
			A->Destroy();
		}
	}
}

void UBossPhaseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BossHealthComp)
		BossHealthComp->OnHealthChanged.RemoveAll(this);

	if (!GetWorld()) return;
	GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);

	Super::EndPlay(EndPlayReason);
}
