// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Interact/InteractInterface.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "BonfireActor.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBonfire, AActor*);

class AJeonwoochiCharacter;
class UInteractTriggerComponent;
class USaveAgentComponent;
class UFadeInHoldOutWidget;
enum class EEffectType : uint8;

UCLASS()
class JEONWOOCHI_API ABonfireActor : public AActor,
	public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABonfireActor();

	virtual void Interact_Implementation(AActor* Target) override;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* BonfireMontage;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UFadeInHoldOutWidget> FindWidgetClass;

	FOnBonfire OnBonfire;

	UPROPERTY(EditAnywhere, Category = "BoxTrriger")
	UInteractTriggerComponent* InteractTrrigerComp = nullptr;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	UNiagaraSystem* Niagara = nullptr;

	UPROPERTY(EditAnywhere, Category = "Cascade")
	UParticleSystem* Emitter = nullptr;

	UPROPERTY()
	USaveAgentComponent* SaveComp = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void OnBonfireMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void PlayEffect(EEffectType EffectType);

	void SetCharacterTransform();

	UPROPERTY(SaveGame)
	bool bIsVisited = false;

	// FX 토글 헬퍼
	void RefreshVisitedFX();
	UNiagaraComponent* GetOrCreateNiagaraComp();
	UParticleSystemComponent* GetOrCreateCascadeComp();

	// 내부 핸들
	AJeonwoochiCharacter* Char = nullptr;
	UPROPERTY(Transient) UNiagaraComponent* NiagaraComp = nullptr;
	UPROPERTY(Transient) UParticleSystemComponent* CascadeComp = nullptr;

	// SaveSubsystem 시그널 수신용
	void BindSaveSignals();
	void UnbindSaveSignals();
	void OnWorldAgentsApplied(); // 저장 적용 후 호출되어 FX 동기화
};
