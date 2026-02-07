// Fill out your copyright ...

#include "Data/Save/SaveSubsystem.h"
#include "Data/Save/JWSaveGame.h"
#include "Data/Save/SaveHelpers.h"
#include "Data/Save/SaveAgentComponent.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UObject/UObjectGlobals.h"


#include "Components/Combat/HealthComponent.h"
#include "Components/Combat/ChiComponent.h"
#include "Components/Combat/PostureComponent.h"

#include "Data/Progression/ProgressionSubsystem.h"


#include "EngineUtils.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Jeonwoochi/JeonwoochiGameMode.h"

static constexpr int32 GUserIndex = 0;


// ====== 수명주기 ======
void USaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &USaveSubsystem::OnPostWorldInit);
}
void USaveSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	Super::Deinitialize();
}
void USaveSubsystem::OnPostWorldInit(UWorld* World, const UWorld::InitializationValues IVs)
{
	if (World && World->IsGameWorld())
	{
		BuildSpawnSeeds();
	}
}

void USaveSubsystem::UpdateTotalPlaySeconds()
{
	UJWSaveGame* Save = nullptr;
	if (USaveGame* Base = UGameplayStatics::LoadGameFromSlot(CurrentGameName, GUserIndex))
	{
		Save = Cast<UJWSaveGame>(Base);
	}

	if (SessionStart > FDateTime(0))
	{
		const FTimespan Delta = FDateTime::Now() - SessionStart;
		if (Delta.GetTicks() > 0)
		{
			Save->TotalPlaySeconds += (int64)Delta.GetTotalSeconds();
			SessionStart = FDateTime::Now(); // 다음 저장을 위한 기준 갱신
		}
	}
}

// ====== 경로/이름 유틸 ======
FString USaveSubsystem::GetSaveDir() const { return FPaths::ProjectSavedDir() / TEXT("SaveGames"); }
FString USaveSubsystem::MakeGamePath(const FString& GameName) const { return GetSaveDir() / (GameName + TEXT(".sav")); }
FString USaveSubsystem::MakeMonotonicGameName(int32 Serial, const FString& Prefix, int32 ZeroPad) const
{
	return FString::Printf(TEXT("%s%0*d"), *Prefix, ZeroPad, Serial); // 예: Game_000
}
int32 USaveSubsystem::ParseSerialFromName(const FString& GameName, const FString& Prefix) const
{
	if (!GameName.StartsWith(Prefix)) return -1;
	const FString Tail = GameName.Mid(Prefix.Len());
	return FCString::Atoi(*Tail);
}


int32 USaveSubsystem::FindInCache(const FString& GameName) const
{
	return CachedGames.IndexOfByPredicate([&](const FJWGameInfo& S) { return S.GameName == GameName; });
}
void USaveSubsystem::UpsertCache(const FString& GameName, const UJWSaveGame* Save)
{
	if (!Save) return;


	FJWGameInfo Info;
	Info.GameName = GameName;
	// Progress가 있으면 그걸, 없으면 Rest/Legacy 순으로 메타 구성
	if (Save->bHasProgress)
	{
		Info.SavedAt = Save->Progress.SavedAt;
		Info.LevelName = Save->Progress.LevelName;
	}
	else if (Save->bHasRest)
	{
		Info.SavedAt = Save->Rest.SavedAt;
		Info.LevelName = Save->Rest.LevelName;
	}
	else
	{
		Info.SavedAt = Save->CreatedAt;
		Info.LevelName = NAME_None;
	}
	Info.TotalPlaySeconds = Save->TotalPlaySeconds;
	if (const FText* Title = Save->LevelTitleOverrides.Find(Info.LevelName))
	{
		Info.LevelTitle = *Title;
	}
	else
	{
		// 폴백(원하면): 내부 레벨명으로 표시
		Info.LevelTitle = FText::FromName(Info.LevelName);
	}

	const int32 Idx = FindInCache(GameName);
	if (Idx >= 0) CachedGames[Idx] = Info; else CachedGames.Insert(Info, 0);
	OnGameListChanged.Broadcast();
}

void USaveSubsystem::SetLevelTitle(const FText& InTitle)
{
	if (CurrentGameName.IsEmpty()) return;

	const int32 Idx = FindInCache(CurrentGameName);
	if (Idx < 0) return;

	// 1) 런타임 캐시 업데이트
	CachedGames[Idx].LevelTitle = InTitle;

	// 저장 파일에 반영
	if (USaveGame* Base = UGameplayStatics::LoadGameFromSlot(CurrentGameName, 0))
		if (auto* Save = Cast<UJWSaveGame>(Base))
		{
			const FName Level = CachedGames[Idx].LevelName;
			Save->LevelTitleOverrides.FindOrAdd(Level) = InTitle;
			UGameplayStatics::SaveGameToSlot(Save, CurrentGameName, 0);
		}


	// 2) UI 갱신 신호
	OnGameListChanged.Broadcast();
}

// ====== 새 게임/선택 ======
FString USaveSubsystem::CreateNewGame(const FString& Prefix, int32 ZeroPad)
{
	const FString Name = MakeMonotonicGameName(NextSerial, Prefix, ZeroPad);

	// 초기 파일 작성
	UJWSaveGame* Save = NewObject<UJWSaveGame>();
	Save->Version = 2;
	Save->CreatedAt = FDateTime::Now();
	Save->TotalPlaySeconds = 0;

	const bool bOK = UGameplayStatics::SaveGameToSlot(Save, Name, GUserIndex);
	if (bOK)
	{
		++NextSerial;
		UpsertCache(Name, Save);
		CurrentGameName = Name;
		SessionStart = FDateTime::Now();
	}
	return bOK ? Name : FString();
}

bool USaveSubsystem::SelectGame(const FString& GameName)
{
	if (GameName.IsEmpty()) return false;
	// 존재 여부 확인(로드 시도)
	if (USaveGame* Base = UGameplayStatics::LoadGameFromSlot(GameName, GUserIndex))
	{
		if (UJWSaveGame* Save = Cast<UJWSaveGame>(Base))
		{
			CurrentGameName = GameName;
			UpsertCache(GameName, Save);
			return true;
		}
	}
	return false;
}

// ====== 저장 엔트리 포인트 ======
bool USaveSubsystem::SaveProgress(bool bAutoSave)
{
	if (CurrentGameName.IsEmpty()) return false;
	return WriteBucket_Internal(CurrentGameName, ESaveBucket::Progress, bAutoSave);
}

bool USaveSubsystem::SaveProgressBeforeExit()
{
	if (CurrentGameName.IsEmpty()) return false;
	// UI 표기 차이만 있을 수 있으므로 동일 처리
	return WriteBucket_Internal(CurrentGameName, ESaveBucket::Progress, /*bAuto*/false);
}

bool USaveSubsystem::SaveRestAtBonfire()
{
	if (CurrentGameName.IsEmpty()) return false;
	const bool bOK = WriteBucket_Internal(CurrentGameName, ESaveBucket::Rest, /*bAuto*/false);
	if (bOK)
	{
		// 휴식 효과: 적 전원 리스폰, 보스 태그 반영
		EnsureSpawnSeedsBuilt();
		ApplyBossDefeatedState();
		RespawnAllEnemiesForRest();
	}
	return bOK;
}

// ====== 로드 엔트리 포인트 ======
bool USaveSubsystem::LoadForContinue(const FString& GameName)
{
	SessionStart = FDateTime::Now(); // 세션 시작 기준 설정
	return LoadAndApply_Internal(GameName, ESaveBucket::Progress);
}


bool USaveSubsystem::LoadFromRest()
{
	if (CurrentGameName.IsEmpty()) return false;
	return LoadAndApply_Internal(CurrentGameName, ESaveBucket::Rest);
}

// ====== 구 API 호환 ======
bool USaveSubsystem::WriteSlot(const FString& SlotName, bool bAutoSave, ESaveBucket Bucket)
{
	return WriteBucket_Internal(SlotName, Bucket, bAutoSave);
}


bool USaveSubsystem::WriteNextSlot(bool bAutoSave, const FString& Prefix, int32 ZeroPad, ESaveBucket Bucket)
{
	const FString Name = MakeMonotonicGameName(NextSerial, Prefix, ZeroPad);
	const bool bOK = WriteBucket_Internal(Name, Bucket, bAutoSave);
	if (bOK) { ++NextSerial; }
	return bOK;
}

bool USaveSubsystem::ReadSlot(const FString& SlotName)
{
	return LoadAndApply_Internal(SlotName, ESaveBucket::Progress);
}

// ====== 내부: 캡처/적용 ======
void USaveSubsystem::CaptureIntoCheckpoint(FJWCheckpoint& Out, bool bHealHPBeforeCapture)
{
	UWorld* World = GetWorld();
	if (!World) return;


	Out.SavedAt = FDateTime::Now();
	Out.LevelName = FName(*UGameplayStatics::GetCurrentLevelName(World, /*RemovePrefix*/true));


	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0))
	{
		FJWActorSaveData& Rec = Out.PlayerData;
		Rec.ActorName = Pawn->GetFName();
		Rec.ActorClass = Pawn->GetClass();
		Rec.Transform = Pawn->GetActorTransform();


		// 화톳불 휴식 시 HP 전회복(Chi는 그대로)
		if (bHealHPBeforeCapture)
		{
			if (auto* HC = Pawn->FindComponentByClass<UHealthComponent>())
			{
				const float MaxHP = HC->GetMaxHealth();
				HC->SetCurrentHealth(MaxHP);
			}
		}


		JW_SerializeToBytes(Pawn, Rec.ActorBytes);


		TInlineComponentArray<UActorComponent*> Comps(Pawn);
		for (UActorComponent* C : Comps)
		{
			if (!C) continue;
			// SaveGame 플래그가 있는 컴포넌트만
			if (!JW_HasSaveGameFields(C->GetClass())) continue;


			FJWComponentSaveData CRec;
			CRec.ComponentName = C->GetFName();
			CRec.ClassPath = C->GetClass();
			JW_SerializeToBytes(C, CRec.Bytes);
			Rec.Components.Add(MoveTemp(CRec));
		}
	}


	if (UProgressionSubsystem* Prog = GetGameInstance()->GetSubsystem<UProgressionSubsystem>())
	{
		Out.ProgressFlags = Prog->Flags;
	}
}

void USaveSubsystem::ApplyCheckpointNow(const FJWCheckpoint& CP, bool bDoRestRespawn)
{
	UWorld* World = GetWorld();
	if (!World) return;


	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0))
	{
		const FJWActorSaveData& Rec = CP.PlayerData;


		Pawn->SetActorLocationAndRotation(
			Rec.Transform.GetLocation(),
			Rec.Transform.Rotator(),
			false, nullptr, ETeleportType::TeleportPhysics);


		if (Rec.ActorBytes.Num() > 0)
		{
			JW_DeserializeFromBytes(Pawn, Rec.ActorBytes);
		}


		TInlineComponentArray<UActorComponent*> Existing(Pawn);
		auto FindByName = [&](FName Name)->UActorComponent*
			{
				for (UActorComponent* C : Existing)
					if (C->GetFName() == Name) return C;
				return nullptr;
			};


		for (const FJWComponentSaveData& CRec : Rec.Components)
		{
			UActorComponent* Comp = FindByName(CRec.ComponentName);
			if (!Comp && CRec.ClassPath.IsValid())
			{
				if (UClass* Cls = Cast<UClass>(CRec.ClassPath.TryLoad()))
				{
					for (UActorComponent* C : Existing)
						if (C->IsA(Cls)) { Comp = C; break; }
				}
			}
			if (Comp)
			{
				JW_DeserializeFromBytes(Comp, CRec.Bytes);
			}
		}
	}


	if (UProgressionSubsystem* Prog = GetGameInstance()->GetSubsystem<UProgressionSubsystem>())
	{
		Prog->Flags = CP.ProgressFlags;
	}

	ApplyAgentsFromCheckpoint(CP);

	EnsureSpawnSeedsBuilt();
	ApplyBossDefeatedState(); // 보스는 태그대로 영구 제거

	if (bDoRestRespawn)
	{
		RespawnAllEnemiesForRest(); // 일반 적은 전원 원위치 리스폰(+AI 초기화)
	}
}

bool USaveSubsystem::WriteBucket_Internal(const FString& GameName, ESaveBucket Bucket, bool bAutoSaveFlagForUI)
{
	UJWSaveGame* Save = nullptr;
	if (USaveGame* Base = UGameplayStatics::LoadGameFromSlot(GameName, GUserIndex))
	{
		Save = Cast<UJWSaveGame>(Base);
	}
	if (!Save)
	{
		Save = NewObject<UJWSaveGame>();
		Save->Version = 2;
		Save->CreatedAt = FDateTime::Now();
		Save->TotalPlaySeconds = 0;
	}


	// v1 → v2 마이그레이션(최초 로드 시 한 번)
	if (Save->Version < 2 && !Save->bHasProgress && Save->PlayerData_Legacy.ActorClass.IsValid())
	{
		Save->bHasProgress = true;
		Save->Progress.SavedAt = Save->SavedAt_Legacy;
		Save->Progress.LevelName = Save->LevelName_Legacy;
		Save->Progress.PlayerData = Save->PlayerData_Legacy;
		Save->Progress.ProgressFlags = Save->ProgressFlags_Legacy;
		Save->Version = 2;
	}

	// 세션 누적: 마지막 SessionStart 이후 경과 시간을 누적
	UpdateTotalPlaySeconds();

	if (Bucket == ESaveBucket::Progress)
	{
		Save->bHasProgress = true;
		CaptureIntoCheckpoint(Save->Progress, /*bHealHPBeforeCapture*/false);
		CaptureAgentsIntoCheckpoint(Save->Progress, Bucket);
	}
	else if (Bucket == ESaveBucket::Rest)
	{
		Save->bHasRest = true;
		CaptureIntoCheckpoint(Save->Rest, /*bHealHPBeforeCapture*/true);
		CaptureAgentsIntoCheckpoint(Save->Rest, Bucket);
	}
	else // FullWorld (지금은 Progress와 동일 취급 — 필요 시 확장)
	{
		Save->bHasProgress = true;
		CaptureIntoCheckpoint(Save->Progress, /*bHealHPBeforeCapture*/false);
		CaptureAgentsIntoCheckpoint(Save->Progress, Bucket); // 필요 시 Progress에 담거나 별도 구조로
	}


	const bool bOK = UGameplayStatics::SaveGameToSlot(Save, GameName, GUserIndex);
	if (bOK)
	{
		UpsertCache(GameName, Save);
	}
	return bOK;
}

bool USaveSubsystem::LoadAndApply_Internal(const FString& GameName, ESaveBucket Bucket)
{
	USaveGame* Base = UGameplayStatics::LoadGameFromSlot(GameName, GUserIndex);
	if (!Base) return false;


	UJWSaveGame* Save = Cast<UJWSaveGame>(Base);
	if (!Save) return false;


	// v1 → v2 마이그레이션 경로
	if (Save->Version < 2 && !Save->bHasProgress && Save->PlayerData_Legacy.ActorClass.IsValid())
	{
		Save->bHasProgress = true;
		Save->Progress.SavedAt = Save->SavedAt_Legacy;
		Save->Progress.LevelName = Save->LevelName_Legacy;
		Save->Progress.PlayerData = Save->PlayerData_Legacy;
		Save->Progress.ProgressFlags = Save->ProgressFlags_Legacy;
		Save->Version = 2;
		UGameplayStatics::SaveGameToSlot(Save, GameName, GUserIndex);
	}


	const FJWCheckpoint* CP = nullptr;
	if (Bucket == ESaveBucket::Progress && Save->bHasProgress)
		CP = &Save->Progress;
	else if (Bucket == ESaveBucket::Rest && Save->bHasRest)
		CP = &Save->Rest;
	else if (Save->bHasProgress)
		CP = &Save->Progress; // 폴백
	else if (Save->bHasRest)
		CP = &Save->Rest; // 폴백


	if (!CP)
		return false;


	UWorld* World = GetWorld();
	if (!World) return false;


	const FName CurrentLevel = FName(*UGameplayStatics::GetCurrentLevelName(World, /*RemovePrefix*/true));
	if (/*CP->LevelName != CurrentLevel*/ true)
	{
		PendingGameToApply = GameName;
		PendingBucket = Bucket;
		FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USaveSubsystem::OnPostLoadMapWithWorld);
		// UGameplayStatics::OpenLevel(this, CP->LevelName);
		if (auto* GM = World->GetAuthGameMode<AJeonwoochiGameMode>())
		{
			GM->OpenLevelHard(CP->LevelName); // 서버 컨텍스트에서 ServerTravel 실행
		}
		return true;
	}

	FJWCheckpoint Merged = *CP;
	if (Bucket == ESaveBucket::Rest && Save->bHasProgress) // ← Rest 로드 때만
	{
		// Rest에 저장된 플래그 + Progress의 최신 플래그를 합친다
		Merged.ProgressFlags.AppendTags(Save->Progress.ProgressFlags);
	}

	ApplyCheckpointNow(Merged, Bucket == ESaveBucket::Rest);
	CurrentGameName = GameName; // 선택 갱신
	
	return true;
}

void USaveSubsystem::OnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
	if (PendingGameToApply.IsEmpty()) return;


	if (USaveGame* Base = UGameplayStatics::LoadGameFromSlot(PendingGameToApply, GUserIndex))
	{
		if (UJWSaveGame* Save = Cast<UJWSaveGame>(Base))
		{
			const FJWCheckpoint* CP = nullptr;
			// 요청했던 버킷을 우선
			if (PendingBucket == ESaveBucket::Rest)
				CP = Save->bHasRest ? &Save->Rest : (Save->bHasProgress ? &Save->Progress : nullptr);
			else
				CP = Save->bHasProgress ? &Save->Progress : (Save->bHasRest ? &Save->Rest : nullptr);

			if (CP)
			{
				const bool bDoRestRespawn = (PendingBucket == ESaveBucket::Rest);
				ApplyCheckpointNow(*CP, bDoRestRespawn);
			}
		}
	}
	CurrentGameName = PendingGameToApply;
	PendingGameToApply.Empty();
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
}

// ====== 게임 파일 스캔/정렬/삭제 ======
void USaveSubsystem::RebuildGameList(const FString& Prefix)
{
	CachedGames.Reset();


	const FString Dir = GetSaveDir();
	TArray<FString> Files;
	IFileManager::Get().FindFiles(Files, *(Dir / TEXT("*.sav")), true, false);


	int32 MaxSerialFound = -1;


	for (const FString& File : Files)
	{
		const FString GameName = FPaths::GetBaseFilename(File);
		if (!GameName.StartsWith(Prefix)) continue;


		if (USaveGame* Base = UGameplayStatics::LoadGameFromSlot(GameName, GUserIndex))
		{
			if (const UJWSaveGame* Save = Cast<UJWSaveGame>(Base))
			{
				const_cast<USaveSubsystem*>(this)->UpsertCache(GameName, Save);
			}
		}
		MaxSerialFound = FMath::Max(MaxSerialFound, ParseSerialFromName(GameName, Prefix));
	}


	NextSerial = FMath::Max(0, MaxSerialFound + 1);
}

bool USaveSubsystem::DeleteGame(const FString& GameName)
{
	const bool bOK = UGameplayStatics::DeleteGameInSlot(GameName, GUserIndex);
	if (bOK)
	{
		CachedGames.RemoveAll([&](const FJWGameInfo& S) { return S.GameName == GameName; });
		OnGameListChanged.Broadcast();
		if (CurrentGameName == GameName) CurrentGameName.Empty();
	}
	return bOK;
}

// ====== 스폰 시드 & 리스폰(기존 로직 유지) ======
void USaveSubsystem::EnsureSpawnSeedsBuilt() { if (SpawnSeeds.Num() == 0) BuildSpawnSeeds(); }


void USaveSubsystem::BuildSpawnSeeds()
{
	UWorld* World = GetWorld(); if (!World) return;

	static FName CachedLevel;                 // 마지막으로 시드를 만든 레벨명
	const FName NowLevel(*UGameplayStatics::GetCurrentLevelName(World, true));

	// 같은 맵에서 이미 만들어졌으면 재빌드 금지
	if (SpawnSeeds.Num() > 0 && CachedLevel == NowLevel) return;

	SpawnSeeds.Reset();
	CachedLevel = NowLevel;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* A = *It; if (!A) continue;
		if (const USaveTargetComponent* ST = A->FindComponentByClass<USaveTargetComponent>())
		{
			FSpawnSeed S;
			S.Id = ST->PersistentId;
			S.Class = ST->SpawnClass.IsValid() ? ST->SpawnClass : A->GetClass();
			S.Config = ST->MakeConfig();   // 전체 설정을 스냅샷

			// ! 템플릿이 컴포넌트/액터 밑이면 Destroy 시 같이 사라진다 → Subsystem 소유로 복제
			if (S.Config.ConfigDataTemplate)
			{
				S.Config.ConfigDataTemplate = 
					DuplicateObject<UAIConfigData>(S.Config.ConfigDataTemplate, this);
			}

			SpawnSeeds.Add(S.Id, MoveTemp(S));
		}
	}
}

AActor* USaveSubsystem::FindLiveById(const FGuid& Id) const
{
	if (!Id.IsValid()) return nullptr;
	UWorld* World = GetWorld(); if (!World) return nullptr;


	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (const USaveTargetComponent* ST = It->FindComponentByClass<USaveTargetComponent>())
			if (ST->PersistentId == Id)
				return *It;
	}
	return nullptr;
}

void USaveSubsystem::RespawnAllEnemiesForRest()
{
	EnsureSpawnSeedsBuilt();
	UWorld* World = GetWorld(); if (!World) return;


	for (const auto& Pair : SpawnSeeds)
	{
		const FSpawnSeed& S = Pair.Value;
		if (S.Config.bIsBoss) continue; // 일반 적만


		AActor* A = FindLiveById(S.Id);
		if (!A) // 액터 destroy되고 다시 스폰할 때.
		{
			UClass* Cls = S.Class.LoadSynchronous();
			if (!Cls) continue;

			A = UGameplayStatics::BeginDeferredActorSpawnFromClass(
				World, Cls, FTransform::Identity,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn, /*Owner=*/nullptr);
			if (!A) continue;

			USaveTargetComponent* ST = A->FindComponentByClass<USaveTargetComponent>();
			if (!ST)
			{
				ST = NewObject<USaveTargetComponent>(A);
			}
			ST->PersistentId = S.Id;
			ST->ApplyConfig(S.Config);                // 모든 설정 적용
			ST->ApplyConfigToOwnerExtras(S.Config);

			A = UGameplayStatics::FinishSpawningActor(A, S.Config.EditorTransform);

			if (!ST->IsRegistered())
			{
				ST->RegisterComponent();
			}
		}
		else // 액터 살아있는 상태에서 처리
		{
			A->SetActorTransform(S.Config.EditorTransform, false, nullptr, ETeleportType::TeleportPhysics);
			if (USaveTargetComponent* ST = A->FindComponentByClass<USaveTargetComponent>())
			{
				ST->PersistentId = S.Id;
				ST->ApplyConfig(S.Config);
				ST->ApplyConfigToOwnerExtras(S.Config);
			}
			ResetAIForActor(A);
		}
	}
}

void USaveSubsystem::ApplyBossDefeatedState()
{
	UWorld* World = GetWorld(); if (!World) return;
	if (SpawnSeeds.Num() == 0) BuildSpawnSeeds();

	UProgressionSubsystem* Prog = GetGameInstance()->GetSubsystem<UProgressionSubsystem>();


	for (const auto& Pair : SpawnSeeds)
	{
		const FSpawnSeed& S = Pair.Value;
		if (!S.Config.bIsBoss) continue;


		const bool bDefeated = (Prog && Prog->Has(S.Config.BossDefeatedTag));
		AActor* A = FindLiveById(S.Id);


		if (bDefeated)
		{
			if (A) { A->Destroy(); }
		}
		else if (!A)
		{
			UClass* Cls = S.Class.LoadSynchronous();
			if (!Cls) continue;


			FActorSpawnParameters P;
			P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			A = World->SpawnActor<AActor>(Cls, S.Config.EditorTransform, P);


			if (A && !A->FindComponentByClass<USaveTargetComponent>())
			{
				if (USaveTargetComponent* ST = NewObject<USaveTargetComponent>(A))
				{
					ST->RegisterComponent();
					ST->PersistentId = S.Id;
					ST->bIsEnemy = true;
					ST->bIsBoss = true;
					ST->EditorTransform = S.Config.EditorTransform;
					ST->SpawnClass = S.Class;
					ST->BossDefeatedTag = S.Config.BossDefeatedTag;
				}
			}
		}
	}
}

void USaveSubsystem::ResetAIForActor(AActor* A)
{
	APawn* P = Cast<APawn>(A); if (!P) return;


	AAIController* C = Cast<AAIController>(P->GetController());
	if (!C)
	{
		P->SpawnDefaultController();
		C = Cast<AAIController>(P->GetController());
	}
	if (!C) return;


	if (UBlackboardComponent* BB = C->GetBlackboardComponent())
	{
		if (const UBlackboardData* Data = BB->GetBlackboardAsset())
		{
			for (const FBlackboardEntry& Entry : Data->Keys)
			{
				BB->ClearValue(Entry.EntryName);
			}
		}
	}
}

static bool HasBucket(const USaveAgentComponent* SA, ESaveBucket Bucket)
{
	if (!SA) return false;
	const int32 Mask = SA->Buckets;
	switch (Bucket)
	{
	case ESaveBucket::Progress:  return !!(Mask & (int32)ESaveBucketFlags::Progress);
	case ESaveBucket::Rest:      return !!(Mask & (int32)ESaveBucketFlags::Rest);
	case ESaveBucket::FullWorld: return !!(Mask & (int32)ESaveBucketFlags::FullWorld);
	default: return false;
	}
}

void USaveSubsystem::CaptureAgentsIntoCheckpoint(FJWCheckpoint& Out, ESaveBucket Bucket)
{
	UWorld* World = GetWorld(); if (!World) return;

	Out.WorldAgents.Reset();

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* A = *It; if (!A) continue;

		// 플레이어/AI 적은 기존 SaveTarget 쪽 루틴에 맡기고,
		// 여기서는 범용 마커만 처리
		USaveAgentComponent* SA = A->FindComponentByClass<USaveAgentComponent>();
		if (!SA || !HasBucket(SA, Bucket)) continue;

		// 액터/컴포넌트 자동 직렬화( SaveGame 필드가 있는 것만 )
		FJWActorSaveData Rec;
		Rec.PersistentId = SA->PersistentId;
		Rec.ActorName = A->GetFName();
		Rec.ActorClass = A->GetClass();
		Rec.Transform = A->GetActorTransform();

		JW_SerializeToBytes(A, Rec.ActorBytes);

		TInlineComponentArray<UActorComponent*> Comps(A);
		for (UActorComponent* C : Comps)
		{
			if (!C) continue;
			if (!JW_HasSaveGameFields(C->GetClass())) continue;

			FJWComponentSaveData CRec;
			CRec.ComponentName = C->GetFName();
			CRec.ClassPath = C->GetClass();
			JW_SerializeToBytes(C, CRec.Bytes);
			Rec.Components.Add(MoveTemp(CRec));
		}
		Out.WorldAgents.Add(MoveTemp(Rec));
	}
}

void USaveSubsystem::ApplyAgentsFromCheckpoint(const FJWCheckpoint& CP)
{
	UWorld* World = GetWorld(); if (!World) return;

	for (const FJWActorSaveData& Rec : CP.WorldAgents)
	{
		// GUID로 현재 월드에서 찾기
		AActor* Target = nullptr;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (USaveAgentComponent* SA = It->FindComponentByClass<USaveAgentComponent>())
			{
				if (SA->PersistentId == Rec.PersistentId) { Target = *It; break; }
			}
		}

		// 없으면 스폰 (SA 설정을 모를 수 있으니 배우 클래스 기준)
		if (!Target)
		{
			UClass* Cls = Rec.ActorClass.LoadSynchronous();
			if (!Cls) continue;

			FActorSpawnParameters P;
			P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// 안전: 스폰은 Identity → 이후 절대 트랜스폼 적용
			Target = World->SpawnActor<AActor>(Cls, FTransform::Identity, P);
			if (!Target) continue;

			// 마커가 없다면 부착
			if (!Target->FindComponentByClass<USaveAgentComponent>())
			{
				auto* SA = NewObject<USaveAgentComponent>(Target);
				SA->PersistentId = Rec.PersistentId;
				SA->SpawnClass = Cls;
				SA->EditorTransform = Rec.Transform;
				SA->RegisterComponent();
			}
		}

		// 절대 트랜스폼 1회 적용(누적 방지)
		Target->SetActorLocationAndRotation(
			Rec.Transform.GetLocation(),
			Rec.Transform.Rotator(),
			false, nullptr, ETeleportType::TeleportPhysics);
		Target->SetActorScale3D(Rec.Transform.GetScale3D());

		// 액터/컴포넌트 역직렬화
		JW_DeserializeFromBytes(Target, Rec.ActorBytes);

		// 컴포넌트
		TInlineComponentArray<UActorComponent*> Existing(Target);
		auto FindByName = [&](FName Name)->UActorComponent*
			{
				for (UActorComponent* C : Existing)
					if (C->GetFName() == Name) return C;
				return nullptr;
			};

		for (const FJWComponentSaveData& CRec : Rec.Components)
		{
			if (UActorComponent* Comp = FindByName(CRec.ComponentName))
			{
				JW_DeserializeFromBytes(Comp, CRec.Bytes);
			}
		}
	}
	OnWorldAgentsApplied.Broadcast();
}