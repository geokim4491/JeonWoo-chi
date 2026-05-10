// Fill out your copyright notice in the Description page of Project Settings.


#include "Miscellaneous/Actor/BonfireActor.h"
#include "Data/Save/SaveSubsystem.h"
#include "Manager/EffectManager.h"
#include "Kismet/GameplayStatics.h"
#include "Jeonwoochi/JeonwoochiCharacter.h"
#include "Components/Utilities/InteractTriggerComponent.h"
#include "Data/Save/SaveAgentComponent.h"
#include "UI/FadeInHoldOutWidget.h"

// Sets default values
ABonfireActor::ABonfireActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	InteractTrrigerComp = 
		CreateDefaultSubobject<UInteractTriggerComponent>(TEXT("InteractTrrigerComp"));
	SaveComp =
		CreateDefaultSubobject<USaveAgentComponent>(TEXT("SaveComp"));

	SaveComp->Buckets = (int32)(ESaveBucketFlags::Progress | ESaveBucketFlags::Rest);

	bIsVisited = false;
}

// Called when the game starts or when spawned
void ABonfireActor::BeginPlay()
{
	Super::BeginPlay();
	BindSaveSignals();
	RefreshVisitedFX(); // 시작 시 한 번 상태 반영
}

void ABonfireActor::Interact_Implementation(AActor* Target)
{
	UWorld* World = GetWorld();
	if (!World || !Target) return;

	bIsVisited = true;

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
	{
		if (USaveSubsystem* SS = GI->GetSubsystem<USaveSubsystem>())
		{
			if (SS->SaveRestAtBonfire())
				UE_LOG(LogTemp, Display, TEXT("Save"));
				
		}
	}

	Char = Cast<AJeonwoochiCharacter>(Target);
	if (!Char) return;

	UAnimInstance* AnimInst = (Char->GetMesh() ? Char->GetMesh()->GetAnimInstance() : nullptr);
	if (AnimInst)
	{
		// 중복 바인딩 방지: 일단 제거 후 추가 (Safe idempotent)
		AnimInst->OnMontageEnded.RemoveDynamic(this, &ABonfireActor::OnBonfireMontageEnded);
		AnimInst->OnMontageEnded.AddDynamic(this, &ABonfireActor::OnBonfireMontageEnded);
	}

	Char->PlayAnimMontage(BonfireMontage);
	Char->ClearAllIMC();

	SetCharacterTransform();

	if (!FindWidgetClass) return;

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (UFadeInHoldOutWidget* Wdg = CreateWidget<UFadeInHoldOutWidget>(PC, FindWidgetClass))
		{
			Wdg->AddToViewport(10000);

			// (선택) 스타일/사운드 주입
			Wdg->SetTitleColor(FLinearColor(1.f, 1.f, 1.f));
			// Wdg->SetTitleFontSize(90);
			// Wdg->AppearSound = AppearSfx;
			// Wdg->DisappearSound = DisappearSfx;

			// 보스별 텍스트를 소스코드에서 지정 (엔터 포함 가능)
			const FText Text = FText::FromString(TEXT("잊혀진 사당 발견"));

			// 시간: 페이드 인 / 홀드 / 페이드 아웃
			Wdg->PlayFelled(Text, /*FadeIn=*/1.6f, /*Hold=*/2.6f, /*FadeOut=*/1.6f);
		}
	}

	PlayEffect(EEffectType::Bonfire);
	OnBonfire.Broadcast(Target);
}

void ABonfireActor::SetCharacterTransform()
{
	const FVector  Center = GetActorLocation();
	const FVector  FrontDir = GetActorForwardVector();

	const float ExtraOffset = 40.f; // 필요에 맞게 조절
	FVector TargetLoc = Center + FrontDir * ExtraOffset;

	// 플레이어가 바라보도록 회전
	FRotator TargetRot = (GetActorLocation() - TargetLoc).Rotation();
	TargetRot.Pitch = 0.f;
	TargetRot.Roll = 0.f;

	Char->SetActorTransform(FTransform(TargetRot, TargetLoc, FVector::OneVector));
}

void ABonfireActor::OnBonfireMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Char)
	{
		Char->ApplyIMC_Default();
	}
	RefreshVisitedFX();
}

void ABonfireActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(Char)
	{
		UAnimInstance* AnimInst = (Char->GetMesh() ? Char->GetMesh()->GetAnimInstance() : nullptr);
		if (AnimInst)
		{
			AnimInst->OnMontageEnded.RemoveDynamic(this, &ABonfireActor::OnBonfireMontageEnded);
		}
	}
	UnbindSaveSignals();
	Super::EndPlay(EndPlayReason);
}

void ABonfireActor::PlayEffect(EEffectType EffectType)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
	{
		if (UEffectManager* EM = GI->GetSubsystem<UEffectManager>())
		{
			EM->PlayEffectOnActor(
				EffectType,
				Char
			);
		}
	}
}

void ABonfireActor::RefreshVisitedFX()
{
	// 둘 중 하나만 쓰고 싶으면 우선순위 정해 선택적으로 활성화하면 됨.
	if (bIsVisited)
	{
		if (Niagara)
		{
			if (auto* NC = GetOrCreateNiagaraComp())
			{
				NC->SetAsset(Niagara);
				NC->Activate(true);
				NC->SetVisibility(true, true);
			}
		}
		if (Emitter)
		{
			if (auto* PC = GetOrCreateCascadeComp())
			{
				PC->SetTemplate(Emitter);
				PC->Activate(true);
				PC->SetVisibility(true, true);
			}
		}
	}
	else
	{
		if (NiagaraComp) { NiagaraComp->Deactivate(); NiagaraComp->SetVisibility(false, true); }
		if (CascadeComp) { CascadeComp->Deactivate(); CascadeComp->SetVisibility(false, true); }
	}
}

UNiagaraComponent* ABonfireActor::GetOrCreateNiagaraComp()
{
	if (!NiagaraComp)
	{
		NiagaraComp = NewObject<UNiagaraComponent>(this, TEXT("VisitedNiagara"));
		NiagaraComp->SetupAttachment(RootComponent);
		NiagaraComp->bAutoActivate = false;
		NiagaraComp->RegisterComponent();
	}
	return NiagaraComp;
}

UParticleSystemComponent* ABonfireActor::GetOrCreateCascadeComp()
{
	if (!CascadeComp)
	{
		CascadeComp = NewObject<UParticleSystemComponent>(this, TEXT("VisitedCascade"));
		CascadeComp->SetupAttachment(RootComponent);
		CascadeComp->bAutoActivate = false;
		CascadeComp->RegisterComponent();
	}
	return CascadeComp;
}

// ===== SaveSubsystem 시그널을 받아 로드 직후에도 FX 동기화 =====
void ABonfireActor::BindSaveSignals()
{
	if (UWorld* World = GetWorld())
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
			if (USaveSubsystem* SS = GI->GetSubsystem<USaveSubsystem>())
				SS->OnWorldAgentsApplied.AddUObject(this, &ABonfireActor::OnWorldAgentsApplied);
}

void ABonfireActor::UnbindSaveSignals()
{
	if (UWorld* World = GetWorld())
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
			if (USaveSubsystem* SS = GI->GetSubsystem<USaveSubsystem>())
				SS->OnWorldAgentsApplied.RemoveAll(this);
}

void ABonfireActor::OnWorldAgentsApplied()
{
	// 세이브 역직렬화가 끝난 직후 bIsVisited 값대로 FX를 맞춘다
	RefreshVisitedFX();
}