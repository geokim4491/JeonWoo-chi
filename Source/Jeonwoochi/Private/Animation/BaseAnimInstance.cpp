// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/BaseAnimInstance.h"
#include "Characters/BaseCharacter.h"
#include "../JeonwoochiCharacter.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Components/Utilities/LockOnComponent.h"

UBaseAnimInstance::UBaseAnimInstance()
    : OwningPawn(nullptr), Speed(0.f), bIsInAir(false), Direction(0.f), bIsTargetSetted(false)
{

}

void UBaseAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwningPawn = TryGetPawnOwner();

    if (OwningPawn)
    {
        LockOnComp = OwningPawn->FindComponentByClass<ULockOnComponent>();
    }
}

void UBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    if (!OwningPawn)
    {
        OwningPawn = TryGetPawnOwner();
    }
    if (!OwningPawn) return;
    

	// Get LockOn Component
    if (!LockOnComp)
    {
        LockOnComp = OwningPawn->FindComponentByClass<ULockOnComponent>();
    }

    bLockOnEnabled = (LockOnComp && LockOnComp->bLockOnEnabled);

    // Update speed
    FVector Velocity = OwningPawn->GetVelocity();

    AController* C = OwningPawn->GetController();
    if (AAIController* AICon = Cast<AAIController>(C))
    {
        // --- AI 전용: BB에서 TargetActor 꺼내오기 ---
        if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
        {
            static const FName Key_Target = TEXT("TargetActor");
            if (AActor* Target = Cast<AActor>(BB->GetValueAsObject(Key_Target)))
            {
                // 2-1) 바라보는 회전 계산
                const FVector SelfLoc = OwningPawn->GetActorLocation();
                const FVector TargetLoc = Target->GetActorLocation();
                FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(SelfLoc, TargetLoc);

                Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, LookAtRot);
                
                bIsTargetSetted = true;

            }
            else
            {
                bIsTargetSetted = false;
                Direction = 0.f;
            }
        }
        else 
        {
            Direction = 0.f;
        }
    }
    else
    {
        // --- Player인 경우: 기존 방식 그대로 ---
        const FRotator ActorRot = OwningPawn->GetActorRotation();
        Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, ActorRot);
    }

    Velocity.Z = 0.f;
    Speed = Velocity.Size();

    // Update air state if character
    if (ABaseCharacter* Character = Cast<ABaseCharacter>(OwningPawn))
    {
        bIsInAir = Character->GetCharacterMovement()->IsFalling();
        Acceleration = Character->GetCharacterMovement()->GetCurrentAcceleration().Size();
    }
    if (AJeonwoochiCharacter* JeonwoochiCharacter = Cast<AJeonwoochiCharacter>(OwningPawn))
    {
        bIsRunning = JeonwoochiCharacter->bIsRunning;
        bIsBlocking = JeonwoochiCharacter->bIsBlocking;
    }
}

void UBaseAnimInstance::PlayMontage(UAnimMontage* Montage, FName StartSection, float PlayRate)
{
    if (!Montage) return;
    Montage_Play(Montage, PlayRate);
    if (StartSection != NAME_None)
    {
        Montage_JumpToSection(StartSection, Montage);
    }
}