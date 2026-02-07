#include "Miscellaneous/Actor/TutorialTriggerBox.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/Utilities/PlayerLevelComponent.h"

void ATutorialTriggerBox::BeginPlay()
{
    Super::BeginPlay();

    // ATriggerBox(ATriggerBase)의 CollisionComponent 접근
    if (UShapeComponent* Coll = GetCollisionComponent())
    {
        Coll->SetGenerateOverlapEvents(true);
        Coll->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Coll->SetCollisionResponseToAllChannels(ECR_Ignore);
        Coll->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }
}

void ATutorialTriggerBox::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (ACharacter* Char = Cast<ACharacter>(OtherActor))
    {
        // 레벨 조건
        if (RequiredLevel > 0)
        {
            if (UPlayerLevelComponent* LC = Char->FindComponentByClass<UPlayerLevelComponent>())
            {
                if (LC->GetLevel() < RequiredLevel)
                {
                    return;
                }
            }
        }

        // 1회 표시 (간단 태깅)
        if (bShowOnce && !TutorialId.IsNone() && Char->Tags.Contains(TutorialId))
        {
            return;
        }

        OnTriggered.Broadcast(Char, TutorialId);

        if (bShowOnce && !TutorialId.IsNone())
        {
            Char->Tags.AddUnique(TutorialId);
        }
    }
}
