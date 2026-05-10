#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "TutorialTriggerBox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTutorialTriggered, AActor*, PlayerActor, FName, TutorialId);

UCLASS()
class JEONWOOCHI_API ATutorialTriggerBox : public ATriggerBox
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Tutorial")
    FTutorialTriggered OnTriggered;

    UPROPERTY(EditAnywhere, Category = "Tutorial")
    FName TutorialId = "TT_Move";

    UPROPERTY(EditAnywhere, Category = "Tutorial")
    bool bShowOnce = true;

    UPROPERTY(EditAnywhere, Category = "Tutorial")
    int32 RequiredLevel = 0;

protected:
    virtual void BeginPlay() override;                    
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;  
};
