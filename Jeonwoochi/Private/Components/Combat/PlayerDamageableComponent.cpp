// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PlayerDamageableComponent.h"
#include "Components/Combat/PlayerDefensiveComponent.h"
#include "Components/Combat/PostureComponent.h"
#include "Components/Combat/HealthComponent.h"
#include "Components/Combat/ChiComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Interface/Combat/HealthInterface.h"

#include "Manager/EffectManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Miscellaneous/Effect/AfterImage.h"

#include "Characters/BaseCharacter.h"


// Sets default values for this component's properties
UPlayerDamageableComponent::UPlayerDamageableComponent() : ParryIdx(0), BlockIdx(0), HitIdx(0)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	DefenseState = EPlayerDefenseState::None;

}


// Called when the game starts
void UPlayerDamageableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UPlayerDefensiveComponent* DefComp =
		GetOwner()->FindComponentByClass<UPlayerDefensiveComponent>())
	{
		OnGetDefenseState.BindUObject(
			DefComp,
			&UPlayerDefensiveComponent::GetPlayerDefenseState
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Defensive Component not found!"))
	}

	CachedHealthComp = GetOwner()->FindComponentByClass<UHealthComponent>();
	
	for (const FReactionMontages& Elem : HitMontages)
	{
		RuntimeHitMontageMap.Add(Elem.ReactionType, Elem.Montages);
	}
}


// Called every frame
void UPlayerDamageableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerDamageableComponent::HandleTakeDamage(const FDamageInfoStruct& Info)
{

	if (OnGetDefenseState.IsBound())
	{
		DefenseState = OnGetDefenseState.Execute();
	}

	if (DefenseState == EPlayerDefenseState::Invincible) return;

	EPlayerDefenseState ResultState;

	switch (Info.AttackCategory)
	{
	case EAttackCategory::NormalAttack:
		switch (DefenseState)
		{
		case EPlayerDefenseState::Parrying:
			// 패링 관련 이벤트 추가
			PlayEffect(EEffectType::Parry);
			ResultState = EPlayerDefenseState::Parrying;

			if (!Info.Attacker)
			{
				UE_LOG(LogTemp, Warning, TEXT("Cant Find Attacker"));
				break;
			}
			if (auto* Posture = Info.Attacker->FindComponentByClass<UPostureComponent>())
				Posture->ReducePostureCount(1);

			break;

		case EPlayerDefenseState::Blocking:
			// 방어 관련 이벤트 추가
			PlayEffect(EEffectType::Block);
			ResultState = EPlayerDefenseState::Blocking;

			break;

		case EPlayerDefenseState::PerfectDodging:
			// perfect dodging 관련 이벤트 추가
			PlayEffect(EEffectType::PerfectDodge);
			ResultState = EPlayerDefenseState::PerfectDodging;
			PerfectDodgeEffect();

			break;

		case EPlayerDefenseState::Dodging:
			// doding 관련 이벤트 추가 
			PlayEffect(EEffectType::Dodge);
			ResultState = EPlayerDefenseState::Dodging;

			break;

		case EPlayerDefenseState::None:
			// 피해 입음 이벤트 추가
			PlayEffect(EEffectType::PlayerHit);
			ResultState = EPlayerDefenseState::None;

			break;
		}
		break;

	case EAttackCategory::DangerousAttack:
		switch (DefenseState)
		{
		case EPlayerDefenseState::PerfectDodging:
			// perfect dodging 관련 이벤트 추가
			PlayEffect(EEffectType::PerfectDodge);
			ResultState = EPlayerDefenseState::PerfectDodging;
			PerfectDodgeEffect();
			
			break;

		case EPlayerDefenseState::Dodging:
			// doding 관련 이벤트 추가 
			PlayEffect(EEffectType::Dodge);
			ResultState = EPlayerDefenseState::Dodging;

			break;

		default:
			PlayEffect(EEffectType::PlayerHit);
			ResultState = EPlayerDefenseState::None;

		}
		break;

	case EAttackCategory::SpecialAttack:
		switch (DefenseState)
		{
		case EPlayerDefenseState::Blink:
			// blink 효과

			ResultState = EPlayerDefenseState::Blink;

			break;
		default:
			PlayEffect(EEffectType::PlayerHit);
			ResultState = EPlayerDefenseState::None;

			break;
		}
		break;
	}

	HandleHit(Info, ResultState);
	IncreaseChi(ResultState);
	HandleReaction(Info, ResultState);
}

void UPlayerDamageableComponent::HandleHit(const FDamageInfoStruct& Info, EPlayerDefenseState ResultState)
{
	AActor* Char = GetOwner();
	if (!Char) return;

	float Damage = 0.f;

	switch (ResultState)
	{
	case EPlayerDefenseState::None:
		Damage = Info.DamageAmount;
		break;

	case EPlayerDefenseState::Blocking:
		Damage = Info.DamageAmount * 0.1f;
		break;

	default:
		return;
	}
	

	// 추가로 방어력 계산해서 최종 데미지 계산 진행

	if (Char->Implements<UHealthInterface>())
	{
		IHealthInterface::Execute_ApplyDamage(Char, Damage);
	}
}

void UPlayerDamageableComponent::IncreaseChi(EPlayerDefenseState ResultState)
{
	AActor* Char = GetOwner();
	if (!Char) return;
	
	switch (ResultState)
	{
	case EPlayerDefenseState::Parrying:
		if (auto* CC = Char->FindComponentByClass<UChiComponent>())
		{
			CC->UpdateCurrentChi(CC->GetMaxChi() * 0.05f);
		}
		break;

	case EPlayerDefenseState::PerfectDodging:
		if (auto* CC = Char->FindComponentByClass<UChiComponent>())
		{
			CC->UpdateCurrentChi(CC->GetMaxChi() * 0.03f);
		}
		break;

	default:
		return;
	}
}

void UPlayerDamageableComponent::HandleReaction(
	const FDamageInfoStruct& Info, 
	EPlayerDefenseState ResultState
)
{
	if (ParryMontages.Num() == 0 || BlockMontages.Num() == 0) return;
	
	if (CachedHealthComp)
	{
		if (CachedHealthComp->IsDead()) return;
	}

	if (RuntimeHitMontageMap.Contains(Info.ReactionType))
	{
		if (ABaseCharacter* Char = Cast<ABaseCharacter>(GetOwner()))
		{
			ParryIdx = (ParryIdx + 1) % ParryMontages.Num();
			UAnimMontage* ParryMontage = ParryMontages[ParryIdx];

			BlockIdx = (BlockIdx + 1) % BlockMontages.Num();
			UAnimMontage* BlockMontage = BlockMontages[BlockIdx];

			HitIdx = (HitIdx + 1) % RuntimeHitMontageMap[Info.ReactionType].Num();
			UAnimMontage* HitMontage = RuntimeHitMontageMap[Info.ReactionType][HitIdx];

			switch (ResultState)
			{
			case EPlayerDefenseState::Parrying:
				Char->PlayAnimMontage(ParryMontage);
				break;

			case EPlayerDefenseState::Blocking:
				Char->PlayAnimMontage(BlockMontage);
				break;

			case EPlayerDefenseState::None:
				Char->PlayAnimMontage(HitMontage);
				UE_LOG(LogTemp, Display, TEXT("Hit Montage"));
				break;

			default:
				break;
			}
		}
	}
}

void UPlayerDamageableComponent::PlayEffect(EEffectType EffectType)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
	{
		if (UEffectManager* EM = GI->GetSubsystem<UEffectManager>())
		{
			EM->PlayEffectOnActor(
				EffectType,
				GetOwner()
			);
		}
	}
}

void UPlayerDamageableComponent::PerfectDodgeEffect()
{
	CameraFadeEffect();
	ApplyHighlightMaterial();

	AfterImageSpawnCount = 0;

	ScheduleNextAfterImage(0.02f);
}

//
// Helper Functions /////////////////////////////////////////////////
//

void UPlayerDamageableComponent::CameraFadeEffect()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	AController* Ctrl = OwnerPawn->GetController();
	if (APlayerController* PC = Cast<APlayerController>(Ctrl))
	{
		auto CamMgr = PC->PlayerCameraManager;

		if (!CamMgr) return;

		// 화면을 페이드 인
		CamMgr->StartCameraFade(
			/* FromAlpha */ 0.f,
			/* ToAlpha   */ 0.3f,
			/* Duration  */ 0.05f,
			/* Color     */ FLinearColor::Black,
			/* bFadeAudio*/ false,
			/* bHold     */ true
		);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				FadeTimerHandle,
				[CamMgr]()
				{
					// 페이드 아웃: 0.5→0
					CamMgr->StartCameraFade(0.3f, 0.f, 0.2f, FLinearColor::Black, false, false);
				}, 0.2f, false);
		}

	}
}

void UPlayerDamageableComponent::SpawnAfterImage()
{
	AfterImageSpawnCount++;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		if (USkeletalMeshComponent* MeshComp = OwnerPawn->FindComponentByClass<USkeletalMeshComponent>())
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (AAfterImage* After = GetWorld()->SpawnActor<AAfterImage>(
				AAfterImage::StaticClass(),
				MeshComp->GetComponentLocation(),
				MeshComp->GetComponentRotation(),
				Params))
			{
				After->Initialize(MeshComp, 0.5f, PerfectDodgeHighlightMaterial);
			}
		}
	}

	// 여러 AfterImage 만들고 싶으면 주석 해제.
	/*
	if (AfterImageSpawnCount < 4)
	{
		ScheduleNextAfterImage(0.075f);
	}
	*/
}

void UPlayerDamageableComponent::ScheduleNextAfterImage(float Delay)
{
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&UPlayerDamageableComponent::SpawnAfterImage,
		Delay,
		false  // one-shot
	);
}

void UPlayerDamageableComponent::ApplyHighlightMaterial()
{
	ABaseCharacter* OwnerChar = Cast<ABaseCharacter>(GetOwner());
	if (!OwnerChar) return;

	Mesh = OwnerChar->GetMesh();
	if (!Mesh) return;

	Mesh->SetOverlayMaterial(PerfectDodgeHighlightMaterial);

	GetWorld()->GetTimerManager().SetTimer(
		MaterialTimerHandle,
		FTimerDelegate::CreateLambda([this]
			{
				Mesh->SetOverlayMaterial(nullptr);
			}),
		0.38f,
		false
	);
}

// 헬퍼: 메타휴먼에서 가장 "큰" 스켈메시(보통 바디)를 골라줌
static USkeletalMeshComponent* PickBestSkeletalMesh(AActor* Owner)
{
	if (!Owner) return nullptr;

	// 1순위: 캐릭터라면 무조건 Body(GetMesh)
	if (ACharacter* C = Cast<ACharacter>(Owner))
	{
		if (USkeletalMeshComponent* Body = C->GetMesh())
			if (Body->GetSkeletalMeshAsset())
				return Body;
	}

	// 2순위: 컴포넌트들 중 Bounds 큰 놈(보통 바디)
	TInlineComponentArray<USkeletalMeshComponent*> Skels(Owner);
	USkeletalMeshComponent* Best = nullptr;
	float BestScore = -1.f;
	for (USkeletalMeshComponent* S : Skels)
	{
		if (!S || !S->GetSkeletalMeshAsset()) continue;
		const float Score = S->Bounds.SphereRadius;
		if (Score > BestScore) { BestScore = Score; Best = S; }
	}
	return Best;
}

void UPlayerDamageableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	OnGetDefenseState.Unbind();

	Super::EndPlay(EndPlayReason);
}
