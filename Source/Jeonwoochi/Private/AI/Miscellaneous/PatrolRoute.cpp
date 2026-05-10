// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Miscellaneous/PatrolRoute.h"
#include "Components/SplineComponent.h"

// Sets default values
APatrolRoute::APatrolRoute()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    PatrolSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PatrolSpline"));
    RootComponent = PatrolSpline;
    PatrolSpline->SetMobility(EComponentMobility::Static);
}
