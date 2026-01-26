/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         JoyDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Joystick Axis Setup dialog implementation.
*/

#include "JoyDlg.h"

#include "GameStructs.h"

// Unreal
#include "Logging/LogMacros.h"

// UMG
#include "Components/Button.h"
#include "Components/TextBlock.h"

// Starshatter
#include "KeyMap.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "Game.h"
#include "Joystick.h"

DEFINE_LOG_CATEGORY_STATIC(LogJoyDlg, Log, All);

// --------------------------------------------------------------------
// Legacy text keys:
static const char* JoyAxisNames[] = {
    "JoyDlg.axis.0",
    "JoyDlg.axis.1",
    "JoyDlg.axis.2",
    "JoyDlg.axis.3",
    "JoyDlg.axis.4",
    "JoyDlg.axis.5",
    "JoyDlg.axis.6",
    "JoyDlg.axis.7"
};

// --------------------------------------------------------------------

UJoyDlg::UJoyDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

// --------------------------------------------------------------------

void UJoyDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (ApplyButton)
        ApplyButton->OnClicked.AddDynamic(this, &UJoyDlg::OnApplyClicked);

    if (CancelButton)
        CancelButton->OnClicked.AddDynamic(this, &UJoyDlg::OnCancelClicked);

    if (AxisButton0)
        AxisButton0->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis0Clicked);

    if (AxisButton1)
        AxisButton1->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis1Clicked);

    if (AxisButton2)
        AxisButton2->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis2Clicked);

    if (AxisButton3)
        AxisButton3->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis3Clicked);
}

// --------------------------------------------------------------------

void UJoyDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Mirror legacy Show() initialization behavior on construct:
    RefreshAxisButtonsFromCurrentBindings();

    // Reset sampling state:
    SelectedAxis = -1;
    SampleAxis = -1;
    for (int i = 0; i < 8; i++)
        Samples[i] = 10000000;
}

// --------------------------------------------------------------------

void UJoyDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

// --------------------------------------------------------------------

void UJoyDlg::SetManager(UMenuScreen* InManager)
{
    Manager = InManager;
}

// --------------------------------------------------------------------
// Legacy parity
// --------------------------------------------------------------------

void UJoyDlg::ExecFrame()
{
    if (SelectedAxis >= 0 && SelectedAxis < 4)
    {
        Joystick* joystick = Joystick::GetInstance();
        if (joystick)
        {
            joystick->Acquire();

            int delta = 1000;

            for (int i = 0; i < 8; i++)
            {
                const int a = Joystick::ReadRawAxis(i + KEY_JOY_AXIS_X);

                int d = a - Samples[i];
                if (d < 0)
                    d = -d;

                if (d > delta && Samples[i] < 1000000)
                {
                    delta = d;
                    SampleAxis = i;
                }

                Samples[i] = a;
            }

            if (SampleAxis >= 0)
            {
                MapAxis[SelectedAxis] = SampleAxis;
                UpdateAxisButtonText(SelectedAxis, JoyAxisNames[SampleAxis]);
            }
            else
            {
                UpdateAxisButtonText(SelectedAxis, "JoyDlg.select");
            }
        }
    }
}

// --------------------------------------------------------------------

void UJoyDlg::OnAxis0Clicked() { HandleAxisClicked(0); }
void UJoyDlg::OnAxis1Clicked() { HandleAxisClicked(1); }
void UJoyDlg::OnAxis2Clicked() { HandleAxisClicked(2); }
void UJoyDlg::OnAxis3Clicked() { HandleAxisClicked(3); }

// --------------------------------------------------------------------

void UJoyDlg::HandleAxisClicked(int AxisIndex)
{
    // Toggle selection like legacy OnAxis(AWEvent*)
    if (AxisIndex < 0 || AxisIndex >= 4)
        return;

    if (SelectedAxis == AxisIndex)
    {
        // deselect, restore mapped/unmapped name
        const int map = MapAxis[AxisIndex];
        if (map >= 0 && map < 8)
            UpdateAxisButtonText(AxisIndex, JoyAxisNames[map]);
        else
            UpdateAxisButtonText(AxisIndex, "JoyDlg.unmapped");

        SelectedAxis = -1;
    }
    else
    {
        // select this one, restore others
        for (int i = 0; i < 4; i++)
        {
            if (i == AxisIndex)
                continue;

            const int map = MapAxis[i];
            if (map >= 0 && map < 8)
                UpdateAxisButtonText(i, JoyAxisNames[map]);
            else
                UpdateAxisButtonText(i, "JoyDlg.unmapped");
        }

        UpdateAxisButtonText(AxisIndex, "JoyDlg.select");
        SelectedAxis = AxisIndex;
        SampleAxis = -1;
    }

    // reset samples
    for (int i = 0; i < 8; i++)
        Samples[i] = 10000000;
}

// --------------------------------------------------------------------

void UJoyDlg::OnApplyClicked()
{
    Starshatter* stars = Starshatter::GetInstance();

    if (stars)
    {
        KeyMap& keymap = stars->GetKeyMap();

        keymap.Bind(KEY_AXIS_YAW, MapAxis[0] + KEY_JOY_AXIS_X, 0);
        keymap.Bind(KEY_AXIS_PITCH, MapAxis[1] + KEY_JOY_AXIS_X, 0);
        keymap.Bind(KEY_AXIS_ROLL, MapAxis[2] + KEY_JOY_AXIS_X, 0);
        keymap.Bind(KEY_AXIS_THROTTLE, MapAxis[3] + KEY_JOY_AXIS_X, 0);

        keymap.Bind(KEY_AXIS_YAW_INVERT, GetInvertButtonState(Invert0) ? 1 : 0, 0);
        keymap.Bind(KEY_AXIS_PITCH_INVERT, GetInvertButtonState(Invert1) ? 1 : 0, 0);
        keymap.Bind(KEY_AXIS_ROLL_INVERT, GetInvertButtonState(Invert2) ? 1 : 0, 0);
        keymap.Bind(KEY_AXIS_THROTTLE_INVERT, GetInvertButtonState(Invert3) ? 1 : 0, 0);

        keymap.SaveKeyMap("key.cfg", 256);
        stars->MapKeys();
    }
    else
    {
        UE_LOG(LogJoyDlg, Warning, TEXT("Starshatter instance not available."));
    }

    if (Manager)
        Manager->ShowCtlDlg();
}

// --------------------------------------------------------------------

void UJoyDlg::OnCancelClicked()
{
    if (Manager)
        Manager->ShowCtlDlg();
}

// --------------------------------------------------------------------
// Helpers
// --------------------------------------------------------------------

void UJoyDlg::RefreshAxisButtonsFromCurrentBindings()
{
    // Populate from current joystick settings:
    for (int i = 0; i < 4; i++)
    {
        const int map = Joystick::GetAxisMap(i) - KEY_JOY_AXIS_X;
        const int inv = Joystick::GetAxisInv(i);

        if (map >= 0 && map < 8)
        {
            MapAxis[i] = map;
            UpdateAxisButtonText(i, JoyAxisNames[map]);
        }
        else
        {
            MapAxis[i] = -1;
            UpdateAxisButtonText(i, "JoyDlg.unmapped");
        }

        // Invert checkboxes:
        if (i == 0) SetInvertButtonState(Invert0, inv != 0);
        if (i == 1) SetInvertButtonState(Invert1, inv != 0);
        if (i == 2) SetInvertButtonState(Invert2, inv != 0);
        if (i == 3) SetInvertButtonState(Invert3, inv != 0);
    }
}

// --------------------------------------------------------------------

void UJoyDlg::UpdateAxisButtonText(int AxisIndex, const char* TextId)
{
    if (!TextId)
        return;

    UButton* Target = nullptr;

    switch (AxisIndex)
    {
    case 0: Target = AxisButton0; break;
    case 1: Target = AxisButton1; break;
    case 2: Target = AxisButton2; break;
    case 3: Target = AxisButton3; break;
    default: break;
    }

    // UButton does not have SetText, so the design assumption is:
    // Each axis button has a child TextBlock named "Label" or similar.
    // We attempt to find the first TextBlock in the button tree and set it.
    if (Target)
    {
        UTextBlock* Label = Target->GetChildAt(0) ? Cast<UTextBlock>(Target->GetChildAt(0)) : nullptr;
        if (!Label)
        {
            // fallback: try message text as debug output if the button label is not directly accessible
            UE_LOG(LogJoyDlg, Verbose, TEXT("Axis button %d label not found; cannot set text."), AxisIndex);
            return;
        }

        const Text t = Game::GetText(TextId);
        Label->SetText(FText::FromString(UTF8_TO_TCHAR(t.data())));
    }
}

// --------------------------------------------------------------------

void UJoyDlg::SetInvertButtonState(UButton* InvertButton, bool bChecked)
{
    // Legacy uses button state; in UMG you typically swap style or use CheckBox.
    // Here we store state by using the "IsEnabled" flag as a minimal stand-in:
    // Enabled = checked, Disabled = unchecked (so the state is queryable).
    // Replace with UCheckBox when you wire the real widget.
    if (InvertButton)
    {
        InvertButton->SetIsEnabled(bChecked);
    }
}

bool UJoyDlg::GetInvertButtonState(const UButton* InvertButton) const
{
    if (!InvertButton)
        return false;

    return InvertButton->GetIsEnabled();
}
