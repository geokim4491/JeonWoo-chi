// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "OverlayMaterialNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UOverlayMaterialNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
    // 에디터에서 설정할 값들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlay")
    UMaterialInterface* OverlayMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlay")
    FName OpacityParamName = TEXT("OverlayOpacity");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlay", meta = (ClampMin = "0"))
    float FadeInTime = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlay", meta = (ClampMin = "0"))
    float FadeOutTime = 0.15f;

    // ignore global time dilation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlay")
    bool bIgnoreTimeDilation = false;

    // restore default material
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlay")
    bool bRestorePreviousOverlayOnEnd = true;

    // AnimNotifyState 오버라이드
    virtual void NotifyBegin(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        float TotalDuration,
        const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyTick(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        float FrameDeltaTime,
        const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

private:
    struct FRuntime
    {
        TWeakObjectPtr<UMaterialInstanceDynamic> MID;
        TWeakObjectPtr<UMaterialInterface> PrevMat;
        float Elapsed = 0.f;
        float Total = 0.f;
        float FadeIn = 0.f;
        float FadeOut = 0.f;
        FName Param;
        bool bIgnoreDil = false;
        bool bRestorePrev = true;
        double LastRealTime = 0.0;
    };

    // 노티파이 애셋은 공용 UObject라 상태를 컴포넌트별로 따로 들고 있어야 함
    static TMap<TWeakObjectPtr<USkeletalMeshComponent>, FRuntime> Runtimes;

    static FRuntime* FindRuntime(USkeletalMeshComponent* Mesh);
    static FRuntime& GetOrAddRuntime(USkeletalMeshComponent* Mesh);
    static void RemoveRuntime(USkeletalMeshComponent* Mesh);
};
