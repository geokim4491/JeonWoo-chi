#include "Characters/AI/CloneAICharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/MeshComponent.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSingleNodeInstance.h"
// Groom
#if __has_include("GroomComponent.h")
#include "GroomComponent.h"      // HairStrandsCore 모듈 필요
#define HAS_GROOM 1
#else
#define HAS_GROOM 0
#endif

// 프로젝트 종속(있으면 그대로 유지)
#include "Components/Combat/HitDetectionComponent.h"

ACloneAICharacter::ACloneAICharacter()
{
	// 기존 매니퀸 보정값은 그대로 두어도 되지만,
	// 아래 CopyAppearanceFrom에서 소스 메쉬의 상대 트랜스폼으로 덮어쓴다.
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	RagdollTime = 1.f;
}

static void CopyAnimSetup(USkeletalMeshComponent* Dest, USkeletalMeshComponent* Src)
{
	if (!Dest || !Src) return;

	const EAnimationMode::Type Mode = Src->GetAnimationMode();
	Dest->SetAnimationMode(Mode);

	if (Mode == EAnimationMode::AnimationBlueprint)
	{
		if (UClass* AnimBPClass = Src->GetAnimClass())      // non-const OK
		{
			Dest->SetAnimInstanceClass(AnimBPClass);
		}
	}
	else if (Mode == EAnimationMode::AnimationSingleNode)
	{
		if (UAnimSingleNodeInstance* SrcInst = Src->GetSingleNodeInstance())
		{
			UAnimationAsset* Asset = SrcInst->GetAnimationAsset();

			Dest->SetAnimationMode(EAnimationMode::AnimationSingleNode);

			// 인스턴스에 직접 세팅
			if (UAnimSingleNodeInstance* DstInst = Dest->GetSingleNodeInstance())
			{
				DstInst->SetAnimationAsset(Asset);
				// UE5에서는 Loop 설정을 인스턴스로 건다
				// (SetLooping은 공식 API) :contentReference[oaicite:0]{index=0}
				// Loop 상태는 SrcInst에서 읽어 반영 (IsLooping 사용)
				DstInst->SetLooping(SrcInst->IsLooping());
				DstInst->SetPlayRate(SrcInst->GetPlayRate());
				DstInst->SetPosition(SrcInst->GetCurrentTime(), /*bFireNotifies*/ false);
				DstInst->SetPlaying(SrcInst->IsPlaying());
			}
		}
	}

	// 애님 초기화
	Dest->InitAnim(/*bForceRestart=*/true);
}

void ACloneAICharacter::CopyAppearanceFrom(ACharacter* Source)
{
	if (!Source || !GetMesh()) return;

	USkeletalMeshComponent* SrcMain = Source->GetMesh();
	USkeletalMeshComponent* DstMain = GetMesh();
	if (!SrcMain || !DstMain) return;

	// ─────────────────────────────────────────────────────────────
	// 0) 메인 바디 메쉬: 메시/머티리얼 복사 + 안정화(중요)
	// ─────────────────────────────────────────────────────────────
	DstMain->SetSkeletalMeshAsset(SrcMain->GetSkeletalMeshAsset());
	for (int32 i = 0; i < SrcMain->GetNumMaterials(); ++i)
	{
		DstMain->SetMaterial(i, SrcMain->GetMaterial(i));
	}

	// ▶ 옷 튐 방지 핵심: 리더(바디)도 항상 포즈 갱신 + URO 끄기
	DstMain->bEnableUpdateRateOptimizations = false;
	DstMain->VisibilityBasedAnimTickOption =
		EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// 커브/모프를 팔로워로 전파(바운드 튐/LOD 이슈 완화)
	DstMain->bPropagateCurvesToFollowers = true;

	// 팔로워(옷) 안정화 공통 처리 람다
	auto StabilizeFollower = [&](USkeletalMeshComponent* Follower, bool bLeaderPose)
		{
			if (!Follower || Follower == DstMain) return;

			// 리더 먼저 → 팔로워 나중에 틱 (1프레임 래그/덜컹 방지)
			Follower->AddTickPrerequisiteComponent(DstMain);

			// 항상 포즈 갱신 + URO 끄기
			Follower->bEnableUpdateRateOptimizations = false;
			Follower->VisibilityBasedAnimTickOption =
				EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

			// 팔로워는 월드 충돌로 튀지 않게 충돌 비활성
			Follower->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// 바운드 정책
			Follower->bUseBoundsFromLeaderPoseComponent = bLeaderPose; // LeaderPose일 땐 true, CopyPose일 땐 false
			// (선택) 살짝 여유
			// Follower->SetBoundsScale(1.1f);

			// PostProcess Anim 인스턴스가 있으면 떨림 유발하므로 제거
			Follower->SetOverridePostProcessAnimBP(nullptr, /*ReinitAnimInstances=*/true);

			// 첫 프레임 Cloth 폭주 방지
			Follower->ForceClothNextUpdateTeleportAndReset();
		};

	// ─────────────────────────────────────────────────────────────
	// 1) 소스의 모든 스켈레탈 메시 수집 → 1-pass: 서브 생성/등록
	// ─────────────────────────────────────────────────────────────
	TInlineComponentArray<USkeletalMeshComponent*> SrcSkels;
	Source->GetComponents(SrcSkels);

	TMap<USkeletalMeshComponent*, USkeletalMeshComponent*> SkelMap; // 소스→복제
	SkelMap.Add(SrcMain, DstMain);

	for (USkeletalMeshComponent* Src : SrcSkels)
	{
		if (!Src || Src == SrcMain) continue;

		const FName NewName(*FString::Printf(TEXT("%s_Clone"), *Src->GetName()));
		USkeletalMeshComponent* NewSkel = NewObject<USkeletalMeshComponent>(
			this, USkeletalMeshComponent::StaticClass(), NewName);

		NewSkel->SetSkeletalMeshAsset(Src->GetSkeletalMeshAsset());
		for (int32 mi = 0; mi < Src->GetNumMaterials(); ++mi)
		{
			NewSkel->SetMaterial(mi, Src->GetMaterial(mi));
		}
		NewSkel->SetRelativeTransform(Src->GetRelativeTransform());

		NewSkel->SetupAttachment(GetRootComponent());
		NewSkel->RegisterComponent();

		SkelMap.Add(Src, NewSkel);
	}

	// ─────────────────────────────────────────────────────────────
	// 2) 원래 부모/소켓으로 재부착
	// ─────────────────────────────────────────────────────────────
	for (auto& Pair : SkelMap)
	{
		USkeletalMeshComponent* Src = Pair.Key;
		USkeletalMeshComponent* Dst = Pair.Value;
		if (!Dst || Dst == DstMain) continue;

		USceneComponent* SrcParent = Src->GetAttachParent();
		const FName Socket = Src->GetAttachSocketName();

		USceneComponent* DstParent = DstMain;
		if (USkeletalMeshComponent* SrcParentSkel = Cast<USkeletalMeshComponent>(SrcParent))
		{
			if (USkeletalMeshComponent** Found = SkelMap.Find(SrcParentSkel))
			{
				DstParent = *Found;
			}
		}

		Dst->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		Dst->AttachToComponent(DstParent, FAttachmentTransformRules::KeepRelativeTransform, Socket);
	}

	// ─────────────────────────────────────────────────────────────
	// 3) 팔로워(옷) → 바디 포즈 따라가게 + 안정화
	// ─────────────────────────────────────────────────────────────
	const USkeleton* MainSkel = DstMain->GetSkeletalMeshAsset()
		? DstMain->GetSkeletalMeshAsset()->GetSkeleton() : nullptr;

	for (auto& Pair : SkelMap)
	{
		USkeletalMeshComponent* Dst = Pair.Value;
		if (!Dst || Dst == DstMain) continue;

		const USkeleton* DstSkel = Dst->GetSkeletalMeshAsset()
			? Dst->GetSkeletalMeshAsset()->GetSkeleton() : nullptr;

		const bool bSameSkeleton = (MainSkel && DstSkel && MainSkel == DstSkel);

		if (bSameSkeleton)
		{
			// LeaderPose 경로: 팔로워는 애님 평가 X
			Dst->SetAnimInstanceClass(nullptr);
			Dst->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			Dst->SetLeaderPoseComponent(DstMain);

			StabilizeFollower(Dst, /*bLeaderPose=*/true);
		}
		else
		{
			if (ClothRetargetABP)
			{
				Dst->SetLeaderPoseComponent(nullptr);
				Dst->SetAnimationMode(EAnimationMode::AnimationBlueprint);
				Dst->SetAnimInstanceClass(ClothRetargetABP);

				if (UAnimInstance* Inst = Dst->GetAnimInstance())
				{
					static const FName VarName(TEXT("BodyMesh")); // ABP 변수명과 동일
					if (FObjectProperty* P = FindFProperty<FObjectProperty>(Inst->GetClass(), VarName))
					{
						P->SetObjectPropertyValue_InContainer(Inst, DstMain);
					}
				}

				// 안정화(이전 답변의 StabilizeFollower 유지)
				Dst->AddTickPrerequisiteComponent(DstMain);
				Dst->bEnableUpdateRateOptimizations = false;
				Dst->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
				Dst->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Dst->bUseBoundsFromLeaderPoseComponent = false; // Retarget 경로에선 보통 false가 안정적
				Dst->ForceClothNextUpdateTeleportAndReset();
				Dst->SuspendClothingSimulation();
			}
		}
	}

#if HAS_GROOM
	// 4) Groom 복제/부착/등록
	{
		TInlineComponentArray<UGroomComponent*> SrcGrooms;
		Source->GetComponents(SrcGrooms);

		for (UGroomComponent* SrcG : SrcGrooms)
		{
			if (!SrcG) continue;

			const FName NewName(*FString::Printf(TEXT("%s_Clone"), *SrcG->GetName()));
			UGroomComponent* NewG = DuplicateObject<UGroomComponent>(SrcG, this, NewName);

			USceneComponent* SrcParent = SrcG->GetAttachParent();
			USceneComponent* DstParent = DstMain;
			if (USkeletalMeshComponent* SrcParentSkel = Cast<USkeletalMeshComponent>(SrcParent))
			{
				if (USkeletalMeshComponent** Found = SkelMap.Find(SrcParentSkel))
				{
					DstParent = *Found;
				}
			}

			NewG->AttachToComponent(
				DstParent,
				FAttachmentTransformRules::KeepRelativeTransform,
				SrcG->GetAttachSocketName()
			);
			NewG->RegisterComponent();
		}
	}
#endif // HAS_GROOM

	// 5) ChildActor(무기 등) 복제 — 부모 매핑 사용
	TInlineComponentArray<UChildActorComponent*> SrcChildComps;
	Source->GetComponents<UChildActorComponent>(SrcChildComps);

	for (UChildActorComponent* SrcCAC : SrcChildComps)
	{
		if (!SrcCAC) continue;

		UChildActorComponent* NewCAC = NewObject<UChildActorComponent>(
			this, UChildActorComponent::StaticClass(),
			FName(*FString::Printf(TEXT("%s_Clone"), *SrcCAC->GetName()))
		);

		NewCAC->SetChildActorClass(SrcCAC->GetChildActorClass());
		NewCAC->SetRelativeTransform(SrcCAC->GetRelativeTransform());

		USceneComponent* SrcParent = SrcCAC->GetAttachParent();
		USceneComponent* DstParent = DstMain;
		if (USkeletalMeshComponent* SrcParentSkel = Cast<USkeletalMeshComponent>(SrcParent))
		{
			if (USkeletalMeshComponent** Found = SkelMap.Find(SrcParentSkel))
			{
				DstParent = *Found;
			}
		}

		NewCAC->AttachToComponent(DstParent, FAttachmentTransformRules::KeepRelativeTransform, SrcCAC->GetAttachSocketName());
		NewCAC->RegisterComponent();
		NewCAC->CreateChildActor();

		if (AActor* DestChild = NewCAC->GetChildActor())
		{
			DestChild->SetOwner(this);

			TArray<UHitDetectionComponent*> HDs;
			DestChild->GetComponents(HDs);
			for (auto* HD : HDs)
			{
				if (HD) HD->RefreshOwnerPointers();
			}
		}
	}

	// 프로젝트별 설정
	CharacterType = ECharacterType::Companion;
}

void ACloneAICharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}