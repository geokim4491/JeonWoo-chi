// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndCreditsWidget.generated.h"

class UScrollBox;
class URichTextBlock;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreditsFinished);

/**
 * Ending Credit 위젯
 */
UCLASS()
class JEONWOOCHI_API UEndCreditsWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** 크레딧 본문(리치 텍스트 마크업 포함)을 설정 */
    UFUNCTION(BlueprintCallable, Category = "Credits")
    void SetCreditsText(const FText& InText);

    /** 스크롤 시작 (속도 옵션) */
    UFUNCTION(BlueprintCallable, Category = "Credits")
    void StartScroll(float InSpeed = 120.f);

    /** 크레딧 종료 델리게이트 (끝까지 올라가면 브로드캐스트) */
    UPROPERTY(BlueprintAssignable, Category = "Credits")
    FOnCreditsFinished OnCreditsFinished;

protected:
    /** UMG에서 BindWidget */
    UPROPERTY(meta = (BindWidget))
    UScrollBox* CreditsScroll;

    /** UMG에서 BindWidget */
    UPROPERTY(meta = (BindWidget))
    URichTextBlock* CreditsRichText;

    /** 새 방식: 줄 단위로 꽂아 넣을 컨테이너 */
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* CreditsList = nullptr;

    /** 줄 스타일(텍스트 스타일 세트: <H0>, <H2>, <H3> 등) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Credits")
    UDataTable* RichTextStyleSet = nullptr;

    /** 빈 줄 하나의 픽셀 높이 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Credits", meta = (ClampMin = "2.0"))
    float EmptyLineHeight = 28.f;

    /** 자동 줄바꿈 여부/폭 (필요 없으면 기본값 유지) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Credits")
    bool bAutoWrap = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Credits", meta = (EditCondition = "bAutoWrap", ClampMin = "0.0"))
    float WrapTextAt = 0.f;

    /** 초당 픽셀 스크롤 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Credits", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
    float ScrollSpeed = 120.f;

    /** 시작 전 여유 여백(보이는 영역 아래쪽에서 시작하도록 오프셋 보정) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Credits")
    float PreRollPad = 0.f;

    /** 끝났을 때 추가로 조금 더 올리고 종료하고 싶으면 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Credits")
    float PostRollPad = 100.f;

private:
    bool bIsScrolling = false;
    float CachedViewHeight = 0.f;
    float CachedContentHeight = 0.f;

    /** 텍스트를 줄 단위 위젯들로 빌드 */
    void BuildCreditsFromRaw(const FString& Raw);

    /** 매 틱 계산을 위한 보조 */
    float ComputeMaxOffset() const;
};
