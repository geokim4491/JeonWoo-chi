// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/JeonwoochiPlayerController.h"
#include "GameFramework/Character.h"
#include "Components/Combat/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/DeadWidget.h"
#include "UI/FadeInHoldOutWidget.h"
#include "TimerManager.h"
#include "UI/MainMenu.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TutorialHintWidget.h"
#include "UI/EndCreditsWidget.h"
#include "Blueprint/UserWidget.h"
#include "Miscellaneous/Actor/TutorialTriggerBox.h"
#include "Data/Save/SaveSubsystem.h"
#include "Jeonwoochi/JeonwoochiCharacter.h"

void AJeonwoochiPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    BindToPawn(InPawn);
}

void AJeonwoochiPlayerController::OnUnPossess()
{
    UnbindFromPawn();
    Super::OnUnPossess();
}

void AJeonwoochiPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 튜토리얼 힌트 위젯 1회 생성(있으면 재사용)
    if (TutorialHintWidgetClass && !TutorialHintWidget)
    {
        TutorialHintWidget = CreateWidget<UTutorialHintWidget>(this, TutorialHintWidgetClass);
        if (TutorialHintWidget)
        {
            TutorialHintWidget->AddToViewport(50);
            TutorialHintWidget->SetVisibility(ESlateVisibility::Collapsed); // 시작 시 숨김
        }
    }

    // 맵에 배치된 모든 TutorialTriggerBox의 브로드캐스트에 바인딩
    BindAllTutorialTriggers();

    bShowMouseCursor = false;

    FInputModeGameOnly Mode;
    Mode.SetConsumeCaptureMouseDown(true); // 필요 시
    SetInputMode(Mode);
}

void AJeonwoochiPlayerController::Transition(APawn* SwitchingActor)
{
	ACharacter* PlayerCharacter = Cast<ACharacter>(GetPawn());

	if (!PlayerCharacter) return;
	PlayerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	SetViewTargetWithBlend(SwitchingActor, BlendTime);

    GetWorld()->GetTimerManager().SetTimer(
        UnpossessTimerHandle,
        [this, SwitchingActor]()
        {
            UnpossessControlledPawn(SwitchingActor);
        },
        BlendTime,  // 블렌드 시간 후 실행
        false
    );
}

void AJeonwoochiPlayerController::UnpossessControlledPawn(APawn* SwitchingActor)
{
    UnPossess();

    Possess(SwitchingActor);
}

void AJeonwoochiPlayerController::BindToPawn(APawn* InPawn)
{
    UnbindFromPawn();

    if (!InPawn) return;
}

void AJeonwoochiPlayerController::UnbindFromPawn()
{
    if (BoundHealthComp)
    {
        BoundHealthComp->OnDeath.RemoveAll(this);
        BoundHealthComp = nullptr;
    }
}

void AJeonwoochiPlayerController::ToggleMainMenu()
{
    bMenuOpen ? CloseMainMenu() : OpenMainMenu();
}

void AJeonwoochiPlayerController::OpenMainMenu()
{
    if (!MainMenu && ensure(MainMenuClass)) MainMenu = CreateWidget<UMainMenu>(this, MainMenuClass);

    if (!MainMenu) return;

    MainMenu->AddToViewport();
    bMenuOpen = true;

    FInputModeGameAndUI Mode;
    Mode.SetHideCursorDuringCapture(false);
    Mode.SetWidgetToFocus(MainMenu->TakeWidget());
    SetInputMode(Mode);

    if (AJeonwoochiCharacter* PlayerCharacter = Cast<AJeonwoochiCharacter>(GetPawn()))
    {
        PlayerCharacter->ApplyIMC_UI();
    }
    bShowMouseCursor = true;
}

void AJeonwoochiPlayerController::CloseMainMenu()
{
    if (!MainMenu) return;

    MainMenu->RemoveFromParent();

    FInputModeGameOnly Mode;
    SetInputMode(Mode);

    bMenuOpen = false;

    if (AJeonwoochiCharacter* PlayerCharacter = Cast<AJeonwoochiCharacter>(GetPawn()))
    {
        PlayerCharacter->ApplyIMC_Default();
    }

    bShowMouseCursor = false;
}

void AJeonwoochiPlayerController::HandleDeath()
{
    /*
    if (DeadWidget || !DeadWidgetClass) return;  // 중복 생성 방지

    DeadWidget = CreateWidget<UDeadWidget>(this, DeadWidgetClass);
    if (DeadWidget)
    {
        DeadWidget->AddToViewport(10);

        bShowMouseCursor = true;
        // 싱글플레이면 필요에 따라 Pause
        // SetPause(true);
    }
    */
    if (UGameInstance* GI = GetGameInstance())
    {
        USaveSubsystem* SaveSub = GI->GetSubsystem<USaveSubsystem>();
        if (SaveSub)
        {
            SaveSub->UpdateTotalPlaySeconds();
            SaveSub->LoadFromRest();
        }
    }
    if (AJeonwoochiCharacter* JC = Cast<AJeonwoochiCharacter>(GetPawn()))
    {
        JC->ReviveFromRest();
    }

    if (YouDiedWidget)
    {
        YouDiedWidget->RemoveFromParent();
        YouDiedWidget = nullptr;
    }

    FInputModeGameOnly Mode;
    SetInputMode(Mode);
}

void AJeonwoochiPlayerController::ShowYouDiedUI()
{
    if (YouDiedWidget || !YouDiedWidgetClass) return;  // 중복 생성 방지

    YouDiedWidget = CreateWidget<UFadeInHoldOutWidget>(this, YouDiedWidgetClass);
    if (YouDiedWidget)
    {
        YouDiedWidget->AddToViewport(10);

        YouDiedWidget->SetTitleColor(FLinearColor(1.f, 0.f, 0.f));
        YouDiedWidget->SetTitleFontSize(110);

        const FText Text = FText::FromString(TEXT("사망"));

        YouDiedWidget->PlayFelled(Text, /*FadeIn=*/1.5f, /*Hold=*/2.f, /*FadeOut=*/1.f);

        FInputModeUIOnly InputMode;
        SetInputMode(InputMode);
        
        FTimerHandle TH;
        GetWorld()->GetTimerManager().SetTimer(TH, this, &AJeonwoochiPlayerController::HandleDeath, 4.8f, false);
    }
}

void AJeonwoochiPlayerController::BindAllTutorialTriggers()
{
    TArray<AActor*> Triggers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATutorialTriggerBox::StaticClass(), Triggers);

    for (AActor* A : Triggers)
    {
        if (auto* T = Cast<ATutorialTriggerBox>(A))
        {
            // 트리거가 브로드캐스트하는 델리게이트에 동적 바인딩
            // 시그니처: (AActor* PlayerActor, FName TutorialId) 가정
            T->OnTriggered.AddDynamic(this, &AJeonwoochiPlayerController::OnTutorialTriggered);
        }
    }
}

void AJeonwoochiPlayerController::OnTutorialTriggered(AActor* PlayerActor, FName TutorialId)
{
    // 로컬 플레이어의 트리거인지 검증
    if (!GetPawn() || PlayerActor != GetPawn()) return;

    if (TutorialHintWidget)
    {
        TutorialHintWidget->ShowById(TutorialId); // TutorialId로 내용 매핑하여 표시
        // 필요시: 특정 조건/시간이 지나면 자동 숨김은 위젯 내부 AutoHideSeconds로 처리
    }
}

void AJeonwoochiPlayerController::ShowEndCredits()
{
    if (!EndCreditsWidgetClass) return;

    UEndCreditsWidget* W = CreateWidget<UEndCreditsWidget>(this, EndCreditsWidgetClass);
    if (!W) return;

    // 리치 텍스트 마크업 문자열 구성 (여기선 간단 예시)
    static const TCHAR* RawCredits = LR"(
\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n
<H0>전우치</>
\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n
<H2>DOSA</>
\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n
<H2>Created by</>
<H3>Sungryung Park</>
<H3>Dohyeon Kim</>
<H3>Yugyeong Lim</>
<H3>Yeseo Park</>
\n\n\n\n\n\n\n\n\n
<H2>Direction & Production</>
<H3>Director - Sungryung Park</>
<H3>Producer - Sungryung Park</>
\n\n\n\n\n\n\n\n\n
<H2>Design</>
<H3>Game Design - Sungryung Park</>
<H3>Combat Design - Sungryung Park</>
<H3>Level Design - Sungryung Park</>
\n\n\n\n\n\n\n\n\n     
<H2>Programming</>
<H3>Main Programming - Sungryung Park</>
<H3>AI Programming - Sungryung Park</>
<H3>Animation Programming - Yeseo Park</>
<H3>UI Programming - Dohyeon Kim</>
<H3>2D Tools/Gameplay - Dohyeon Kim</>
\n\n\n\n\n\n\n\n\n
<H2>Art & Animation</>
<H3>Art - Yugyeong Lim</>
<H3>Character - Yugyeong Lim</>
<H3>2D Animations - Yugyeong Lim</>
<H3>UI/UX Art - Yugyeong Lim</>
\n\n\n\n\n\n\n\n\n
<H2>Cinematics & Camera</>
<H3>Sequencer - Sungryung Park</>
<H3>Camera Work - Sungryung Park</>
\n\n\n\n\n\n\n\n\n
<H2>QA & Playtest</>
<H3>Build/Packaging - Sungryung Park</>
<H3>Playtesters - Sungryung Park</>
<H3>Playtesters - Yeseo Park</>
<H3>Playtesters - Dohyeon Kim</>
\n\n\n\n\n\n\n\n\n
<H2>Contact & Links</>
<H3>Website / Steam Page - ....</>
<H3>Support / Email - geokim4499@khu.ac.kr</>
\n\n\n\n\n\n\n\n\n
<H2>build info</>
<H3>Version - 1.0.4</>
<H3>Build Date - 2025.12.04</>
\n\n\n\n\n\n\n\n\n
<H2>Special Thanks</>
<H3>Unreal Engine Community</>
<H3>From Software (Inspiration)</>
<H3>Shift Up (Inspiration)</>
<H3>Game Science (Inspiration)</>
\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n
<H2>UNREAL ENGINE 5</>
\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n
<H2>Thank you</>
\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n     
<H3>버그 관련 제보는 언제든 환영입니다!</>
\n\n\n
<H3>Jeon Woo - chi will continue to be updated...</>
\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n)";

    FString Credits(RawCredits);
    Credits.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
    Credits.ReplaceInline(TEXT("\\n"), TEXT("\n")); // 이 두 줄만 유지

    W->AddToViewport(100);
    W->SetCreditsText(FText::FromString(Credits)); // 내부에서 BuildCreditsFromRaw로 줄/스페이서 생성
    W->StartScroll(140.f);
}

