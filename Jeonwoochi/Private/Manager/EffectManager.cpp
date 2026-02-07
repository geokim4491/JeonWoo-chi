// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/EffectManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Engine/World.h"
#include "Manager/EffectDevSettings.h"
#include "GameFramework/PlayerController.h"
#include "Components/SceneComponent.h"
#include "Jeonwoochi/JeonwoochiCharacter.h"

void UEffectManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    const UEffectDevSettings* Settings = GetDefault<UEffectDevSettings>();
    EffectsMap = Settings->EffectsMap;
}

USceneComponent* UEffectManager::ResolveAttachComponent(AActor* ContextActor) const
{
    if (!ContextActor) return nullptr;
    return ContextActor->GetRootComponent(); // 필요하면 여기서 특정 컴포넌트를 찾아 반환하도록 확장 가능
}

void UEffectManager::ComputeFromBase(const FTransform& Base, const FEffectSpawnTuning& S, FVector& OutLoc, FRotator& OutRot) const
{
    const FVector Fwd = Base.GetUnitAxis(EAxis::X);
    const FVector Right = Base.GetUnitAxis(EAxis::Y);
    const FVector Up = Base.GetUnitAxis(EAxis::Z);

    OutLoc = Base.GetLocation() + Fwd * S.Forward + Right * S.Right + Up * S.Up;
    OutRot = (Base.Rotator() + S.RotationOffset).GetNormalized();
}

void UEffectManager::PlayEffectOnActor(EEffectType Type, AActor* ContextActor, bool bUseCameraShake, bool bUseHaptic)
{
    if (!ContextActor) return;

    if (const FEffectSettings* Settings = EffectsMap.Find(Type))
    {
        UWorld* World = GetWorld();
        if (!World) return;

        const FTransform Base = ContextActor->GetActorTransform();
        USceneComponent* AttachComp = ResolveAttachComponent(ContextActor);

        // 1) Sound (부착/비부착 선택)
        if (Settings->Sound)
        {
            // 사운드는 보정값이 1개가 아니라면 첫 엔트리에 맞춰 줄 수도 있는데,
            // 일반적으로 사운드는 위치 하나면 충분해서 Forward/Right/Up = 0 으로 재생.
            UGameplayStatics::PlaySoundAtLocation(World, Settings->Sound, Base.GetLocation());
        }

        // 2a) Niagara
        for (const FNiagaraEffectEntry& E : Settings->NiagaraEffects)
        {
            if (!E.System || Settings->bNiagaraRandomOneCandidate) continue;

            FVector Loc; FRotator Rot;
            ComputeFromBase(Base, E.Spawn, Loc, Rot);

            UNiagaraComponent* Comp = nullptr;
            if (E.Spawn.bAttachToContext && AttachComp)
            {
                Comp = UNiagaraFunctionLibrary::SpawnSystemAttached(
                    E.System, AttachComp, E.Spawn.AttachSocketName,
                    FVector(E.Spawn.Forward, E.Spawn.Right, E.Spawn.Up),
                    E.Spawn.RotationOffset,
                    EAttachLocation::KeepRelativeOffset,
                    /*bAutoDestroy*/ Settings->bAutoDestroyOnFinish,
                    /*bAutoActivate*/ true
                );
                if (Comp) Comp->SetWorldScale3D(E.Spawn.Scale);
            }
            else
            {
                Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    World, E.System, Loc, Rot, E.Spawn.Scale,
                    /*bAutoActivate*/ true,
                    /*bAutoDestroy*/ Settings->bAutoDestroyOnFinish
                );
            }

            if (Comp && !Settings->bAutoDestroyOnFinish)
                ActiveNiagaras.FindOrAdd(Type).Add(Comp);
        }

        TArray<int32> NCandidates;
        for (int32 i = 0; i < Settings->NiagaraEffects.Num(); ++i)
        {
            const auto& E = Settings->NiagaraEffects[i];
            if (E.System && Settings->bNiagaraRandomOneCandidate) NCandidates.Add(i);
        }
        if (NCandidates.Num() > 0)
        {
            const int32 Pick = NCandidates[FMath::RandRange(0, NCandidates.Num() - 1)];
            const auto& E = Settings->NiagaraEffects[Pick];

            FVector Loc; FRotator Rot; ComputeFromBase(Base, E.Spawn, Loc, Rot);
            UNiagaraComponent* Comp = nullptr;

            if (E.Spawn.bAttachToContext && AttachComp)
            {
                Comp = UNiagaraFunctionLibrary::SpawnSystemAttached(
                    E.System, AttachComp, E.Spawn.AttachSocketName,
                    FVector(E.Spawn.Forward, E.Spawn.Right, E.Spawn.Up),
                    E.Spawn.RotationOffset,
                    EAttachLocation::KeepRelativeOffset,
                    Settings->bAutoDestroyOnFinish, true
                );
                if (Comp) Comp->SetWorldScale3D(E.Spawn.Scale);
            }
            else
            {
                Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    World, E.System, Loc, Rot, E.Spawn.Scale, true, Settings->bAutoDestroyOnFinish
                );
            }
            if (Comp && !Settings->bAutoDestroyOnFinish) ActiveNiagaras.FindOrAdd(Type).Add(Comp);
        }

        // 2b) Cascade
        for (const FCascadeEffectEntry& E : Settings->CascadeEffects)
        {
            if (!E.System || Settings->bCascadeRandomOneCandidate) continue;

            FVector Loc; FRotator Rot;
            ComputeFromBase(Base, E.Spawn, Loc, Rot);

            UParticleSystemComponent* PSC = nullptr;
            if (E.Spawn.bAttachToContext && AttachComp)
            {
                PSC = UGameplayStatics::SpawnEmitterAttached(
                    E.System, AttachComp, E.Spawn.AttachSocketName,
                    FVector(E.Spawn.Forward, E.Spawn.Right, E.Spawn.Up),
                    E.Spawn.RotationOffset,
                    EAttachLocation::KeepRelativeOffset,
                    /*bAutoDestroy*/ Settings->bAutoDestroyOnFinish
                );
                if (PSC) PSC->SetWorldScale3D(E.Spawn.Scale);
            }
            else
            {
                PSC = UGameplayStatics::SpawnEmitterAtLocation(
                    World, E.System, Loc, Rot, E.Spawn.Scale,
                    /*bAutoDestroy*/ Settings->bAutoDestroyOnFinish,
                    EPSCPoolMethod::None,
                    /*bAutoActivate*/ true
                );
            }

            if (PSC && !Settings->bAutoDestroyOnFinish)
                ActiveCascades.FindOrAdd(Type).Add(PSC);
        }
        TArray<int32> Candidates;
        for (int32 i = 0; i < Settings->CascadeEffects.Num(); ++i)
        {
            const auto& E = Settings->CascadeEffects[i];
            if (E.System && Settings->bCascadeRandomOneCandidate) Candidates.Add(i);
        }
        if (Candidates.Num() > 0)
        {
            const int32 Pick = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
            const auto& E = Settings->CascadeEffects[Pick];

            FVector Loc; FRotator Rot; ComputeFromBase(Base, E.Spawn, Loc, Rot);

            UParticleSystemComponent* PSC = nullptr;
            if (E.Spawn.bAttachToContext && AttachComp)
            {
                PSC = UGameplayStatics::SpawnEmitterAttached(
                    E.System, AttachComp, E.Spawn.AttachSocketName,
                    FVector(E.Spawn.Forward, E.Spawn.Right, E.Spawn.Up),
                    E.Spawn.RotationOffset,
                    EAttachLocation::KeepRelativeOffset,
                    Settings->bAutoDestroyOnFinish
                );
                if (PSC) PSC->SetWorldScale3D(E.Spawn.Scale);
            }
            else
            {
                PSC = UGameplayStatics::SpawnEmitterAtLocation(
                    World, E.System, Loc, Rot, E.Spawn.Scale,
                    Settings->bAutoDestroyOnFinish, EPSCPoolMethod::None, true
                );
            }
            if (PSC && !Settings->bAutoDestroyOnFinish) ActiveCascades.FindOrAdd(Type).Add(PSC);
        }

        // 3) Camera Shake
        if (Settings->bEnableCameraShake && Settings->CameraShakeClass && bUseCameraShake)
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
            {
                PC->PlayerCameraManager->StartCameraShake(Settings->CameraShakeClass, Settings->CameraShakeScale);
            }
        }

        // 4) Haptic
        if (Settings->bEnableHaptic && Settings->ForceFeedbackEffect && bUseHaptic)
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
            {
                FForceFeedbackParameters Params; Params.bIgnoreTimeDilation = true;
                PC->ClientPlayForceFeedback(Settings->ForceFeedbackEffect, Params);
            }
        }

        // 5) SlowMotion
        const auto& TD = Settings->TimeDilation;
        if (TD.Duration > 0.f && TD.DilationFactor != 1.f)
        {
            UGameplayStatics::SetGlobalTimeDilation(World, TD.DilationFactor);
            World->GetTimerManager().SetTimer(
                TimeDilationTimerHandle,
                [World]() { if (World) UGameplayStatics::SetGlobalTimeDilation(World, 1.f); },
                TD.Duration, false
            );
        }
    }
}


void UEffectManager::PlayEffect(EEffectType Type, const FVector& Location, const FRotator& Rotation, bool bUseHaptic)
{
    if (const FEffectSettings* Settings = EffectsMap.Find(Type))
    {
        UWorld* World = GetWorld();
        if (!World) return;

        const FTransform Base(Rotation, Location);

        // 1) Sound
        if (Settings->Sound)
        {
            UGameplayStatics::PlaySoundAtLocation(World, Settings->Sound, Location);
        }

        // 2a) Niagara
        for (const FNiagaraEffectEntry& E : Settings->NiagaraEffects)
        {
            if (!E.System) continue;

            FVector Loc; FRotator Rot;
            ComputeFromBase(Base, E.Spawn, Loc, Rot);

            UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                World, E.System, Loc, Rot, E.Spawn.Scale, true, Settings->bAutoDestroyOnFinish
            );

            if (Comp && !Settings->bAutoDestroyOnFinish)
                ActiveNiagaras.FindOrAdd(Type).Add(Comp);
        }

        // 2b) Cascade
        for (const FCascadeEffectEntry& E : Settings->CascadeEffects)
        {
            if (!E.System) continue;

            FVector Loc; FRotator Rot;
            ComputeFromBase(Base, E.Spawn, Loc, Rot);

            UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
                World, E.System, Loc, Rot, E.Spawn.Scale,
                Settings->bAutoDestroyOnFinish, EPSCPoolMethod::None, true
            );

            if (PSC && !Settings->bAutoDestroyOnFinish)
                ActiveCascades.FindOrAdd(Type).Add(PSC);
        }

        // 3) Camera Shake
        if (Settings->bEnableCameraShake && Settings->CameraShakeClass)
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
            {
                PC->PlayerCameraManager->StartCameraShake(Settings->CameraShakeClass, Settings->CameraShakeScale);
            }
        }

        // 4) Haptic
        if (Settings->bEnableHaptic && Settings->ForceFeedbackEffect && bUseHaptic)
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
            {
                FForceFeedbackParameters Params; Params.bIgnoreTimeDilation = true;
                PC->ClientPlayForceFeedback(Settings->ForceFeedbackEffect, Params);
            }
        }

        // 5) SlowMotion
        const auto& TD = Settings->TimeDilation;
        if (TD.Duration > 0.f && TD.DilationFactor != 1.f)
        {
            UGameplayStatics::SetGlobalTimeDilation(World, TD.DilationFactor);
            World->GetTimerManager().SetTimer(
                TimeDilationTimerHandle,
                [World]() { if (World) UGameplayStatics::SetGlobalTimeDilation(World, 1.f); },
                TD.Duration, false
            );
        }
    }
}

void UEffectManager::StopEffect(EEffectType Type)
{
    // Niagara
    if (auto* Arr = ActiveNiagaras.Find(Type))
    {
        for (auto* Comp : *Arr)
            if (Comp)
            {
                Comp->Deactivate();
                Comp->DestroyComponent();
            }
        Arr->Empty();
    }

    // Cascade
    if (auto* Arr2 = ActiveCascades.Find(Type))
    {
        for (auto* Comp : *Arr2)
            if (Comp)
            {
                Comp->DeactivateSystem();
                Comp->DestroyComponent();
            }
        Arr2->Empty();
    }
}

void UEffectManager::Deinitialize()
{
    Super::Deinitialize();

    // -- 1) 남은 이펙트 정리 --
    for (auto& Pair : ActiveNiagaras) {
        for (auto* Comp : Pair.Value) if (Comp) Comp->Deactivate();
    }
    ActiveNiagaras.Empty();

    for (auto& Pair : ActiveCascades) {
        for (auto* PSC : Pair.Value) if (PSC) PSC->DeactivateSystem();
    }
    ActiveCascades.Empty();

    // -- 2) 타이머 정리 및 시간 배율 복구 --
    if (UWorld* World = GetWorld()) {
        World->GetTimerManager().ClearAllTimersForObject(this);
        UGameplayStatics::SetGlobalTimeDilation(World, 1.f);
    }
}