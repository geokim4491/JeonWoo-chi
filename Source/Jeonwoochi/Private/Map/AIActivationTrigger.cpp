// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/AIActivationTrigger.h"
#include "Components/BoxComponent.h"
#include "AI/AIController/BaseAIController.h"
#include "GameFramework/Pawn.h"

AAIActivationTrigger::AAIActivationTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);

	// 트리거 충돌 기본값
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldDynamic);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);

	Box->OnComponentBeginOverlap.AddDynamic(this, &AAIActivationTrigger::OnBoxBeginOverlap);
}

void AAIActivationTrigger::OnBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bDone && bFireOnce) return;

	APawn* OverlapPawn = Cast<APawn>(OtherActor);
	if (!OverlapPawn) return;

	if (bPlayerOnly && !OverlapPawn->IsPlayerControlled())
	{
		return;
	}

	ActivateAIs();

	if (bFireOnce)
	{
		bDone = true;

		if (bDelete)
		{
			Destroy();   // 자기 자신 제거
		}
		else
		{
			Box->SetGenerateOverlapEvents(false); // 재발동만 차단
		}
	}
}

void AAIActivationTrigger::ActivateAIs()
{
	for (APawn* P : Targets)
	{
		if (!IsValid(P)) continue;

		// 이미 컨트롤러가 있으면 옵션에 따라 처리
		if (AController* Existing = P->GetController())
		{
			if (!bForceRepossess) continue;

			// 강제 재빙의(=OnPossess 재호출)
			if (ABaseAIController* AIC = Cast<ABaseAIController>(Existing))
			{
				AIC->UnPossess();
				AIC->Possess(P);         // 여기서 OnPossess 자동 호출
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
