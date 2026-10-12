#include "Components/2D/DimensionShiftComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "Characters/AI/PaperAICharacter.h"
#include "Jeonwoochi/JeonwoochiPaperCharacter.h"
#include "Jeonwoochi/JeonwoochiCharacter.h"
#include "Characters/BaseCharacter.h"
#include "Characters/AI/Boss/TutorialBossCharacter.h"

#include "AI/AIController/AIController2D.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/Controller.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"

#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"


UDimensionShiftComponent::UDimensionShiftComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDimensionShiftComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	const FTransform PaintingTransform = Owner->GetActorTransform();

	RemainEnemies = 0;
	bEnemiesConditionCompleted = false;
	bLocationConditionCompleted = false;
	bReturnTransitionStarted = false;

	const bool bNeedEnemies = CombackCondition != EComebackCondition::ReachLocation;
	const bool bNeedLocation = CombackCondition != EComebackCondition::KillEnemies;

	for (APawn* P : ActiveTargets)
	{
		if (!IsValid(P)) continue;

		if (auto* PAC = Cast<APaperAICharacter>(P))
		{
			if (bNeedEnemies)
			{
				PAC->HealthComponent->OnDeath.AddUObject(
					this, &UDimensionShiftComponent::ReduceRemainEnemies);
				++RemainEnemies;
			}

			FVector Local = PaintingTransform.InverseTransformPosition(PAC->GetActorLocation());

			// 그림 평면 기준 깊이 오프셋 적용
			Local.Y = LocalYOffset;

			// 보정된 로컬 좌표를 월드 좌표로 변환
			FTransform FinalTransform = FTransform(
				PaintingTransform.GetRotation(),
				PaintingTransform.TransformPosition(Local),
				PAC->GetActorScale());

			PAC->SetActorTransform(FinalTransform);
		}
	}

	// All 모드에서는 적 처치와 목표 지점 도달을 모두 완료해야 한다.
	RemainConditions = static_cast<int32>(bNeedEnemies) + static_cast<int32>(bNeedLocation);

	if (!ReachLocationBox)
	{
		ReachLocationBox = NewObject<UBoxComponent>(Owner, TEXT("ReachLocationBox"));
		ReachLocationBox->SetupAttachment(Owner->GetRootComponent());
		ReachLocationBox->SetMobility(EComponentMobility::Movable);
		ReachLocationBox->RegisterComponent();

		ReachLocationBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		ReachLocationBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		ReachLocationBox->SetGenerateOverlapEvents(false);
		ReachLocationBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (bNeedLocation)
	{
		ReachLocationBox->OnComponentBeginOverlap.AddUniqueDynamic(
			this, &UDimensionShiftComponent::OnBoxBeginOverlap);
	}

	// 2D 캐릭터가 Possess된 이후부터 도달 조건을 판정한다.
	SetReachLocationCollisionEnabled(false);

	for (F2DTransitionCharacters TransitionCharacter : TransitionCharacters)
	{
		if (ABaseCharacter* Char = TransitionCharacter.Character)
		{
			if (ATutorialBossCharacter* TC = Cast<ATutorialBossCharacter>(Char))
			{
				TC->SetCachedDimensionComp(this);
			}
		}
	}
}

void UDimensionShiftComponent::ReduceRemainEnemies(AActor* Char, ECharacterType CharType)
{
	if (Char) Char->Destroy();

	if (bEnemiesConditionCompleted || RemainEnemies <= 0) return;

	--RemainEnemies;

	if (RemainEnemies == 0)
	{
		bEnemiesConditionCompleted = true;
		RemainConditions = FMath::Max(0, RemainConditions - 1);
		Start3DTransition();
	}
}

void UDimensionShiftComponent::Start2DTransition()
{
	if (!GetWorld() || !Player2DClass) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Owner = GetOwner();
	if (!Owner || !CachedChar) return;

	const FTransform PaintingTransform = Owner->GetActorTransform();

	// 플레이어 위치를 그림 액터의 로컬 좌표로 변환
	FVector Local = 
		PaintingTransform.InverseTransformPosition(CachedChar->GetActorLocation());

	// 그림 평면 기준 깊이 오프셋 적용
	Local.Y = LocalYOffset;
	Local.Z += 40.f;

	// 보정된 로컬 좌표를 월드 좌표로 변환
	FVector SpawnLoc = 
		PaintingTransform.TransformPosition(Local);

	// 그림 액터의 방향을 기준으로 2D 캐릭터 정렬
	FRotator SpawnRot = Owner->GetActorRotation();

	Player2DCharacter = GetWorld()->SpawnActor<AJeonwoochiPaperCharacter>(
		Player2DClass, SpawnLoc, SpawnRot, Params);

	if (!Player2DCharacter) return;

	PlayVFX(true);
	ActiveAIs();

	CachedChar->SetActorHiddenInGame(true);
	CachedChar->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		float BlendTime = 0.5f;

		PC->SetViewTargetWithBlend(Player2DCharacter, BlendTime);

		FTimerHandle TH;
		GetWorld()->GetTimerManager().SetTimer(
			TH,
			FTimerDelegate::CreateWeakLambda(this, [this, PC]()
				{
					if (!IsValid(PC) || !IsValid(Player2DCharacter)) return;

					PC->Possess(Player2DCharacter);
					Player2DCharacter->ApplyIMC();
					SetCharactersTransform(true);

					// 처치할 대상이 없으면 적 처치 조건은 이미 충족된 상태다.
					if (CombackCondition != EComebackCondition::ReachLocation
						&& RemainEnemies == 0 && !bEnemiesConditionCompleted)
					{
						bEnemiesConditionCompleted = true;
						RemainConditions = FMath::Max(0, RemainConditions - 1);
					}

					if (CombackCondition != EComebackCondition::KillEnemies)
					{
						SetReachLocationCollisionEnabled(true);
					}

					// 2D Possess 전에 조건이 충족된 경우도 복귀를 판정한다.
					Start3DTransition();
				}),
			BlendTime,
			false
		);
	}
}

void UDimensionShiftComponent::Start3DTransition()
{
	if (RemainConditions != 0 || bReturnTransitionStarted) return;
	if (!GetWorld() || !IsValid(CachedChar) || !IsValid(Player2DCharacter)
		|| !Player2DCharacter->IsPlayerControlled()) return;

	bReturnTransitionStarted = true;
	SetReachLocationCollisionEnabled(false);

	SetCharactersTransform(false);
	PlayVFX(false);

	CachedChar->SetActorHiddenInGame(false);
	CachedChar->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->Possess(CachedChar);
		PC->SetViewTarget(CachedChar);
	}

	if (Player2DCharacter)
	{
		Player2DCharacter->Destroy();
	}

	// 전환 종료 시 남아 있는 2D AI 정리
	for (APawn* P : ActiveTargets)
	{
		if (!IsValid(P)) continue;

		P->Destroy();
	}

	OnCompleteComebackCondition.Broadcast(CombackCondition);

	BreakOwnerAndDestroy();
}

void UDimensionShiftComponent::SetCharactersTransform(bool bIsBeforeTransform)
{
	for (F2DTransitionCharacters TransitionCharacter : TransitionCharacters)
	{
		if (ABaseCharacter* Char = TransitionCharacter.Character)
		{
			if (bIsBeforeTransform)
				Char->SetActorTransform(TransitionCharacter.BeforeTransform);
			else
				Char->SetActorTransform(TransitionCharacter.AfterTransform);
		}
	}
}

void UDimensionShiftComponent::OnBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	APawn* OverlapPawn = Cast<APawn>(OtherActor);
	if (bLocationConditionCompleted || OverlapPawn != Player2DCharacter
		|| !OverlapPawn || !OverlapPawn->IsPlayerControlled()) return;

	// All 모드에서도 위치 조건은 정확히 한 번만 완료한다.
	bLocationConditionCompleted = true;
	SetReachLocationCollisionEnabled(false);
	ReachLocationBox->OnComponentBeginOverlap.RemoveDynamic(
		this, &UDimensionShiftComponent::OnBoxBeginOverlap);

	RemainConditions = FMath::Max(0, RemainConditions - 1);
	Start3DTransition();
}

void UDimensionShiftComponent::ActiveAIs()
{
	for (APawn* P : ActiveTargets)
	{
		if (!IsValid(P)) continue;
		if (!Cast<APaperAICharacter>(P)) continue;

		if (AController* Existing = P->GetController())
		{
			if (AAIController2D* AIC = Cast<AAIController2D>(Existing))
			{
				// 재빙의하여 OnPossess 기반 초기화 재실행
				AIC->UnPossess();
				AIC->Possess(P);

				AIC->SetTargetActor(Player2DCharacter);

				if (UBrainComponent* Brain = AIC->GetBrainComponent())
				{
					Brain->RestartLogic();
				}
			}
			else
			{
				P->DetachFromControllerPendingDestroy();
				P->SpawnDefaultController();
			}

			continue;
		}

		P->SpawnDefaultController();
	}
}

void UDimensionShiftComponent::PlayVFX(bool bTransition2DSound)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (bPlayIn3DChar)
	{
		if (!CachedChar) return;

		if (NiagaraSystem)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World, NiagaraSystem,
				CachedChar->GetActorLocation() + VFXAddLocation,
				CachedChar->GetActorRotation(), VFXScale);

		if (ParticleSystem)
			UGameplayStatics::SpawnEmitterAtLocation(
				World, ParticleSystem,
				CachedChar->GetActorLocation() + VFXAddLocation,
				CachedChar->GetActorRotation(), VFXScale);
	}
	else
	{
		if (!Player2DCharacter) return;

		if (NiagaraSystem)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World, NiagaraSystem,
				Player2DCharacter->GetActorLocation() + VFXAddLocation,
				Player2DCharacter->GetActorRotation(), VFXScale);

		if (ParticleSystem)
			UGameplayStatics::SpawnEmitterAtLocation(
				World, ParticleSystem,
				Player2DCharacter->GetActorLocation() + VFXAddLocation,
				Player2DCharacter->GetActorRotation(), VFXScale);
	}

	if (bTransition2DSound)
	{
		if (Transition2DSound)
			UGameplayStatics::PlaySound2D(World, Transition2DSound);
	}
	else
	{
		if (Transition3DSound)
			UGameplayStatics::PlaySound2D(World, Transition3DSound);
	}
}

void UDimensionShiftComponent::SetCachedChar(AActor* Char)
{
	CachedChar = Cast<AJeonwoochiCharacter>(Char);
}

void UDimensionShiftComponent::SetReachLocationCollisionEnabled(bool bEnable)
{
	if (!ReachLocationBox) return;

	if (bEnable)
	{
		ReachLocationBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ReachLocationBox->SetGenerateOverlapEvents(true);
	}
	else
	{
		ReachLocationBox->SetGenerateOverlapEvents(false);
		ReachLocationBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UDimensionShiftComponent::BreakOwnerAndDestroy()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	SpawnFractureActorAndKickSim(Owner);

	// 파괴 연출 동안 원본 Actor 비활성화
	Owner->SetActorHiddenInGame(true);
	Owner->SetActorEnableCollision(false);

	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
	{
		RootPrim->SetGenerateOverlapEvents(false);
		RootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RootPrim->SetSimulatePhysics(false);
	}

	FTimerHandle H;
	GetWorld()->GetTimerManager().SetTimer(
		H,
		FTimerDelegate::CreateWeakLambda(this, [Owner]()
			{
				if (IsValid(Owner))
				{
					Owner->Destroy();
				}
			}),
		DestroyDelay,
		false
	);
}

void UDimensionShiftComponent::SpawnFractureActorAndKickSim(AActor* Owner)
{
	if (!Owner || !GetWorld()) return;

	UGeometryCollection* Asset =
		FractureAsset.IsValid() ? FractureAsset.Get() : FractureAsset.LoadSynchronous();

	if (!Asset) return;

	const FTransform Xf = Owner->GetActorTransform();

	// 파괴 조각에 원본 Actor의 속도 전달
	FVector InheritLinVel = FVector::ZeroVector;

	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
	{
		InheritLinVel = RootPrim->GetComponentVelocity();
	}

	FActorSpawnParameters SP;
	SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGeometryCollectionActor* GCA = GetWorld()->SpawnActor<AGeometryCollectionActor>(
		Xf.GetLocation(), Xf.Rotator(), SP);

	if (!GCA) return;

	GCA->SetLifeSpan(DestroyDelay);

	UGeometryCollectionComponent* GCC = GCA->GetGeometryCollectionComponent();
	if (!GCC) return;

	GCC->SetRestCollection(Asset);
	GCC->SetMobility(EComponentMobility::Movable);
	GCC->SetSimulatePhysics(true);
	GCC->SetEnableGravity(true);
	GCC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	GCC->SetPhysicsLinearVelocity(InheritLinVel);

	GCC->AddRadialImpulse(
		Xf.GetLocation(),
		RadialImpulseRadius,
		RadialImpulseStrength,
		ERadialImpulseFalloff::RIF_Linear,
		true
	);
}

void UDimensionShiftComponent::EndPlay(EEndPlayReason::Type EndplayReason)
{
	if (ReachLocationBox)
	{
		ReachLocationBox->OnComponentBeginOverlap.RemoveDynamic(
			this, &UDimensionShiftComponent::OnBoxBeginOverlap);

		SetReachLocationCollisionEnabled(false);
	}

	Super::EndPlay(EndplayReason);
}