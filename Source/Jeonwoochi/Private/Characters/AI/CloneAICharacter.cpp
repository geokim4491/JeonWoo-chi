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
	{
		return;
	}

	USkeletalMeshComponent* SrcMain = Source->GetMesh();
	USkeletalMeshComponent* CloneBodyMesh = GetMesh();

	if (!SrcMain || !CloneBodyMesh)
	{
		return;
	}

	// 1. 메인 Skeletal Mesh 복제
	CloneBodyMesh->SetSkeletalMeshAsset(SrcMain->GetSkeletalMeshAsset());

	for (int32 MaterialIndex = 0;
		MaterialIndex < SrcMain->GetNumMaterials();
		++MaterialIndex)
	{
		CloneBodyMesh->SetMaterial(
			MaterialIndex,
			SrcMain->GetMaterial(MaterialIndex));
	}

	// 2. Sub Skeletal Mesh 생성
	TInlineComponentArray<USkeletalMeshComponent*> SrcSkels;
	Source->GetComponents(SrcSkels);

	TMap<USkeletalMeshComponent*, USkeletalMeshComponent*> SkelMap;
	SkelMap.Add(SrcMain, CloneBodyMesh);

	for (USkeletalMeshComponent* SrcSkel : SrcSkels)
	{
		if (!SrcSkel || SrcSkel == SrcMain)
		{
			continue;
		}

		const FName CloneName(
			*FString::Printf(TEXT("%s_Clone"), *SrcSkel->GetName()));

		USkeletalMeshComponent* CloneSkel =
			NewObject<USkeletalMeshComponent>(
				this,
				USkeletalMeshComponent::StaticClass(),
				CloneName);

		if (!CloneSkel)
		{
			continue;
		}

		CloneSkel->SetSkeletalMeshAsset(
			SrcSkel->GetSkeletalMeshAsset());

		for (int32 MaterialIndex = 0;
			MaterialIndex < SrcSkel->GetNumMaterials();
			++MaterialIndex)
		{
			CloneSkel->SetMaterial(
				MaterialIndex,
				SrcSkel->GetMaterial(MaterialIndex));
		}

		CloneSkel->SetRelativeTransform(
			SrcSkel->GetRelativeTransform());

		CloneSkel->SetupAttachment(GetRootComponent());
		CloneSkel->RegisterComponent();

		SkelMap.Add(SrcSkel, CloneSkel);
	}

	// 원본 Component의 부모 관계를 복제본에 대응시킴
	auto ResolveCloneParent =
		[&](USceneComponent* SrcParent) -> USceneComponent*
		{
			if (USkeletalMeshComponent* SrcParentSkel =
				Cast<USkeletalMeshComponent>(SrcParent))
			{
				if (USkeletalMeshComponent** Found =
					SkelMap.Find(SrcParentSkel))
				{
					return *Found;
				}
			}

			return CloneBodyMesh;
		};

	// 3. 원본과 같은 부모 / Socket으로 재부착
	for (const auto& Pair : SkelMap)
	{
		USkeletalMeshComponent* SrcSkel = Pair.Key;
		USkeletalMeshComponent* CloneSkel = Pair.Value;

		if (!CloneSkel || CloneSkel == CloneBodyMesh)
		{
			continue;
		}

		USceneComponent* CloneParent =
			ResolveCloneParent(SrcSkel->GetAttachParent());

		CloneSkel->DetachFromComponent(
			FDetachmentTransformRules::KeepRelativeTransform);

		CloneSkel->AttachToComponent(
			CloneParent,
			FAttachmentTransformRules::KeepRelativeTransform,
			SrcSkel->GetAttachSocketName());
	}

	// 4. Skeleton 구조에 따라 Pose 동기화 방식 분기
	const USkeleton* MainSkeleton =
		CloneBodyMesh->GetSkeletalMeshAsset()
		? CloneBodyMesh->GetSkeletalMeshAsset()->GetSkeleton()
		: nullptr;

	for (const auto& Pair : SkelMap)
	{
		USkeletalMeshComponent* CloneSkel = Pair.Value;

		if (!CloneSkel || CloneSkel == CloneBodyMesh)
		{
			continue;
		}

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
			CloneSkel->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			CloneSkel->SetAnimInstanceClass(ClothRetargetABP);
		}

	}

	// 5. Groom 복제
	TInlineComponentArray<UGroomComponent*> SrcGrooms;
	Source->GetComponents(SrcGrooms);

	for (UGroomComponent* SrcGroom : SrcGrooms)
	{
		if (!SrcGroom)
		{
			continue;
		}

		const FName CloneName(
			*FString::Printf(
				TEXT("%s_Clone"),
				*SrcGroom->GetName()));

		UGroomComponent* CloneGroom =
			DuplicateObject<UGroomComponent>(
				SrcGroom,
				this,
				CloneName);

		if (!CloneGroom)
		{
			continue;
		}

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
		{
			continue;
		}

		const FName CloneName(
			*FString::Printf(
				TEXT("%s_Clone"),
				*SrcChildActor->GetName()));

		UChildActorComponent* CloneChildActor =
			NewObject<UChildActorComponent>(
				this,
				UChildActorComponent::StaticClass(),
				CloneName);

		if (!CloneChildActor)
		{
			continue;
		}

		CloneChildActor->SetChildActorClass(
			SrcChildActor->GetChildActorClass());

		CloneChildActor->SetRelativeTransform(
			SrcChildActor->GetRelativeTransform());

		USceneComponent* CloneParent =
			ResolveCloneParent(
				SrcChildActor->GetAttachParent());

		CloneChildActor->AttachToComponent(
			CloneParent,
			FAttachmentTransformRules::KeepRelativeTransform,
			SrcChildActor->GetAttachSocketName());

		CloneChildActor->RegisterComponent();
		CloneChildActor->CreateChildActor();

		AActor* DestChild =
			CloneChildActor->GetChildActor();

		if (!DestChild)
		{
			continue;
		}

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