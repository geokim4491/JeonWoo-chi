// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Utilities/CameraActionRouterComponent.h"
#include "Miscellaneous/Camera/CameraActionPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UCameraActionRouterComponent::UCameraActionRouterComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    // Default to spawn base CameraActionPlayer if no class specified
    PlayerClass = ACameraActionPlayer::StaticClass();
}

const FCameraActionBucket* UCameraActionRouterComponent::FindBucket(FName ActionId) const
{
    for (const FCameraActionBucket& Bucket : Buckets)
    {
        if (Bucket.ActionId == ActionId)
        {
            return &Bucket;
        }
    }
    return nullptr;
}

const FCameraSeqOption* UCameraActionRouterComponent::FindOption(const FCameraActionBucket& Bucket, FName OptionId) const
{
    for (const FCameraSeqOption& Opt : Bucket.Options)
    {
        if (Opt.OptionId == OptionId)
        {
            return &Opt;
        }
    }
    return nullptr;
}

bool UCameraActionRouterComponent::PlayAction(FName ActionId)
{
    // Do not start if an action is already playing and blocking is enabled
    if (bBlockWhilePlaying && ActivePlayer.IsValid())
    {
        return false;
    }

    const FCameraActionBucket* Bucket = FindBucket(ActionId);
    if (!Bucket || Bucket->Options.Num() == 0)
    {
        return false;  // No such action or no sequence options available
    }

    // Choose the first sequence option as the default for this action
    const FCameraSeqOption& DefaultOption = Bucket->Options[0];
    if (!DefaultOption.Sequence)
    {
        return false;
    }

    // Get the world and owner actor for spawn transform
    UWorld* World = GetWorld();
    AActor* OwnerActor = GetOwner();
    if (!World || !OwnerActor)
    {
        return false;
    }

    // Get the player controller (only proceed if one is available)
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return false;
    }

    // Determine spawn transform: place the CameraActionPlayer at the owner's location and rotation
    FTransform SpawnTransform = OwnerActor->GetActorTransform();

    // Spawn the CameraActionPlayer actor (deferred spawn to allow initialization before it fully spawns)
    ACameraActionPlayer* NewPlayer = World->SpawnActorDeferred<ACameraActionPlayer>(
        (PlayerClass != nullptr ? *PlayerClass : ACameraActionPlayer::StaticClass()),
        SpawnTransform, OwnerActor, nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );
    if (!NewPlayer)
    {
        return false;
    }

    // Initialize the CameraActionPlayer's sequence and blend settings before finishing spawn
    NewPlayer->Sequence = DefaultOption.Sequence;
    NewPlayer->BlendInTime = DefaultOption.BlendInTime;
    NewPlayer->BlendOutTime = DefaultOption.BlendOutTime;

    // Complete the spawning process
    UGameplayStatics::FinishSpawningActor(NewPlayer, SpawnTransform);

    // Keep track of the active player and bind to its destruction event for cleanup
    ActivePlayer = NewPlayer;
    NewPlayer->OnDestroyed.AddDynamic(this, &UCameraActionRouterComponent::OnActivePlayerDestroyed);

    // Play the camera action sequence via the newly spawned player
    NewPlayer->PlayCameraAction(PC);
    return true;
}

bool UCameraActionRouterComponent::PlayActionByOption(FName ActionId, FName OptionId)
{
    // Do not start if blocked by an existing active sequence
    if (bBlockWhilePlaying && ActivePlayer.IsValid())
    {
        return false;
    }

    const FCameraActionBucket* Bucket = FindBucket(ActionId);
    if (!Bucket)
    {
        return false;
    }

    const FCameraSeqOption* SelectedOption = FindOption(*Bucket, OptionId);
    if (!SelectedOption || !SelectedOption->Sequence)
    {
        return false;
    }

    UWorld* World = GetWorld();
    AActor* OwnerActor = GetOwner();
    if (!World || !OwnerActor)
    {
        return false;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return false;
    }

    FTransform SpawnTransform = OwnerActor->GetActorTransform();
    ACameraActionPlayer* NewPlayer = World->SpawnActorDeferred<ACameraActionPlayer>(
        (PlayerClass != nullptr ? *PlayerClass : ACameraActionPlayer::StaticClass()),
        SpawnTransform, OwnerActor, nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );
    if (!NewPlayer)
    {
        return false;
    }

    // Set up the sequence and blend settings for the chosen option
    NewPlayer->Sequence = SelectedOption->Sequence;
    NewPlayer->BlendInTime = SelectedOption->BlendInTime;
    NewPlayer->BlendOutTime = SelectedOption->BlendOutTime;

    UGameplayStatics::FinishSpawningActor(NewPlayer, SpawnTransform);

    ActivePlayer = NewPlayer;
    NewPlayer->OnDestroyed.AddDynamic(this, &UCameraActionRouterComponent::OnActivePlayerDestroyed);

    NewPlayer->PlayCameraAction(PC);
    return true;
}

void UCameraActionRouterComponent::OnActivePlayerDestroyed(AActor* /*DestroyedActor*/)
{
    // Clear the ActivePlayer pointer when the CameraActionPlayer is destroyed
    ActivePlayer = nullptr;
}