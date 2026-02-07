// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CameraOverrideAnimNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UCameraOverrideAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
    /** 들어갈 때/나올 때 각각 블렌드 시간 */
    UPROPERTY(EditAnywhere, Category = "Camera") float BlendInTime = 0.4f;
    UPROPERTY(EditAnywhere, Category = "Camera") float BlendOutTime = 0.2f;

    /** 최단 회전 경로 강제 여부(영상에 있던 옵션) */
    UPROPERTY(EditAnywhere, Category = "Camera") bool bForceShortestPath = true;

    virtual void NotifyBegin(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        float TotalDuration,
        const FAnimNotifyEventReference& EventReference
    ) override;

    virtual void NotifyEnd(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference
    ) override;
	
};
