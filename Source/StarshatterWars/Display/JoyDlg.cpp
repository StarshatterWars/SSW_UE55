/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         JoyDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Joystick Options Dialog (Unreal UUserWidget)

    Notes (FORM -> UMG mapping):
    - Title label (id 100): create a UTextBlock (optional) for "Joystick Axis Setup"
    - Axis labels (ids 101-104): UTextBlocks for X Axis, Y Axis, Rudder, Throttle
    - Message (id 11): BindWidgetOptional UTextBlock* message
    - Axis select buttons (ids 201-204): BindWidgetOptional UButton* axis_button_0..3
    - Invert "checkbox" buttons (ids 301-304): Prefer UCheckBox* invert_checkbox_0..3
    - Apply/Cancel buttons (ids 1/2): BindWidgetOptional UButton* ApplyBtn / CancelBtn
*/

#include "GameStructs.h"

#include "JoyDlg.h"
#include "BaseScreen.h"

#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Starshatter core:
#include "Starshatter.h"
#include "KeyMap.h"
#include "Game.h"
#include "Joystick.h"

// +--------------------------------------------------------------------+

static const char* joy_axis_names[] =
{
    "JoyDlg.axis.0",
    "JoyDlg.axis.1",
    "JoyDlg.axis.2",
    "JoyDlg.axis.3",
    "JoyDlg.axis.4",
    "JoyDlg.axis.5",
    "JoyDlg.axis.6",
    "JoyDlg.axis.7"
};

// Keep legacy sampling state file-static (single-instance safe enough for menu UI):
static int sample_axis = -1;
static int samples[8] = { 10000000,10000000,10000000,10000000,10000000,10000000,10000000,10000000 };
static int map_axis[4] = { -1,-1,-1,-1 };

// +--------------------------------------------------------------------+

UJoyDlg::UJoyDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UJoyDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Apply / Cancel:
    if (ApplyBtn)
    {
        ApplyBtn->OnClicked.AddDynamic(this, &UJoyDlg::OnApplyClicked);
    }

    if (CancelBtn)
    {
        CancelBtn->OnClicked.AddDynamic(this, &UJoyDlg::OnCancelClicked);
    }

    // Axis select buttons:
    if (axis_button_0) axis_button_0->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis0Clicked);
    if (axis_button_1) axis_button_1->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis1Clicked);
    if (axis_button_2) axis_button_2->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis2Clicked);
    if (axis_button_3) axis_button_3->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis3Clicked);

    // Invert checkboxes:
    if (invert_checkbox_0) invert_checkbox_0->OnCheckStateChanged.AddDynamic(this, &UJoyDlg::OnInvert0Changed);
    if (invert_checkbox_1) invert_checkbox_1->OnCheckStateChanged.AddDynamic(this, &UJoyDlg::OnInvert1Changed);
    if (invert_checkbox_2) invert_checkbox_2->OnCheckStateChanged.AddDynamic(this, &UJoyDlg::OnInvert2Changed);
    if (invert_checkbox_3) invert_checkbox_3->OnCheckStateChanged.AddDynamic(this, &UJoyDlg::OnInvert3Changed);

    // Initialize UI from current bindings:
    for (int i = 0; i < 4; i++)
    {
        const int map = Joystick::GetAxisMap(i) - KEY_JOY_AXIS_X;
        const int inv = Joystick::GetAxisInv(i);

        map_axis[i] = (map >= 0 && map < 8) ? map : -1;

        if (i == 0 && invert_checkbox_0) invert_checkbox_0->SetIsChecked(inv != 0);
        if (i == 1 && invert_checkbox_1) invert_checkbox_1->SetIsChecked(inv != 0);
        if (i == 2 && invert_checkbox_2) invert_checkbox_2->SetIsChecked(inv != 0);
        if (i == 3 && invert_checkbox_3) invert_checkbox_3->SetIsChecked(inv != 0);
    }

    // Default state:
    selected_axis = -1;
    sample_axis = -1;

    // Reset samples:
    for (int i = 0; i < 8; i++)
    {
        samples[i] = 10000000;
    }

    // Focus so Enter/Escape works:
    SetKeyboardFocus();
}

void UJoyDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Sample raw joystick input when an axis slot is active:
    if (selected_axis >= 0 && selected_axis < 4)
    {
        Joystick* joystick = Joystick::GetInstance();
        if (joystick)
        {
            joystick->Acquire();

            int delta = 1000;

            for (int i = 0; i < 8; i++)
            {
                const int a = Joystick::ReadRawAxis(i + KEY_JOY_AXIS_X);

                int d = a - samples[i];
                if (d < 0) d = -d;

                if (d > delta && samples[i] < 1000000)
                {
                    delta = d;
                    sample_axis = i;
                }

                samples[i] = a;
            }

            // If we found a moved axis, bind it immediately:
            if (sample_axis >= 0)
            {
                map_axis[selected_axis] = sample_axis;

                // UMG text on buttons is typically separate (TextBlock in button),
                // so we don't SetText() like legacy. Use the message field for feedback.
                if (message)
                {
                    const Text axisText = Game::GetText(joy_axis_names[sample_axis]);
                    message->SetText(FText::FromString(UTF8_TO_TCHAR(axisText.data())));
                }
            }
            else
            {
                if (message)
                {
                    const Text selText = Game::GetText("JoyDlg.select");
                    message->SetText(FText::FromString(UTF8_TO_TCHAR(selText.data())));
                }
            }
        }
    }
}

FReply UJoyDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey key = InKeyEvent.GetKey();

    if (key == EKeys::Enter || key == EKeys::Virtual_Accept)
    {
        Apply();
        return FReply::Handled();
    }

    if (key == EKeys::Escape || key == EKeys::Virtual_Back)
    {
        Cancel();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// +--------------------------------------------------------------------+
// Button events:

void UJoyDlg::OnApplyClicked()
{
    Apply();
}

void UJoyDlg::OnCancelClicked()
{
    Cancel();
}

static void ToggleAxisSelection(UJoyDlg* dlg, int axisIndex)
{
    if (!dlg) return;

    // Toggle selection:
    if (dlg->selected_axis == axisIndex)
    {
        dlg->selected_axis = -1;
        sample_axis = -1;

        if (dlg->message)
        {
            // Show mapping summary for the axis we just toggled off:
            const int map = map_axis[axisIndex];
            Text name = Game::GetText("JoyDlg.unmapped");
            if (map >= 0 && map < 8)
                name = Game::GetText(joy_axis_names[map]);

            dlg->message->SetText(FText::FromString(UTF8_TO_TCHAR(name.data())));
        }
    }
    else
    {
        dlg->selected_axis = axisIndex;
        sample_axis = -1;

        if (dlg->message)
        {
            const Text selText = Game::GetText("JoyDlg.select");
            dlg->message->SetText(FText::FromString(UTF8_TO_TCHAR(selText.data())));
        }

        // Reset samples so we detect the next motion cleanly:
        for (int i = 0; i < 8; i++)
            samples[i] = 10000000;
    }
}

void UJoyDlg::OnAxis0Clicked() { ToggleAxisSelection(this, 0); }
void UJoyDlg::OnAxis1Clicked() { ToggleAxisSelection(this, 1); }
void UJoyDlg::OnAxis2Clicked() { ToggleAxisSelection(this, 2); }
void UJoyDlg::OnAxis3Clicked() { ToggleAxisSelection(this, 3); }

void UJoyDlg::OnInvert0Changed(bool bIsChecked) { /* Stored on Apply() */ }
void UJoyDlg::OnInvert1Changed(bool bIsChecked) { /* Stored on Apply() */ }
void UJoyDlg::OnInvert2Changed(bool bIsChecked) { /* Stored on Apply() */ }
void UJoyDlg::OnInvert3Changed(bool bIsChecked) { /* Stored on Apply() */ }

// +--------------------------------------------------------------------+
// Apply / Cancel:

static void CallManagerByName(UBaseScreen* manager, const TCHAR* fnName)
{
    if (!manager || !fnName) return;

    const FName fname(fnName);
    UFunction* fn = manager->FindFunction(fname);
    if (fn)
    {
        manager->ProcessEvent(fn, nullptr);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("JoyDlg: Manager does not implement '%s'."), fnName);
    }
}

void UJoyDlg::Apply()
{
    Starshatter* stars = Starshatter::GetInstance();

    if (stars)
    {
        KeyMap& keymap = stars->GetKeyMap();

        keymap.Bind(KEY_AXIS_YAW, map_axis[0] + KEY_JOY_AXIS_X, 0);
        keymap.Bind(KEY_AXIS_PITCH, map_axis[1] + KEY_JOY_AXIS_X, 0);
        keymap.Bind(KEY_AXIS_ROLL, map_axis[2] + KEY_JOY_AXIS_X, 0);
        keymap.Bind(KEY_AXIS_THROTTLE, map_axis[3] + KEY_JOY_AXIS_X, 0);

        const int inv0 = invert_checkbox_0 ? (invert_checkbox_0->IsChecked() ? 1 : 0) : 0;
        const int inv1 = invert_checkbox_1 ? (invert_checkbox_1->IsChecked() ? 1 : 0) : 0;
        const int inv2 = invert_checkbox_2 ? (invert_checkbox_2->IsChecked() ? 1 : 0) : 0;
        const int inv3 = invert_checkbox_3 ? (invert_checkbox_3->IsChecked() ? 1 : 0) : 0;

        keymap.Bind(KEY_AXIS_YAW_INVERT, inv0, 0);
        keymap.Bind(KEY_AXIS_PITCH_INVERT, inv1, 0);
        keymap.Bind(KEY_AXIS_ROLL_INVERT, inv2, 0);
        keymap.Bind(KEY_AXIS_THROTTLE_INVERT, inv3, 0);

        keymap.SaveKeyMap("key.cfg", 256);
        stars->MapKeys();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("JoyDlg: Starshatter instance not available; cannot apply joystick bindings."));
    }

    // Return to Controls dialog (dynamic call avoids hard dependency on UBaseScreen API):
    CallManagerByName(manager, TEXT("ShowCtlDlg"));
}

void UJoyDlg::Cancel()
{
    // Return to Controls dialog:
    CallManagerByName(manager, TEXT("ShowCtlDlg"));
}
