/*  Project STARSHATTER WARS
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR: John DiCamillo
    ORIGINAL STUDIO: Destroyer Studios

    SUBSYSTEM:    Stars.exe
    FILE:         QuitView.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UQuitView
    - Unreal (UMG) port of legacy QuitView (End Mission menu).
    - Inherits from UView (UMG base).
    - UI uses UMG buttons (no manual mouse hit-testing).
    - Keeps all legacy mission/campaign logic.
*/

#include "QuitView.h"

// UMG:
#include "Components/Button.h"
#include "Components/TextBlock.h"

// Gameplay/legacy:
#include "Sim.h"
#include "Ship.h"
#include "SimContact.h"
#include "Campaign.h"
#include "Starshatter.h"
#include "Game.h"
#include "RadioView.h"
#include "GameScreen.h"
#include "GameStructs.h"

// Unreal:
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

UQuitView::UQuitView(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UQuitView::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Bind buttons once:
    if (BtnAccept)   BtnAccept->OnClicked.AddDynamic(this, &UQuitView::OnAcceptClicked);
    if (BtnDiscard)  BtnDiscard->OnClicked.AddDynamic(this, &UQuitView::OnDiscardClicked);
    if (BtnResume)   BtnResume->OnClicked.AddDynamic(this, &UQuitView::OnResumeClicked);
    if (BtnControls) BtnControls->OnClicked.AddDynamic(this, &UQuitView::OnControlsClicked);
}

void UQuitView::NativeConstruct()
{
    Super::NativeConstruct();

    // Start hidden by default:
    SetVisibility(ESlateVisibility::Hidden);
    bMenuShown = false;

    sim = Sim::GetSim();
    SetMessageText(TEXT(""));
}

void UQuitView::ExecFrame(float DeltaTime)
{
    // Keep as legacy hook point (called by your game loop if desired).
}

bool UQuitView::IsMenuShown() const
{
    return bMenuShown && GetVisibility() == ESlateVisibility::Visible;
}

void UQuitView::ShowMenu()
{
    if (IsMenuShown())
        return;

    bMenuShown = true;

    // Pause like legacy:
    if (Starshatter* Stars = Starshatter::GetInstance())
        Stars->Pause(true);

    // Show UI:
    SetMessageText(TEXT(""));
    SetVisibility(ESlateVisibility::Visible);
    ApplyMenuInputMode(true);
}

void UQuitView::CloseMenu()
{
    if (!IsMenuShown())
        return;

    bMenuShown = false;

    SetVisibility(ESlateVisibility::Hidden);
    ApplyMenuInputMode(false);

    if (Starshatter* Stars = Starshatter::GetInstance())
        Stars->Pause(false);
}

void UQuitView::ApplyMenuInputMode(bool bEnableMenu)
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
        return;

    if (bEnableMenu)
    {
        bPrevShowMouseCursor = PC->bShowMouseCursor;
        PC->bShowMouseCursor = true;

        // UI-only focus:
        FInputModeUIOnly Mode;
        Mode.SetWidgetToFocus(TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(Mode);
    }
    else
    {
        PC->bShowMouseCursor = bPrevShowMouseCursor;

        FInputModeGameOnly Mode;
        PC->SetInputMode(Mode);
    }
}

void UQuitView::SetMessageText(const FString& InText)
{
    if (TxtMessage)
    {
        TxtMessage->SetText(FText::FromString(InText));
    }
}

bool UQuitView::CanAccept()
{
    sim = Sim::GetSim();
    if (!sim)
        return false;

    Ship* PlayerShip = sim->GetPlayerShip();
    if (!PlayerShip)
        return false;

    // Legacy: must fly at least 60 seconds:
    if (PlayerShip->MissionClock() < 60000)
    {
        RadioView::Message(Game::GetText("QuitView.too-soon"));
        RadioView::Message(Game::GetText("QuitView.abort"));
        SetMessageText(TEXT("TOO SOON TO ACCEPT RESULTS."));
        return false;
    }

    ListIter<SimContact> iter = PlayerShip->ContactList();
    while (++iter)
    {
        SimContact* c = iter.value();
        if (!c) continue;

        Ship* cship = c->GetShip();
        int   ciff = c->GetIFF(PlayerShip);

        if (c->Threat(PlayerShip))
        {
            RadioView::Message(Game::GetText("QuitView.threats-present"));
            RadioView::Message(Game::GetText("QuitView.abort"));
            SetMessageText(TEXT("THREATS PRESENT. CANNOT ACCEPT RESULTS."));
            return false;
        }
        else if (cship && ciff > 0 && ciff != PlayerShip->GetIFF())
        {
            const FVector Delta = (c->Location() - PlayerShip->Location());
            const double  Dist = Delta.Length();

            if (cship->IsDropship() && Dist < 50e3)
            {
                RadioView::Message(Game::GetText("QuitView.threats-present"));
                RadioView::Message(Game::GetText("QuitView.abort"));
                SetMessageText(TEXT("DROPSHIP THREAT NEARBY. CANNOT ACCEPT RESULTS."));
                return false;
            }
            else if (cship->IsStarship() && Dist < 100e3)
            {
                RadioView::Message(Game::GetText("QuitView.threats-present"));
                RadioView::Message(Game::GetText("QuitView.abort"));
                SetMessageText(TEXT("STARSHIP THREAT NEARBY. CANNOT ACCEPT RESULTS."));
                return false;
            }
        }
    }

    return true;
}

// ------------------------------------------------------------
// Button handlers (same legacy actions, event-driven)
// ------------------------------------------------------------

void UQuitView::OnAcceptClicked()
{
    if (!CanAccept())
        return;

    CloseMenu();
    Game::SetTimeCompression(1);

    if (Starshatter* Stars = Starshatter::GetInstance())
        Stars->SetGameMode((int)EMODE::PLAN_MODE);
}

void UQuitView::OnDiscardClicked()
{
    CloseMenu();
    Game::SetTimeCompression(1);

    Starshatter* Stars = Starshatter::GetInstance();
    Campaign* CampaignPtr = Campaign::GetCampaign();

    // Discard mission and events:
    sim = Sim::GetSim();
    if (sim) sim->UnloadMission();
    else ShipStats::Initialize();

    if (CampaignPtr && CampaignPtr->GetCampaignId() < Campaign::SINGLE_MISSIONS)
    {
        CampaignPtr->RollbackMission();
        if (Stars) Stars->SetGameMode((int)EMODE::CMPN_MODE);
    }
    else
    {
        if (Stars) Stars->SetGameMode((int)EMODE::MENU_MODE);
    }
}

void UQuitView::OnResumeClicked()
{
    CloseMenu();
}

void UQuitView::OnControlsClicked()
{
    // Legacy behavior: open controls dialog from GameScreen:
    if (UGameScreen* GameScreen = UGameScreen::GetInstance())
    {
        CloseMenu();
        GameScreen->ShowCtlDlg();
    }
    else
    {
        CloseMenu();
    }
}
