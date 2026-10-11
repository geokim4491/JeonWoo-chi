#include "Characters/AI/CloneAICharacter.h"

#include "Components/ChildActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GroomComponent.h"

#include "Components/Combat/HitDetectionComponent.h"


ACloneAICharacter::ACloneAICharacter()
{
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	RagdollTime = 1.f;
	CharacterType = ECharacterType::Companion;
}




void ACloneAICharacter::CopyAppearanceFrom(ACharacter* Source)
{
    if (!Source)
        return;

    USkeletalMeshComponent* SrcMain = Source->GetMesh();
    USkeletalMeshComponent* CloneBodyMesh = GetMesh();

    if (!SrcMain || !CloneBodyMesh)
        return;

    // 1. 메인 Skeletal Mesh 복제
    CloneBodyMesh->SetSkeletalMeshAsset(
        SrcMain->GetSkeletalMeshAsset());

    for (int32 i = 0; i < SrcMain->GetNumMaterials(); ++i)
    {
        CloneBodyMesh->SetMaterial(
            i, SrcMain->GetMaterial(i));
    }

    // 2. Sub Skeletal Mesh 생성 및 부착
    TInlineComponentArray<USkeletalMeshComponent*> SrcSkels;
    Source->GetComponents(SrcSkels);

    using FSkelPair = TPair<
        USkeletalMeshComponent*,
        USkeletalMeshComponent*
    >;

    TArray<FSkelPair> SkelPairs;
    SkelPairs.Reserve(SrcSkels.Num());
    SkelPairs.Emplace(SrcMain, CloneBodyMesh);

    for (USkeletalMeshComponent* SrcSkel : SrcSkels)
    {
        if (!SrcSkel || SrcSkel == SrcMain)
            continue;

        const FName CloneName(
            *FString::Printf(
                TEXT("%s_Clone"), *SrcSkel->GetName()));

        USkeletalMeshComponent* CloneSkel =
            NewObject<USkeletalMeshComponent>(
                this,
                USkeletalMeshComponent::StaticClass(),
                CloneName);

        if (!CloneSkel)
            continue;

        CloneSkel->SetSkeletalMeshAsset(
            SrcSkel->GetSkeletalMeshAsset());

        for (int32 i = 0; i < SrcSkel->GetNumMaterials(); ++i)
        {
            CloneSkel->SetMaterial(
                i, SrcSkel->GetMaterial(i));
        }

        // 모든 Sub Skeletal Mesh는 메인 메시 직속이라고 가정
        CloneSkel->SetupAttachment(
            CloneBodyMesh,
            SrcSkel->GetAttachSocketName());

        CloneSkel->SetRelativeTransform(
            SrcSkel->GetRelativeTransform());

        CloneSkel->RegisterComponent();

        // Groom / 무기 부모 탐색에 사용할 매핑
        SkelPairs.Emplace(SrcSkel, CloneSkel);
    }

    // 3. 원본 부모에 대응하는 복제 부모 탐색
    auto ResolveCloneParent =
        [&](USceneComponent* SrcParent) -> USceneComponent*
        {
            for (const FSkelPair& Pair : SkelPairs)
            {
                if (Pair.Key == SrcParent)
                    return Pair.Value;
            }

            return CloneBodyMesh;
        };

    // 4. Skeleton에 따른 Pose 동기화
    const USkeleton* MainSkeleton =
        CloneBodyMesh->GetSkeletalMeshAsset()
        ? CloneBodyMesh->GetSkeletalMeshAsset()->GetSkeleton()
        : nullptr;

    for (const FSkelPair& Pair : SkelPairs)
    {
        USkeletalMeshComponent* CloneSkel = Pair.Value;

        if (CloneSkel == CloneBodyMesh)
            continue;

        const USkeleton* FollowerSkeleton =
            CloneSkel->GetSkeletalMeshAsset()
            ? CloneSkel->GetSkeletalMeshAsset()->GetSkeleton()
            : nullptr;

        const bool bSameSkeleton =
            MainSkeleton &&
            FollowerSkeleton &&
            MainSkeleton == FollowerSkeleton;

        if (bSameSkeleton)
        {
            CloneSkel->SetLeaderPoseComponent(CloneBodyMesh);
            CloneSkel->bUseBoundsFromLeaderPoseComponent = true;
        }
        else if (ClothRetargetABP)
        {
            CloneSkel->SetAnimationMode(
                EAnimationMode::AnimationBlueprint);

            CloneSkel->SetAnimInstanceClass(
                ClothRetargetABP);
        }
    }

    // 5. Groom 복제
    TInlineComponentArray<UGroomComponent*> SrcGrooms;
    Source->GetComponents(SrcGrooms);

    for (UGroomComponent* SrcGroom : SrcGrooms)
    {
        if (!SrcGroom)
            continue;

        const FName CloneName(
            *FString::Printf(
                TEXT("%s_Clone"), *SrcGroom->GetName()));

        UGroomComponent* CloneGroom =
            DuplicateObject<UGroomComponent>(
                SrcGroom,
                this,
                CloneName);

        if (!CloneGroom)
            continue;

        USceneComponent* CloneParent =
            ResolveCloneParent(
                SrcGroom->GetAttachParent());

        CloneGroom->AttachToComponent(
            CloneParent,
            FAttachmentTransformRules::KeepRelativeTransform,
            SrcGroom->GetAttachSocketName());

        CloneGroom->RegisterComponent();
    }

    // 6. ChildActorComponent 복제
    TInlineComponentArray<UChildActorComponent*> SrcChildActors;
    Source->GetComponents(SrcChildActors);

    for (UChildActorComponent* SrcChildActor : SrcChildActors)
    {
        if (!SrcChildActor)
            continue;

        const FName CloneName(
            *FString::Printf(
                TEXT("%s_Clone"), *SrcChildActor->GetName()));

        UChildActorComponent* CloneChildActor =
            NewObject<UChildActorComponent>(
                this,
                UChildActorComponent::StaticClass(),
                CloneName);

        if (!CloneChildActor)
            continue;

        CloneChildActor->SetChildActorClass(
            SrcChildActor->GetChildActorClass());

        CloneChildActor->SetRelativeTransform(
            SrcChildActor->GetRelativeTransform());

        USceneComponent* CloneParent =
            ResolveCloneParent(
                SrcChildActor->GetAttachParent());

        CloneChildActor->SetupAttachment(
            CloneParent,
            SrcChildActor->GetAttachSocketName());

        CloneChildActor->RegisterComponent();
        CloneChildActor->CreateChildActor();

        AActor* DestChild = CloneChildActor->GetChildActor();

        if (!DestChild)
            continue;

        DestChild->SetOwner(this);

        TArray<UHitDetectionComponent*> HitDetectionComponents;
        DestChild->GetComponents(HitDetectionComponents);

        for (UHitDetectionComponent* HitDetection :
            HitDetectionComponents)
        {
            if (HitDetection)
            {
                HitDetection->RefreshOwnerPointers();
            }
        }
    }
}
