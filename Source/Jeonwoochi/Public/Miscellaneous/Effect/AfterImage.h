// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AfterImage.generated.h"

class UPoseableMeshComponent;
class USkeletalMeshComponent;

UCLASS()
class JEONWOOCHI_API AAfterImage : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAfterImage();

	/** 소스 메시 컴포넌트와 지속시간 설정 (Spawn 직후 호출) */
	void Initialize(
		USkeletalMeshComponent* InSourceMesh,
		float InLifeSpan = 0.5f,
		UMaterialInterface* OverlayMaterial = nullptr);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	/** 실제 렌더링할 포즈 가능한 메시 */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPoseableMeshComponent* PoseableMesh;

	/** 복사할 원본 메시 */
	USkeletalMeshComponent* SourceMesh;

	// 바디 스냅샷을 따라가는 Skeletal Follower (소켓/그룸용 부모)
	UPROPERTY(Transient)
	USkeletalMeshComponent* BodyFollower = nullptr;

	// 소스 스켈메시 -> 스냅샷된 컴포넌트(바디는 Follower, 의상도 Follower)
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, class USceneComponent*> SnapshotMap;

	/** 자동 제거될 시간(초) */
	float LifeSpan = 2.0f;

};
