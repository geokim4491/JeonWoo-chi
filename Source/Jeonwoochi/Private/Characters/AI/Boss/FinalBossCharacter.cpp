// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AI/Boss/FinalBossCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Jeonwoochi/JeonwoochiCharacter.h"
#include "UI/FadeInHoldOutWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Utilities/PlayerUIComponent.h"
#include "Sound/MusicSubsystem.h"
#include "Controller/JeonwoochiPlayerController.h"

AFinalBossCharacter::AFinalBossCharacter()
{
	BossName = FText::FromString("유야흑군");
	RagdollTime = 7.f;
}

void AFinalBossCharacter::ChangePhase_Implementation(int32 CurrentPhase)
{
	Super::ChangePhase_Implementation(CurrentPhase);

	StartCombatTypeLoop();

	switch (CurrentPhase)
	{
	case 2: // 2 페이즈

		break;
	case 3: // 3 페이즈

		break;
	}
}

void AFinalBossCharacter::StartCombatTypeLoop()
{
	// 안전장치
	if (CombatTypes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FinalBoss: CombatTypes is EMPTY!"));
		return;
	}

	// 타이머/히스토리 리셋 후 첫 픽
	GetWorldTimerManager().ClearTimer(TypeTimerHandle);
	RecentHistory.Reset();
	CurrentIndex = INDEX_NONE;

	PickAndApplyNextCombatType(/*bFirstPick=*/true);
}

void AFinalBossCharacter::OnTypeDurationElapsed()
{
	PickAndApplyNextCombatType(/*bFirstPick=*/false);
}

void AFinalBossCharacter::PickAndApplyNextCombatType(bool bFirstPick)
{
	if (CombatTypes.Num() == 0) return;

	// 1) 후보 집합 만들기: 최근 히스토리(RecentMemory) 포함 타입 제외
	TArray<int32> Candidates;
	Candidates.Reserve(CombatTypes.Num());

	// 최근 타입들을 빠르게 조회하기 위해 Set으로 변환
	TSet<ECombatType> Excluded;
	for (int32 i = FMath::Max(0, RecentHistory.Num() - RecentMemory); i < RecentHistory.Num(); ++i)
	{
		Excluded.Add(RecentHistory[i]);
	}

	for (int32 i = 0; i < CombatTypes.Num(); ++i)
	{
		const ECombatType T = CombatTypes[i].CombatType;
		if (!Excluded.Contains(T))
		{
			Candidates.Add(i);
		}
	}

	// 2) 후보가 비었으면 룰 완화(즉시 반복만 금지 → 직전 1개만 제외)
	if (Candidates.Num() == 0 && CombatTypes.Num() >= 2)
	{
		Candidates.Reset();
		const ECombatType LastType = RecentHistory.Num() > 0 ? RecentHistory.Last() : ECombatType::Wand;
		for (int32 i = 0; i < CombatTypes.Num(); ++i)
		{
			if (CombatTypes[i].CombatType != LastType)
			{
				Candidates.Add(i);
			}
		}
	}

	// 3) 그래도 없으면(고유 타입 수가 1개뿐) 전부 허용
	if (Candidates.Num() == 0)
	{
		for (int32 i = 0; i < CombatTypes.Num(); ++i)
		{
			Candidates.Add(i);
		}
	}

	// 4) 랜덤 선택
	const int32 PickedIdxInCandidates = UKismetMathLibrary::RandomIntegerInRange(0, Candidates.Num() - 1);
	const int32 NextIndex = Candidates[PickedIdxInCandidates];
	CurrentIndex = NextIndex;

	const ECombatType ChosenType = CombatTypes[CurrentIndex].CombatType;

	// 5) 히스토리 갱신 (길이 유지)
	if (RecentHistory.Num() >= RecentMemory)
	{
		RecentHistory.RemoveAt(0);
	}
	RecentHistory.Add(ChosenType);

	// 6) 블랙보드에 반영
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			// BB의 해당 Key는 Enum(ECombatType) 타입이어야 하며, Enum Type에 ECombatType을 지정해야 합니다.
			BB->SetValueAsEnum(CombatTypeBBKey.SelectedKeyName, static_cast<uint8>(ChosenType));
		}
	}

	// 7) 다음 전환 타이머
	const float Duration = FMath::Max(0.01f, GetDurationForIndex(CurrentIndex));
	GetWorldTimerManager().SetTimer(
		TypeTimerHandle,
		this,
		&AFinalBossCharacter::OnTypeDurationElapsed,
		Duration,
		false
	);

	UE_LOG(LogTemp, Display, TEXT("FinalBoss: ChosenType=%d, Duration=%.2f"), (int32)ChosenType, Duration);
}

float AFinalBossCharacter::GetDurationForIndex(int32 Index) const
{
	return CombatTypes.IsValidIndex(Index) ? FMath::Max(0.01f, CombatTypes[Index].TypeDuration) : 5.f;
}

void AFinalBossCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(TypeTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(EndTimer);
	Super::EndPlay(EndPlayReason);
}

void AFinalBossCharacter::Die_Implementation()
{
	Super::Die_Implementation();

	if (!GreatFelledWidgetClass) return;

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (UFadeInHoldOutWidget* Wdg = CreateWidget<UFadeInHoldOutWidget>(PC, GreatFelledWidgetClass))
		{
			Wdg->AddToViewport(10000);

			// (선택) 스타일/사운드 주입
			Wdg->SetTitleColor(FLinearColor(1.f, 0.f, 0.f));
			// Wdg->SetTitleFontSize(90);
			// Wdg->AppearSound = AppearSfx;
			// Wdg->DisappearSound = DisappearSfx;

			// 보스별 텍스트를 소스코드에서 지정 (엔터 포함 가능)
			const FText Text = FText::FromString(TEXT("유야흑군 재가되어 사라지다"));
			// 예) 한글/멀티라인: FText::FromString(TEXT("위대한 적 처치\n(GREAT ENEMY FELLED)"));

			// 시간: 페이드 인 / 홀드 / 페이드 아웃
			Wdg->PlayFelled(Text, /*FadeIn=*/1.6f, /*Hold=*/2.6f, /*FadeOut=*/1.6f);
		}
	}

	if (UWorld* World = GetWorld())
	{
		UMusicSubsystem* MusicSystem = World->GetSubsystem<UMusicSubsystem>();
		if (MusicSystem)
		{
			MusicSystem->StopBGM();
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
		EndTimer,
		this,
		&AFinalBossCharacter::Ending,
		6.f,
		false
	);
}

void AFinalBossCharacter::Ending()
{
	UWorld* World = GetWorld();
	if (!World) return;

	ACharacter* Char = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!Char) return;

	if (AJeonwoochiCharacter* Player = Cast<AJeonwoochiCharacter>(Char))
	{
		Player->ClearAllIMC();
		Player->GetUIComponent()->SetCinematicUIHidden(true);
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC) return;

	if (AJeonwoochiPlayerController* JPC = Cast<AJeonwoochiPlayerController>(PC))
	{
		JPC->SetShowMouseCursor(true);
		JPC->ShowEndCredits();
	}
}
