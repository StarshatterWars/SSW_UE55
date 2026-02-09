
/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025–2026.

    DIALOG:      FirstTimeDlg
    FILE:        FirstTimeDlg.h
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
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"

// Legacy core
#include "Starshatter.h"
#include "PlayerCharacter.h"
#include "Ship.h"
#include "KeyMap.h"

// Router
#include "MenuScreen.h"

DEFINE_LOG_CATEGORY_STATIC(LogFirstTimeDlg, Log, All);

UFirstTimeDlg::UFirstTimeDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsFocusable(true);
}

void UFirstTimeDlg::NativeConstruct()
{
    Super::NativeConstruct();

    PopulateDefaultsIfNeeded();

    if (AcceptBtn)
    {
        AcceptBtn->OnClicked.AddDynamic(this, &UFirstTimeDlg::OnAcceptClicked);
    }
    else
    {
        UE_LOG(LogFirstTimeDlg, Error, TEXT("AcceptBtn is null (BindWidget name mismatch or not marked 'Is Variable' in BP)."));
    }
}

void UFirstTimeDlg::PopulateDefaultsIfNeeded()
{
    // Populate combo items once (avoid duplicates on rebuild)
    if (PlayStyleCombo)
    {
        if (PlayStyleCombo->GetOptionCount() == 0)
        {
            PlayStyleCombo->AddOption(TEXT("Arcade Style"));
            PlayStyleCombo->AddOption(TEXT("Standard Model"));
        }

        // Legacy default: index 0 (Arcade)
        PlayStyleCombo->SetSelectedIndex(0);
    }
    else
    {
        UE_LOG(LogFirstTimeDlg, Error, TEXT("PlayStyleCombo is null (BindWidget mismatch)."));
    }

    if (ExperienceCombo)
    {
        if (ExperienceCombo->GetOptionCount() == 0)
        {
            ExperienceCombo->AddOption(TEXT("Cadet (First timer)"));
            ExperienceCombo->AddOption(TEXT("Admiral (Experienced)"));
        }

        // Legacy default: index 0 (Cadet)
        ExperienceCombo->SetSelectedIndex(0);
    }
    else
    {
        UE_LOG(LogFirstTimeDlg, Error, TEXT("ExperienceCombo is null (BindWidget mismatch)."));
    }

    // Optional: give a reasonable default name placeholder
    if (NameEdit && NameEdit->GetText().IsEmpty())
    {
        NameEdit->SetHintText(FText::FromString(TEXT("Enter player name")));
    }
}

void UFirstTimeDlg::OnAcceptClicked()
{
    Starshatter* Stars = Starshatter::GetInstance();
    PlayerCharacter* PlayerObj = PlayerCharacter::GetCurrentPlayer();

    if (!PlayerObj)
    {
        UE_LOG(LogFirstTimeDlg, Error, TEXT("OnAcceptClicked: Player::GetCurrentPlayer() returned null"));
        return;
    }

    // -----------------------------
    // NAME + PASSWORD (legacy)
    // -----------------------------
    if (NameEdit)
    {
        const FString PlayerName = NameEdit->GetText().ToString().TrimStartAndEnd();
        if (!PlayerName.IsEmpty())
        {
            // Legacy: sprintf_s("%08x", (DWORD) Random(0, 2e9));
            const int32 RandVal = FMath::RandRange(0, 2000000000);
            const FString Password = FString::Printf(TEXT("%08x"), RandVal);

            // Legacy Player likely expects const char* (ANSI/UTF-8). Use UTF-8.
            PlayerObj->SetName(TCHAR_TO_UTF8(*PlayerName));
            PlayerObj->SetPassword(TCHAR_TO_UTF8(*Password));
        }
    }

    // -----------------------------
    // PLAY STYLE (legacy)
    // -----------------------------
    int32 PlayStyleIndex = 0;
    if (PlayStyleCombo)
    {
        PlayStyleIndex = PlayStyleCombo->GetSelectedIndex();
        if (PlayStyleIndex < 0)
            PlayStyleIndex = 0;
    }

    // 0 = Arcade, else = Standard/Hardcore
    if (PlayStyleIndex == 0)
    {
        // ARCADE:
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
        // STANDARD/HARDCORE:
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

    // -----------------------------
    // EXPERIENCE (legacy)
    // -----------------------------
    int32 ExpIndex = 0;
    if (ExperienceCombo)
    {
        ExpIndex = ExperienceCombo->GetSelectedIndex();
        if (ExpIndex < 0)
            ExpIndex = 0;
    }

    // Legacy: if (cmb_experience && selectedIndex > 0) -> experienced
    if (ExpIndex > 0)
    {
        PlayerObj->SetRank(2);       // Lieutenant
        PlayerObj->SetTrained(255);  // Fully Trained
    }

    PlayerCharacter::Save();

    // Return to menu through router
    if (Manager)
    {
        Manager->ShowMenuDlg();
    }
    else
    {
        UE_LOG(LogFirstTimeDlg, Warning, TEXT("OnAcceptClicked: Manager is null (MenuScreen didn't assign it)."));
    }
}