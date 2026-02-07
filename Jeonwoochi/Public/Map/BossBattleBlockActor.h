// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossBattleBlockActor.generated.h"

UCLASS()
class JEONWOOCHI_API ABossBattleBlockActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABossBattleBlockActor();

	/** 지정한 대상의 Die 이벤트를 바인딩 */
	UPROPERTY(EditInstanceOnly, Category = "Block")
	TObjectPtr<class ABossCharacter> BindingTarget;

	UPROPERTY(EditAnywhere, Category = "Block")
	UStaticMeshComponent* StaticMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION() virtual void DestroyActor(AActor* DestroyActor, ECharacterType CharType);
};
