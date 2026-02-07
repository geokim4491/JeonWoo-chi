// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AI/Boss/DemonLordCharacter.h"
#include "UI/FadeInHoldOutWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Data/Save/SaveTargetComponent.h"

void ADemonLordCharacter::ChangePhase_Implementation(int32 CurrentPhase)
{
    UE_LOG(LogTemp, Warning, TEXT("DemonLord 2 Phase!!!!!"));
    BossName = FText::FromString("악마군주");

    SaveComp->BossId = "DemonLord";
}

void ADemonLordCharacter::Die_Implementation()
{
	Super::Die_Implementation();
	if (!GreatFelledWidgetClass) return;

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (UFadeInHoldOutWidget* Wdg = CreateWidget<UFadeInHoldOutWidget>(PC, GreatFelledWidgetClass))
        {
            Wdg->AddToViewport(10000);

            // (선택) 스타일/사운드 주입
            Wdg->SetTitleColor(FLinearColor(1.f, 0.f, 0.f));
            // Wdg->SetTitleFontSize(90);
            // Wdg->AppearSound = AppearSfx;
            // Wdg->DisappearSound = DisappearSfx;

            // 보스별 텍스트를 소스코드에서 지정 (엔터 포함 가능)
            const FText Text = FText::FromString(TEXT("악마 군주 쓰러지다"));
            // 예) 한글/멀티라인: FText::FromString(TEXT("위대한 적 처치\n(GREAT ENEMY FELLED)"));

            // 시간: 페이드 인 / 홀드 / 페이드 아웃
            Wdg->PlayFelled(Text, /*FadeIn=*/1.6f, /*Hold=*/2.6f, /*FadeOut=*/1.6f);
        }
    }
}