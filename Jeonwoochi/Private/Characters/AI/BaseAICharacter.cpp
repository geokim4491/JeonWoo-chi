// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AI/BaseAICharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/Combat/ExecutionComponent.h"
#include "Components/Combat/AmbushComponent.h"
#include "Components/Combat/AmbushedComponent.h"
#include "Data/Save/SaveTargetComponent.h"
#include "Components/WidgetComponent.h"

#include "UI/AIHpBar.h"

#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

ABaseAICharacter::ABaseAICharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CharacterType = ECharacterType::NormalEnemy;

	HealthComp = CreateDefaultSubobject<UHealthComponent>
		(TEXT("HealthComp"));
	HealthComp->MaxHealth = 100.f;

	DamageableComp = CreateDefaultSubobject<UAIDamageableComponent>
		(TEXT("DamageableComp"));

	AttackComp = CreateDefaultSubobject<UAttackComponent>
		(TEXT("AttackComp"));

	WeaponManageComp = CreateDefaultSubobject<UWeaponManageComponent>
		(TEXT("WeaponManageComp"));

	PostureComp = CreateDefaultSubobject<UPostureComponent>
		(TEXT("PostureComp"));

	AmbushedComp = CreateDefaultSubobject<UAmbushedComponent>
		(TEXT("AmbushedComp"));

	SaveComp = CreateDefaultSubobject<USaveTargetComponent>
		(TEXT("SaveComp"));

	LockMarkerComp = CreateDefaultSubobject<UWidgetComponent>
		(TEXT("LockMarkerComp"));

	ExecutionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ExecutionZone"));
	ExecutionZone->SetupAttachment(GetMesh());                     // 또는 Root
	ExecutionZone->SetRelativeLocation(ExecZoneOffset);
	ExecutionZone->SetBoxExtent(ExecZoneExtent);
	ExecutionZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ExecutionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExecutionZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // Pawn만
	ExecutionZone->SetGenerateOverlapEvents(true);
	ExecutionZone->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnExecZoneBeginOverlap);
	ExecutionZone->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnExecZoneEndOverlap);

	AmbushZone = CreateDefaultSubobject<UBoxComponent>(TEXT("AmbushZone"));
	AmbushZone->SetupAttachment(GetMesh());
	AmbushZone->SetRelativeLocation(AmbushZoneOffset);
	AmbushZone->SetBoxExtent(AmbushZoneExtent);
	AmbushZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AmbushZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	AmbushZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AmbushZone->SetGenerateOverlapEvents(true);
	AmbushZone->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAmbushZoneBeginOverlap);
	AmbushZone->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnAmbushZoneEndOverlap);

	AIControllerClass = ABaseAIController::StaticClass();

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	AmbushWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("AmbushWidgetComponent"));
	AmbushWidgetComp->SetupAttachment(GetRootComponent());
	AmbushWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
	AmbushWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	AmbushWidgetComp->SetUsingAbsoluteRotation(true);
	AmbushWidgetComp->SetDrawAtDesiredSize(true);
	AmbushWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AmbushWidgetComp->SetPivot(FVector2D(0.5f, 0.5f));

	static ConstructorHelpers::FClassFinder<UUserWidget> WBP_AmbushWidget(
		TEXT("/Game/UI/WBP_Ambush")   // <- 콘텐츠 브라우저 경로 (에셋 이름까지만, _C 붙이지 않음)
	);

	if (WBP_AmbushWidget.Succeeded())
	{
		AmbushWidgetComp->SetWidgetClass(WBP_AmbushWidget.Class);
	}

	ExecutionWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("ExecutionWidgetComp"));
	ExecutionWidgetComp->SetupAttachment(GetRootComponent());
	ExecutionWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 35.f));
	ExecutionWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	ExecutionWidgetComp->SetUsingAbsoluteRotation(true);
	ExecutionWidgetComp->SetDrawAtDesiredSize(true);
	ExecutionWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ExecutionWidgetComp->SetPivot(FVector2D(0.5f, 0.5f));

	static ConstructorHelpers::FClassFinder<UUserWidget> WBP_ExecutionWidget(
		TEXT("/Game/UI/WBP_Execution")   // <- 콘텐츠 브라우저 경로 (에셋 이름까지만, _C 붙이지 않음)
	);

	if (WBP_ExecutionWidget.Succeeded())
	{
		ExecutionWidgetComp->SetWidgetClass(WBP_ExecutionWidget.Class);
	}

	HPBarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarComponent"));
	HPBarComp->SetupAttachment(GetRootComponent());

	// 월드 공간 위젯
	HPBarComp->SetWidgetSpace(EWidgetSpace::World);
	// HPBarComp->SetDrawAtDesiredSize(true);
	HPBarComp->SetPivot(FVector2D(0.5f, 0.0f));
	HPBarComp->SetDrawSize(HPBarSize);

	// 위치(머리 위로 살짝)
	HPBarComp->SetRelativeLocation(HPBarOffset);

	HPBarComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HPBarComp->SetUsingAbsoluteRotation(true);
	HPBarComp->SetMobility(EComponentMobility::Movable);
	HPBarComp->SetVisibility(false);
	HPBarComp->SetHiddenInGame(true);

	static ConstructorHelpers::FClassFinder<UUserWidget> WBP_AIHpBar(
		TEXT("/Game/UI/WBP_AIHpBar")   // <- 콘텐츠 브라우저 경로 (에셋 이름까지만, _C 붙이지 않음)
	);

	if (WBP_AIHpBar.Succeeded())
	{
		HPBarComp->SetWidgetClass(WBP_AIHpBar.Class);
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> WBP_LockOn(
		TEXT("/Game/UI/WBP_LockOn")   // <- 콘텐츠 브라우저 경로 (에셋 이름까지만, _C 붙이지 않음)
	);

	if (WBP_LockOn.Succeeded())
	{
		LockMarkerComp->SetWidgetClass(WBP_LockOn.Class);
	}

	LockMarkerComp->SetupAttachment(GetRootComponent());
	LockMarkerComp->SetWidgetSpace(EWidgetSpace::Screen);
	// LockMarkerComp->SetDrawAtDesiredSize(true);                // WBP가 64x64 등으로 맞춰져 있어야 함
	LockMarkerComp->SetDrawSize(FVector2D(15.f, 15.f));
	LockMarkerComp->SetPivot(FVector2D(0.5f, 0.5f));           // 중심 기준
	LockMarkerComp->SetTwoSided(true);
	LockMarkerComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LockMarkerComp->SetUsingAbsoluteRotation(true); // 부모 회전에 영향 안 받게
	LockMarkerComp->SetTranslucentSortPriority(10); // 겹침 우선순위(가려짐 방지용, 선택)
	LockMarkerComp->SetVisibility(false);
	LockMarkerComp->SetHiddenInGame(true);

}

void ABaseAICharacter::BeginPlay()
{
	Super::BeginPlay();

	OwnerController = Cast<AAIController>(GetController());
	SetGenericTeamId(FGenericTeamId(static_cast<uint8>(ECharacterType::NormalEnemy)));

	if (HPBarComp)
	{
		if (UUserWidget* UW = HPBarComp->GetUserWidgetObject())
		{
			UAIHpBar* HPWidgetRef = Cast<UAIHpBar>(UW);
			if (HPWidgetRef)
			{
				// 시작 시 즉시 동기화
				HPWidgetRef->UpdateHp(this, CharacterType, HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
				HPWidgetRef->UpdatePosture(PostureComp->GetCurPosture(), PostureComp->GetMaxPosture());
				HealthComp->OnHealthChanged.AddUObject(HPWidgetRef, &UAIHpBar::UpdateHp);
				PostureComp->OnPostureUpdated.AddUObject(HPWidgetRef, &UAIHpBar::UpdatePosture);
			}
		}
		HPBarComp->SetRelativeLocation(HPBarOffset);
	}

	if (ExecutionWidgetComp)
	{
		if (UUserWidget* UW = ExecutionWidgetComp->GetUserWidgetObject())
		{
			UW->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (AmbushWidgetComp)
	{
		if (UUserWidget* UW = AmbushWidgetComp->GetUserWidgetObject())
		{
			UW->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	UpdateLockMarkerTransform();
}

void ABaseAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// HPBar
	if (HPBarComp) FaceCamera(HPBarComp);

	// 락온 링
	if (LockMarkerComp) UpdateLockMarkerTransform();
}

#if WITH_EDITOR
void ABaseAICharacter::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	if (bFinished)
	{
		if (auto* ST = FindComponentByClass<USaveTargetComponent>())
		{
			ST->CaptureFromActor(); // 소유 액터의 현재 트랜스폼을 EditorTransform으로 기록
		}
	}
}

void ABaseAICharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 루트 트랜스폼을 숫자 입력으로 바꾼 경우에도 대응하고 싶다면(선택)
	static const FName NAME_RelativeLocation = TEXT("RelativeLocation");
	static const FName NAME_RelativeRotation = TEXT("RelativeRotation");
	static const FName NAME_RelativeScale3D = TEXT("RelativeScale3D");

	if (PropertyChangedEvent.MemberProperty)
	{
		const FName N = PropertyChangedEvent.MemberProperty->GetFName();
		if (N == NAME_RelativeLocation || N == NAME_RelativeRotation || N == NAME_RelativeScale3D)
		{
			if (auto* ST = FindComponentByClass<USaveTargetComponent>())
			{
				ST->CaptureFromActor();
			}
		}
	}
}
#endif // WITH_EDITOR

void ABaseAICharacter::Ragdoll()
{
	// 중복 호출 방지
	if (GetMesh()->IsSimulatingPhysics()) return;

	// 1) 붙어 있는 Child Actor 분리
	TArray<UChildActorComponent*> ChildComps;
	GetComponents<UChildActorComponent>(ChildComps);
	for (UChildActorComponent* ChildComp : ChildComps)
	{
		AActor* ChildActor = ChildComp->GetChildActor();
		if (ChildActor)
		{
			// 월드(transform) 그대로 분리
			ChildActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

			// 분리된 액터 안에 Static/Skeletal Mesh가 있으면 물리 활성화
			if (UStaticMeshComponent* SM = ChildActor->FindComponentByClass<UStaticMeshComponent>())
			{
				SM->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
				SM->SetSimulatePhysics(true);
			}
			else if (USkeletalMeshComponent* SK = ChildActor->FindComponentByClass<USkeletalMeshComponent>())
			{
				SK->SetCollisionProfileName(TEXT("Ragdoll"));
				SK->SetAllBodiesSimulatePhysics(true);
				SK->SetSimulatePhysics(true);
				SK->WakeAllRigidBodies();
				SK->bBlendPhysics = true;
			}

			// (선택) ChildActorComponent 제거 so it won't reattach on destroy
			ChildComp->DestroyComponent();
		}
	}

	// 2) 컨트롤러 분리
	DetachFromControllerPendingDestroy();

	// 3) 이동 중지 & 콜리전 끄기
	if (auto* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 4) 본체 메쉬 물리 전환
	USkeletalMeshComponent* Skel = GetMesh();
	Skel->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Skel->SetGenerateOverlapEvents(false);
	Skel->SetCollisionProfileName(TEXT("Ragdoll"));          // 에디터에서 'Ragdoll' 프로필 설정 권장
	Skel->SetAllBodiesSimulatePhysics(true);                 // 모든 본에 물리 시뮬레이션
	Skel->SetSimulatePhysics(true);                          // 물리 본 활성화
	Skel->WakeAllRigidBodies();                              // 물리 본 활성화
	Skel->bBlendPhysics = true;                              // 애니→물리 부드럽게 전환


	// (A) 전체 보디에 질량을 2배로 늘려서 무겁게
	Skel->SetAllMassScale(2.0f);

	// (B) 관성 저항(댐핑) 높여서 쉽게 날아가지 않게
	Skel->SetLinearDamping(8.0f);
	Skel->SetAngularDamping(8.0f);


	Skel->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Skel->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);

	// 5) 사라질 타이머 (옵션)
	SetLifeSpan(RagdollTime);
}

void ABaseAICharacter::illusioned(float Duration, bool bDeath)
{
	if (CharacterType == ECharacterType::Boss)
	{
		// 보스한테 환술걸려할 때, 로직.
		return;
	}

	if (GetGenericTeamId() == 
		FGenericTeamId(static_cast<uint8>(ECharacterType::Player))) return;

	UE_LOG(LogTemp, Warning, TEXT("Team Id: %d"), (uint8)GetGenericTeamId());

	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		// 1) 팀 ID를 **Pawn + Controller** 모두 변경
		SetGenericTeamId(static_cast<uint8>(ECharacterType::Player));
		AICon->SetGenericTeamId(static_cast<uint8>(ECharacterType::Player));

		UAIPerceptionComponent* Perception = AICon->GetPerceptionComponent();
		UBlackboardComponent* BB = AICon->GetBlackboardComponent();
		UBrainComponent* BrainComp = AICon->GetBrainComponent();

		// 2) 시야 끄고 메모리 초기화
		Perception->Deactivate();
		Perception->ForgetAll();

		// 3) 새 타깃 선정
		if (BB)
		{
			BB->SetValueAsObject(TEXT("TargetActor"), FindClosestEnemy());
		}

		// 4) BT 재시작
		if (BrainComp)
		{
			BrainComp->RestartLogic();
		}

		// 5) 시야 다시 켜고, 팀 변경 사항 반영
		Perception->RequestStimuliListenerUpdate();
		Perception->Activate(true);

		billusioned = true;
	}

	if (bDeath && HealthComp && !HealthComp->IsDead())
	{
		// 중복 예약 방지
		GetWorldTimerManager().ClearTimer(illusionTimer);

		FTimerDelegate D;
		D.BindUObject(this, &ABaseAICharacter::HandleIllusionExpire);
		GetWorldTimerManager().SetTimer(illusionTimer, D, Duration, false);
	}
}

void ABaseAICharacter::HandleIllusionExpire()
{
	if (IsValid(HealthComp) && !HealthComp->IsDead())
	{
		HealthComp->Die();
	}
}

void ABaseAICharacter::SetTargetToClosetEnemy()
{
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		UBlackboardComponent* BB = AICon->GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsObject(TEXT("TargetActor"), FindClosestEnemy());
		}
	}
}

AActor* ABaseAICharacter::FindClosestEnemy(float Radius) const
{
	TArray<AActor*> Overlapped;
	// Pawn 채널만 검사 (필요하면 ObjectTypes 배열에 추가/제거)
	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes = {
		UEngineTypes::ConvertToObjectType(ECC_Pawn) };

	// Sphere 오버랩
	UKismetSystemLibrary::SphereOverlapActors(
		this,                          // WorldContextObject
		GetActorLocation(),            // Center
		Radius,                        // 반경
		ObjTypes,
		APawn::StaticClass(),          // 클래스 필터
		{ const_cast<ABaseAICharacter*>(this) }, // 제외할 Actor들
		Overlapped);

	// 디버그용 구(선택)
	DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 16, FColor::Cyan, false, 1.f);

	// 가장 가까운 적 찾기
	AActor* Closest = nullptr;
	float   BestSq = FLT_MAX;

	for (AActor* Actor : Overlapped)
	{
		const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Actor);
		if (!TeamAgent) continue;

		if (TeamAgent->GetGenericTeamId() == GetGenericTeamId())  // 같은 편
		{
			continue;
		}
			

		const float DistSq = FVector::DistSquared(Actor->GetActorLocation(), GetActorLocation());
		if (DistSq < BestSq)
		{
			BestSq = DistSq;
			Closest = Actor;
		}
	}
	return Closest;
}

void ABaseAICharacter::StopBT()
{
	if (!OwnerController)
	{
		OwnerController = Cast<AAIController>(GetController());
	}
	if (UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(OwnerController->BrainComponent))
	{
		BT->StopTree(EBTStopMode::Safe);   // ← Service까지 완전 정지
	}
}

void ABaseAICharacter::RestartBT()
{
	if (!OwnerController)
	{
		OwnerController = Cast<AAIController>(GetController());
	}

	if (UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(OwnerController->BrainComponent))
	{
		// 1) 정지
		BT->StopTree(EBTStopMode::Safe);

		// 2) 재시작
		BT->RestartTree();

		// 3) 서브트리 재주입
		BT->SetDynamicSubtree(FGameplayTag::RequestGameplayTag(TEXT("AI.IdleVariant")),
			ConfigData->IdleSubtree);
		BT->SetDynamicSubtree(FGameplayTag::RequestGameplayTag(TEXT("AI.CombatVariant")),
			ConfigData->CombatSubtree);
	}
}

// =========================== Ambushed ============================

void ABaseAICharacter::Ambushed()
{
	if (AmbushedComp)
		AmbushedComp->OnAmbushed();
}

void ABaseAICharacter::SetAmbushZoneActive(bool bActive)
{
	if (!AmbushZone && billusioned) return;

	if (bActive)
	{
		if (!AmbushZone->ComponentTags.Contains(TEXT("AmbushZone")))
			AmbushZone->ComponentTags.Add(TEXT("AmbushZone"));

		AmbushZone->SetGenerateOverlapEvents(true);
		AmbushZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		AmbushZone->UpdateOverlaps();
	}
	else
	{
		AmbushZone->SetGenerateOverlapEvents(false);
		AmbushZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ABaseAICharacter::OnAmbushZoneBeginOverlap(UPrimitiveComponent*, AActor* Other,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (auto* AmbushComp = Other->FindComponentByClass<UAmbushComponent>())
	{
		if (AmbushWidgetComp && !billusioned && CharacterType != ECharacterType::Boss)
		{
			AmbushComp->SetCanAmbush(true);
			AmbushComp->SetAmbushedTarget(this);

			UUserWidget* Widget = AmbushWidgetComp->GetUserWidgetObject();
			if (Widget)
			{
				Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
		}
		
	}
}

void ABaseAICharacter::OnAmbushZoneEndOverlap(UPrimitiveComponent*, AActor* Other,
	UPrimitiveComponent*, int32)
{
	if (auto* AmbushComp = Other->FindComponentByClass<UAmbushComponent>())
	{
		if (AmbushComp->GetAmbushedTarget() == this)
		{
			if (AmbushWidgetComp && !billusioned && CharacterType != ECharacterType::Boss)
			{
				AmbushComp->SetCanAmbush(false);
				AmbushComp->SetAmbushedTarget(nullptr);
				UUserWidget* Widget = AmbushWidgetComp->GetUserWidgetObject();
				if (Widget)
				{
					Widget->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}
	}
}

bool ABaseAICharacter::CalcAmbushAlignTransform(FTransform& OutTransform) const
{
	if (!AmbushZone) return false;

	const FVector  Center = AmbushZone->GetComponentLocation();
	const FVector  FrontDir = AmbushZone->GetRightVector();
	const FVector  Extent = AmbushZone->GetScaledBoxExtent();

	// 박스 "앞"으로 약간 떨어진 지점(여유 오프셋은 캐릭터 캡슐 반지름 고려)
	const float ExtraOffset = 30.f; // 필요에 맞게 조절
	FVector TargetLoc = Center - FrontDir * (Extent.Y + ExtraOffset);

	// 플레이어가 적을 바라보도록 회전
	FRotator TargetRot = (GetActorLocation() - TargetLoc).Rotation();
	TargetRot.Pitch = 0.f;
	TargetRot.Roll = 0.f;

	OutTransform = FTransform(TargetRot, TargetLoc, FVector::OneVector);
	return true;
}

// =========================== Executed ============================

void ABaseAICharacter::Executed()
{
	if(PostureComp)
		PostureComp->FinishExecution();
}

void ABaseAICharacter::SetExecZoneActive(bool bActive)
{
	if (!ExecutionZone) return;

	if (bActive && !HealthComp->IsDead())
	{
		// 태그로 필터링할 거면 한 번만 세팅
		if (!ExecutionZone->ComponentTags.Contains(TEXT("ExecZone")))
			ExecutionZone->ComponentTags.Add(TEXT("ExecZone"));

		ExecutionZone->SetGenerateOverlapEvents(true);
		ExecutionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		// 이미 겹쳐있던 액터가 있으면 BeginOverlap이 안 올 수 있으니 수동 처리
		ExecutionZone->UpdateOverlaps();
		NotifyPlayersInZone_On();
	}
	else
	{
		// 끄기 전에, 현재 겹쳐있는 플레이어에게 해제 통지
		NotifyPlayersInZone_Off();

		ExecutionZone->SetGenerateOverlapEvents(false);
		ExecutionZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// (선택) 디버그용 표시 on/off도 여기서
		// ExecutionZone->SetHiddenInGame(true);
	}
}

void ABaseAICharacter::NotifyPlayersInZone_On()
{
	TArray<AActor*> Overlapping;
	ExecutionZone->GetOverlappingActors(Overlapping, APawn::StaticClass());

	for (AActor* Other : Overlapping)
	{
		APawn* Pawn = Cast<APawn>(Other);
		if (!Pawn || !Pawn->IsPlayerControlled()) continue;

		if (auto* ExecComp = Other->FindComponentByClass<UExecutionComponent>())
		{
			// 이 적을 타깃으로 실행 가능
			ExecComp->SetExecutedTarget(this);
			ExecComp->SetCanExecute(true);
		}
	}
}

void ABaseAICharacter::NotifyPlayersInZone_Off()
{
	TArray<AActor*> Overlapping;
	ExecutionZone->GetOverlappingActors(Overlapping, APawn::StaticClass());

	for (AActor* Other : Overlapping)
	{
		APawn* Pawn = Cast<APawn>(Other);
		if (!Pawn || !Pawn->IsPlayerControlled()) continue;

		if (auto* ExecComp = Other->FindComponentByClass<UExecutionComponent>())
		{
			// 현재 타깃이 나(이 적)일 때만 끈다 — 다른 적 존에 들어가 있을 수 있음
			if (ExecComp->GetExecutedTarget() == this)
			{
				ExecComp->SetCanExecute(false);
				ExecComp->SetExecutedTarget(nullptr);
				if (ExecutionWidgetComp)
				{
					UUserWidget* Widget = ExecutionWidgetComp->GetUserWidgetObject();
					if (Widget)
					{
						Widget->SetVisibility(ESlateVisibility::Collapsed);
					}
				}
			}
		}
	}
}

void ABaseAICharacter::OnExecZoneBeginOverlap(UPrimitiveComponent*, AActor* Other,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (auto* ExecComp = Other->FindComponentByClass<UExecutionComponent>())
	{
		ExecComp->SetCanExecute(true);
		ExecComp->SetExecutedTarget(this);
		if (ExecutionWidgetComp)
		{
			UUserWidget* Widget = ExecutionWidgetComp->GetUserWidgetObject();
			if (Widget)
			{
				Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
		}
	}
}

void ABaseAICharacter::OnExecZoneEndOverlap(UPrimitiveComponent*, AActor* Other,
	UPrimitiveComponent*, int32)
{
	if (auto* ExecComp = Other->FindComponentByClass<UExecutionComponent>())
	{
		if (ExecComp->GetExecutedTarget() == this)
		{
			ExecComp->SetCanExecute(false);
			ExecComp->SetExecutedTarget(nullptr);
			if (ExecutionWidgetComp)
			{
				UUserWidget* Widget = ExecutionWidgetComp->GetUserWidgetObject();
				if (Widget)
				{
					Widget->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}
	}
}

bool ABaseAICharacter::CalcExecAlignTransform(FTransform& OutTransform) const
{
	if (!ExecutionZone) return false;

	const FVector  Center = ExecutionZone->GetComponentLocation();
	const FVector  FrontDir = ExecutionZone->GetRightVector();
	const FVector  Extent = ExecutionZone->GetScaledBoxExtent();

	// 박스 "앞"으로 약간 떨어진 지점(여유 오프셋은 캐릭터 캡슐 반지름 고려)
	const float ExtraOffset = 30.f; // 필요에 맞게 조절
	FVector TargetLoc = Center - FrontDir * (Extent.Y + ExtraOffset);

	// 플레이어가 적을 바라보도록 회전
	FRotator TargetRot = (GetActorLocation() - TargetLoc).Rotation();
	TargetRot.Pitch = 0.f;
	TargetRot.Roll = 0.f;

	OutTransform = FTransform(TargetRot, TargetLoc, FVector::OneVector);
	return true;
}

void ABaseAICharacter::SetIsBoss(bool Set)
{
	 SaveComp->bIsBoss = Set; 
}

// ======================== UI 함수 ======================

void ABaseAICharacter::ShowLockMarker(bool bShow)
{
	if (!LockMarkerComp) return;

	LockMarkerComp->SetVisibility(bShow, true);
	LockMarkerComp->SetHiddenInGame(!bShow, true);

	if (bShow)
	{
		UpdateLockMarkerTransform(); // 켜지는 순간에도 한 번 맞춰주기
	}
}

void ABaseAICharacter::ShowHPBar(bool bShow)
{
	if (!HPBarComp) return;

	// 1) 모든 타이머 정리 (중요)
	GetWorldTimerManager().ClearTimer(TH_HPAutoHide);
	GetWorldTimerManager().ClearTimer(TH_HPFade);
	bIsFadingHPBar = false;

	// 2) 컴포넌트는 항상 켠 상태 유지 (RT/Invalidation 흔들지 않기)
	HPBarComp->SetVisibility(true, true);
	HPBarComp->SetHiddenInGame(false, true);

	// 3) 오직 Opacity/Visibility로만 제어
	if (UUserWidget* UW = HPBarComp->GetUserWidgetObject())
	{
		UW->StopAllAnimations();
		UW->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		UW->SetRenderOpacity(bShow ? 1.f : 0.f);
	}
}

void ABaseAICharacter::ShowHPBarTimed(float Seconds)
{
	// 락온 중이면 항상 표시 유지
	if (bHPBarPinnedByLockOn)
	{
		ShowHPBar(true);
		return;
	}

	// 즉시 보이게
	ShowHPBar(true);

	// Seconds==0이면 바로 숨김 시도
	GetWorldTimerManager().ClearTimer(TH_HPAutoHide);
	if (Seconds <= 0.f)
	{
		HideHPBarIfNotPinned();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			TH_HPAutoHide, this, &ThisClass::HideHPBarIfNotPinned, Seconds, false);
	}
}

void ABaseAICharacter::HideHPBarIfNotPinned()
{
	if (bHPBarPinnedByLockOn || !HPBarComp || bIsFadingHPBar) return;

	// Opacity만 서서히 0으로
	FadeOutHPBar(0.25f);
}

void ABaseAICharacter::SetHPBarPinnedByLockOn(bool bPinned, float HideTime)
{
	bHPBarPinnedByLockOn = bPinned;

	if (bPinned)
	{
		// 항상 표시 + 모든 타이머/페이드 정지
		GetWorldTimerManager().ClearTimer(TH_HPAutoHide);
		GetWorldTimerManager().ClearTimer(TH_HPFade);
		bIsFadingHPBar = false;
		ShowHPBar(true);
	}
	else
	{
		// 일반 규칙 복귀
		ShowHPBarTimed(HideTime);
	}
}

void ABaseAICharacter::FadeOutHPBar(float Duration)
{
	if (!HPBarComp || bIsFadingHPBar) return;

	if (UUserWidget* UW = HPBarComp->GetUserWidgetObject())
	{
		bIsFadingHPBar = true;

		// 컴포넌트는 항상 켜둠 (렌더 리소스/생명주기 안정)
		HPBarComp->SetVisibility(true, true);
		HPBarComp->SetHiddenInGame(false, true);

		UW->StopAllAnimations();
		UW->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		const float Step = 0.016f; // 60fps
		float Elapsed = 0.f;

		TWeakObjectPtr<UUserWidget> WeakUW = UW;
		TWeakObjectPtr<UWidgetComponent> WeakComp = HPBarComp;

		FTimerDelegate Tick;
		Tick.BindWeakLambda(this, [this, WeakUW, WeakComp, Duration, Step, Elapsed]() mutable
			{
				// 소유자/컴포넌트/위젯이 사라졌으면 안전 종료
				if (!IsValid(this) || !WeakComp.IsValid() || !WeakUW.IsValid())
				{
					if (IsValid(this))
					{
						GetWorldTimerManager().ClearTimer(TH_HPFade);
						bIsFadingHPBar = false;
					}
					return;
				}

				Elapsed += Step;
				const float Alpha = FMath::Clamp(1.f - (Elapsed / Duration), 0.f, 1.f);
				WeakUW->SetRenderOpacity(Alpha);

				if (Alpha <= 0.f)
				{
					GetWorldTimerManager().ClearTimer(TH_HPFade);
					bIsFadingHPBar = false;

					// 컴포넌트 끄지 말고, 위젯만 접기
					WeakUW->SetVisibility(ESlateVisibility::Collapsed);
					WeakUW->SetRenderOpacity(1.f); // 다음 표시 대비 복원
				}
			});

		GetWorldTimerManager().SetTimer(TH_HPFade, Tick, Step, true);
	}
}

void ABaseAICharacter::UpdateLockMarkerTransform()
{
	if (!LockMarkerComp) return;

	// 중앙 기준점
	FVector Center = GetActorLocation() + CenterOffset;

	// 카메라 쪽으로 살짝 띄우기(가려짐 방지용)
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->PlayerCameraManager)
	{
		const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
		const FVector ToCam = (CamLoc - Center).GetSafeNormal();
		const float   DepthBias = 30.f;           // 필요 시 50~80까지 올려보세요
		Center += ToCam * DepthBias;
	}

	LockMarkerComp->SetWorldLocation(Center);
	FaceCamera(LockMarkerComp);                   // 항상 카메라 보게
}

void ABaseAICharacter::FaceCamera()
{
	if (!HPBarComp) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}
	const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
	const FVector CompLoc = HPBarComp->GetComponentLocation();

	// 월드업(Z)을 유지하면서 카메라를 향하게: Pitch/ Roll 안정
	const FVector ToCam = (CamLoc - CompLoc).GetSafeNormal();

	// X축(Forward)이 카메라를 보게, Z축은 월드업으로 고정
	FRotator LookAt = FRotationMatrix::MakeFromXZ(ToCam, FVector::UpVector).Rotator();

	// 프로젝트/위젯 앞면 축에 따라 뒤집혀 보이면 이 줄만 켜줘
	//LookAt.Yaw += 180.f;

	HPBarComp->SetWorldRotation(LookAt);
}

void ABaseAICharacter::FaceCamera(UWidgetComponent* Comp)
{
	if (!Comp) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !PC->PlayerCameraManager) return;

	const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
	const FVector CompLoc = Comp->GetComponentLocation();

	// 월드업(Z)을 유지하면서 카메라를 향하게
	const FVector ToCam = (CamLoc - CompLoc).GetSafeNormal();
	FRotator LookAt = FRotationMatrix::MakeFromXZ(ToCam, FVector::UpVector).Rotator();

	// 앞뒤가 뒤집혀 보이면 주석 해제
	// LookAt.Yaw += 180.f;

	Comp->SetWorldRotation(LookAt);
}

// ======================== Interface 함수 ======================

void ABaseAICharacter::Die_Implementation()
{
	PostureComp->ReducePostureCount(1000);
	SetHPBarPinnedByLockOn(false);
	FadeOutHPBar(0.35f);

	if (auto* Stim = FindComponentByClass<UAIPerceptionStimuliSourceComponent>())
	{
		Stim->UnregisterFromPerceptionSystem();
		Stim->DestroyComponent();
	}

	// 자동 등록돼 있던 Sight·Hearing 타깃 해제
	if (UAIPerceptionSystem* PS = UAIPerceptionSystem::GetCurrent(GetWorld()))
	{
		PS->UnregisterSource(*this, UAISense_Sight::StaticClass());
		PS->UnregisterSource(*this, UAISense_Hearing::StaticClass());
	}

	// (선택) 이미 본 AI들의 캐시 삭제
	for (TObjectIterator<UAIPerceptionComponent> It; It; ++It)
	{
		It->ForgetActor(this);   // 또는 ForgetAll()
	}

	// 1) 멈춰야 할 AIController 얻기
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		// 2) BehaviorTree 멈추기
		if (UBrainComponent* Brain = AICon->BrainComponent)
		{
			Brain->StopLogic("Died");
		}

		// 3) 언포지션 (Possession 해제)
		AICon->UnPossess();

		// 4) (선택) 이동 정지
		AICon->StopMovement();

	}

	Ragdoll();

	if (billusioned)
	{
		GetMesh()->SetOverlayMaterial(nullptr);
	}
}

void ABaseAICharacter::TakeDamage_Implementation(const FDamageInfoStruct& DamageInfo)
{
	DamageableComp->HandleTakeDamage(DamageInfo);
}

float ABaseAICharacter::GetCurrentHealth_Implementation() const
{
	return HealthComp->GetCurrentHealth();
}

float ABaseAICharacter::GetMaxHealth() const
{
	return HealthComp->GetMaxHealth();
}

void ABaseAICharacter::ApplyDamage_Implementation(const float& DamageAmount)
{
	HealthComp->ApplyDamage(DamageAmount);
	ShowHPBarTimed(HPBarAutoHideDelay);
}

void ABaseAICharacter::ApplyDotDamage_Implementation(const FDotDealInfoStruct& DotInfo)
{
	HealthComp->ApplyDotDamage(DotInfo);
}

void ABaseAICharacter::Attack(const FDamageInfoStruct& DamageInfo)
{
	AttackComp->Attack(DamageInfo);
}

void ABaseAICharacter::DotAttack(const FDotDealInfoStruct& DotInfo, AActor* Target)
{
	AttackComp->DotAttack(DotInfo, Target);
}

void ABaseAICharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	if (HPBarComp)
	{
		if (UUserWidget* UW = HPBarComp->GetUserWidgetObject())
		{
			UAIHpBar* HPWidgetRef = Cast<UAIHpBar>(UW);
			if (HPWidgetRef)
			{
				HealthComp->OnHealthChanged.RemoveAll(HPWidgetRef);
				PostureComp->OnPostureUpdated.RemoveAll(HPWidgetRef);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}
