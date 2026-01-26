/*  Project STARSHATTER WARS
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR: John DiCamillo
    ORIGINAL STUDIO: Destroyer Studios

    SUBSYSTEM:    Stars.exe
    FILE:         QuitView.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UQuitView
    - Unreal (UMG) port of legacy QuitView (End Mission menu).
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

void
UQuitView::ExecFrame(float DeltaTime)
{

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
    Starshatter::GetInstance()->Pause(true);

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

    Starshatter::GetInstance()->Pause(false);
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

    Ship* player_ship = sim->GetPlayerShip();
    if (!player_ship)
        return false;

    // Legacy: must fly at least 60 seconds:
    if (player_ship->MissionClock() < 60000)
    {
        RadioView::Message(Game::GetText("QuitView.too-soon"));
        RadioView::Message(Game::GetText("QuitView.abort"));
        SetMessageText(TEXT("TOO SOON TO ACCEPT RESULTS."));
        return false;
    }

    ListIter<SimContact> iter = player_ship->ContactList();
    while (++iter)
    {
        SimContact* c = iter.value();
        if (!c) continue;

        Ship* cship = c->GetShip();
        int   ciff = c->GetIFF(player_ship);

        if (c->Threat(player_ship))
        {
            RadioView::Message(Game::GetText("QuitView.threats-present"));
            RadioView::Message(Game::GetText("QuitView.abort"));
            SetMessageText(TEXT("THREATS PRESENT. CANNOT ACCEPT RESULTS."));
            return false;
        }
        else if (cship && ciff > 0 && ciff != player_ship->GetIFF())
        {
            const FVector Delta = FVector(c->Location() - player_ship->Location());
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
// Button handlers (same legacy actions, just event-driven)
// ------------------------------------------------------------

void UQuitView::OnAcceptClicked()
{
    // Was mission long enough to accept?
    if (!CanAccept())
        return;

    CloseMenu();
    Game::SetTimeCompression(1);

    Starshatter* stars = Starshatter::GetInstance();
    if (stars)
        stars->SetGameMode((int)EMODE::PLAN_MODE);
}

void UQuitView::OnDiscardClicked()
{
    CloseMenu();
    Game::SetTimeCompression(1);

    Starshatter* stars = Starshatter::GetInstance();
    Campaign* campaign = Campaign::GetCampaign();

    // Discard mission and events:
    sim = Sim::GetSim();
    if (sim) sim->UnloadMission();
    else ShipStats::Initialize();

    if (campaign && campaign->GetCampaignId() < Campaign::SINGLE_MISSIONS)
    {
        campaign->RollbackMission();
        if (stars) stars->SetGameMode((int)EMODE::CMPN_MODE);
    }
    else
    {
        if (stars) stars->SetGameMode((int)EMODE::MENU_MODE);
    }
}

void UQuitView::OnResumeClicked()
{
    CloseMenu();
}

void UQuitView::OnControlsClicked()
{
    // Legacy behavior: open controls dialog from GameScreen:
    if (UGameScreen* game_screen = UGameScreen::GetInstance())
    {
        CloseMenu();
        game_screen->ShowCtlDlg();
    }
    else
    {
        CloseMenu();
    }
}
