// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ExecutionWidget.h"
#include "Components/CanvasPanel.h"

void UExecutionWidget::SetVisibility(ESlateVisibility InVisibility)
{
    Super::SetVisibility(InVisibility);

    if (InVisibility == ESlateVisibility::Visible || InVisibility == ESlateVisibility::SelfHitTestInvisible)
    {
        HandleCanExecution();
    }
    else
    {
        HandleCannotExecution();
    }
}

void UExecutionWidget::HandleCanExecution()
{
    // 애니메이션 무한 반복 재생
    if (ExecutionAnimation)
    {
        if (!IsAnimationPlaying(ExecutionAnimation))
        {
            // StartAtTime=0.f, NumLoopsToPlay=0(무한), PlayMode=Forward, PlayRate=1.f
            PlayAnimation(ExecutionAnimation, 0.f, 0, EUMGSequencePlayMode::Forward, 1.f);
        }
    }
}

void UExecutionWidget::HandleCannotExecution()
{
    // 애니메이션 정지
    if (ExecutionAnimation && IsAnimationPlaying(ExecutionAnimation))
    {
        StopAnimation(ExecutionAnimation);
    }
}