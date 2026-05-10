// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "JWSaveGame.generated.h"

USTRUCT()
struct FJWComponentSaveData
{
    GENERATED_BODY()

    UPROPERTY() FName          ComponentName;   // 1차 매칭
    UPROPERTY() FSoftClassPath ClassPath;       // 2차 폴백 매칭
    UPROPERTY() TArray<uint8>  Bytes;           // SaveGame 필드 직렬화
};

USTRUCT()
struct FJWActorSaveData
{
    GENERATED_BODY()

    UPROPERTY() FGuid PersistentId;

    UPROPERTY() FName                   ActorName;
    UPROPERTY() TSoftClassPtr<AActor>   ActorClass;
    UPROPERTY() FTransform              Transform;

    UPROPERTY() TArray<uint8>           ActorBytes;   // (선택) 액터 자체 SaveGame 필드
    UPROPERTY() TArray<FJWComponentSaveData> Components; // 컴포넌트별 바이트
};

USTRUCT()
struct FJWCheckpoint
{
    GENERATED_BODY()


    UPROPERTY() FDateTime SavedAt;
    UPROPERTY() FName LevelName; // 저장 당시 레벨

    UPROPERTY() TArray<FJWActorSaveData> WorldAgents; // USaveAgentComponent가 붙은 액터들

    UPROPERTY() FJWActorSaveData PlayerData; // 플레이어(액터+컴포넌트)
    UPROPERTY() FGameplayTagContainer ProgressFlags; // 전역 진행 태그
};

/**
 * 
 */
UCLASS()
class JEONWOOCHI_API UJWSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
    // 버전 2: Progress/Rest 이중 체크포인트 구조
    UPROPERTY() int32 Version = 2;

    // 메타
    UPROPERTY() FDateTime CreatedAt; // 게임 파일 생성 시각
    UPROPERTY() FString GameTitle; // (선택) 유저가 붙인 제목
    UPROPERTY() TMap<FName, FText> LevelTitleOverrides;
    UPROPERTY() int64 TotalPlaySeconds = 0; // 누적 플레이 타임(초)


    // Progress(진행) 체크포인트
    UPROPERTY() bool bHasProgress = false;
    UPROPERTY() FJWCheckpoint Progress;


    // Rest(화톳불) 체크포인트
    UPROPERTY() bool bHasRest = false;
    UPROPERTY() FJWCheckpoint Rest;


    // --- (구버전 호환용) v1 필드 ---
    // v1에서 사용되던 단일 스냅샷 필드. 남겨두면 구세이브를 읽어 최신 구조로 마이그레이션 가능
    UPROPERTY() FDateTime SavedAt_Legacy;
    UPROPERTY() FName LevelName_Legacy;
    UPROPERTY() bool bAutoSave_Legacy = false;
    UPROPERTY() FJWActorSaveData PlayerData_Legacy;
    UPROPERTY() TArray<FJWActorSaveData> WorldActors_Legacy; // 미사용
    UPROPERTY() FGameplayTagContainer ProgressFlags_Legacy;
};
