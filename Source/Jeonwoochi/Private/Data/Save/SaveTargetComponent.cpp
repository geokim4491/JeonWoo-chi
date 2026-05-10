// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/Save/SaveTargetComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "AI/Miscellaneous/PatrolRoute.h"
#include "Components/Combat/HealthComponent.h"
#include "Components/Combat/PostureComponent.h"
#include "EngineUtils.h"

FSaveTargetConfig USaveTargetComponent::MakeConfig() const
{
    FSaveTargetConfig C;
    C.bIsEnemy = bIsEnemy;
    C.bIsBoss = bIsBoss;
    C.BossId = BossId;
    C.BossDefeatedTag = BossDefeatedTag;
    C.EditorTransform = EditorTransform;
    C.SpawnClass = SpawnClass;

    if (AActor* Owner = GetOwner())
    {
        // 1) SpawnMontage
        if (FObjectPropertyBase* P = FindFProperty<FObjectPropertyBase>(Owner->GetClass(), TEXT("SpawnMontage")))
        {
            if (P->PropertyClass->IsChildOf(UAnimMontage::StaticClass()))
                if (UObject* V = P->GetObjectPropertyValue_InContainer(Owner))
                    C.SpawnMontage = Cast<UAnimMontage>(V);
        }

        // 2) AssignedRoute(Soft + 이름 폴백)
        if (FObjectPropertyBase* P = FindFProperty<FObjectPropertyBase>(Owner->GetClass(), TEXT("AssignedRoute")))
        {
            if (P->PropertyClass->IsChildOf(APatrolRoute::StaticClass()))
            {
                if (UObject* V = P->GetObjectPropertyValue_InContainer(Owner))
                {
                    APatrolRoute* R = Cast<APatrolRoute>(V);
                    C.AssignedRouteSoft = R;
                    C.AssignedRouteByName = R ? R->GetFName() : NAME_None;
                }
            }
        }

        // 3) Instanced ConfigData → 템플릿 복제 저장
        if (FObjectPropertyBase* P = FindFProperty<FObjectPropertyBase>(Owner->GetClass(), TEXT("ConfigData")))
        {
            if (P->PropertyClass->IsChildOf(UAIConfigData::StaticClass()))
            {
                if (UAIConfigData* Src = Cast<UAIConfigData>(P->GetObjectPropertyValue_InContainer(Owner)))
                {
                    C.ConfigDataTemplate = DuplicateObject<UAIConfigData>(Src, const_cast<USaveTargetComponent*>(this));
                }
            }
        }

        // 4) Health/Posture Max
        if (const UHealthComponent* HC = Owner->FindComponentByClass<UHealthComponent>())
            C.SeedMaxHealth = HC->GetMaxHealth();

        if (const UPostureComponent* PC = Owner->FindComponentByClass<UPostureComponent>())
            C.SeedMaxPostureCount = PC->GetMaxPosture();
    }
    return C;
}


void USaveTargetComponent::ApplyConfig(const FSaveTargetConfig& C)
{
    bIsEnemy = C.bIsEnemy;
    bIsBoss = C.bIsBoss;
    BossId = C.BossId;
    BossDefeatedTag = C.BossDefeatedTag;
    EditorTransform = C.EditorTransform;
    SpawnClass = C.SpawnClass;
}


void USaveTargetComponent::ApplyConfigToOwnerExtras(const FSaveTargetConfig& C)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // (주의) 트랜스폼은 리스폰 루틴에서 '한 번만' 절대값으로 적용하세요.
    // 여기서는 에디터 설정만 Owner 쪽에 반영.

    // 1) SpawnMontage
    if (FObjectPropertyBase* P = FindFProperty<FObjectPropertyBase>(Owner->GetClass(), TEXT("SpawnMontage")))
    {
        if (P->PropertyClass->IsChildOf(UAnimMontage::StaticClass()))
        {
            UAnimMontage* M = C.SpawnMontage.IsNull() ? nullptr : C.SpawnMontage.LoadSynchronous();
            P->SetObjectPropertyValue_InContainer(Owner, M);
        }
    }

    // 2) AssignedRoute (soft → 실패 시 이름 폴백)
    if (FObjectPropertyBase* P = FindFProperty<FObjectPropertyBase>(Owner->GetClass(), TEXT("AssignedRoute")))
    {
        if (P->PropertyClass->IsChildOf(APatrolRoute::StaticClass()))
        {
            APatrolRoute* Route = C.AssignedRouteSoft.Get();
            if (!Route && C.AssignedRouteByName != NAME_None)
            {
                for (TActorIterator<APatrolRoute> It(Owner->GetWorld()); It; ++It)
                    if (It->GetFName() == C.AssignedRouteByName) { Route = *It; break; }
            }
            P->SetObjectPropertyValue_InContainer(Owner, Route);
        }
    }

    // 3) ConfigData(Instanced) 새 Owner로 복제 후 꽂기
    if (FObjectPropertyBase* P = FindFProperty<FObjectPropertyBase>(Owner->GetClass(), TEXT("ConfigData")))
    {
        if (P->PropertyClass->IsChildOf(UAIConfigData::StaticClass()))
        {
            UAIConfigData* NewCfg = C.ConfigDataTemplate ? DuplicateObject<UAIConfigData>(C.ConfigDataTemplate, Owner) : nullptr;
            P->SetObjectPropertyValue_InContainer(Owner, NewCfg);
        }
    }

    // 4) Health/Posture Max → 컴포넌트에 적용(+ Current 초기화)
    if (UHealthComponent* HC = Owner->FindComponentByClass<UHealthComponent>())
    {
        if (C.SeedMaxHealth > 0.f) HC->SetMaxHealth(C.SeedMaxHealth);
        HC->SetCurrentHealth(HC->GetMaxHealth());
    }
    if (UPostureComponent* PC = Owner->FindComponentByClass<UPostureComponent>())
    {
        if (C.SeedMaxPostureCount > 0) PC->SetMaxPosture(C.SeedMaxPostureCount);
        PC->ResetCurrentPosture();
    }
}

void USaveTargetComponent::CaptureFromActor()
{
    if (AActor* Owner = GetOwner())
    {
        EditorTransform = Owner->GetActorTransform();
        bEditorTransformCaptured = true;

        // BossId가 있으면 태그 계산(등록되어 있지 않으면 Invalid일 수 있지만 OK)
        if (bIsBoss && BossId != NAME_None)
        {
            const FString TagStr = FString::Printf(TEXT("boss.%s.defeated"),
                *BossId.ToString().ToLower());
            BossDefeatedTag = FGameplayTag::RequestGameplayTag(
                FName(*TagStr), /*ErrorIfNotFound*/false);
        }
    }
}

void USaveTargetComponent::OnRegister()
{
    Super::OnRegister();

    // GUID 부여
    if (!PersistentId.IsValid())
        PersistentId = FGuid::NewGuid();

    // 스폰 클래스 기본값
    if (!SpawnClass.IsValid())
        if (const AActor* Owner = GetOwner())
            SpawnClass = Owner->GetClass();

    // 런타임/패키지에서도, 아직 캡처 안 되어 있으면 한 번 캡처
    if (!bEditorTransformCaptured)
    {
        if (AActor* Owner = GetOwner())
        {
            EditorTransform = Owner->GetActorTransform();
            bEditorTransformCaptured = true;
        }
    }

    // 보스 태그 자동 계산
    if (bIsBoss && !BossDefeatedTag.IsValid() && BossId != NAME_None)
    {
        const FString TagStr = FString::Printf(TEXT("boss.%s.defeated"),
            *BossId.ToString().ToLower());
        BossDefeatedTag = FGameplayTag::RequestGameplayTag(
            FName(*TagStr), /*ErrorIfNotFound*/false);
    }
}