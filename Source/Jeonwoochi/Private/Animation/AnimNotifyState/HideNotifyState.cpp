// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState/HideNotifyState.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

void UHideNotifyState::GatherMeshesRecursive(AActor* Root, TArray<UMeshComponent*>& Out) const
{
	if (!IsValid(Root)) return;

	// 1) 현재 액터의 UMeshComponent 수집
	TInlineComponentArray<UMeshComponent*, 32> LocalMeshes;
	Root->GetComponents(LocalMeshes, /*bIncludeFromChildActors=*/false);

	for (UMeshComponent* Comp : LocalMeshes)
	{
		if (IsValid(Comp) && PassFilter(Comp))
		{
			Out.Add(Comp);
		}
	}

	// 2) 붙어있는 자식 액터까지 재귀 수집
	if (bIncludeAttachedActors)
	{
		TArray<AActor*> Children;
		Root->GetAttachedActors(Children, /*bResetArray=*/true, /*bRecursivelyIncludeAttachedActors=*/true);
		for (AActor* Child : Children)
		{
			GatherMeshesRecursive(Child, Out);
		}
	}
}

bool UHideNotifyState::PassFilter(UMeshComponent* Comp) const
{
	if (bSkeletalMeshesOnly && !Comp->IsA<USkeletalMeshComponent>())
	{
		return false;
	}
	if (bUseTagFilter && TargetComponentTags.Num() > 0)
	{
		// 컴포넌트 태그에 하나라도 있으면 통과
		for (const FName& Tag : TargetComponentTags)
		{
			if (Comp->ComponentHasTag(Tag))
			{
				return true;
			}
		}
		return false;
	}
	return true;
}

void UHideNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* /*Anim*/, float /*TotalDuration*/)
{
	if (!IsValid(MeshComp)) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) return;

	// 메시 인스턴스별 캐시 확보
	FPerMeshCache& Cache = PerInstanceCache.FindOrAdd(MeshComp);

	// 메타휴먼(몸/얼굴/헤어/눈썹 등) + 붙은 무기/액세서리까지 모두 수집
	TArray<UMeshComponent*> Targets;
	GatherMeshesRecursive(Owner, Targets);

	for (UMeshComponent* Comp : Targets)
	{
		FCachedInfo& Info = Cache.FindOrAdd(Comp);

		// 중첩 진입: 최초 진입 시 원래 가시 상태 저장
		if (Info.Depth == 0)
		{
			Info.bVisible = Comp->IsVisible();
		}
		Info.Depth++;

		// 본인만 숨김(Propagate=false). 개별 복구를 위해.
		Comp->SetVisibility(false, /*PropagateToChildren=*/false);
	}
}

void UHideNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* /*Anim*/)
{
	FPerMeshCache* CachePtr = PerInstanceCache.Find(MeshComp);
	if (!CachePtr) return;

	for (auto It = CachePtr->CreateIterator(); It; ++It)
	{
		UMeshComponent* Comp = It->Key.Get();
		FCachedInfo& Info = It->Value;

		if (!IsValid(Comp)) continue;

		// 중첩 해제
		Info.Depth = FMath::Max(0, Info.Depth - 1);

		// 완전히 빠져나온 경우 원래 상태로 복구
		if (Info.Depth == 0)
		{
			Comp->SetVisibility(Info.bVisible, /*PropagateToChildren=*/false);
			It.RemoveCurrent();
		}
	}

	// 해당 인스턴스 캐시 비우기
	if (CachePtr->Num() == 0)
	{
		PerInstanceCache.Remove(MeshComp);
	}
}