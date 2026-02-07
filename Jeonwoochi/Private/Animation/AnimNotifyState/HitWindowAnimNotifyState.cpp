// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimNotifyState/HitWindowAnimNotifyState.h"
#include "../../../Public/Weapon/BaseWeaponActor.h"
#include "../../../Public/Components/Utilities/WeaponManageComponent.h"

void UHitWindowAnimNotifyState::ProcessHitWindow(
    USkeletalMeshComponent* MeshComp,
    bool bStart
)
{
    if (!MeshComp) return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    TArray<ABaseWeaponActor*> Weapons;

    /* 1 WeaponManageComponent 경유 */
    if (auto* WManage = Owner->FindComponentByClass<UWeaponManageComponent>())
    {
        Weapons.Append(WManage->GetEquippedWeapons());
    }

    /* 2 AActor::GetAttachedActors() 경유 (전통적 부착) */
    TArray<AActor*> Attached;
    Owner->GetAttachedActors(Attached);
    for (AActor* A : Attached)
        if (auto WA = Cast<ABaseWeaponActor>(A))
            Weapons.AddUnique(WA);

    /* 3 ChildActorComponent 경유 ── 클론·소환수 무기 대응 */
    TArray<UChildActorComponent*> ChildComps;
    Owner->GetComponents<UChildActorComponent>(ChildComps);
    for (UChildActorComponent* CAC : ChildComps)
        if (auto WA = Cast<ABaseWeaponActor>(CAC->GetChildActor()))
            Weapons.AddUnique(WA);

    /* 필터링 (TargetWeaponName) */
    TArray<ABaseWeaponActor*> Targets;
    if (TargetWeaponName != NAME_None)
    {
        for (ABaseWeaponActor* W : Weapons)
            if (W && W->WeaponID == TargetWeaponName)
                Targets.Add(W);
    }
    else
    {
        Targets = Weapons;
    }

    /* HitWindow 호출 */
    for (ABaseWeaponActor* W : Targets)
    {
        if (!W) continue;
        if (auto* HitComp = W->FindComponentByClass<UHitDetectionComponent>())
        {
            bStart ? HitComp->StartHitWindow()
                : HitComp->EndHitWindow();
        }
    }
}

void UHitWindowAnimNotifyState::NotifyBegin(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float TotalDuration
)
{
    ProcessHitWindow(MeshComp, /*bStart=*/true);
}

void UHitWindowAnimNotifyState::NotifyEnd(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation
)
{
    ProcessHitWindow(MeshComp, /*bStart=*/false);
}
