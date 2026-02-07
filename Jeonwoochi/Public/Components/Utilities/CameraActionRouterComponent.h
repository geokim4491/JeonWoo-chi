// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraActionRouterComponent.generated.h"

class ACameraActionPlayer;
class ULevelSequence;

/**
 * Defines a specific camera sequence option with an identifier, the sequence asset, and blend times.
 */
USTRUCT(BlueprintType)
struct FCameraSeqOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Sequence")
    FName OptionId;  // Identifier for this option (e.g., different variations of an action)

    UPROPERTY(EditAnywhere, Category = "Sequence")
    ULevelSequence* Sequence = nullptr;  // The level sequence to play for this option

    UPROPERTY(EditAnywhere, Category = "Blend")
    float BlendInTime = 0.35f;   // View blend-in duration for this sequence

    UPROPERTY(EditAnywhere, Category = "Blend")
    float BlendOutTime = 0.50f;  // View blend-out duration for this sequence
};

/**
 * A router component that manages and triggers camera action sequences.
 * It holds buckets of sequences for various action types and spawns a CameraActionPlayer to play them.
 */
USTRUCT(BlueprintType)
struct FCameraActionBucket
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "CameraAction")
    FName ActionId;  // Identifier for the action (e.g., "Attack", "Skill", etc.)

    UPROPERTY(EditAnywhere, Category = "CameraAction")
    TArray<FCameraSeqOption> Options;  // List of sequence options for this action (first element is default)
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class JEONWOOCHI_API UCameraActionRouterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCameraActionRouterComponent();

    /** Defined camera action sequences grouped by action type. */
    UPROPERTY(EditAnywhere, Category = "CameraAction")
    TArray<FCameraActionBucket> Buckets;

    /** If true, prevent starting a new camera action while one is already playing. */
    UPROPERTY(EditAnywhere, Category = "CameraAction")
    bool bBlockWhilePlaying = true;

    /** The CameraActionPlayer class to spawn for playing sequences (can be subclassed for different camera behaviors). */
    UPROPERTY(EditAnywhere, Category = "CameraAction")
    TSubclassOf<ACameraActionPlayer> PlayerClass;

    /**
     * Play the default camera sequence for the given ActionId.
     * If multiple options exist for that action, the first option is considered the default.
     * @return true if a sequence was successfully started.
     */
    UFUNCTION(BlueprintCallable, Category = "CameraAction")
    bool PlayAction(FName ActionId);

    /**
     * Play a specific camera sequence option for the given ActionId.
     * @return true if the specified sequence was successfully started.
     */
    UFUNCTION(BlueprintCallable, Category = "CameraAction")
    bool PlayActionByOption(FName ActionId, FName OptionId);

private:
    /** Pointer to the currently active CameraActionPlayer (if any). */
    TWeakObjectPtr<ACameraActionPlayer> ActivePlayer;

    /** Helper to find an action bucket by ActionId. */
    const FCameraActionBucket* FindBucket(FName ActionId) const;

    /** Helper to find a sequence option by OptionId within a given bucket. */
    const FCameraSeqOption* FindOption(const FCameraActionBucket& Bucket, FName OptionId) const;

    /** Callback when the active CameraActionPlayer is destroyed (to clear the ActivePlayer pointer). */
    UFUNCTION()
    void OnActivePlayerDestroyed(AActor* DestroyedActor);
};