// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/DimensionShiftActor.h"

#include "Components/2D/DimensionShiftComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ADimensionShiftActor::ADimensionShiftActor()
{
 	PrimaryActorTick.bCanEverTick = false;

	DimensionShiftComp = CreateDefaultSubobject<UDimensionShiftComponent>(TEXT("DimensionShiftComp"));

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	Box->SetupAttachment(RootComponent);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADimensionShiftActor::BeginPlay()
{
	Super::BeginPlay();

	Box->OnComponentBeginOverlap.AddDynamic(this, &ADimensionShiftActor::OnBoxBeginOverlap);

}

void ADimensionShiftActor::OnBoxBeginOverlap(
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

	DimensionShiftComp->SetCachedChar(OtherActor);
	
	DimensionShiftComp->Start2DTransition();

	// 한번만 발동하게 설정.
	Box->OnComponentBeginOverlap.RemoveAll(this);
	Box->SetGenerateOverlapEvents(false);
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADimensionShiftActor::EndPlay(EEndPlayReason::Type EndplayReason)
{
	if (Box)
	{
		Box->OnComponentBeginOverlap.RemoveDynamic(
			this, &ADimensionShiftActor::OnBoxBeginOverlap);
	}

	Super::EndPlay(EndplayReason);
}