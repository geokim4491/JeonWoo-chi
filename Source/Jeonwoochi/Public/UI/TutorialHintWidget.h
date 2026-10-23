// TutorialHintWidget.h  (신규 파일)
// TutorialId(FName) → 콘텐츠 매핑으로 간단히 표시하는 UMG 위젯

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialHintWidget.generated.h"

class UTextBlock;
class UImage;

USTRUCT(BlueprintType)
struct FTutorialHintContent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Title;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = "true")) FText Body;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) UTexture2D* Icon = nullptr;
};

UCLASS()
class JEONWOOCHI_API UTutorialHintWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 에디터에서 TutorialId → 콘텐츠 매핑(또는 DataTable로 대체 가능)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
    TMap<FName, FTutorialHintContent> ContentById;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial")
    float AutoHideSeconds = 0.f; // 0이면 자동 숨김 없음

    // 위젯 컴포넌트 바인딩
    UPROPERTY(meta = (BindWidget)) UTextBlock* TitleText = nullptr;
    UPROPERTY(meta = (BindWidget)) UTextBlock* BodyText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UImage* IconImage = nullptr;

    UFUNCTION(BlueprintCallable)
    void ShowById(FName TutorialId);

    UFUNCTION(BlueprintCallable)
    void HideHint();

protected:
    virtual void NativeConstruct() override;

private:
    FTimerHandle AutoHideHandle;
};
