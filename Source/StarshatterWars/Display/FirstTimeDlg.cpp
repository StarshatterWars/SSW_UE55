/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025–2026.

    DIALOG:      FirstTimeDlg
    FILE:        FirstTimeDlg.cpp
    AUTHOR:      Carlos Bott

    OVERVIEW
    ========
    First-time player setup dialog.

    Unreal UMG replacement for legacy FirstTimeDlg.frm.
    Handles:
      - Player name creation
      - Play style selection (Arcade / Standard)
      - Experience level (Cadet / Admiral)
      - Initial key bindings
      - Player save

    IMPLEMENTATION NOTES
    ====================
    - Authoritative profile state is FS_PlayerGameInfo in UStarshatterPlayerSubsystem.
    - This dialog does NOT call legacy PlayerCharacter mutators (not present in your port).
    - Runtime control model + key bindings are applied through Starshatter/KeyMap/Ship.
*/

#include "FirstTimeDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"

// Legacy runtime hooks:
#include "Starshatter.h"
#include "Ship.h"
#include "KeyMap.h"

// Router:
#include "MenuScreen.h"
#include "MenuDlg.h"

// Save model:
#include "StarshatterPlayerSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogFirstTimeDlg, Log, All);

/* --------------------------------------------------------------------
   Construction
   -------------------------------------------------------------------- */

UFirstTimeDlg::UFirstTimeDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsFocusable(true);
}

/* --------------------------------------------------------------------
   NativeConstruct
   -------------------------------------------------------------------- */

void UFirstTimeDlg::NativeConstruct()
{
    Super::NativeConstruct();

    PopulateDefaultsIfNeeded();

    if (AcceptBtn)
    {
        AcceptBtn->OnClicked.RemoveAll(this);
        AcceptBtn->OnClicked.AddDynamic(this, &UFirstTimeDlg::OnAcceptClicked);
    }
    else
    {
        UE_LOG(LogFirstTimeDlg, Error,
            TEXT("AcceptBtn is NULL (BindWidget name mismatch or not marked 'Is Variable' in BP)."));
    }
}

/* --------------------------------------------------------------------
   PopulateDefaultsIfNeeded
   -------------------------------------------------------------------- */

void UFirstTimeDlg::PopulateDefaultsIfNeeded()
{
    // Play Style
    if (PlayStyleCombo)
    {
        if (PlayStyleCombo->GetOptionCount() == 0)
        {
            PlayStyleCombo->AddOption(TEXT("Arcade Style"));
            PlayStyleCombo->AddOption(TEXT("Standard Model"));
        }

        if (PlayStyleCombo->GetSelectedIndex() < 0)
            PlayStyleCombo->SetSelectedIndex(0);
    }
    else
    {
        UE_LOG(LogFirstTimeDlg, Error, TEXT("PlayStyleCombo is NULL (BindWidget mismatch)."));
    }

    // Experience
    if (ExperienceCombo)
    {
        if (ExperienceCombo->GetOptionCount() == 0)
        {
            ExperienceCombo->AddOption(TEXT("Cadet (First timer)"));
            ExperienceCombo->AddOption(TEXT("Admiral (Experienced)"));
        }

        if (ExperienceCombo->GetSelectedIndex() < 0)
            ExperienceCombo->SetSelectedIndex(0);
    }
    else
    {
        UE_LOG(LogFirstTimeDlg, Error, TEXT("ExperienceCombo is NULL (BindWidget mismatch)."));
    }

    // Name hint
    if (NameEdit)
    {
        if (NameEdit->GetHintText().IsEmpty())
            NameEdit->SetHintText(FText::FromString(TEXT("Enter player name")));
    }
    else
    {
        UE_LOG(LogFirstTimeDlg, Warning, TEXT("NameEdit is NULL (BindWidget mismatch)."));
    }
}

/* --------------------------------------------------------------------
   OnAcceptClicked
   -------------------------------------------------------------------- */

void UFirstTimeDlg::OnAcceptClicked()
{
    UE_LOG(LogFirstTimeDlg, Log, TEXT("UFirstTimeDlg::OnAcceptClicked()"));

    Starshatter* Stars = Starshatter::GetInstance();

    // ------------------------------------------------------------
    // Acquire authoritative save model
    // ------------------------------------------------------------

    UStarshatterPlayerSubsystem* PlayerSS = UStarshatterPlayerSubsystem::Get(this);
    if (!PlayerSS)
    {
        UE_LOG(LogFirstTimeDlg, Error, TEXT("OnAcceptClicked: PlayerSubsystem is null"));
        return;
    }

    FS_PlayerGameInfo& Info = PlayerSS->GetMutablePlayerInfo();

    // ------------------------------------------------------------
    // Name / identity
    // ------------------------------------------------------------

    FString PlayerName;
    if (NameEdit)
        PlayerName = NameEdit->GetText().ToString().TrimStartAndEnd();

    if (!PlayerName.IsEmpty())
        Info.Name = PlayerName;
    else if (Info.Name.IsEmpty())
        Info.Name = TEXT("Ready Player One");

    // Optional: stamp create time if you want (campaign time uses sim clock)
    if (Info.CreateTime == 0)
        Info.CreateTime = FDateTime::UtcNow().ToUnixTimestamp();

    // ------------------------------------------------------------
    // Play Style -> profile prefs + runtime control model
    // ------------------------------------------------------------

    int32 PlayStyleIndex = 0; // 0=Arcade, 1=Standard
    if (PlayStyleCombo)
    {
        PlayStyleIndex = PlayStyleCombo->GetSelectedIndex();
        if (PlayStyleIndex < 0) PlayStyleIndex = 0;
    }

    if (PlayStyleIndex == 0)
    {
        // ARCADE (mirrors your legacy intent)
        Info.FlightModel = 2;
        Info.LandingMode = 1;
        Info.HudMode = 0;
        Info.GunSightMode = true;
        Info.TrainingMode = true;  // optional, but matches "first time" vibe

        // Runtime key/control model
        if (Stars)
        {
            KeyMap& Keymap = Stars->GetKeyMap();
            Keymap.Bind(KEY_CONTROL_MODEL, 1, 0);
            Keymap.SaveKeyMap("key.cfg", 256);
            Stars->MapKeys();
        }

        Ship::SetControlModel(1);
    }
    else
    {
        // STANDARD/HARDCORE
        Info.FlightModel = 0;
        Info.LandingMode = 0;
        Info.HudMode = 0;
        Info.GunSightMode = false;
        Info.TrainingMode = false;

        if (Stars)
        {
            KeyMap& Keymap = Stars->GetKeyMap();
            Keymap.Bind(KEY_CONTROL_MODEL, 0, 0);
            Keymap.SaveKeyMap("key.cfg", 256);
            Stars->MapKeys();
        }

        Ship::SetControlModel(0);
    }

    // ------------------------------------------------------------
    // Experience -> rank + training completion
    // ------------------------------------------------------------

    int32 ExpIndex = 0; // 0=Cadet, 1=Admiral
    if (ExperienceCombo)
    {
        ExpIndex = ExperienceCombo->GetSelectedIndex();
        if (ExpIndex < 0) ExpIndex = 0;
    }

    if (ExpIndex > 0)
    {
        // "Experienced" defaults
        Info.Rank = 2;   // Lieutenant (adjust if your rank table differs)

        // Your FS_PlayerGameInfo already has helpers; use them.
        // Legacy "fully trained" was 255; your struct treats Trained/Highest as "highest mission".
        // We will mark all bits 0..63 and set HighestTrainingMission accordingly.
        Info.HighestTrainingMission = 63;
        Info.Trained = Info.HighestTrainingMission;
        Info.TrainingMask = ~0ll; // all 64 bits set (int64)
        Info.TrainingMode = false;
    }
    else
    {
        // Cadet defaults
        Info.Rank = 0;
        Info.TrainingMode = true;

        // Ensure mask is consistent with "no training yet"
        // (Leave as-is if you want to preserve any pre-existing partial progress.)
        if (Info.TrainingMask == 0)
        {
            Info.HighestTrainingMission = 0;
            Info.Trained = 0;
        }
    }

    // ------------------------------------------------------------
    // Save
    // ------------------------------------------------------------

    PlayerSS->MarkDirty();
    if (!PlayerSS->SavePlayer(true))
    {
        UE_LOG(LogFirstTimeDlg, Error, TEXT("SavePlayer failed"));
        return;
    }

    // ------------------------------------------------------------
    // Return to menu via router
    // ------------------------------------------------------------

    if (MenuManager)
    {
        MenuManager->ShowMenuDlg();

        if (UMenuDlg* MenuDlg = MenuManager->GetMenuDlg())
        {
            MenuDlg->RefreshFromPlayerState();
            MenuDlg->ApplyMenuGating();
            MenuDlg->SetDialogInputEnabled(true);
        }
    }
}
