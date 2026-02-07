// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data/Save/SaveTargetComponent.h"
#include "SaveSubsystem.generated.h"

UENUM(BlueprintType)
enum class ESaveBucket : uint8
{
    Progress, // 일반 진행(자동 저장/종료 전 수동 저장)
    Rest, // 화톳불 체크포인트(사망 시 이 시점으로 복원)
    FullWorld // (옵션) 전체 월드 스냅샷 — 필요 시 유지
};

USTRUCT()
struct FSpawnSeed
{
    GENERATED_BODY()
    UPROPERTY() FGuid                 Id;
    UPROPERTY() TSoftClassPtr<AActor> Class; // 안전용
    UPROPERTY() struct FSaveTargetConfig     Config;
};

// UI에 뿌릴 슬롯 메타 정보
USTRUCT(BlueprintType)
struct FJWGameInfo
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FString GameName; // 예: "Game_000"
    UPROPERTY(BlueprintReadOnly) FDateTime SavedAt; // Progress 최종 저장 시각
    UPROPERTY(BlueprintReadOnly) FName LevelName; // Progress 저장 당시 레벨
    UPROPERTY(BlueprintReadOnly) FText LevelTitle; // 저장 당시 맵 이름
    UPROPERTY(BlueprintReadOnly) int64 TotalPlaySeconds = 0; // 누적 플레이 타임(초)
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameListChanged);
DECLARE_MULTICAST_DELEGATE(FOnWorldAgentsApplied);


/**
 * 
 */
UCLASS()
class JEONWOOCHI_API USaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    // === 신규 API ===
    UFUNCTION(BlueprintCallable) FString CreateNewGame(
        const FString& Prefix = TEXT("게임_"), int32 ZeroPad = 3);
    UFUNCTION(BlueprintCallable) bool SelectGame(const FString& GameName); // 현재 진행할 게임 파일 지정


    // 이벤트마다 호출: 진행 자동 저장
    UFUNCTION(BlueprintCallable) bool SaveProgress(bool bAutoSave = true);
    // 종료 직전 수동 저장: 결국 Progress 업데이트와 동일(표기만 다를 수 있음)
    UFUNCTION(BlueprintCallable) bool SaveProgressBeforeExit();

    // TotalPlay Time 업데이트
    UFUNCTION(BlueprintCallable) void UpdateTotalPlaySeconds();
    
    // 화톳불에서 휴식 저장(HP 전회복, Chi는 회복 X, 적 전원 리스폰)
    UFUNCTION(BlueprintCallable) bool SaveRestAtBonfire();


    // 메인 메뉴에서 이어하기(Progress 적용)
    UFUNCTION(BlueprintCallable) bool LoadForContinue(const FString& GameName);
    // 사망 시 리스폰(Rest 적용)
    UFUNCTION(BlueprintCallable) bool LoadFromRest();

    // UI에 보일 맵 이름 지정
    UFUNCTION(BlueprintCallable) void SetLevelTitle(const FText& InTitle);

    // 게임 파일 목록
    UFUNCTION(BlueprintCallable) void GetGameFiles(TArray<FJWGameInfo>& Out) const { Out = CachedGames; }
    UFUNCTION(BlueprintCallable) void RebuildGameList(const FString& Prefix = TEXT("게임_"));
    UFUNCTION(BlueprintCallable) bool DeleteGame(const FString& GameName);


    UPROPERTY(BlueprintAssignable) FOnGameListChanged OnGameListChanged;
    FOnWorldAgentsApplied OnWorldAgentsApplied;


    // === (구 API, 필요 시 유지 — 내부적으로 신규 API로 연결) ===
    UFUNCTION(BlueprintCallable) bool WriteSlot(const FString& SlotName, bool bAutoSave = false, ESaveBucket Bucket = ESaveBucket::Progress);
    UFUNCTION(BlueprintCallable) bool ReadSlot(const FString& SlotName); // Progress 기준으로 로드
    UFUNCTION(BlueprintCallable) bool WriteNextSlot(bool bAutoSave = false, const FString& Prefix = TEXT("Game_"), int32 ZeroPad = 3, ESaveBucket Bucket = ESaveBucket::Progress);


private:
    // 현재 선택된 게임 파일 이름(예: Game_000)
    UPROPERTY() FString CurrentGameName;

    // === 세션 타이밍 ===
    UPROPERTY() FDateTime SessionStart; // 현재 플레이 세션 시작 시각(누적 갱신용)

    // === 캐시 & 시리얼 ===
    UPROPERTY() TArray<FJWGameInfo> CachedGames;
    UPROPERTY() int32 NextSerial = 0; // 다음에 쓸 번호(항상 증가)


    // 맵 이동 후 적용을 위한 임시
    FString PendingGameToApply;
    ESaveBucket PendingBucket = ESaveBucket::Progress;

    // 내부 처리
    bool WriteBucket_Internal(const FString& GameName, ESaveBucket Bucket, bool bAutoSaveFlagForUI);
    bool LoadAndApply_Internal(const FString& GameName, ESaveBucket Bucket);


    void CaptureIntoCheckpoint(struct FJWCheckpoint& Out, bool bHealHPBeforeCapture);
    void ApplyCheckpointNow(const struct FJWCheckpoint& CP, bool bDoRestRespawn);


    void OnPostLoadMapWithWorld(UWorld* LoadedWorld);


    // === 적/보스 시드 & 리스폰 ===
    UPROPERTY() TMap<FGuid, FSpawnSeed> SpawnSeeds; // 현재 월드의 '원래 배치' 사본


    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    void OnPostWorldInit(UWorld* World, const UWorld::InitializationValues IVs);


    void BuildSpawnSeeds();
    void EnsureSpawnSeedsBuilt();
    class AActor* FindLiveById(const FGuid& Id) const;


    void RespawnAllEnemiesForRest();
    void ApplyBossDefeatedState();
    void ResetAIForActor(class AActor* A);

    // === 경로/이름 유틸 ===
    FString GetSaveDir() const;
    FString MakeGamePath(const FString& GameName) const;
    FString MakeMonotonicGameName(int32 Serial, const FString& Prefix, int32 ZeroPad) const;
    int32 ParseSerialFromName(const FString& GameName, const FString& Prefix) const;

    // === 캐시 유틸 ===
    int32 FindInCache(const FString& GameName) const;
    void UpsertCache(const FString& GameName, const class UJWSaveGame* Save);

    // === 저장 루틴에 붙일 유틸 === 
    void CaptureAgentsIntoCheckpoint(struct FJWCheckpoint& Out, ESaveBucket Bucket);
    void ApplyAgentsFromCheckpoint(const struct FJWCheckpoint& CP);
};