// Fill out your copyright notice in the Description page of Project Settings.


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


// Sets default values for this component's properties
UDimensionShiftComponent::UDimensionShiftComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UDimensionShiftComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	const FTransform OwnerXf = Owner->GetActorTransform();

	for (APawn* P : ActiveTargets)
	{
		if (!IsValid(P)) continue;

		if (auto* PAC = Cast<APaperAICharacter>(P))
		{
			// PAC의 Death 이벤트 바인딩 -> ReduceRemainEnemies
			PAC->HealthComponent->OnDeath.AddUObject(this, &UDimensionShiftComponent::ReduceRemainEnemies);

			FVector Local = OwnerXf.InverseTransformPosition(PAC->GetActorLocation());

			// 0으로 하면, 그림에 딱 붙어있는 모양이 안되서 LocalOffset으로 조정.
			Local.Y = LocalYOffset;
			
			// 다시 월드 좌표로 변환 → 스폰 위치 완성
			FTransform FinalTransform = FTransform(OwnerXf.GetRotation(), OwnerXf.TransformPosition(Local), PAC->GetActorScale());
			PAC->SetActorTransform(FinalTransform);

			
			RemainEnemies++;
		}
	}

	switch (CombackCondition)
	{
	case EComebackCondition::All:
		RemainConditions = 2;
		break;
	default:
		RemainConditions = 1;
		break;
	}


	// 1) 박스 없으면 생성(런타임)
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

	if (CombackCondition == EComebackCondition::ReachLocation)
	{
		ReachLocationBox->OnComponentBeginOverlap.AddUniqueDynamic(
			this, &UDimensionShiftComponent::OnBoxBeginOverlap);

		SetReachLocationCollisionEnabled(true);
	}
	else
	{
		SetReachLocationCollisionEnabled(false);
	}

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
	RemainEnemies = FMath::Max<int32>(0, RemainEnemies - 1);

	if (Char) Char->Destroy();

	if (RemainEnemies == 0)
	{
		RemainConditions--;
		Start3DTransition();
	}
}

void UDimensionShiftComponent::Start2DTransition()
{
	if (!GetWorld() || !Player2DClass) return;

	// 2D 캐릭터 스폰
	if (Player2DClass)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FRotator SpawnRot = GetOwner()->GetActorRotation();

		AActor* Owner = GetOwner();
		const FTransform OwnerXf = Owner->GetActorTransform();

		// 1) 플레이어 위치를 소유 액터의 '로컬' 좌표로 변환
		FVector Local = OwnerXf.InverseTransformPosition(CachedChar->GetActorLocation());

		// 2) 0으로 하면, 그림에 딱 붙어있는 모양이 됨.
		Local.Y = 0.f;
		Local.Z += 40.f;

		// 3) 다시 월드 좌표로 변환 → 스폰 위치 완성
		FVector SpawnLoc = OwnerXf.TransformPosition(Local);

		Player2DCharacter = GetWorld()->
			SpawnActor<AJeonwoochiPaperCharacter>(Player2DClass, SpawnLoc, SpawnRot, Params);

		if (!Player2DCharacter) return;
	}
	
	PlayVFX(true);
	ActiveAIs();

	CachedChar->SetActorHiddenInGame(true);
	// CachedChar->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// CachedChar->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CachedChar->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// 2D 캐릭터로 변경
	if (APlayerController* PC =
		UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		float BlendTime = 0.5f;

		PC->SetViewTargetWithBlend(Player2DCharacter, /*BlendTime=*/BlendTime);
		FTimerHandle TH;
		GetWorld()->GetTimerManager().SetTimer(
			TH,
			FTimerDelegate::CreateWeakLambda(this, [this, PC]()
				{
					PC->Possess(Player2DCharacter);
					Player2DCharacter->ApplyIMC();
					SetCharactersTransform(true);
				}),
			BlendTime,
			false
		);
	}
}

void UDimensionShiftComponent::Start3DTransition()
{
	if (RemainConditions != 0) return;

	if (!GetWorld() || !CachedChar) return;

	SetCharactersTransform(false);

	PlayVFX(false);

	CachedChar->SetActorHiddenInGame(false);
	// CachedChar->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// CachedChar->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CachedChar->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	if (APlayerController* PC =
		UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->SetViewTargetWithBlend(CachedChar, /*BlendTime=*/1.f);
		PC->Possess(CachedChar);
	}

	if (Player2DCharacter)
	{
		Player2DCharacter->Destroy();
	}

	// Destroy 안한 적 있으면, Destroy
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
	if (!OverlapPawn) return;

	if (!OverlapPawn->IsPlayerControlled()) return;

	RemainConditions--;
	Start3DTransition();

	// 한번만 발동하게 설정.
	SetReachLocationCollisionEnabled(false);

	// 재진입 방지.
	ReachLocationBox->OnComponentBeginOverlap.RemoveDynamic(
		this, &UDimensionShiftComponent::OnBoxBeginOverlap);
	
}

void UDimensionShiftComponent::ActiveAIs()
{
	for (APawn* P : ActiveTargets)
	{
		if (!IsValid(P)) continue;

		if (!Cast<APaperAICharacter>(P)) continue;

		// 이미 컨트롤러가 있으면 옵션에 따라 처리
		if (AController* Existing = P->GetController())
		{

			// 강제 재빙의(=OnPossess 재호출)
			if (AAIController2D* AIC = Cast<AAIController2D>(Existing))
			{
				AIC->UnPossess();
				AIC->Possess(P);         // 여기서 OnPossess 자동 호출

				AIC->SetTargetActor(Player2DCharacter);

				if (UBrainComponent* Brain = AIC->GetBrainComponent())
				{
					Brain->RestartLogic();
				}
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
				World, NiagaraSystem, CachedChar->GetActorLocation() + VFXAddLocation, CachedChar->GetActorRotation(), VFXScale
			);

		if (ParticleSystem)
			UGameplayStatics::SpawnEmitterAtLocation(
				World, ParticleSystem, CachedChar->GetActorLocation() + VFXAddLocation, CachedChar->GetActorRotation(), VFXScale
			);
	}
	else
	{
		if (!Player2DCharacter) return;

		if (NiagaraSystem)
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World, NiagaraSystem, Player2DCharacter->GetActorLocation() + VFXAddLocation, Player2DCharacter->GetActorRotation(), VFXScale
			);

		if (ParticleSystem)
			UGameplayStatics::SpawnEmitterAtLocation(
				World, ParticleSystem, Player2DCharacter->GetActorLocation() + VFXAddLocation, Player2DCharacter->GetActorRotation(), VFXScale
			);
	}

	if (bTransition2DSound)
	{
		if (Transition2DSound) UGameplayStatics::PlaySound2D(World, Transition2DSound);
	}
	else
	{
		if (Transition3DSound) UGameplayStatics::PlaySound2D(World, Transition3DSound);
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

	// 1) 부서지는 액터 스폰 + 물리 시작
	SpawnFractureActorAndKickSim(Owner);

	// 2) 원본은 보이지 않게/충돌 끄기 (바로 Destroy하지 말 것!)
	Owner->SetActorHiddenInGame(true);
	Owner->SetActorEnableCollision(false);
	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
	{
		RootPrim->SetGenerateOverlapEvents(false);
		RootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RootPrim->SetSimulatePhysics(false);
	}

	// 3) 연출이 어느 정도 진행된 뒤 원본 삭제
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
		/*bLoop=*/false
	);
}

void UDimensionShiftComponent::SpawnFractureActorAndKickSim(AActor* Owner)
{
	if (!Owner || !GetWorld()) return;

	// 에셋 동기 로드(필요 시)
	UGeometryCollection* Asset = FractureAsset.IsValid() ? FractureAsset.Get() : FractureAsset.LoadSynchronous();
	if (!Asset) return;

	const FTransform Xf = Owner->GetActorTransform();

	// 원본의 현재 속도를 이어받아 자연스럽게 날리기 위함(선택)
	FVector InheritLinVel = FVector::ZeroVector;
	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
	{
		InheritLinVel = RootPrim->GetComponentVelocity();
	}

	// 1) GeometryCollection 액터 스폰
	FActorSpawnParameters SP;
	SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGeometryCollectionActor* GCA = GetWorld()->SpawnActor<AGeometryCollectionActor>(
		Xf.GetLocation(), Xf.Rotator(), SP);

	if (!GCA) return;

	GCA->SetLifeSpan(DestroyDelay);

	UGeometryCollectionComponent* GCC = GCA->GetGeometryCollectionComponent();
	if (!GCC) return;

	// 2) 에셋 지정 + 물리/충돌 설정
	GCC->SetRestCollection(Asset);
	GCC->SetMobility(EComponentMobility::Movable);
	GCC->SetSimulatePhysics(true);
	GCC->SetEnableGravity(true);

	GCC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);


	// 초기 속도 부여(선택)
	GCC->SetPhysicsLinearVelocity(InheritLinVel);

	// 3) 산산조각 나가게 임펄스(간단 버전): AddRadialImpulse 사용
	GCC->AddRadialImpulse(
		Xf.GetLocation(),
		RadialImpulseRadius,
		RadialImpulseStrength,
		ERadialImpulseFalloff::RIF_Linear,
		/*bVelChange=*/true
	);

	// 참고: 더 정교한 파괴는 Field System으로 "Strain" 필드/브레이킹 필드 적용을 고려
	// (RadialFalloff + CullingField 등)
}

void UDimensionShiftComponent::EndPlay(EEndPlayReason::Type EndplayReason)
{

	if (ReachLocationBox)
	{
		ReachLocationBox->OnComponentBeginOverlap.RemoveDynamic(
			this, &UDimensionShiftComponent::OnBoxBeginOverlap);
		// 원한다면 종료 시 완전히 비활성화
		SetReachLocationCollisionEnabled(false);
	}
	
	Super::EndPlay(EndplayReason);
}