// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "WeightedRandomSelector.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UWeightedRandomSelector : public UBTCompositeNode
{
	GENERATED_BODY()
	
public:
	UWeightedRandomSelector(const FObjectInitializer& ObjectInitializer);

	/** 자식별 가중치 (BT 에디터 Details 패널에서 설정) */
	UPROPERTY(EditAnywhere, Category = "Selector")
	TArray<float> Weights;

	// BTCompositeNode 인터페이스
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTMemoryClear::Type CleanupType) const override;
	virtual int32 GetNextChildHandler(struct FBehaviorTreeSearchData& SearchData,
		int32 PrevChild,
		EBTNodeResult::Type LastResult) const override;

#if WITH_EDITOR
	/** 자식 수가 변할 때 Weight 배열을 자동으로 맞춰 줌 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	mutable TArray<int32> ExecutedChildren;
	mutable int32         LastSuccessfulChildIdx;
};
