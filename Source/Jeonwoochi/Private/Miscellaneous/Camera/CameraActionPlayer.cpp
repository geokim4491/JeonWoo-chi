// Fill out your copyright notice in the Description page of Project Settings.


#include "Miscellaneous/Camera/CameraActionPlayer.h"
#include "Camera/CameraComponent.h"            // Base camera component
// #include "CineCameraComponent.h"              // Cinematic camera component (for cinematic properties)
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "DefaultLevelSequenceInstanceData.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

ACameraActionPlayer::ACameraActionPlayer()
{
    // Create a Cine Camera Component (inherits from UCameraComponent) and use it as the root component.
    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    SetRootComponent(CameraComp);

    PrimaryActorTick.bCanEverTick = false;
}

void ACameraActionPlayer::PlayCameraAction(APlayerController* PC)
{
    if (!Sequence)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraActionPlayer: No sequence assigned."));
        return;
    }

    // Use the given PlayerController or get the local player controller if not provided.
    if (!PC)
    {
        PC = UGameplayStatics::GetPlayerController(this, 0);
    }
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraActionPlayer: No PlayerController found to play sequence."));
        return;
    }

    // Save the current camera view target so we can restore it later, then blend view to this actor's camera.
    PrevViewTarget = PC->GetViewTarget();
    PC->SetViewTargetWithBlend(this, BlendInTime, EViewTargetBlendFunction::VTBlend_Cubic, 1.f, true);

    // Create the Level Sequence player and actor.
    FMovieSceneSequencePlaybackSettings PlaybackSettings;
    PlaybackSettings.bAutoPlay = false;
    PlaybackSettings.bPauseAtEnd = false;  // Do not pause at end; sequence will "Stop" at end to trigger finished events.
    SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Sequence, PlaybackSettings, SequenceActor);
    if (!SequencePlayer || !SequenceActor)
    {
        UE_LOG(LogTemp, Error, TEXT("CameraActionPlayer: Failed to create LevelSequencePlayer or actor."));
        // If sequence player could not be created, immediately restore view target and destroy this actor.
        if (PC && PrevViewTarget.IsValid())
        {
            PC->SetViewTargetWithBlend(PrevViewTarget.Get(), 0.0f);  // Immediately switch back if something went wrong.
        }
        Destroy();
        return;
    }

    // Dynamically bind the sequence's camera track to this actor (using the binding tag).
    if (CameraBindingTag != NAME_None)
    {
        TArray<AActor*> ActorsToBind;
        ActorsToBind.Add(this);
        SequenceActor->SetBindingByTag(CameraBindingTag, ActorsToBind, /*bAllowBindingsFromAsset=*/false);
        // Note: bAllowBindingsFromAsset=false replaces any default binding in the asset with this actor.
    }

    // Override sequence instance origin so that all sequence transforms are relative to this actor's transform.
    SequenceActor->bOverrideInstanceData = true;
    if (UDefaultLevelSequenceInstanceData* InstanceData = Cast<UDefaultLevelSequenceInstanceData>(SequenceActor->DefaultInstanceData))
    {
        InstanceData->TransformOrigin = GetActorTransform();
    }

    // Bind our finish handler to both finish and stop events of the sequence player.
    SequencePlayer->OnFinished.AddDynamic(this, &ACameraActionPlayer::OnSequenceFinished);
    SequencePlayer->OnStop.AddDynamic(this, &ACameraActionPlayer::OnSequenceFinished);

    // Start playback of the sequence.
    SequencePlayer->Play();
}

void ACameraActionPlayer::OnSequenceFinished()
{
    // When sequence ends or is stopped, restore the original view target.
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (PrevViewTarget.IsValid())
        {
            PC->SetViewTargetWithBlend(PrevViewTarget.Get(), BlendOutTime, EViewTargetBlendFunction::VTBlend_Cubic, 1.f, true);
        }
    }

    // Destroy this actor after blend-out to clean up. (Set a short lifespan to auto-destroy)
    SetLifeSpan(BlendOutTime + 0.2f);
}