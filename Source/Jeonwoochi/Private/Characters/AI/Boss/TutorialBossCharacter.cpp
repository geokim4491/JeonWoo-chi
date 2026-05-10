// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AI/Boss/TutorialBossCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Data/Save/SaveSubsystem.h"
#include "Components/2D/DimensionShiftComponent.h"
#include "Jeonwoochi/JeonwoochiGameMode.h"

ATutorialBossCharacter::ATutorialBossCharacter()
{
	BossName = FText::FromString("정체 불명의 사술사");
}

void ATutorialBossCharacter::ChangePhase_Implementation(int32 CurrentPhase)
{
	Super::ChangePhase_Implementation(CurrentPhase);

	StartCombatTypeLoop();

	switch (CurrentPhase)
	{
	case 2: // 2 페이즈
		StopBT();
		if (CachedDimensionComp)
		{
			CachedDimensionComp->OnCompleteComebackCondition.AddUObject(this, &ATutorialBossCharacter::RestartBossBT);
		}
		break;

	case 3: // 3 페이즈는 없는데 죽을 때, 컷 신 로직을 다시 붙이기 복잡해서 여기서 컷 신 재생 후 Die 호출.
		ApplyDamage_Implementation(1000.f);
	}
}

void ATutorialBossCharacter::StartCombatTypeLoop()
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

void ATutorialBossCharacter::OnTypeDurationElapsed()
{
	PickAndApplyNextCombatType(/*bFirstPick=*/false);
}

void ATutorialBossCharacter::PickAndApplyNextCombatType(bool bFirstPick)
{
	if (CombatTypes.Num() == 0) return;

	// 1) 후보 집합 만들기: 최근 히스토리(RecentMemory) 포함 타입 제외
	TArray<int32> Candidates;
	Candidates.Reserve(CombatTypes.Num());

	// 최근 타입들을 빠르게 조회하기 위해 Set으로 변환
	TSet<ETutoCombatType> Excluded;
	for (int32 i = FMath::Max(0, RecentHistory.Num() - RecentMemory); i < RecentHistory.Num(); ++i)
	{
		Excluded.Add(RecentHistory[i]);
	}

	for (int32 i = 0; i < CombatTypes.Num(); ++i)
	{
		const ETutoCombatType T = CombatTypes[i].CombatType;
		if (!Excluded.Contains(T))
		{
			Candidates.Add(i);
		}
	}

	// 2) 후보가 비었으면 룰 완화(즉시 반복만 금지 → 직전 1개만 제외)
	if (Candidates.Num() == 0 && CombatTypes.Num() >= 2)
	{
		Candidates.Reset();
		const ETutoCombatType LastType = RecentHistory.Num() > 0 ? RecentHistory.Last() : ETutoCombatType::Wand;
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

	const ETutoCombatType ChosenType = CombatTypes[CurrentIndex].CombatType;

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
		&ATutorialBossCharacter::OnTypeDurationElapsed,
		Duration,
		false
	);

	UE_LOG(LogTemp, Display, TEXT("FinalBoss: ChosenType=%d, Duration=%.2f"), (int32)ChosenType, Duration);
}

float ATutorialBossCharacter::GetDurationForIndex(int32 Index) const
{
	return CombatTypes.IsValidIndex(Index) ? FMath::Max(0.01f, CombatTypes[Index].TypeDuration) : 5.f;
}

void ATutorialBossCharacter::RestartBossBT(EComebackCondition CombackCondition)
{
	Super::RestartBT();
}

void ATutorialBossCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(TypeTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void ATutorialBossCharacter::Die_Implementation()
{
	Super::Die_Implementation();

	UWorld* World = GetWorld();
	if (!World) return;

	if (auto* GM = World->GetAuthGameMode<AJeonwoochiGameMode>())
	{
		GM->OpenLevelHard(TEXT("/Game/ModularGothicFantasyEnvironment/Maps/DemoMapDay"), 1); // 로딩창과 함께 openlevel
	}
}


