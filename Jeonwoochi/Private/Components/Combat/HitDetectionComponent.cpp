// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/HitDetectionComponent.h"
#include "Characters/BaseCharacter.h"

// Sets default values for this component's properties
UHitDetectionComponent::UHitDetectionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	HitShape = CreateDefaultSubobject<UBoxComponent>(TEXT("HitShapeComp"));
	// ...
}


// Called when the game starts
void UHitDetectionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HitShape)
	{
		/* 1 반드시 RegisterComponent() */
		if (!HitShape->IsRegistered())
		{
			HitShape->RegisterComponent();
		}

		/* 2 Collision 프로필 / 응답 설정 */
		HitShape->SetCollisionObjectType(ECC_WorldDynamic);
		HitShape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitShape->SetGenerateOverlapEvents(true);

		// 전체에 대해서는 Ignore, Pawn 채널만 Overlap 같은 식
		HitShape->SetCollisionResponseToAllChannels(ECR_Ignore);
		HitShape->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		HitShape->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

		/* 3 HitDetectionComponent → BoxComponent 부착 (이미 돼 있다면 생략 가능) */
		HitShape->AttachToComponent(
			this,
			FAttachmentTransformRules::KeepRelativeTransform);

		/* 4 Overlap 델리게이트 바인딩 */
		HitShape->OnComponentBeginOverlap.AddDynamic(
			this, &UHitDetectionComponent::HandleOverlap);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s : HitShape is NULL!"), *GetOwner()->GetName());
	}

	// 1) 이 컴포넌트를 소유한 액터(WeaponActor)
	AActor* WeaponActor = GetOwner();
	if (!WeaponActor) return;

	// 2) 그 액터가 붙어 있는 부모(플레이어 캐릭터) 얻기, 투사체라면 투사체를 발사한 액터
	//    ChildActorComponent 로 붙인 경우 AttachParentActor 가, 
	//    직접 Spawn+SetOwner 했다면 GetOwner() 가 유효합니다.
	CachedOwnerActor = WeaponActor->GetAttachParentActor();
	if (!CachedOwnerActor)
	{
		CachedOwnerActor = WeaponActor->GetOwner();
	}
	if (!CachedOwnerActor) return;

	// 3) 캐릭터의 AttackComponent 를 찾아서 저장
	CachedAttackComp = CachedOwnerActor->FindComponentByClass<UAttackComponent>();

	CachedOwnerChar = Cast<ABaseCharacter>(CachedOwnerActor);
}



// Called every frame
void UHitDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHitDetectionComponent::EnableDetection()
{
	// 스윙마다 중복 히트 방지 리스트 초기화
	AlreadyHitActors.Empty();

	if (HitShape)
	{
		HitShape->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void UHitDetectionComponent::DisableDetection()
{
	if (HitShape)
	{
		HitShape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UHitDetectionComponent::HandleOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 자기 자신이거나 이미 히트한 액터면 무시
	if (!OtherActor || OtherActor == CachedOwnerActor || AlreadyHitActors.Contains(OtherActor))
		return;
	// 같은 팀끼리는 공격 적용 x
	if (CachedOwnerChar)
	{
		if (ABaseCharacter* OtherChar = Cast<ABaseCharacter>(OtherActor))
		{
			if (CachedOwnerChar->GetGenericTeamId() == OtherChar->GetGenericTeamId())
			{
				return;
			}
		}
	}

	// 중복 방지 저장
	AlreadyHitActors.Add(OtherActor);

	if (CachedAttackComp)
	{
		CachedAttackComp->HandleHit(OtherActor, SweepResult);
	}
}

void UHitDetectionComponent::StartHitWindow()
{
	// 이전 바인딩 초기화 및 탐지 활성화
	EnableDetection();

}

void UHitDetectionComponent::EndHitWindow()
{
	DisableDetection();
}

void UHitDetectionComponent::RefreshOwnerPointers()
{
	AActor* WeaponActor = GetOwner();          // BP_Stick
	AActor* Character = nullptr;

	/* 1) SetOwner() 로 연결한 경우                        */
	if (WeaponActor)
		Character = WeaponActor->GetOwner();   // BP_Clone

	/* 2) ChildActorComponent 로 붙인 경우 (AttachParent)  */
	if (!Character && WeaponActor)
		Character = WeaponActor->GetAttachParentActor();

	if (!Character) return;                    // 실패 시 그대로

	CachedOwnerActor = Character;
	CachedAttackComp = Character->FindComponentByClass<UAttackComponent>();
	CachedOwnerChar = Cast<ABaseCharacter>(Character);
}

void UHitDetectionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HitShape)
	{
		HitShape->OnComponentBeginOverlap.RemoveAll(this);
	}

	DisableDetection();

	Super::EndPlay(EndPlayReason);
}

