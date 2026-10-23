// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/EndCreditsWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/ScrollBox.h"
#include "Components/RichTextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBoxSlot.h"

void UEndCreditsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (CreditsScroll)
    {
        CreditsScroll->SetScrollOffset(0.f);
    }

    if (CreditsList && CreditsList->Slot)
    {
        if (UScrollBoxSlot* SBS = Cast<UScrollBoxSlot>(CreditsList->Slot))
        {
            SBS->SetHorizontalAlignment(HAlign_Fill);
        }
    }

    bIsScrolling = false;
    CachedViewHeight = 0.f;
    CachedContentHeight = 0.f;

    if (CreditsRichText)
    {
        CreditsRichText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UEndCreditsWidget::SetCreditsText(const FText& InText)
{
    // 문자열을 줄 단위로 위젯화
    BuildCreditsFromRaw(InText.ToString());

    // 레이아웃 강제 갱신 (스크롤 시작 전에 한 번)
    if (CreditsList) CreditsList->ForceLayoutPrepass();
    if (CreditsScroll) CreditsScroll->ForceLayoutPrepass();
}

void UEndCreditsWidget::BuildCreditsFromRaw(const FString& Raw)
{
    if (!CreditsList) return;
    CreditsList->ClearChildren();

    // DPI 스케일(에디터/패키징 간 일관 보장)
    const float Dpi = UWidgetLayoutLibrary::GetViewportScale(this) > 0.f
        ? UWidgetLayoutLibrary::GetViewportScale(this) : 1.f;

    // 1) 개행 정규화
    FString S = Raw;
    S.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
    S.ReplaceInline(TEXT("\\n"), TEXT("\n"));

    // 2) 줄 단위 분리(빈 줄 유지)
    TArray<FString> Lines;
    S.ParseIntoArrayLines(Lines, /*CullEmpty=*/false);

    auto AddRowCentered = [&](const FString& TextLine)
        {
            UHorizontalBox* Row = NewObject<UHorizontalBox>(this);

            // Left Fill
            {
                USpacer* Left = NewObject<USpacer>(this);
                if (UHorizontalBoxSlot* Ls = Row->AddChildToHorizontalBox(Left))
                {
                    Ls->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                }
            }

            // Center RichText
            URichTextBlock* R = NewObject<URichTextBlock>(this);
            if (RichTextStyleSet) R->SetTextStyleSet(RichTextStyleSet);
            R->SetJustification(ETextJustify::Center);
            R->SetText(FText::FromString(TextLine));

            if (UHorizontalBoxSlot* Cs = Row->AddChildToHorizontalBox(R))
            {
                Cs->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
                Cs->SetHorizontalAlignment(HAlign_Center);
            }

            // Right Fill
            {
                USpacer* Right = NewObject<USpacer>(this);
                if (UHorizontalBoxSlot* Rs = Row->AddChildToHorizontalBox(Right))
                {
                    Rs->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                }
            }

            if (UVerticalBoxSlot* Vs = CreditsList->AddChildToVerticalBox(Row))
            {
                Vs->SetHorizontalAlignment(HAlign_Fill);
            }
        };

    auto AddGapPx = [&](float Px)
        {
            if (Px <= 0.f) return;
            USpacer* Gap = NewObject<USpacer>(this);
            Gap->SetSize(FVector2D(1.f, Px * Dpi));
            CreditsList->AddChild(Gap);
        };

    // 3) 연속 빈줄을 '묶음'으로 감지해서 큰 Spacer로 변환
    const float LineGap = EmptyLineHeight;           // 한 줄 빈줄의 px(에디터에서 조정)
    const float RowGap = 6.f;                       // 텍스트 줄 사이 기본 간격(원하면 UPROPERTY로 승격)

    int32 i = 0;
    while (i < Lines.Num())
    {
        // 연속 빈 줄 카운트
        if (Lines[i].TrimStartAndEnd().IsEmpty())
        {
            int32 j = i;
            while (j < Lines.Num() && Lines[j].TrimStartAndEnd().IsEmpty())
                ++j;

            const int32 emptyCount = j - i;               // 연속 빈줄 개수
            AddGapPx(emptyCount * LineGap);               // ★ 한 번에 큰 Spacer로

            i = j;
            continue;
        }

        // 일반 텍스트 줄
        AddRowCentered(Lines[i]);

        // 다음 줄이 텍스트(빈줄 아님)라면 행 간격 추가
        if (i + 1 < Lines.Num() && !Lines[i + 1].TrimStartAndEnd().IsEmpty())
        {
            AddGapPx(RowGap);
        }

        ++i;
    }

    // 레이아웃 프리패스
    CreditsList->ForceLayoutPrepass();
    if (CreditsScroll) CreditsScroll->ForceLayoutPrepass();
}
void UEndCreditsWidget::StartScroll(float InSpeed)
{
    ScrollSpeed = InSpeed;
    bIsScrolling = true;

    // 시작 시 약간 아래에서부터 올라오게 하고 싶다면 PreRollPad 사용
    if (CreditsScroll && PreRollPad > 0.f)
    {
        CreditsScroll->SetScrollOffset(FMath::Max(0.f, -PreRollPad));
    }

    // 스크롤 시작 직전에 한 번 더 레이아웃 보정
    if (CreditsList) CreditsList->ForceLayoutPrepass();
    if (CreditsScroll) CreditsScroll->ForceLayoutPrepass();
}

float UEndCreditsWidget::ComputeMaxOffset() const
{
    const float ViewH = CachedViewHeight;
    const float ContentH = CachedContentHeight;

    if (ContentH <= ViewH) return 0.f;
    return (ContentH - ViewH) + PostRollPad;
}

void UEndCreditsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    if (!bIsScrolling || !CreditsScroll) return;

    // 매 틱 화면 높이/콘텐츠 높이 갱신
    CachedViewHeight = MyGeometry.GetLocalSize().Y;

    // ★ 이제는 List 전체 높이로 계산
    if (CreditsList)
    {
        CachedContentHeight = CreditsList->GetDesiredSize().Y;
    }
    else if (CreditsRichText)
    {
        // (백업) 예전 방식
        CachedContentHeight = CreditsRichText->GetDesiredSize().Y;
    }
    else
    {
        CachedContentHeight = 0.f;
    }

    const float MaxOffset = ComputeMaxOffset();
    const float Current = CreditsScroll->GetScrollOffset();
    const float Next = Current + ScrollSpeed * InDeltaTime;

    CreditsScroll->SetScrollOffset(Next);

    if (Next >= MaxOffset)
    {
        bIsScrolling = false;
        OnCreditsFinished.Broadcast();
    }
}