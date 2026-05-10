// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Composite/WeightedRandomSelector.h"

UWeightedRandomSelector::UWeightedRandomSelector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), LastSuccessfulChildIdx(INDEX_NONE)
{
	NodeName = TEXT("Weighted Random Selector");
}

void UWeightedRandomSelector::InitializeMemory(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	InitializeNodeMemory<FBTCompositeMemory>(NodeMemory, InitType);
	ExecutedChildren.Empty();
	LastSuccessfulChildIdx = INDEX_NONE;
}

void UWeightedRandomSelector::CleanupMemory(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	CleanupNodeMemory<FBTCompositeMemory>(NodeMemory, CleanupType);
	ExecutedChildren.Empty();
	LastSuccessfulChildIdx = INDEX_NONE;
}

/** 가중치-룰렛 휠 */
static int32 SelectByWeight(const TArray<int32>& Candidates,
	const TArray<float>& Weights)
{
	float Sum = 0.f;
	for (int32 Idx : Candidates)
	{
		Sum += (Idx < Weights.Num() && Weights[Idx] > 0.f) ? Weights[Idx] : 1.f;
	}
	const float Pick = FMath::FRandRange(0.f, Sum);

	float Acc = 0.f;
	for (int32 Idx : Candidates)
	{
		Acc += (Idx < Weights.Num() && Weights[Idx] > 0.f) ? Weights[Idx] : 1.f;
		if (Pick <= Acc)
			return Idx;
	}
	return Candidates.Last(); // fallback (이론상 못 옴)
}

int32 UWeightedRandomSelector::GetNextChildHandler(FBehaviorTreeSearchData& SearchData,
	int32 PrevChild,
	EBTNodeResult::Type LastResult) const
{
	// ---------- 첫 실행 ----------
	if (PrevChild == BTSpecialChild::NotInitialized)
	{
		TArray<int32> All;
		for (int32 i = 0; i < GetChildrenNum(); ++i)
			All.Add(i);

		// 직전 성공 노드 반복 방지
		if (LastSuccessfulChildIdx != INDEX_NONE && All.Num() > 1)
			All.Remove(LastSuccessfulChildIdx);

		return SelectByWeight(All, Weights);
	}

	// ---------- 이전 실행 결과 처리 ----------
	if (LastResult == EBTNodeResult::Succeeded)
	{
		LastSuccessfulChildIdx = PrevChild;
		ExecutedChildren.Empty();
		return BTSpecialChild::ReturnToParent;
	}

	if (LastResult == EBTNodeResult::Failed)
	{
		ExecutedChildren.AddUnique(PrevChild);

		// 모든 자식 실패 → 부모에게 실패 리턴
		if (ExecutedChildren.Num() >= GetChildrenNum())
		{
			ExecutedChildren.Empty();
			LastSuccessfulChildIdx = INDEX_NONE;
			return BTSpecialChild::ReturnToParent;
		}

		// 아직 안 돌린 자식 중에서 가중치 선택
		TArray<int32> Remaining;
		for (int32 i = 0; i < GetChildrenNum(); ++i)
			if (!ExecutedChildren.Contains(i))
				Remaining.Add(i);

		return SelectByWeight(Remaining, Weights);
	}

	// Abort, InProgress 등은 그대로 재호출
	return PrevChild;
}

#if WITH_EDITOR
void UWeightedRandomSelector::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 자식 수 변화 시 Weight 배열 크기 보정
	const int32 ChildNum = GetChildrenNum();
	if (ChildNum != Weights.Num())
	{
		Weights.SetNum(ChildNum);
		for (float& W : Weights)
			if (W <= 0.f) W = 1.f;
	}
}
#endif