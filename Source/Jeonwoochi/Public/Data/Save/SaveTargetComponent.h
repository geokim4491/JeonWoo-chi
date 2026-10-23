// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Data/AI/AIConfigData.h"
#include "SaveTargetComponent.generated.h"

USTRUCT(BlueprintType)
struct FSaveTargetConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsEnemy = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsBoss = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName BossId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag BossDefeatedTag;

    // 에디터 기준 리스폰 트랜스폼(위치/회전/스케일 모두)
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FTransform EditorTransform;

    // 이 인스턴스를 스폰할 클래스(비워두면 소유 액터의 클래스)
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftClassPtr<AActor> SpawnClass;

    UPROPERTY() TSoftObjectPtr<class UAnimMontage>       SpawnMontage;
    UPROPERTY() TSoftObjectPtr<class APatrolRoute>       AssignedRouteSoft; // 레벨 액터 SoftPath
    UPROPERTY() FName                                    AssignedRouteByName; // 폴백용

    // Instanced 서브오브젝트 템플릿(Seed에 보관 → 스폰 시 Owner로 복제)
    UPROPERTY() class UAIConfigData* ConfigDataTemplate = nullptr;

    // 컴포넌트 Max 값(인스턴스마다 다를 수 있음)
    UPROPERTY() float                                     SeedMaxHealth = 0.f;
    UPROPERTY() int32                                     SeedMaxPostureCount = 0;
};

UCLASS( ClassGroup=(Save), meta=(BlueprintSpawnableComponent) )
class JEONWOOCHI_API USaveTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    // 고정 식별자(리네임/복제 대비). 에디터에서 붙이면 자동 생성
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
    FGuid PersistentId;

    // 일반 적/보스 식별
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
    bool bIsEnemy = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
    bool bIsBoss = false;

    // 보스 ID(예: Godric). GameplayTags에서 boss.<ID>.defeated 로 쓰임
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save", meta = (EditCondition = "bIsBoss", EditConditionHides))
    FName BossId = NAME_None;

    // 에디터 배치 위치 (리스폰 시 복귀 지점)
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Save")
    FTransform EditorTransform;

    // 이 인스턴스의 기준 Transform을 에디터에서 자동 캡처할지
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Save", AdvancedDisplay)
    bool bAutoCaptureInEditor = true;

    // 캡처 여부 플래그(런타임 최초 등록 시 한 번만 자동 세팅하는 데 사용)
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Save", AdvancedDisplay)
    bool bEditorTransformCaptured = false;

    // 스폰 클래스(기본: 소유 액터 클래스)
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Save")
    TSoftClassPtr<AActor> SpawnClass;

    // 보스 처치 태그 (boss.<BossId>.defeated). BossId가 있으면 자동 계산
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
    FGameplayTag BossDefeatedTag;


public:

    UFUNCTION(BlueprintCallable, Category = "Save")
    FSaveTargetConfig MakeConfig() const;

    UFUNCTION(BlueprintCallable, Category = "Save")
    void ApplyConfig(const FSaveTargetConfig& C);

    UFUNCTION(BlueprintCallable) 
    void ApplyConfigToOwnerExtras(const FSaveTargetConfig& C);

    // 수동 캡처 버튼(디테일 패널에서 클릭)
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Save")
    void CaptureFromActor();

    virtual void OnRegister() override;
};
