// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolRoute.generated.h"

class USplineComponent;

UCLASS()
class JEONWOOCHI_API APatrolRoute : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	APatrolRoute();

	// 에디터에서 경로를 구성할 수 있게 VisibleAnywhere
	UPROPERTY(VisibleAnywhere, Category = "Patrol")
	USplineComponent* PatrolSpline;

};
