// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"

static void DisableDecalsRecursive(AActor* Actor)
{
	if (!Actor) return;

	// 1) 이 액터의 모든 Primitive 컴포넌트(StaticMesh, SkeletalMesh 등)
	TArray<UPrimitiveComponent*> prims;
	Actor->GetComponents<UPrimitiveComponent>(prims);
	for (UPrimitiveComponent* pc : prims)
	{
		if (pc) pc->SetReceivesDecals(false);
	}

	// 2) 소켓/Attach로 달린 자식 액터들에도 재귀 적용 (무기, 망토 등)
	TArray<AActor*> attached;
	Actor->GetAttachedActors(attached);
	for (AActor* child : attached)
	{
		DisableDecalsRecursive(child);
	}
}

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CharacterType = ECharacterType::None;

	StimuliSourceComp = CreateDefaultSubobject
		<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourceComp"));

	StimuliSourceComp->RegisterForSense(UAISense_Sight::StaticClass());
	StimuliSourceComp->RegisterForSense(UAISense_Hearing::StaticClass());
	
	// StimuliSourceComp->RegisterWithPerceptionSystem();
	// 등록은 AIC에선 Possess이후에서 해야 하기 때문에 이곳에서 하지 않음. 플레이어는 생성자에서 TeamID 확정해서
	// 생성자에서 등록해줌

	MotionWarpingComp = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComp"));

	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	GetMesh()->SetReceivesDecals(false);
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DisableDecalsRecursive(this);
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

ECharacterType ABaseCharacter::GetCharacterType_Implementation()
{
	return CharacterType;
}

void ABaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 1) Perception Source 해제
	if (StimuliSourceComp)
	{
		StimuliSourceComp->UnregisterFromPerceptionSystem();
	}

	// 2) 부모 클래스 로직
	Super::EndPlay(EndPlayReason);
}