/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004.

    SUBSYSTEM:    Stars.exe
    FILE:         FirstTimeDlg.cpp
    AUTHOR:       John DiCamillo

    UNREAL PORT:
    - Converted from FormWindow to UBaseScreen (UUserWidget-derived).
    - Uses FRandomStream seeded from MachineId (deterministic per machine).
    - Preserves original logic and flow.
*/

#include "FirstTimeDlg.h"

#include "BaseScreen.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"

// Starshatter port headers:
#include "Starshatter.h"
#include "PlayerCharacter.h"
#include "Ship.h"
#include "KeyMap.h"

// UE:
#include "Misc/Guid.h"
#include "Misc/Crc.h"
#include "HAL/PlatformMisc.h"
#include "Math/RandomStream.h"

// +--------------------------------------------------------------------+

UFirstTimeDlg::UFirstTimeDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

// +--------------------------------------------------------------------+

void UFirstTimeDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    RegisterControls();
}

void UFirstTimeDlg::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
}

void UFirstTimeDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

// +--------------------------------------------------------------------+

void UFirstTimeDlg::RegisterControls()
{
    if (btn_apply)
    {
        btn_apply->OnClicked.RemoveAll(this);
        btn_apply->OnClicked.AddDynamic(this, &UFirstTimeDlg::OnApply);
    }
}

// +--------------------------------------------------------------------+

void UFirstTimeDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);
    SetKeyboardFocus();

    // Match original behavior:
    if (edt_name && edt_name->GetText().IsEmpty())
    {
        edt_name->SetText(FText::FromString(TEXT("Noobie")));
    }
}

// +--------------------------------------------------------------------+

void UFirstTimeDlg::ExecFrame()
{
    // Intentionally empty (matches original implementation)
}

// +--------------------------------------------------------------------+

void UFirstTimeDlg::OnApply()
{
    Starshatter* stars = Starshatter::GetInstance();
    PlayerCharacter* player = PlayerCharacter::GetCurrentPlayer();

    if (player)
    {
        if (edt_name)
        {
            // Deterministic per machine:
            // - Get a stable machine identifier (FGuid)
            // - Hash it to a 32-bit seed
            // - Use FRandomStream to generate the password value
            const FGuid MachineGuid = FPlatformMisc::GetMachineId();
            const FString MachineIdString = MachineGuid.ToString(EGuidFormats::DigitsWithHyphens);
            const uint32 Seed = FCrc::StrCrc32(*MachineIdString);

            FRandomStream Stream((int32)Seed);

            // Match original intent: 8 hex digits
            const uint32 RandVal = (uint32)Stream.RandRange(0, 2000000000);
            const FString Password = FString::Printf(TEXT("%08x"), RandVal);

            const FString NewName = edt_name->GetText().ToString();
            player->SetName(TCHAR_TO_ANSI(*NewName));
            player->SetPassword(TCHAR_TO_ANSI(*Password));
        }

        if (cmb_playstyle)
        {
            const int32 StyleIndex = cmb_playstyle->GetSelectedIndex();

            // ARCADE:
            if (StyleIndex == 0)
            {
                player->SetFlightModel(2);
                player->SetLandingModel(1);
                player->SetHUDMode(0);
                player->SetGunsight(1);

                if (stars)
                {
                    KeyMap& keymap = stars->GetKeyMap();
                    keymap.Bind(KEY_CONTROL_MODEL, 1, 0);
                    keymap.SaveKeyMap("key.cfg", 256);
                    stars->MapKeys();
                }

                Ship::SetControlModel(1);
            }
            // STANDARD / HARDCORE:
            else
            {
                player->SetFlightModel(0);
                player->SetLandingModel(0);
                player->SetHUDMode(0);
                player->SetGunsight(0);

                if (stars)
                {
                    KeyMap& keymap = stars->GetKeyMap();
                    keymap.Bind(KEY_CONTROL_MODEL, 0, 0);
                    keymap.SaveKeyMap("key.cfg", 256);
                    stars->MapKeys();
                }

                Ship::SetControlModel(0);
            }
        }

        if (cmb_experience && cmb_experience->GetSelectedIndex() > 0)
        {
            player->SetRank(2);      // Lieutenant
            player->SetTrained(255); // Fully Trained
        }

        PlayerCharacter::Save();
    }

    // Legacy: manager->ShowMenuDlg();
    // UE: hide this dialog and let owning screen decide what comes next.
    SetVisibility(ESlateVisibility::Collapsed);
}
