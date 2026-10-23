// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/2D/PaperAIAnimationComponent.h"
#include "PaperFlipbookComponent.h"
#include "Characters/AI/PaperAICharacter.h"
#include "PaperFlipbook.h"

void UPaperAIAnimationComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<APaperAICharacter>(GetOwner());
}

void UPaperAIAnimationComponent::Enter(EAnimState Next)
{
    Exit(State);
    State = Next;
    bQueuedNext = false;
    QueuedNextState = EAnimState::Idle;


    switch (State)
    {
    case EAnimState::Idle:   PlayLoop(FB_Idle);   break;
    case EAnimState::Walk:   PlayLoop(FB_Walk);   break;
    case EAnimState::Run:    PlayLoop(FB_Run);    break;
    case EAnimState::Attack1:
        PlayOnce(FB_Attack1); 
        OwnerCharacter->Attack1();
        break;    
    case EAnimState::Attack2:
        PlayOnce(FB_Attack2);
        OwnerCharacter->Attack2();
        break;     
    case EAnimState::Hit:    PlayOnce(FB_Hit);    break;     
    default: break;
    }
}

void UPaperAIAnimationComponent::Dispatch(EAnimEvent Event, bool bIsOnGround, float Speed, float WalkT, float RunT)
{
    // “모든 상태에서 Hit 우선” 규칙
    if (Event == EAnimEvent::HitReceived)
    {
        Enter(EAnimState::Hit);
        return;
    }

    switch (State)
    {
        /* ----- Idle / Walk / Run: 기본 이동 & 입력 진입 허용 ----- */
    case EAnimState::Idle:
    case EAnimState::Walk:
    case EAnimState::Run:
        if (Event == EAnimEvent::BTCalled)
        {
            Enter(BTCallingState);
            return;
        }
        if (Event == EAnimEvent::LocomotionUpdate)
        {
            if (!bIsOnGround)
            {
                return;
            }
            // 지상 이동 상태 결정
            if (Speed >= RunT)      Enter(EAnimState::Run);
            else if (Speed >= WalkT)Enter(EAnimState::Walk);
            else                    Enter(EAnimState::Idle);
        }
        break;
    case EAnimState::Attack1:
        if (Event == EAnimEvent::FlipbookFinished)
        {
            Enter(EAnimState::Idle);
            return;
        }
        break;

    case EAnimState::Attack2:
        if (Event == EAnimEvent::FlipbookFinished)
        {
            Enter(EAnimState::Idle);
            return;
        }
        break;

        /* ----- Hit: 1회 재생 후 Idle ----- */
    case EAnimState::Hit:
        if (Event == EAnimEvent::FlipbookFinished)
        {
            Enter(EAnimState::Idle);
            return;
        }
        break;

    default:
        break;
    }
}