// TutorialHintWidget.cpp  (신규 파일)

#include "UI/TutorialHintWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UTutorialHintWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetVisibility(ESlateVisibility::Collapsed); // 시작 시 숨김
}

void UTutorialHintWidget::ShowById(FName TutorialId)
{
    const FTutorialHintContent* Found = ContentById.Find(TutorialId);
    if (!Found)
    {
        // 찾지 못하면 표시하지 않음(필요 시 로그)
        return;
    }

    if (TitleText) TitleText->SetText(Found->Title);
    if (BodyText)  BodyText->SetText(Found->Body);
    if (IconImage) IconImage->SetBrushFromTexture(Found->Icon, true);

    // 바로 보여주기
    if (!IsInViewport())
    {
        AddToViewport(50);
    }
    SetVisibility(ESlateVisibility::HitTestInvisible);

    // 자동 숨김
    if (AutoHideSeconds > 0.f)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(AutoHideHandle);
            World->GetTimerManager().SetTimer(
                AutoHideHandle,
                this, &UTutorialHintWidget::HideHint,
                AutoHideSeconds, false
            );
        }
    }
}

void UTutorialHintWidget::HideHint()
{
    SetVisibility(ESlateVisibility::Collapsed);
}
