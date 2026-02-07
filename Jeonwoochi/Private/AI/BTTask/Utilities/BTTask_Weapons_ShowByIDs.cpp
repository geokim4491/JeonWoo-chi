// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask/Utilities/BTTask_Weapons_ShowByIDs.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"

#include "Weapon/BaseWeaponActor.h"

// ===== UBTTask_Weapons_ShowByIDs =====

UBTTask_Weapons_ShowByIDs::UBTTask_Weapons_ShowByIDs()
{
    NodeName = TEXT("Weapons: Show By IDs (Only)");
    bNotifyTick = false; // 이 태스크는 즉시 적용 후 성공
}

TSet<FName> UBTTask_Weapons_ShowByIDs::NormalizeIDs(const TArray<FName>& In)
{
    TSet<FName> Out;
    for (const FName& N : In)
        if (!N.IsNone()) Out.Add(N);
    return Out;
}

void UBTTask_Weapons_ShowByIDs::CollectWeaponsAndSnapshot(APawn* OwnerPawn, FShowByIDsMemory* Mem) const
{
    Mem->Snapshots.Reset();

    if (!OwnerPawn) return;

    TArray<AActor*> Children;
    OwnerPawn->GetAttachedActors(Children);

    for (AActor* A : Children)
    {
        if (ABaseWeaponActor* W = Cast<ABaseWeaponActor>(A))
        {
            FWeaponVisSnapshot_Show S;
            S.Weapon = W;
            S.bPrevHidden = W->IsHidden();
            Mem->Snapshots.Add(S);
        }
    }
}

TArray<TWeakObjectPtr<ABaseWeaponActor>> UBTTask_Weapons_ShowByIDs::PickByWeaponIDs(
    const TArray<FWeaponVisSnapshot_Show>& Snapshots,
    const TSet<FName>& IDs) const
{
    TArray<TWeakObjectPtr<ABaseWeaponActor>> Result;
    if (IDs.Num() == 0) return Result;

    for (const FWeaponVisSnapshot_Show& S : Snapshots)
    {
        if (ABaseWeaponActor* W = S.Weapon.Get())
        {
            if (IDs.Contains(W->WeaponID))
            {
                Result.Add(W);
            }
        }
    }
    return Result;
}

void UBTTask_Weapons_ShowByIDs::ForceHideWeapon(ABaseWeaponActor* W) const
{
    if (!W) return;

    W->SetActorHiddenInGame(true);

    if (bAlsoToggleComponentVisibility)
    {
        TArray<UPrimitiveComponent*> Prims;
        W->GetComponents<UPrimitiveComponent>(Prims);
        for (UPrimitiveComponent* C : Prims)
        {
            if (!C) continue;
            C->SetHiddenInGame(true);
            C->SetVisibility(false, true);
        }
    }
}

void UBTTask_Weapons_ShowByIDs::ForceShowWeapon(ABaseWeaponActor* W) const
{
    if (!W) return;

    W->SetActorHiddenInGame(false);

    if (bAlsoToggleComponentVisibility)
    {
        TArray<UPrimitiveComponent*> Prims;
        W->GetComponents<UPrimitiveComponent>(Prims);
        for (UPrimitiveComponent* C : Prims)
        {
            if (!C) continue;
            C->SetHiddenInGame(false);
            C->SetVisibility(true, true);
            // 메시 충돌은 필요시 별도 프로파일로
        }
    }
}

void UBTTask_Weapons_ShowByIDs::ApplySelection(
    const TArray<FWeaponVisSnapshot_Show>& Snapshots,
    const TArray<TWeakObjectPtr<ABaseWeaponActor>>& SelectedSet) const
{
    // 0) 모두 숨김 
    for (const FWeaponVisSnapshot_Show& S : Snapshots)
        if (ABaseWeaponActor* W = S.Weapon.Get())
            ForceHideWeapon(W);

    // 1) 선택된 것들만 보이기 
    for (const auto& Ptr : SelectedSet)
        if (ABaseWeaponActor* W = Ptr.Get())
            ForceShowWeapon(W);
}

void UBTTask_Weapons_ShowByIDs::Restore(const TArray<FWeaponVisSnapshot_Show>& Snapshots) const
{
    for (const FWeaponVisSnapshot_Show& S : Snapshots)
    {
        if (ABaseWeaponActor* W = S.Weapon.Get())
        {
            if (bRestoreVisibilityOnExit)
            {
                W->SetActorHiddenInGame(S.bPrevHidden);

                if (bAlsoToggleComponentVisibility)
                {
                    TArray<UPrimitiveComponent*> Prims;
                    W->GetComponents<UPrimitiveComponent>(Prims);
                    for (UPrimitiveComponent* C : Prims)
                    {
                        if (!C) continue;
                        C->SetHiddenInGame(S.bPrevHidden);
                        C->SetVisibility(!S.bPrevHidden, true);
                    }
                }
            }
        }
    }
}

UPrimitiveComponent* UBTTask_Weapons_ShowByIDs::GetWeaponHitShape(ABaseWeaponActor* W) const
{
    if (!W) return nullptr;
    if (UHitDetectionComponent* HDC = W->GetHitDetectionComp())
    {
        return HDC->HitShape;
    }
    return nullptr;
}

EBTNodeResult::Type UBTTask_Weapons_ShowByIDs::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* Mem = reinterpret_cast<FShowByIDsMemory*>(NodeMemory);
    Mem->Snapshots.Reset();
    Mem->SelectedSet.Reset();

    AAIController* AI = OwnerComp.GetAIOwner();
    APawn* Pawn = AI ? AI->GetPawn() : nullptr;
    if (!Pawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ShowByIDs] Pawn null"));
        return EBTNodeResult::Failed;
    }

    // 무기 수집 + 스냅샷
    CollectWeaponsAndSnapshot(Pawn, Mem);

    const TSet<FName> IDs = NormalizeIDs(DesiredWeaponIDs);
    Mem->SelectedSet = PickByWeaponIDs(Mem->Snapshots, IDs);

    if (Mem->SelectedSet.Num() == 0)
    {
        if (DesiredWeaponIDs.Num() > 0)
        {
            TArray<FString> Names; Names.Reserve(DesiredWeaponIDs.Num());
            for (const FName& N : DesiredWeaponIDs) Names.Add(N.ToString());

            UE_LOG(LogTemp, Warning,
                TEXT("[ShowByIDs] 지정한 IDs(%s) 중 일치하는 무기가 없습니다."),
                *FString::Join(Names, TEXT(",")));
        }

        if (bFailIfNotFound) return EBTNodeResult::Failed;

        // 못찾아도 “모두 숨김” 상태 유지
        ApplySelection(Mem->Snapshots, Mem->SelectedSet);
        return EBTNodeResult::Succeeded;
    }

    // 전부 숨김 → 선택만 보임(+히트박스 ON)
    ApplySelection(Mem->Snapshots, Mem->SelectedSet);

    // 이 태스크는 즉시 성공 (상태는 유지; 복구는 하지 않음)
    return EBTNodeResult::Succeeded;
}

EBTNodeResult::Type UBTTask_Weapons_ShowByIDs::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* Mem = reinterpret_cast<FShowByIDsMemory*>(NodeMemory);
    if (!Mem) return Super::AbortTask(OwnerComp, NodeMemory);

    if (bHideAllOnAbort && Mem->Snapshots.Num() > 0)
    {
        // 중단 시엔 안전하게 전부 숨김
        for (const FWeaponVisSnapshot_Show& S : Mem->Snapshots)
            if (ABaseWeaponActor* W = S.Weapon.Get())
                ForceHideWeapon(W);
    }
    else
    {
        // 원하면 스냅샷대로 되돌리기
        if (bRestoreVisibilityOnExit)
            Restore(Mem->Snapshots);
    }

    return Super::AbortTask(OwnerComp, NodeMemory);
}