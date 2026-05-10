// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/Save/SaveAgentComponent.h"
#include "GameFramework/Actor.h"

void USaveAgentComponent::OnRegister()
{
    Super::OnRegister();

#if WITH_EDITOR
    if (GetWorld() && !GetWorld()->IsGameWorld())
    {
        if (!PersistentId.IsValid())
        {
            Modify();                        // 레벨에 변경사항 기록
            PersistentId = FGuid::NewGuid(); // 고정 GUID 발급
            MarkPackageDirty();
        }
    }
#endif

    if (!SpawnClass.IsValid() && GetOwner())
        SpawnClass = GetOwner()->GetClass();

    if (EditorTransform.Equals(FTransform::Identity) && GetOwner())
        EditorTransform = GetOwner()->GetActorTransform();
}

void USaveAgentComponent::CaptureFromActor()
{
    if (AActor* Owner = GetOwner())
        EditorTransform = Owner->GetActorTransform();
}
