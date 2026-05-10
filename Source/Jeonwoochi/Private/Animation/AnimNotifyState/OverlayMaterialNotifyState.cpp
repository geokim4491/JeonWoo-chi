// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState/OverlayMaterialNotifyState.h"

#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

TMap<TWeakObjectPtr<USkeletalMeshComponent>, UOverlayMaterialNotifyState::FRuntime> UOverlayMaterialNotifyState::Runtimes;

UOverlayMaterialNotifyState::FRuntime* UOverlayMaterialNotifyState::FindRuntime(USkeletalMeshComponent* Mesh)
{
    if (!Mesh) return nullptr;
    return Runtimes.Find(Mesh);
}

UOverlayMaterialNotifyState::FRuntime& UOverlayMaterialNotifyState::GetOrAddRuntime(USkeletalMeshComponent* Mesh)
{
    return Runtimes.FindOrAdd(Mesh);
}

void UOverlayMaterialNotifyState::RemoveRuntime(USkeletalMeshComponent* Mesh)
{
    if (Mesh) Runtimes.Remove(Mesh);
}

void UOverlayMaterialNotifyState::NotifyBegin(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float TotalDuration,
    const FAnimNotifyEventReference& EventReference
)
{
    if (!MeshComp || !OverlayMaterial) return;

    FRuntime& Rt = GetOrAddRuntime(MeshComp);

    // 이전 오버레이 저장
    Rt.PrevMat = MeshComp->GetOverlayMaterial();

    // MID 생성 및 적용
    Rt.MID = UMaterialInstanceDynamic::Create(OverlayMaterial, MeshComp);
    if (!Rt.MID.IsValid()) return;

    MeshComp->SetOverlayMaterial(Rt.MID.Get());

    // 런타임 파라미터 저장
    Rt.Param = OpacityParamName;
    Rt.FadeIn = FMath::Max(0.f, FadeInTime);
    Rt.FadeOut = FMath::Max(0.f, FadeOutTime);
    Rt.Total = FMath::Max(0.f, TotalDuration);  // 노티 길이(초)
    Rt.Elapsed = 0.f;
    Rt.bIgnoreDil = bIgnoreTimeDilation;
    Rt.bRestorePrev = bRestorePreviousOverlayOnEnd;
    Rt.LastRealTime = MeshComp->GetWorld()
        ? MeshComp->GetWorld()->GetRealTimeSeconds()
        : FPlatformTime::Seconds();

    // 시작 시점: 페이드 인이면 0, 아니면 1로
    const float StartAlpha = (Rt.FadeIn > 0.f) ? 0.f : 1.f;
    Rt.MID->SetScalarParameterValue(Rt.Param, StartAlpha);

    return;
}

void UOverlayMaterialNotifyState::NotifyTick(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float FrameDeltaTime,
    const FAnimNotifyEventReference& EventReference
)
{
    FRuntime* Rt = FindRuntime(MeshComp);
    if (!Rt || !Rt->MID.IsValid()) return;

    // 경과 시간 갱신 (딜레이션 무시 옵션 지원)
    float Delta = FrameDeltaTime;
    if (Rt->bIgnoreDil)
    {
        const double Now = MeshComp->GetWorld()
            ? MeshComp->GetWorld()->GetRealTimeSeconds()
            : FPlatformTime::Seconds();
        Delta = static_cast<float>(Now - Rt->LastRealTime);
        Rt->LastRealTime = Now;
    }

    Rt->Elapsed += FMath::Max(0.f, Delta);

    // 알파 계산
    const float T = Rt->Elapsed;
    const float Total = Rt->Total;
    float Alpha = 1.f;

    if (Total <= 0.f)
    {
        // 길이가 0이면 페이드 인만 고려
        Alpha = (Rt->FadeIn > 0.f) ? FMath::Clamp(T / Rt->FadeIn, 0.f, 1.f) : 1.f;
    }
    else
    {
        if (T < Rt->FadeIn)
        {
            Alpha = (Rt->FadeIn > 0.f) ? (T / Rt->FadeIn) : 1.f;
        }
        else if (T > Total - Rt->FadeOut)
        {
            const float Remain = Total - T; // 남은 시간
            Alpha = (Rt->FadeOut > 0.f) ? FMath::Clamp(Remain / Rt->FadeOut, 0.f, 1.f) : 0.f;
        }
        else
        {
            Alpha = 1.f;
        }
    }

    Rt->MID->SetScalarParameterValue(Rt->Param, Alpha);
    return;
}

void UOverlayMaterialNotifyState::NotifyEnd(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference
)
{
    FRuntime* Rt = FindRuntime(MeshComp);
    if (!Rt) return;

    // 내가 적용한 MID가 그대로 남아있을 때만 복구 (겹침 방지)
    if (Rt->MID.IsValid() && MeshComp && MeshComp->GetOverlayMaterial() == Rt->MID.Get())
    {
        if (Rt->bRestorePrev)
        {
            MeshComp->SetOverlayMaterial(Rt->PrevMat.IsValid() ? Rt->PrevMat.Get() : nullptr);
        }
        else
        {
            // 유지하고 싶으면 그대로 두되, 알파를 0으로 내리고 싶다면 여기서 0으로
            // Rt->MID->SetScalarParameterValue(Rt->Param, 0.f);
        }
    }

    RemoveRuntime(MeshComp);
    return;
}