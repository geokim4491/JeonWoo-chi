// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/2D/PaperAnimationComponent.h"
#include "PaperFlipbookComponent.h"
#include "Jeonwoochi/JeonwoochiPaperCharacter.h"
#include "PaperFlipbook.h"

// Sets default values for this component's properties
UPaperAnimationComponent::UPaperAnimationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	State = EAnimState::Idle;
	bQueuedNext = false;
	QueuedNextState = EAnimState::Idle;
	// ...
}


// Called when the game starts
void UPaperAnimationComponent::BeginPlay()
{
	Super::BeginPlay();

	Enter(EAnimState::Idle);
	// ...
	
}


// Called every frame
void UPaperAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPaperAnimationComponent::Init(UPaperFlipbookComponent* InFlipbookComp)
{
	FlipbookComp = InFlipbookComp;
    Character = Cast<AJeonwoochiPaperCharacter>(GetOwner());
	if (FlipbookComp)
	{
		FlipbookComp->OnFinishedPlaying.RemoveDynamic(this, &UPaperAnimationComponent::HandleFlipbookFinished);
		FlipbookComp->OnFinishedPlaying.AddDynamic(this, &UPaperAnimationComponent::HandleFlipbookFinished);
	}
}


/* ============ 공용 입력/Event 진입점 ============ */

void UPaperAnimationComponent::OnAttackPressed()
{
	Dispatch(EAnimEvent::AttackPressed);
}
void UPaperAnimationComponent::OnJumpPressed()
{
	Dispatch(EAnimEvent::JumpPressed);
}
void UPaperAnimationComponent::OnHitReceived()
{
	Dispatch(EAnimEvent::HitReceived);
}
void UPaperAnimationComponent::OnLocomotionUpdate(bool bIsOnGround, float Speed, float WalkThreshold, float RunThreshold)
{
	Dispatch(EAnimEvent::LocomotionUpdate, bIsOnGround, Speed, WalkThreshold, RunThreshold);
}

/* ============ 내부: 상태 전이/재생 ============ */

void UPaperAnimationComponent::PlayLoop(UPaperFlipbook* FB)
{
    if (!FlipbookComp || !FB) return;
    if (FlipbookComp->GetFlipbook() == FB && FlipbookComp->IsLooping() && FlipbookComp->IsPlaying()) return;
    FlipbookComp->SetFlipbook(FB);
    FlipbookComp->SetLooping(true);
    FlipbookComp->Play();
}

void UPaperAnimationComponent::PlayOnce(UPaperFlipbook* FB)
{
    if (!FlipbookComp || !FB) return;
    UE_LOG(LogTemp, Warning, TEXT("Played: %d"), (int)State);
    FlipbookComp->SetFlipbook(FB);
    FlipbookComp->SetLooping(false);
    FlipbookComp->PlayFromStart();
}

void UPaperAnimationComponent::Enter(EAnimState Next)
{
    Exit(State);
    State = Next;
    bQueuedNext = false;
    QueuedNextState = EAnimState::Idle;

    switch (State)
    {
    case EAnimState::Falling: PlayLoop(FB_FallStart); break;
    case EAnimState::FallEnd: PlayOnce(FB_FallEnd); break;
    case EAnimState::Idle:   PlayLoop(FB_Idle);   break;
    case EAnimState::Walk:   PlayLoop(FB_Walk);   break;
    case EAnimState::Run:    PlayLoop(FB_Run);    break;
    case EAnimState::SideJump: PlayOnce(FB_SideJump); break;
    case EAnimState::Jump:   PlayOnce(FB_Jump);   break;      // 끝나면 Idle
    case EAnimState::Attack1:
        PlayOnce(FB_Attack1); 
        Character->Attack1();
        break;      // 끝나면 Idle or Attack2
    case EAnimState::Attack2:
        PlayOnce(FB_Attack2); 
        Character->Attack2();
        break;      // 끝나면 Idle or Attack1
    case EAnimState::Hit:    PlayOnce(FB_Hit);    break;      // 끝나면 Idle
    default: break;
    }
}

void UPaperAnimationComponent::Exit(EAnimState Prev)
{
    // 특별한 정리 필요 시(락 해제, 무적 해제 등) 여기서 처리
}

void UPaperAnimationComponent::HandleFlipbookFinished()
{
    Dispatch(EAnimEvent::FlipbookFinished);
}

/* ============ FSM 전이 규칙 ============ */

void UPaperAnimationComponent::Dispatch(EAnimEvent Event, bool bIsOnGround, float Speed, float WalkT, float RunT)
{
    if (Event == EAnimEvent::Death)
    {
        Enter(EAnimState::Dead);
        return;
    }
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
        if (Event == EAnimEvent::JumpPressed)
        {
            if (State == EAnimState::Walk || State == EAnimState::Run) Enter(EAnimState::SideJump);
            else Enter(EAnimState::Jump);
            return;
        }
        if (Event == EAnimEvent::AttackPressed)
        {
            Enter(EAnimState::Attack1);
            return;
        }
        if (Event == EAnimEvent::LocomotionUpdate)
        {
            if (!bIsOnGround)
            {
                Enter(EAnimState::Falling);
                return;
            }
            // 지상 이동 상태 결정
            if (Speed >= RunT)      Enter(EAnimState::Run);
            else if (Speed >= WalkT)Enter(EAnimState::Walk);
            else                    Enter(EAnimState::Idle);
        }
        break;

    case EAnimState::Falling:
        if (Event == EAnimEvent::LocomotionUpdate)
        {
            if (bIsOnGround)
            {
                Enter(EAnimState::FallEnd);
                return;
            }
        }

    case EAnimState::FallEnd:
        if (Event == EAnimEvent::FlipbookFinished)
        {
            Enter(EAnimState::Idle);
            return;
        }

        /* ----- Jump: 1회 재생 후 Idle ----- */
    case EAnimState::Jump:
        if (Event == EAnimEvent::FlipbookFinished)
        {
            Enter(EAnimState::Idle);
            return;
        }
        // 점프 중 공격 입력은 무시(원하면 큐잉 로직 추가 가능)
        break;

    case EAnimState::SideJump:
        if (Event == EAnimEvent::FlipbookFinished)
        {
            Enter(EAnimState::Idle);
            return;
        }
        // 점프 중 공격 입력은 무시(원하면 큐잉 로직 추가 가능)
        break;

        /* ----- Attack1: 끝나기 전에 한 번 더 → Attack2 큐 ----- */
    case EAnimState::Attack1:
        if (Event == EAnimEvent::AttackPressed)
        {
            // “끝나기 전에” 조건은 단순히 ‘이미 끝나지 않았을 때’ 입력으로 가정.
            bQueuedNext = true;
            QueuedNextState = EAnimState::Attack2;
            return;
        }
        if (Event == EAnimEvent::FlipbookFinished)
        {
            if (bQueuedNext && QueuedNextState == EAnimState::Attack2)
            {
                Enter(EAnimState::Attack2);  // 끝나는 타이밍에 Attack2로
            }
            else
            {
                Enter(EAnimState::Idle);
            }
            return;
        }
        break;

        /* ----- Attack2: 끝나기 전에 한 번 더 → Attack1 큐 ----- */
    case EAnimState::Attack2:
        if (Event == EAnimEvent::AttackPressed)
        {
            bQueuedNext = true;
            QueuedNextState = EAnimState::Attack1; // 요구사항: 2에서 다시 1로
            return;
        }
        if (Event == EAnimEvent::FlipbookFinished)
        {
            if (bQueuedNext && QueuedNextState == EAnimState::Attack1)
            {
                Enter(EAnimState::Attack1);  // 끝나는 타이밍에 Attack1로
            }
            else
            {
                Enter(EAnimState::Idle);
            }
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