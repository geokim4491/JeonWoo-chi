// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CameraActionPlayer.generated.h"

class UCameraComponent;   // Cinematic Camera Component (requires CinematicCamera module)
class ULevelSequence;
class ULevelSequencePlayer;
class ALevelSequenceActor;

/**
 * CameraActionPlayer
 *
 * An actor that plays a Level Sequence (ULevelSequence) containing a camera animation.
 * It dynamically binds the sequence's camera track (identified by a binding tag) to its own camera component,
 * and uses Transform Origin offset so the sequence can play at this actor's location.
 * The actor blends the player's view to its camera when playing and back to the previous view target when finished.
 */
UCLASS()
class JEONWOOCHI_API ACameraActionPlayer : public AActor
{
    GENERATED_BODY()

public:
    ACameraActionPlayer();

    /** Tag name used in the Level Sequence for the camera binding (e.g. "ActionCam"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraAction")
    FName CameraBindingTag = TEXT("ActionCam");

    /** Level Sequence asset to play for this camera action. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraAction")
    ULevelSequence* Sequence = nullptr;

    /** Duration of blend when switching view to this camera. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraAction")
    float BlendInTime = 0.35f;

    /** Duration of blend when switching view back after sequence finishes. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraAction")
    float BlendOutTime = 0.50f;

    /**
     * Play the camera action sequence.
     * @param PC (Optional) PlayerController whose view will be controlled. If not provided, will use the local player controller.
     */
    UFUNCTION(BlueprintCallable, Category = "CameraAction")
    void PlayCameraAction(APlayerController* PC = nullptr);

private:
    /** The camera component used for this cinematic (cine camera component for cinematic effects). */
    UPROPERTY()
    UCameraComponent* CameraComp = nullptr;

    /** The Level Sequence player instance playing the sequence. */
    UPROPERTY()
    ULevelSequencePlayer* SequencePlayer = nullptr;

    /** The Level Sequence actor spawned to play the sequence. */
    UPROPERTY()
    ALevelSequenceActor* SequenceActor = nullptr;

    /** Previous view target saved so we can restore it after the sequence ends. */
    TWeakObjectPtr<AActor> PrevViewTarget;

    /** Called when the sequence finishes or is stopped to restore the view and clean up. */
    UFUNCTION()
    void OnSequenceFinished();
};
