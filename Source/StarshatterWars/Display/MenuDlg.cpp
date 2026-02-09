/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MenuDlg.cpp
    AUTHOR:       Carlos Bott
*/

#include "MenuDlg.h"

// UMG
#include "Components/Button.h"
#include "Components/TextBlock.h"

// Legacy
#include "Starshatter.h"
#include "Campaign.h"
#include "MenuScreen.h"
#include "StarshatterPlayerSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogMenuDlg, Log, All);

UMenuDlg::UMenuDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UMenuDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Static strings (only need to set once)
    AltStart = TEXT("Start a new game, or resume your current game");
    AltCampaign = TEXT("Start a new dynamic campaign, or load a saved game");
    AltMission = TEXT("Play or create a scripted mission exercise");
    AltMulti = TEXT("Start or join a multiplayer scenario");
    AltPlayer = TEXT("Manage your logbook and player preferences");
    AltOptions = TEXT("Audio, Video, Gameplay, Control, and Mod configuration options");
    AltTac = TEXT("View ship and weapon stats and mission roles");
    AltQuit = TEXT("Exit Starshatter and return to Windows");

    BindUMGDelegates();
}

void UMenuDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache legacy singletons (fine to do here)
    Stars = Starshatter::GetInstance();
    CampaignPtr = Campaign::GetCampaign();

    // Version text
    if (VersionText)
    {
        VersionText->SetText(FText::FromString(Game::VersionInfo));
    }
    else
    {
        UE_LOG(LogMenuDlg, Warning, TEXT("VersionText is not bound (BindWidgetOptional)."));
    }

    RefreshFromPlayerState();
    ApplyMenuGating();

    // First run routing AFTER gating
    if (bFirstRun_NoPlayerSave)
    {
        if (Manager)
        {
            Manager->ShowFirstTimeDlg();
        }
        else
        {
            UE_LOG(LogMenuDlg, Warning, TEXT("First run detected, but Manager is null (ExposeOnSpawn not set?)."));
        }
    }

    Show();
}

void UMenuDlg::BindUMGDelegates()
{
    if (bDelegatesBound)
        return;

    // Click bindings:
    if (BtnStart)    BtnStart->OnClicked.AddDynamic(this, &UMenuDlg::OnStart);
    if (BtnCampaign) BtnCampaign->OnClicked.AddDynamic(this, &UMenuDlg::OnCampaign);
    if (BtnMission)  BtnMission->OnClicked.AddDynamic(this, &UMenuDlg::OnMission);
    if (BtnPlayer)   BtnPlayer->OnClicked.AddDynamic(this, &UMenuDlg::OnPlayer);
    if (BtnMulti)    BtnMulti->OnClicked.AddDynamic(this, &UMenuDlg::OnMultiplayer);

    if (BtnVideo)    BtnVideo->OnClicked.AddDynamic(this, &UMenuDlg::OnVideo);
    if (BtnOptions)  BtnOptions->OnClicked.AddDynamic(this, &UMenuDlg::OnOptions);
    if (BtnControls) BtnControls->OnClicked.AddDynamic(this, &UMenuDlg::OnControls);

    if (BtnMod)      BtnMod->OnClicked.AddDynamic(this, &UMenuDlg::OnMod);
    if (BtnTac)      BtnTac->OnClicked.AddDynamic(this, &UMenuDlg::OnTacReference);
    if (BtnQuit)     BtnQuit->OnClicked.AddDynamic(this, &UMenuDlg::OnQuit);

    // Hover bindings:
    if (BtnStart) { BtnStart->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Start);      BtnStart->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Start); }
    if (BtnCampaign) { BtnCampaign->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Campaign); BtnCampaign->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Campaign); }
    if (BtnMission) { BtnMission->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Mission);  BtnMission->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Mission); }
    if (BtnPlayer) { BtnPlayer->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Player);   BtnPlayer->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Player); }
    if (BtnMulti) { BtnMulti->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Multi);     BtnMulti->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Multi); }
    if (BtnOptions) { BtnOptions->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Options); BtnOptions->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Options); }
    if (BtnTac) { BtnTac->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Tac);         BtnTac->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Tac); }
    if (BtnQuit) { BtnQuit->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Quit);       BtnQuit->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Quit); }

    // If *everything* is null, your BP widget names don’t match, or they’re not marked “Is Variable”
    const bool bAny =
        (BtnStart || BtnCampaign || BtnMission || BtnPlayer || BtnMulti ||
            BtnVideo || BtnOptions || BtnControls || BtnMod || BtnTac || BtnQuit);

    if (!bAny)
    {
        UE_LOG(LogMenuDlg, Error, TEXT("NO BUTTON WIDGETS ARE BOUND. Check BP widget names match C++ (BtnStart, BtnCampaign, etc) and are variables."));
    }

    bDelegatesBound = true;
}

void UMenuDlg::RefreshFromPlayerState()
{
    // Default assumptions
    bFirstRun_NoPlayerSave = false;
    bHasCampaignSelected = true;

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UStarshatterPlayerSubsystem* PlayerSS = GI->GetSubsystem<UStarshatterPlayerSubsystem>())
        {
            if (!PlayerSS->HasLoaded())
            {
                PlayerSS->LoadFromBoot();
            }

            const bool bHasPlayerSave = PlayerSS->HadExistingSaveOnLoad();
            bFirstRun_NoPlayerSave = !bHasPlayerSave;

            if (bHasPlayerSave)
            {
                const FS_PlayerGameInfo& Info = PlayerSS->GetPlayerInfo();
                bHasCampaignSelected = (Info.Campaign > 0) && (!Info.CampaignRowName.IsNone());
            }
            else
            {
                bHasCampaignSelected = false;
            }
        }
    }
}

void UMenuDlg::Show()
{
    // Legacy behavior: disable multiplayer when UseFileSystem() is true:
    if (BtnMulti && Starshatter::UseFileSystem())
        BtnMulti->SetIsEnabled(false);

    ClearDescription();
}

void UMenuDlg::ExecFrame(double DeltaTime)
{
    // Legacy was empty.
}

// --------------------------------------------------------------------
// Click handlers
// --------------------------------------------------------------------

void UMenuDlg::OnStart()
{
    ClearDescription();
    UE_LOG(LogMenuDlg, Log, TEXT("OnStart()"));

    if (Stars)
        Stars->StartOrResumeGame();
    else
        UE_LOG(LogMenuDlg, Warning, TEXT("Stars is null in OnStart()."));
}

void UMenuDlg::OnCampaign()
{
    ClearDescription();
    UE_LOG(LogMenuDlg, Log, TEXT("OnCampaign()"));

    if (Manager)
        Manager->ShowCampaignSelectDlg();
    else
        UE_LOG(LogMenuDlg, Warning, TEXT("Manager is null in OnCampaign()."));
}

void UMenuDlg::OnMission()
{
    ClearDescription();
    UE_LOG(LogMenuDlg, Log, TEXT("OnMission()"));

    if (Manager)
        Manager->ShowMissionSelectDlg();
    else
        UE_LOG(LogMenuDlg, Warning, TEXT("Manager is null in OnMission()."));
}

void UMenuDlg::OnPlayer()
{
    ClearDescription();
    UE_LOG(LogMenuDlg, Log, TEXT("OnPlayer()"));

    if (Manager)
        Manager->ShowPlayerDlg();
    else
        UE_LOG(LogMenuDlg, Warning, TEXT("Manager is null in OnPlayer()."));
}

void UMenuDlg::OnMultiplayer()
{
    ClearDescription();
    UE_LOG(LogMenuDlg, Log, TEXT("OnMultiplayer()"));

    // Hook this up when you implement multiplayer lobby routing
    // if (Manager) Manager->ShowMultiplayerDlg();
}

void UMenuDlg::OnMod()
{
    ClearDescription();
    UE_LOG(LogMenuDlg, Log, TEXT("OnMod()"));
    // Hook later
}

void UMenuDlg::OnVideo()
{
    ClearDescription();
    UE_LOG(LogMenuDlg, Log, TEXT("OnVideo()"));

    if (Manager)
        Manager->ShowOptionsScreen();
    else
        UE_LOG(LogMenuDlg, Warning, TEXT("Manager is null in OnVideo()."));
}

void UMenuDlg::OnOptions()
{
    ClearDescription();
    UE_LOG(LogMenuDlg, Log, TEXT("OnOptions()"));

    if (Manager)
        Manager->ShowOptionsScreen();
    else
        UE_LOG(LogMenuDlg, Warning, TEXT("Manager is null in OnOptions()."));
}

void UMenuDlg::OnControls()
{
    ClearDescription();
    UE_LOG(LogMenuDlg, Log, TEXT("OnControls()"));

    if (Manager)
        Manager->ShowOptionsScreen();
    else
        UE_LOG(LogMenuDlg, Warning, TEXT("Manager is null in OnControls()."));
}

void UMenuDlg::OnTacReference()
{
    ClearDescription();
    UE_LOG(LogMenuDlg, Log, TEXT("OnTacReference()"));

    if (Stars)
        Stars->OpenTacticalReference();
    else
        UE_LOG(LogMenuDlg, Warning, TEXT("Stars is null in OnTacReference()."));
}

void UMenuDlg::OnQuit()
{
    ClearDescription();
    UE_LOG(LogMenuDlg, Log, TEXT("OnQuit()"));

    if (Manager)
        Manager->ShowExitDlg();
    else
        UE_LOG(LogMenuDlg, Warning, TEXT("Manager is null in OnQuit()."));
}

// --------------------------------------------------------------------
// Hover handlers
// --------------------------------------------------------------------

void UMenuDlg::OnButtonEnter_Start() { SetDescription(AltStart); }
void UMenuDlg::OnButtonExit_Start() { ClearDescription(); }

void UMenuDlg::OnButtonEnter_Campaign() { SetDescription(AltCampaign); }
void UMenuDlg::OnButtonExit_Campaign() { ClearDescription(); }

void UMenuDlg::OnButtonEnter_Mission() { SetDescription(AltMission); }
void UMenuDlg::OnButtonExit_Mission() { ClearDescription(); }

void UMenuDlg::OnButtonEnter_Player() { SetDescription(AltPlayer); }
void UMenuDlg::OnButtonExit_Player() { ClearDescription(); }

void UMenuDlg::OnButtonEnter_Multi() { SetDescription(AltMulti); }
void UMenuDlg::OnButtonExit_Multi() { ClearDescription(); }

void UMenuDlg::OnButtonEnter_Options() { SetDescription(AltOptions); }
void UMenuDlg::OnButtonExit_Options() { ClearDescription(); }

void UMenuDlg::OnButtonEnter_Tac() { SetDescription(AltTac); }
void UMenuDlg::OnButtonExit_Tac() { ClearDescription(); }

void UMenuDlg::OnButtonEnter_Quit() { SetDescription(AltQuit); }
void UMenuDlg::OnButtonExit_Quit() { ClearDescription(); }

// --------------------------------------------------------------------
// Helpers
// --------------------------------------------------------------------

void UMenuDlg::ClearDescription()
{
    if (DescriptionText)
        DescriptionText->SetText(FText::GetEmpty());
}

void UMenuDlg::SetDescription(const FString& Text)
{
    if (DescriptionText)
        DescriptionText->SetText(FText::FromString(Text));
}

void UMenuDlg::SetButtonEnabled(UButton* Button, bool bEnable)
{
    if (Button)
        Button->SetIsEnabled(bEnable);
}

void UMenuDlg::EnableMenuButtons(bool bEnable)
{
    SetButtonEnabled(BtnStart, bEnable);
    SetButtonEnabled(BtnCampaign, bEnable);
    SetButtonEnabled(BtnMission, bEnable);
    SetButtonEnabled(BtnPlayer, bEnable);

    if (BtnMulti && Starshatter::UseFileSystem())
        SetButtonEnabled(BtnMulti, false);
    else
        SetButtonEnabled(BtnMulti, bEnable);

    SetButtonEnabled(BtnVideo, bEnable);
    SetButtonEnabled(BtnOptions, bEnable);
    SetButtonEnabled(BtnControls, bEnable);

    SetButtonEnabled(BtnMod, bEnable);
    SetButtonEnabled(BtnTac, bEnable);

    // Quit is always allowed in your legacy logic
    SetButtonEnabled(BtnQuit, true);
}

void UMenuDlg::ApplyMenuGating()
{
    // FIRST RUN: disable everything, no exceptions
    if (bFirstRun_NoPlayerSave)
    {
        EnableMenuButtons(false);
        return;
    }

    // Default: enable everything (subject to multiplayer rule inside EnableMenuButtons)
    EnableMenuButtons(true);

    // NO CAMPAIGN SELECTED: only Exit/Tac/Options/Campaign enabled
    if (!bHasCampaignSelected)
    {
        EnableMenuButtons(false);

        SetButtonEnabled(BtnQuit, true);
        SetButtonEnabled(BtnTac, true);
        SetButtonEnabled(BtnCampaign, true);

        // Options hub buttons
        SetButtonEnabled(BtnOptions, true);
        SetButtonEnabled(BtnVideo, true);
        SetButtonEnabled(BtnControls, true);

        // Preserve legacy multiplayer filesystem rule
        if (BtnMulti && Starshatter::UseFileSystem())
            SetButtonEnabled(BtnMulti, false);
    }
}
