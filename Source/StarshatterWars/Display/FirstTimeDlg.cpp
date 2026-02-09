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
*/

#include "FirstTimeDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"

// Legacy core:
#include "Starshatter.h"
#include "PlayerCharacter.h"
#include "Ship.h"
#include "KeyMap.h"

// Router:
#include "MenuScreen.h"

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

    // ------------------------------------------------------------
    // Accept button binding (idempotent)
    // ------------------------------------------------------------

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
    // ------------------------------------------------------------
    // Play Style
    // ------------------------------------------------------------

    if (PlayStyleCombo)
    {
        if (PlayStyleCombo->GetOptionCount() == 0)
        {
            PlayStyleCombo->AddOption(TEXT("Arcade Style"));
            PlayStyleCombo->AddOption(TEXT("Standard Model"));
        }

        if (PlayStyleCombo->GetSelectedIndex() < 0)
        {
            PlayStyleCombo->SetSelectedIndex(0);
        }
    }
    else
    {
        UE_LOG(LogFirstTimeDlg, Error, TEXT("PlayStyleCombo is NULL (BindWidget mismatch)."));
    }

    // ------------------------------------------------------------
    // Experience
    // ------------------------------------------------------------

    if (ExperienceCombo)
    {
        if (ExperienceCombo->GetOptionCount() == 0)
        {
            ExperienceCombo->AddOption(TEXT("Cadet (First timer)"));
            ExperienceCombo->AddOption(TEXT("Admiral (Experienced)"));
        }

        if (ExperienceCombo->GetSelectedIndex() < 0)
        {
            ExperienceCombo->SetSelectedIndex(0);
        }
    }
    else
    {
        UE_LOG(LogFirstTimeDlg, Error, TEXT("ExperienceCombo is NULL (BindWidget mismatch)."));
    }

    // ------------------------------------------------------------
    // Name hint
    // ------------------------------------------------------------

    if (NameEdit)
    {
        if (NameEdit->GetHintText().IsEmpty())
        {
            NameEdit->SetHintText(FText::FromString(TEXT("Enter player name")));
        }
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
    UE_LOG(LogFirstTimeDlg, Warning, TEXT("UFirstTimeDlg::OnAcceptClicked()"));

    Starshatter* Stars = Starshatter::GetInstance();

    // ------------------------------------------------------------
    // Ensure legacy player exists
    // ------------------------------------------------------------

    // Preferred (add this to PlayerCharacter):
    // PlayerObj = PlayerCharacter::EnsureCurrentPlayer();

    // Fallback if you don't have EnsureCurrentPlayer yet:
    PlayerCharacter* PlayerObj = PlayerCharacter::EnsureCurrentPlayer();
    if (!PlayerObj)
    {
        UE_LOG(LogFirstTimeDlg, Warning, TEXT("No current player; creating a default PlayerCharacter"));
        PlayerCharacter::SetCurrentPlayer(new PlayerCharacter("Ready Player One"));
    }

    if (!PlayerObj)
    {
        UE_LOG(LogFirstTimeDlg, Error, TEXT("OnAcceptClicked: Failed to acquire PlayerCharacter"));
        return;
    }

    // ------------------------------------------------------------
    // Name + Password
    // ------------------------------------------------------------

    if (NameEdit)
    {
        const FString PlayerName = NameEdit->GetText().ToString().TrimStartAndEnd();
        if (!PlayerName.IsEmpty())
        {
            const int32 RandVal = FMath::RandRange(0, 2000000000);
            const FString Password = FString::Printf(TEXT("%08x"), RandVal);

            PlayerObj->SetName(TCHAR_TO_UTF8(*PlayerName));
            PlayerObj->SetPassword(TCHAR_TO_UTF8(*Password));
        }
    }

    // ------------------------------------------------------------
    // Play Style
    // ------------------------------------------------------------

    int32 PlayStyleIndex = 0;
    if (PlayStyleCombo)
    {
        PlayStyleIndex = PlayStyleCombo->GetSelectedIndex();
        if (PlayStyleIndex < 0) PlayStyleIndex = 0;
    }

    if (PlayStyleIndex == 0)
    {
        // ARCADE
        PlayerObj->SetFlightModel(2);
        PlayerObj->SetLandingModel(1);
        PlayerObj->SetHUDMode(0);
        PlayerObj->SetGunsight(1);

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
        PlayerObj->SetFlightModel(0);
        PlayerObj->SetLandingModel(0);
        PlayerObj->SetHUDMode(0);
        PlayerObj->SetGunsight(0);

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
    // Experience
    // ------------------------------------------------------------

    int32 ExpIndex = 0;
    if (ExperienceCombo)
    {
        ExpIndex = ExperienceCombo->GetSelectedIndex();
        if (ExpIndex < 0) ExpIndex = 0;
    }

    if (ExpIndex > 0)
    {
        PlayerObj->SetRank(2);       // Lieutenant
        PlayerObj->SetTrained(255);  // Fully trained
    }

    // ------------------------------------------------------------
    // Save
    // ------------------------------------------------------------

    PlayerCharacter::Save();

    // ------------------------------------------------------------
    // Return to menu via router
    // ------------------------------------------------------------

    if (MenuManager)
    {
        MenuManager->ShowMenuDlg();
    }
    else
    {
        UE_LOG(LogFirstTimeDlg, Warning,
            TEXT("OnAcceptClicked: MenuManager is NULL (MenuScreen didn't assign it)."));
    }
}
