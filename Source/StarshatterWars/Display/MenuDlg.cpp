/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MenuDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Main Menu dialog (legacy MenuDlg) implementation for Unreal UMG.
*/

#include "MenuDlg.h"

// Unreal
#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "Starshatter.h"
#include "Campaign.h"
#include "MenuScreen.h"
#include "Game.h"

DEFINE_LOG_CATEGORY_STATIC(LogMenuDlg, Log, All);

// Provided by your project somewhere (legacy code used extern):
extern const char* versionInfo;

// --------------------------------------------------------------------

UMenuDlg::UMenuDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

// --------------------------------------------------------------------

void UMenuDlg::NativeConstruct()
{
    Super::NativeConstruct();

    Stars = Starshatter::GetInstance();
    CampaignPtr = Campaign::GetCampaign();

    RegisterControls();
    Show();
}

// --------------------------------------------------------------------

void UMenuDlg::RegisterControls()
{
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

    // Hover “alt text” (UMG doesn't have alt strings by default, so we store them):
    AltStart = TEXT("Start a new game, or resume your current game");
    AltCampaign = TEXT("Start a new dynamic campaign, or load a saved game");
    AltMission = TEXT("Play or create a scripted mission exercise");
    AltMulti = TEXT("Start or join a multiplayer scenario");
    AltPlayer = TEXT("Manage your logbook and player preferences");
    AltOptions = TEXT("Audio, Video, Gameplay, Control, and Mod configuration options");
    AltTac = TEXT("View ship and weapon stats and mission roles");
    AltQuit = TEXT("Exit Starshatter and return to Windows");

    // Hover bindings (OnHovered/OnUnhovered):
    if (BtnStart)
    {
        BtnStart->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Start);
        BtnStart->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Start);
    }

    if (BtnCampaign)
    {
        BtnCampaign->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Campaign);
        BtnCampaign->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Campaign);
    }

    if (BtnMission)
    {
        BtnMission->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Mission);
        BtnMission->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Mission);
    }

    if (BtnPlayer)
    {
        BtnPlayer->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Player);
        BtnPlayer->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Player);
    }

    if (BtnMulti)
    {
        BtnMulti->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Multi);
        BtnMulti->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Multi);
    }

    if (BtnOptions)
    {
        BtnOptions->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Options);
        BtnOptions->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Options);
    }

    if (BtnTac)
    {
        BtnTac->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Tac);
        BtnTac->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Tac);
    }

    if (BtnQuit)
    {
        BtnQuit->OnHovered.AddDynamic(this, &UMenuDlg::OnButtonEnter_Quit);
        BtnQuit->OnUnhovered.AddDynamic(this, &UMenuDlg::OnButtonExit_Quit);
    }

    // Version text:
    if (VersionText)
    {
        const char* Ver = versionInfo ? versionInfo : "";
        VersionText->SetText(FText::FromString(UTF8_TO_TCHAR(Ver)));
    }
}

// --------------------------------------------------------------------

void UMenuDlg::Show()
{
    // Legacy behavior: disable multiplayer when UseFileSystem() is true:
    if (BtnMulti && Starshatter::UseFileSystem())
    {
        BtnMulti->SetIsEnabled(false);
    }

    ClearDescription();
}

// --------------------------------------------------------------------

void UMenuDlg::ExecFrame()
{
    // Legacy was empty.
}

// --------------------------------------------------------------------
// Click handlers
// --------------------------------------------------------------------

void UMenuDlg::OnStart()
{
    ClearDescription();
    if (Stars)
        Stars->StartOrResumeGame();
}

void UMenuDlg::OnCampaign()
{
    ClearDescription();
    if (Manager)
        Manager->ShowCampaignSelectDlg();
}

void UMenuDlg::OnMission()
{
    ClearDescription();
    if (Manager)
        Manager->ShowMissionSelectDlg();
}

void UMenuDlg::OnPlayer()
{
    ClearDescription();
    if (Manager)
        Manager->ShowPlayerDlg();
}

void UMenuDlg::OnVideo()
{
    ClearDescription();
    if (Manager)
        Manager->ShowVidDlg();
}

void UMenuDlg::OnOptions()
{
    ClearDescription();
    if (Manager)
        Manager->ShowOptDlg();
}

void UMenuDlg::OnControls()
{
    ClearDescription();
    if (Manager)
        Manager->ShowCtlDlg();
}

void UMenuDlg::OnTacReference()
{
    ClearDescription();
    if (Stars)
        Stars->OpenTacticalReference();
}

void UMenuDlg::OnQuit()
{
    ClearDescription();
    if (Manager)
        Manager->ShowExitDlg();
}

// --------------------------------------------------------------------
// Hover handlers (Enter/Exit)
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
