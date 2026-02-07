// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/BossActivateTriggerBox.h"
#include "Components/BoxComponent.h"
#include "Components/Boss/BossActivateComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"

// Sets default values
ABossActivateTriggerBox::ABossActivateTriggerBox()
{
 	PrimaryActorTick.bCanEverTick = false;

	BossActivateComp = 
		CreateDefaultSubobject<UBossActivateComponent>(TEXT("BossActivateComp"));

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(Box);

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetupAttachment(Box);

	// 트리거 충돌 기본값
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldDynamic);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);
	
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMesh->SetCollisionObjectType(ECC_WorldStatic);

	Box->OnComponentBeginOverlap.AddDynamic(this, &ABossActivateTriggerBox::OnBoxBeginOverlap);
}

// Called when the game starts or when spawned
void ABossActivateTriggerBox::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABossActivateTriggerBox::OnBoxBeginOverlap(
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

	BossActivateComp->SetCachedChar(OtherActor);

	BossActivateComp->ActivateBoss();

	StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	StaticMesh->SetCollisionResponseToAllChannels(ECR_Block);
	StaticMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	// 한번만 발동하게 설정.
	Box->OnComponentBeginOverlap.RemoveAll(this);
	Box->SetGenerateOverlapEvents(false);
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
