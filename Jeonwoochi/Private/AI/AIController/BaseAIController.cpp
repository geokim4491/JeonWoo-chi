// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIController/BaseAIController.h"
#include "Characters/AI/BaseAICharacter.h"

/** Perception System */
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"

/** Componenet */
#include "Components/Combat/AIDamageableComponent.h"
#include "Components/Combat/HealthComponent.h"
#include "Components/Combat/CombatModeComponent.h"

#include "Kismet/GameplayStatics.h"

ABaseAIController::ABaseAIController()
{
	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SetPerceptionComponent(*PerceptionComp);

	/** Main BT 설정 */
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTObj(
		TEXT("BehaviorTree'/Game/AI/BT/BT_Main.BT_Main'")
	);

	if (BTObj.Succeeded())
	{
		MainBT = BTObj.Object;
	}

	/* RunBehaviorTree()나 StopLogic(), RestartLogic() 함수를
	* BrainComponent에서 사용하지 않고 BehaviorComp에서 모두 처리
	* 하는 방향으로 설계하진 않음
	*/
	BrainComponent = BehaviorComp;

	/** Perception 관련 컴포넌트 설정 */
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1200.f;
	SightConfig->LoseSightRadius = 1500.f;
	SightConfig->PeripheralVisionAngleDegrees = 70.f;
	SightConfig->SetMaxAge(10.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	PerceptionComp->ConfigureSense(*SightConfig);

	
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 500.f;
	HearingConfig->SetMaxAge(10.f);
	HearingConfig->DetectionByAffiliation = SightConfig->DetectionByAffiliation;
	PerceptionComp->ConfigureSense(*HearingConfig);


	DamageSenseConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	DamageSenseConfig->SetMaxAge(10.f);
	PerceptionComp->ConfigureSense(*DamageSenseConfig);

	/** Sight에 우선 순위를 제일 높게 설정 */
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

	/** 변수 초기화 */
	bCanSwitchTarget = true;

	TeamId = FGenericTeamId(static_cast<uint8>(ECharacterType::NormalEnemy));

}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();

}

void ABaseAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetGenericTeamId(TeamId);

	// 지각 이벤트 바인딩
	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
		this, &ABaseAIController::OnTargetPerceptionUpdated);

	if (auto* Stim = InPawn->FindComponentByClass<UAIPerceptionStimuliSourceComponent>())
		Stim->RegisterWithPerceptionSystem();

	if (InPawn->Implements<UCharacterTypeInterface>())
	{
		OwnerType = ICharacterTypeInterface::Execute_GetCharacterType(InPawn);
	}

	if (auto* MyChar = Cast<ABaseAICharacter>(InPawn))
	{
		MyChar->SetGenericTeamId(TeamId);     // 목표가 될 Pawn

		if (MyChar->SpawnMontage)
		{
			float Duration = MyChar->PlayAnimMontage(MyChar->SpawnMontage);

			UE_LOG(LogTemp, Display, TEXT("Duration: %f"), Duration);

			// 2) 재생 시간만큼 지연 후 BT 시작
			GetWorld()->GetTimerManager().SetTimer(
				SpawnMontageTimerHandle,
				FTimerDelegate::CreateUObject(
					this, &ABaseAIController::StartAIBehaviorTree),
				Duration,
				false
			);

			return; // 타이머 콜백에서 BT 시작
		}
		StartAIBehaviorTree();
	}

	/** 델리게이트 바인딩 */
	if (auto* HC = InPawn->FindComponentByClass<UHealthComponent>())
	{
		HC->OnDeath.AddUObject(this, &ABaseAIController::OnSelfDead);
	}
}

void ABaseAIController::StartAIBehaviorTree()
{
	/** BT 연결 */
	IdleTag = FGameplayTag::RequestGameplayTag(TEXT("AI.IdleVariant"), true);
	CombatTag = FGameplayTag::RequestGameplayTag(TEXT("AI.CombatVariant"), true);

	if (auto MyPawn = Cast<ABaseAICharacter>(GetPawn()))
	{
		if (MyPawn->AssignedRoute)
		{
			PatrolSpline = MyPawn->AssignedRoute->PatrolSpline;
		}

		if (UseBlackboard(MainBT->BlackboardAsset, BlackboardComp))
		{
			// 1) 트리 시작
			RunBehaviorTree(MainBT);

			// 2) Idle/Combat 서브트리 동적 주입
			if (MyPawn->ConfigData) {
				if (MyPawn->ConfigData->IdleSubtree)
				{
					BehaviorComp->SetDynamicSubtree(IdleTag, MyPawn->ConfigData->IdleSubtree);
				}

				if (MyPawn->ConfigData->CombatSubtree)
				{
					BehaviorComp->SetDynamicSubtree(CombatTag, MyPawn->ConfigData->CombatSubtree);
				}
			}
		}
		if (OwnerType == ECharacterType::Companion)
		{
			BlackboardComp->SetValueAsObject(TEXT("TargetActor"), MyPawn->FindClosestEnemy());
		}
		else
		{
			if (PerceptionComp)
			{
				TArray<AActor*> Sensed;
				PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), Sensed);

				if (Sensed.Num() > 0 && BlackboardComp)
				{
					BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Sensed[0]);
				}
			}
		}
	}
}

void ABaseAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	const auto SightID = UAISense::GetSenseID<UAISense_Sight>();
	const auto DamageID = UAISense::GetSenseID<UAISense_Damage>();
	const auto HearingID = UAISense::GetSenseID<UAISense_Hearing>();

	if (Stimulus.Type == DamageID
		&& Stimulus.WasSuccessfullySensed())
	{
		SetTargetActor(Actor);	
	}
	else if (Stimulus.Type == SightID
		&& Stimulus.WasSuccessfullySensed())
	{
		SetTargetActor(Actor);
	}
	
	
	if (!Stimulus.WasSuccessfullySensed() && Stimulus.GetAge() > 1.f &&
		(Stimulus.Type == SightID || Stimulus.Type == DamageID || Stimulus.Type == HearingID))
	{
		// 1) 액터에 대한 모든 자극 기록 가져오기
		FActorPerceptionBlueprintInfo Info;
		PerceptionComp->GetActorsPerception(Actor, Info);

		// 2) 하나라도 여전히 살아있는(Watched) 자극이 있으면 무시
		for (const auto& S : Info.LastSensedStimuli)
		{
			if (S.WasSuccessfullySensed())
				return;
		}
		if (auto* MyChar = Cast<ABaseAICharacter>(GetPawn()))
		{
			OnClearTarget(Actor, MyChar->GetCharacterType_Implementation());
		}

		UE_LOG(LogTemp, Display, TEXT("!all sense"));
		// BlackboardComp->ClearValue(TEXT("TargetActor"));
	}

}

void ABaseAIController::SetTargetActor(AActor* NewTarget)
{
	if (!BlackboardComp || !BlackboardComp->GetBlackboardAsset() || !NewTarget)
	{
		return;
	}

	if (BindingOtherHealthComp.IsValid())
	{
		BindingOtherHealthComp->OnDeath.Remove(DeathHandle);
		BindingOtherHealthComp = nullptr;
	}

	if (UHealthComponent* OtherHealthComp = NewTarget->FindComponentByClass<UHealthComponent>())
	{
		if (OtherHealthComp->IsDead()) return;
		DeathHandle = OtherHealthComp->OnDeath.AddUObject(this, &ABaseAIController::OnClearTarget);
		BindingOtherHealthComp = OtherHealthComp;
	}

	BlackboardComp->SetValueAsObject(TEXT("TargetActor"), NewTarget);

	if (auto MyPawn = Cast<ABaseAICharacter>(GetPawn()))
	{
		MyPawn->SetAmbushZoneActive(false);
	}

	if (UCombatModeComponent* C = NewTarget->FindComponentByClass<UCombatModeComponent>())
	{
		C->NotifyEnemyTargetAcquired(GetPawn());
	}
}

void ABaseAIController::OnAggroTimerExpired()
{
	bCanSwitchTarget = true;
}

void ABaseAIController::OnClearTarget(AActor* Target, ECharacterType CharType)
{
	if (BindingOtherHealthComp.IsValid())
	{
		BindingOtherHealthComp->OnDeath.Remove(DeathHandle);
		BindingOtherHealthComp = nullptr;
	}

	if (Target == BlackboardComp->GetValueAsObject(TEXT("TargetActor")))
	{
		BlackboardComp->ClearValue(TEXT("TargetActor"));
	}

	// 분신 로직
	if (GetGenericTeamId() == FGenericTeamId((uint8)ECharacterType::Player))
	{
		if (auto MyChar = Cast<ABaseAICharacter>(GetPawn()))
		{
			SetTargetActor(MyChar->FindClosestEnemy());
		}
	}

	// 일반 적 로직
	else if (GetGenericTeamId() == FGenericTeamId((uint8)ECharacterType::NormalEnemy))
	{
		if (PerceptionComp && BrainComponent)
		{
			PerceptionComp->Deactivate();
			PerceptionComp->ForgetAll();
			BrainComponent->RestartLogic();
			PerceptionComp->RequestStimuliListenerUpdate();
			PerceptionComp->Activate(true);
		}
	}
	if (auto MyPawn = Cast<ABaseAICharacter>(GetPawn()))
	{
		MyPawn->SetAmbushZoneActive(true);
	}

	if (UCombatModeComponent* C = Target->FindComponentByClass<UCombatModeComponent>())
	{
		C->NotifyEnemyLostTarget(GetPawn());
	}
}

void ABaseAIController::OnSelfDead(AActor* DeadSelf, ECharacterType)
{
	if (UCombatModeComponent* HardC = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
		->FindComponentByClass<UCombatModeComponent>())
	{
		HardC->NotifyEnemyLostTarget(DeadSelf);
	}
	
}

void ABaseAIController::OnUnPossess()
{
	// 1) Perception 델리게이트 해제
	if (PerceptionComp)
	{
		PerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(
			this, &ABaseAIController::OnTargetPerceptionUpdated);
	}

	if (BindingOtherHealthComp.IsValid())
	{
		BindingOtherHealthComp->OnDeath.Remove(DeathHandle);
		BindingOtherHealthComp = nullptr;
	}

	// 2) Behavior Tree 정지
	if (BehaviorComp && BehaviorComp->IsRunning())
	{
		BehaviorComp->StopTree(EBTStopMode::Safe);
	}

	// 3) 타이머 클리어
	GetWorldTimerManager().ClearTimer(AggroTimerHandle);

	Super::OnUnPossess();
}

void ABaseAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 혹시 OnUnPossess이 호출되지 않을 경우를 대비해 중복 방어
	if (PerceptionComp)
	{
		PerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(
			this, &ABaseAIController::OnTargetPerceptionUpdated);
	}
	
	if (BindingOtherHealthComp.IsValid())
	{
		BindingOtherHealthComp->OnDeath.Remove(DeathHandle);
		BindingOtherHealthComp = nullptr;
	}

	if (BehaviorComp && BehaviorComp->IsRunning())
	{
		BehaviorComp->StopTree(EBTStopMode::Safe);
	}

	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}