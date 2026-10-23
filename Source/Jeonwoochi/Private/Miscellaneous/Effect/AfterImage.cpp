// Fill out your copyright notice in the Description page of Project Settings.


#include "Miscellaneous/Effect/AfterImage.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ChildActorComponent.h"

#if __has_include("GroomComponent.h")
#include "GroomComponent.h"      // HairStrandsCore 모듈 필요
#define HAS_GROOM 1
#else
#define HAS_GROOM 0
#endif

// 의상/바디용 Skeletal Follower 생성 (Leader=Poseable)
static USkeletalMeshComponent* SpawnFollowerSkel(
    AActor* Owner, USkeletalMeshComponent* Src,
    USkinnedMeshComponent* Leader, UMaterialInterface* Overlay, USceneComponent* Parent)
{
    if(!Owner || !Src || !Src->GetSkeletalMeshAsset() || !Leader) return nullptr;

    auto* NewSkel = NewObject<USkeletalMeshComponent>(Owner, USkeletalMeshComponent::StaticClass(),
        FName(*FString::Printf(TEXT("AF_%s"), *Src->GetName())));
    NewSkel->RegisterComponent();

    NewSkel->SetWorldTransform(Src->GetComponentTransform());
    NewSkel->SetSkeletalMeshAsset(Src->GetSkeletalMeshAsset());

    // LeaderPose로 본/커브 동기화
    NewSkel->SetLeaderPoseComponent(Leader);
    NewSkel->bUseBoundsFromLeaderPoseComponent = true;

    // 항상 뼈/바운드 갱신 (중요)
    NewSkel->SetComponentTickEnabled(true);
    NewSkel->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

    if (Overlay) NewSkel->SetOverlayMaterial(Overlay);
    NewSkel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    NewSkel->SetReceivesDecals(false);
    NewSkel->bCastDynamicShadow = Src->bCastDynamicShadow;

    NewSkel->AttachToComponent(Parent, FAttachmentTransformRules::KeepWorldTransform);

    // 즉시 렌더 업데이트
    NewSkel->UpdateBounds();
    NewSkel->MarkRenderTransformDirty();
    NewSkel->MarkRenderDynamicDataDirty();
    return NewSkel;
}

AAfterImage::AAfterImage()
{
    PrimaryActorTick.bCanEverTick = false;

    // 포즈블 메시 컴포넌트 생성 & 루트로 설정
    PoseableMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PoseableMesh"));
    RootComponent = PoseableMesh;
}

void AAfterImage::Initialize(
    USkeletalMeshComponent* InSourceMesh,
    float InLifeSpan,
    UMaterialInterface* OverlayMaterial)
{
    SourceMesh = InSourceMesh;
    LifeSpan = InLifeSpan;

    if (!SourceMesh || !PoseableMesh) { SetLifeSpan(LifeSpan); return; }
    AActor* OwnerActor = SourceMesh->GetOwner();
    if (!OwnerActor) { SetLifeSpan(LifeSpan); return; }

    SnapshotMap.Empty();

    // 1) 바디를 Poseable로 '정지 스냅샷'
    PoseableMesh->RegisterComponent();
    PoseableMesh->SetWorldTransform(SourceMesh->GetComponentTransform());
    PoseableMesh->SetSkeletalMesh(SourceMesh->GetSkeletalMeshAsset());
    PoseableMesh->CopyPoseFromSkeletalComponent(SourceMesh);
    if (OverlayMaterial) PoseableMesh->SetOverlayMaterial(OverlayMaterial);
    PoseableMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PoseableMesh->SetReceivesDecals(false);
    PoseableMesh->UpdateBounds();
    PoseableMesh->MarkRenderTransformDirty();
    PoseableMesh->MarkRenderDynamicDataDirty();

    // 2) 바디용 Skeletal Follower (소켓/그룸의 부모 역할)
    BodyFollower = SpawnFollowerSkel(this, SourceMesh, /*Leader=*/PoseableMesh, OverlayMaterial, RootComponent);
    SnapshotMap.Add(SourceMesh, BodyFollower ? static_cast<USceneComponent*>(BodyFollower)
        : static_cast<USceneComponent*>(PoseableMesh));

    // 3) 의상 등 하위 스켈메시 → 각각 Skeletal Follower 생성 (Leader는 Poseable)
    {
        TInlineComponentArray<USkeletalMeshComponent*> Skels(OwnerActor);
        for (USkeletalMeshComponent* SrcSkel : Skels)
        {
            if (!SrcSkel || SrcSkel == SourceMesh || !SrcSkel->GetSkeletalMeshAsset()) continue;

            USkeletalMeshComponent* Follower =
                SpawnFollowerSkel(this, SrcSkel, /*Leader=*/PoseableMesh, OverlayMaterial, RootComponent);

            SnapshotMap.Add(SrcSkel, Follower ? static_cast<USceneComponent*>(Follower)
                : static_cast<USceneComponent*>(PoseableMesh));
        }
    }

    // 4) ChildActor(Stick 등) → 원래 부모 스켈메시의 '스냅샷 컴포넌트'에 소켓으로 부착
    {
        TArray<UChildActorComponent*> SrcChildComps;
        OwnerActor->GetComponents<UChildActorComponent>(SrcChildComps);

        for (UChildActorComponent* SrcCAC : SrcChildComps)
        {
            if (!SrcCAC) continue;

            USceneComponent* TargetSnap = BodyFollower ? static_cast<USceneComponent*>(BodyFollower)
                : static_cast<USceneComponent*>(PoseableMesh);

            if (USkeletalMeshComponent* OrigParentSkel = Cast<USkeletalMeshComponent>(SrcCAC->GetAttachParent()))
            {
                if (USceneComponent** Found = SnapshotMap.Find(OrigParentSkel))
                    TargetSnap = *Found;
            }

            UChildActorComponent* NewCAC =
                NewObject<UChildActorComponent>(this, UChildActorComponent::StaticClass());
            NewCAC->RegisterComponent();

            // 소켓 정밀 부착
            NewCAC->AttachToComponent(
                TargetSnap,
                FAttachmentTransformRules::SnapToTargetIncludingScale,
                SrcCAC->GetAttachSocketName());

            NewCAC->SetChildActorClass(SrcCAC->GetChildActorClass());
            NewCAC->CreateChildActor();
        }
    }

    // 5) Groom(머리카락/눈썹) → 반드시 Skeletal(BodyFollower/의상 Follower)에 부착
#if HAS_GROOM
    {
        TInlineComponentArray<UGroomComponent*> SrcGrooms(OwnerActor);
        for (UGroomComponent* SrcG : SrcGrooms)
        {
            if (!SrcG) continue;

            USceneComponent* TargetSnap = BodyFollower ? static_cast<USceneComponent*>(BodyFollower)
                : static_cast<USceneComponent*>(PoseableMesh);

            if (USkeletalMeshComponent* OrigParentSkel = Cast<USkeletalMeshComponent>(SrcG->GetAttachParent()))
            {
                if (USceneComponent** Found = SnapshotMap.Find(OrigParentSkel))
                    TargetSnap = *Found;
            }

            // Groom은 Duplicate로 세팅 복사
            UGroomComponent* NewG =
                DuplicateObject<UGroomComponent>(SrcG, this,
                    FName(*FString::Printf(TEXT("%s_AfterImage"), *SrcG->GetName())));

            // Skeletal에 소켓 부착 (Poseable 대신 Skeletal에 붙여야 바인딩/소켓이 안정적)
            NewG->AttachToComponent(
                TargetSnap,
                FAttachmentTransformRules::SnapToTargetIncludingScale,
                SrcG->GetAttachSocketName());

            NewG->PrimaryComponentTick.bCanEverTick = false;
            NewG->RegisterComponent();
        }
    }
#endif

    SetLifeSpan(LifeSpan);
}

void AAfterImage::BeginPlay()
{
    Super::BeginPlay();

    // Spawn 직후 Initialize()가 호출되지 않았을 경우 안전장치
    if (SourceMesh && PoseableMesh)
    {
        PoseableMesh->SetSkeletalMesh(SourceMesh->GetSkeletalMeshAsset());
        PoseableMesh->CopyPoseFromSkeletalComponent(SourceMesh);
    }
}