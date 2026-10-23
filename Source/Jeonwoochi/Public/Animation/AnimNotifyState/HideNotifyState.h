// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "HideNotifyState.generated.h"

/**
 * 
 */

 /** 각 컴포넌트의 원래 상태 + 중첩 깊이 */
USTRUCT()
struct FCachedInfo
{
	GENERATED_BODY()

	bool  bVisible = true;  // 원래 보이던 상태
	int32 Depth = 0;     // 중첩 카운트
};

/** 메시 1개당 캐시 테이블 */
using FPerMeshCache = TMap<TWeakObjectPtr<UMeshComponent>, FCachedInfo>;

UCLASS()
class JEONWOOCHI_API UHideNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	/** 자식 액터(ChildActorComponent, Attach된 액터 포함)까지 모두 숨길지 */
	UPROPERTY(VisibleAnywhere, Category = "Hide")
	bool bIncludeAttachedActors = true;

	/** 스켈레탈 메시만 숨길지(헤어 Groom/카드, 스태틱 메시 등을 제외) */
	UPROPERTY(VisibleAnywhere, Category = "Hide")
	bool bSkeletalMeshesOnly = false;

	/** 태그로 필터링(설정 시 해당 태그가 있는 컴포넌트만 대상) */
	UPROPERTY(EditAnywhere, Category = "Hide", meta = (EditCondition = "bUseTagFilter"))
	bool bUseTagFilter = false;

	UPROPERTY(EditAnywhere, Category = "Hide", meta = (EditCondition = "bUseTagFilter"))
	TArray<FName> TargetComponentTags;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Anim, float TotalDuration) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Anim) override;

private:
	/** 노티파이별(메시 인스턴스별) 캐시 */
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FPerMeshCache> PerInstanceCache;

	/** 루트 액터로부터 대상 UMeshComponent들을 재귀적으로 수집 */
	void GatherMeshesRecursive(AActor* Root, TArray<UMeshComponent*>& Out) const;

	/** 컴포넌트가 필터 조건에 부합하는지 */
	bool PassFilter(UMeshComponent* Comp) const;


};
