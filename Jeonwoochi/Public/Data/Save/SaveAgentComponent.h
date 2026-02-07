// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SaveAgentComponent.generated.h"


// 어떤 버킷(진행/휴식/풀월드)에서 이 액터를 저장/로드할지 플래그
UENUM(meta = (Bitflags))
enum class ESaveBucketFlags : uint8
{
    None = 0,
    Progress = 1 << 0,   // 이어하기용 진행 저장에 포함
    Rest = 1 << 1,   // 화톳불 휴식 저장에 포함(원하면)
    FullWorld = 1 << 2,   // 디버그/전체 저장
};
ENUM_CLASS_FLAGS(ESaveBucketFlags)

UCLASS(ClassGroup = (Save), meta = (BlueprintSpawnableComponent))
class JEONWOOCHI_API USaveAgentComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    // 고정 GUID (레벨 배치/런타임 스폰 모두)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save|Agent")
    FGuid PersistentId;

    // 어떤 버킷에 포함할지(기본: Progress)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save|Agent", meta = (Bitmask, BitmaskEnum = "/Script/Jeonwoochi.ESaveBucketFlags"))
    int32 Buckets = (int32)ESaveBucketFlags::Progress;

    // 액터 스폰 허용(로드 시 없으면 스폰)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save|Agent")
    bool bSpawnIfMissingOnLoad = true;

    // 스폰 클래스(비우면 소유 액터 클래스)
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Save|Agent")
    TSoftClassPtr<AActor> SpawnClass;

    // (선택) 에디터 기준 변환 저장(절대 리스토어용)
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Save|Agent")
    FTransform EditorTransform;

    // 에디터에서 한 번에 캡처하는 버튼
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Save|Agent")
    void CaptureFromActor();

public:
    virtual void OnRegister() override;
};