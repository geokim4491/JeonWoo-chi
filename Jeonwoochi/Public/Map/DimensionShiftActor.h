// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DimensionShiftActor.generated.h"

class UDimensionShiftComponent;
class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class JEONWOOCHI_API ADimensionShiftActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADimensionShiftActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndplayReason) override;

	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "Dimension")
	UDimensionShiftComponent* DimensionShiftComp;

	UPROPERTY(EditAnywhere, Category = "Trigger")
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* StaticMesh;

private:


};
