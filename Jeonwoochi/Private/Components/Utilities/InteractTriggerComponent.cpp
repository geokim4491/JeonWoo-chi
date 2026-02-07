// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Utilities/InteractTriggerComponent.h"
#include "GameFramework/Actor.h"
#include "Components/Utilities/PlayerInteractComponent.h"


UInteractTriggerComponent::UInteractTriggerComponent()
{
	// 충돌/오버랩 기본값
	InitBoxExtent(BoxExtent);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	// 감지하고 싶은 채널만 Overlap으로
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 트리거 역할
	SetGenerateOverlapEvents(true);
}

void UInteractTriggerComponent::OnRegister()
{
	Super::OnRegister();

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 중복 바인딩 방지
	OnComponentBeginOverlap.RemoveAll(this);
	OnComponentEndOverlap.RemoveAll(this);

	OnComponentBeginOverlap.AddDynamic(this, &UInteractTriggerComponent::HandleBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UInteractTriggerComponent::HandleEndOverlap);
}

void UInteractTriggerComponent::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	if (auto* IC = OtherActor->FindComponentByClass<UPlayerInteractComponent>())
	{
		IC->SetInteractTarget(GetOwner());
	}
}

void UInteractTriggerComponent::HandleEndOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	if (auto* IC = OtherActor->FindComponentByClass<UPlayerInteractComponent>())
	{
		IC->SetInteractTarget(nullptr);
	}
}