// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Utilities/LockOnComponent.h"

#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"    // 디버그용
#include "Interface/Character/CharacterTypeInterface.h"
#include "Enum/CharacterTypeEnum.h"

#include "Characters/AI/BaseAICharacter.h"

#include "../../../JeonwoochiCharacter.h"



// Sets default values for this component's properties
ULockOnComponent::ULockOnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TargetActor = nullptr;
	TraceDistance = 1500.f;
	TraceRadius = 100.f;
	
	RotationInterpSpeed = 5.f;
	MaxLockDistance = 3000.f;

	bLockOnEnabled = false;
	SetComponentTickEnabled(false);
}


// Called when the game starts
void ULockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(false);
	
	if (APawn* P = Cast<APawn>(GetOwner()))
	{
		PC = Cast<APlayerController>(P->GetController());
	}
}


void ULockOnComponent::LockOn(AActor* NewTarget)
{
	if (!NewTarget) return;

	TargetActor = NewTarget;
	bLockOnEnabled = (TargetActor != nullptr);
	SetComponentTickEnabled(true);

	if (BoundHealthComp.IsValid())
	{
		BoundHealthComp->OnDeath.Remove(DeathHandle);
		BoundHealthComp = nullptr;
	}

	if (UHealthComponent* BindHealthComp =
		TargetActor->FindComponentByClass<UHealthComponent>())
	{
		DeathHandle = BindHealthComp->OnDeath.AddUObject(
			this, &ULockOnComponent::OnTargetDeath
		);
		BoundHealthComp = BindHealthComp;
	}

	if (ABaseAICharacter* NewAI = Cast<ABaseAICharacter>(TargetActor))
	{
		NewAI->ShowLockMarker(true);
		NewAI->SetHPBarPinnedByLockOn(true);
	}

	OnLockOnTarget.Broadcast(TargetActor);
}

void ULockOnComponent::ClearLockOn()
{
	if (BoundHealthComp.IsValid())
	{
		BoundHealthComp->OnDeath.Remove(DeathHandle);
		BoundHealthComp = nullptr;
	}

	if (ABaseAICharacter* NewAI = Cast<ABaseAICharacter>(TargetActor))
	{
		NewAI->ShowLockMarker(false);
		NewAI->SetHPBarPinnedByLockOn(false, 1.f);
	}

	TargetActor = nullptr;
	bLockOnEnabled = false;

	SetComponentTickEnabled(false);
	
	OnLockOnTarget.Broadcast(TargetActor);
}

void ULockOnComponent::OnTargetDeath(AActor* DeadActor, ECharacterType DeadCharType)
{
	if (TargetActor != DeadActor) return;
	ClearLockOn();
}

void ULockOnComponent::ToggleLockOn()
{
	if (bLockOnEnabled)
	{
		ClearLockOn();
	}
	else
	{
		AActor* Hit = TryAutoTarget();
		if (Hit)
		{
			LockOn(Hit);
		}
	}
}

void ULockOnComponent::ChangeLockOn(AActor* NewTarget)
{
	if (!NewTarget || NewTarget == TargetActor) return;

	// 이전 바인딩 해제
	if (BoundHealthComp.IsValid())
	{
		BoundHealthComp->OnDeath.Remove(DeathHandle);
		BoundHealthComp = nullptr;
	}

	if (ABaseAICharacter* OldAI = Cast<ABaseAICharacter>(TargetActor))
	{
		OldAI->ShowLockMarker(false);
		OldAI->SetHPBarPinnedByLockOn(false, 0.f);
	}

	// 타겟 교체
	TargetActor = NewTarget;

	// 새 바인딩 설정
	if (UHealthComponent* BindHealthComp =
		TargetActor->FindComponentByClass<UHealthComponent>())
	{
		DeathHandle = BindHealthComp->OnDeath.AddUObject(
			this, &ULockOnComponent::OnTargetDeath
		);
		BoundHealthComp = BindHealthComp;
	}

	if (ABaseAICharacter* NewAI = Cast<ABaseAICharacter>(TargetActor))
	{
		NewAI->ShowLockMarker(true);
		NewAI->SetHPBarPinnedByLockOn(true);
	}

	OnLockOnTarget.Broadcast(TargetActor);
}

AActor* ULockOnComponent::TryAutoTarget(bool bChangeLockOn)
{
	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	AJeonwoochiCharacter* MyChar = Cast<AJeonwoochiCharacter>(Owner);
	if (!MyChar) return nullptr;

	UCameraComponent* CamComp = MyChar->GetFollowCamera();
	if (!CamComp) return nullptr;

	FVector Start = Owner->GetActorLocation();	
	FVector Forward = CamComp->GetForwardVector();

	TraceRadius = 100.f;

	if (bChangeLockOn && PC)
	{
		Forward.Z = 0.f;
		Forward.Normalize();

		/* ───── Yaw 오프셋 ───── */
		float YawOffset = 0.f;

		// 1 마우스
		{
			float MouseX, MouseY;
			PC->GetInputMouseDelta(MouseX, MouseY);

			constexpr float DegPerPixel = 10.f;
			YawOffset += MouseX * DegPerPixel;
		}

		// 2 패드 : Key 상태 직접 조회
		{
			constexpr float StickDegPerUnit = 30.f;     

			float StickX = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightX);

			YawOffset += StickX * StickDegPerUnit;

		}

		// 3 클램프 & 적용
		constexpr float MaxYawDeg = 60.f;
		YawOffset = FMath::Clamp(YawOffset, -MaxYawDeg, MaxYawDeg);
		Forward = Forward.RotateAngleAxis(YawOffset, FVector::UpVector);

		TraceRadius = 30.f;
	}

	FVector End = Start + Forward * TraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams Params(NAME_None, /*bTraceComplex=*/false, Owner);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	bool bHit = GetWorld()->SweepSingleByObjectType(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ObjectParams,                                
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	// 디버그 선

	if (bHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 20.f, 12, FColor::Yellow, false, 2.f);
	}

	FVector TraceVec = Forward * TraceDistance;
	FVector Center = Start + TraceVec * 0.5f;
	float HalfHeight = TraceDistance * 0.5f + TraceRadius;
	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	FColor DrawColor = bHit ? FColor::Green : FColor::Red;
	float DebugLifeTime = 3.0f;

	DrawDebugCapsule
	(
		GetWorld(),
		Center,
		HalfHeight,
		TraceRadius,
		CapsuleRot,
		DrawColor,
		false,
		DebugLifeTime
	);

	// Hit 처리
	if (bHit && HitResult.GetActor() && HitResult.GetActor() != Owner)
	{
		AActor* HitActor = HitResult.GetActor();

		const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(HitActor);
		if (!TeamAgent) return nullptr;

		if (TeamAgent->GetGenericTeamId() != FGenericTeamId((uint8)ECharacterType::Player))  // 상대 편
		{
			return HitActor;
		}
	}
	return nullptr;
}

// Called every frame
void ULockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bLockOnEnabled || !TargetActor) return;

	// 1) 실제 마우스 움직임 감지 (컨트롤러->GetInputMouseDelta 로 가져옴)
	if (PC)
	{
		float MouseX, MouseY;
		PC->GetInputMouseDelta(MouseX, MouseY);
		float StickX = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightX);

		bool bSwitchRequested =
			FMath::Abs(MouseX) >= MouseDeltaThreshold ||
			FMath::Abs(StickX) >= StickDeadZone;

		// X축(좌우) 이동이 임계치 이상일 때만 타겟 스위치 시도
		if (bSwitchRequested)
		{
			if (AActor* NewTgt = TryAutoTarget(true))
			{
				ChangeLockOn(NewTgt);
			}
		}

	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	const FVector OwnerLoc = Owner->GetActorLocation();

	// 1) 거리 계산 및 체크
	float Dist = FVector::Dist(
		OwnerLoc, TargetActor->GetActorLocation());
	if (Dist > MaxLockDistance)
	{
		ClearLockOn();
		return;
	}

	// 2) 거리 비율(0~1) 계산: 가까우면 0, 멀면 1
	float Alpha = FMath::GetMappedRangeValueClamped(
		FVector2D(MinOffsetDist, MaxOffsetDist),
		FVector2D(0.f, 1.f),
		Dist
	);

	// 3) Z 오프셋 보간
	float ZOffset = FMath::Lerp(MinZOffset, MaxZOffset, Alpha);

	FVector TargetLoc = TargetActor->GetActorLocation();
	TargetLoc.Z -= ZOffset;

	FRotator CtrlDesired = UKismetMathLibrary::FindLookAtRotation(
		OwnerLoc,
		TargetLoc
	);

	// 컨트롤러 회전
	if (APawn* P = Cast<APawn>(Owner))
	{
		if (AController* Controller = P->GetController())
		{
			FRotator CtrlRot = Controller->GetControlRotation();
			
			FRotator DesiredRot = FMath::RInterpTo(
				CtrlRot,
				CtrlDesired,
				DeltaTime,
				RotationInterpSpeed
			);
			
			FRotator NewControlRotation(
				DesiredRot.Pitch,
				DesiredRot.Yaw,
				CtrlRot.Roll
			);

			Controller->SetControlRotation(NewControlRotation);
		}
	}

	
	FRotator ActDesired = UKismetMathLibrary::FindLookAtRotation(
		OwnerLoc,
		TargetActor->GetActorLocation()
	);



	Owner->SetActorRotation(
		FRotator(0, ActDesired.Yaw, 0)
	);
}

void ULockOnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 1) 바인딩 해제, 틱 비활성화
	ClearLockOn();

	Super::EndPlay(EndPlayReason);
}