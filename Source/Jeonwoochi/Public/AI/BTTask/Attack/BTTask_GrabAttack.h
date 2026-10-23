// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "../../../Characters/AI/BaseAICharacter.h"
#include "../../../Structure/Combat/DamageInfoStruct.h"
#include "../../../Structure/Combat/DotDealInfoStruct.h"
#include "Animation/AnimMontage.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

#include "BTTask_GrabAttack.generated.h"

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UBTTask_GrabAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_GrabAttack();

	// Task 시작 시 호출
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

    // 에디터에서 블랙보드 키를 선택할 수 있게 합니다.
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;
	
protected:
    // 에디터에서 지정할 잡기 몽타주
    UPROPERTY(EditAnywhere, Category = "Animation")
    FDamageInfoStruct GrabInfo;

    UPROPERTY(EditAnywhere, Category = "DamageInfo")
    FDotDealInfoStruct DotDamageInfo;

    virtual void OnTaskFinished(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        EBTNodeResult::Type TaskResult
    ) override;

    // 몽타주 종료 콜백
    UFUNCTION()
    void OnGrabMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    

    FOnMontageEnded GrabEndDelegate;
    FOnMontageEnded AttackEndDelegate;
    FOnMontageEnded EmptyDelegate;



private:
    UBehaviorTreeComponent* MyOwnerComp = nullptr;
    ABaseAICharacter* EnemyChar = nullptr;
    UBlackboardComponent* BlackBoardComp;
};
