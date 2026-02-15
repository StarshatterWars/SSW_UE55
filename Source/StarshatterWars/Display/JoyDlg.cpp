/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:      StarshatterWars (Unreal Engine)
    FILE:           JoyDlg.cpp
    AUTHOR:         Carlos Bott

    IMPLEMENTATION
    ==============
    UJoyDlg - joystick axis mapping dialog.
=============================================================================*/

#include "JoyDlg.h"

// UMG
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"

// Router
#include "OptionsScreen.h"

// Starshatter legacy
#include "Starshatter.h"
#include "KeyMap.h"
#include "Joystick.h"
#include "Game.h"

DEFINE_LOG_CATEGORY_STATIC(LogJoyDlg, Log, All);

// Legacy axis name tokens (for Game::GetText)
static const char* JoyAxisNames[] =
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

UJoyDlg::UJoyDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetDialogInputEnabled(true);
}

void UJoyDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BindDelegates();
}

void UJoyDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // BaseScreen Enter/Escape routing:
    ApplyButton = ApplyBtn;
    CancelButton = CancelBtn;

    BindDelegates();

    RefreshAxisUIFromCurrentBindings();

    SelectedAxis = -1;
    SampleAxis = -1;

    // Warm-up samples so first delta doesn't instantly "win"
    for (int32 i = 0; i < 8; ++i)
        Samples[i] = 10000000;

    if (MessageText)
        MessageText->SetText(FText::FromString(TEXT("SELECT AN AXIS SLOT, THEN MOVE A JOYSTICK AXIS")));
}

void UJoyDlg::BindDelegates()
{
    if (bDelegatesBound)
        return;

    if (ApplyBtn)  ApplyBtn->OnClicked.AddUniqueDynamic(this, &UJoyDlg::OnApplyClicked);
    if (CancelBtn) CancelBtn->OnClicked.AddUniqueDynamic(this, &UJoyDlg::OnCancelClicked);

    if (AxisButton0) AxisButton0->OnClicked.AddUniqueDynamic(this, &UJoyDlg::OnAxis0Clicked);
    if (AxisButton1) AxisButton1->OnClicked.AddUniqueDynamic(this, &UJoyDlg::OnAxis1Clicked);
    if (AxisButton2) AxisButton2->OnClicked.AddUniqueDynamic(this, &UJoyDlg::OnAxis2Clicked);
    if (AxisButton3) AxisButton3->OnClicked.AddUniqueDynamic(this, &UJoyDlg::OnAxis3Clicked);

    bDelegatesBound = true;
}

void UJoyDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    (void)MyGeometry;
    ExecFrame((double)InDeltaTime);
}

void UJoyDlg::ExecFrame(double /*DeltaTime*/)
{
    // Only sample while user is selecting a slot:
    if (SelectedAxis < 0 || SelectedAxis >= 4)
        return;

    Joystick* Js = Joystick::GetInstance();
    if (!Js)
        return;

    Js->Acquire();

    int32 BestDelta = 1000;
    int32 BestAxis = -1;

    for (int32 i = 0; i < 8; ++i)
    {
        const int32 Raw = Joystick::ReadRawAxis(i + KEY_JOY_AXIS_X);
        const int32 Delta = FMath::Abs(Raw - Samples[i]);

        // Ignore the initial warm-up window:
        if (Delta > BestDelta && Samples[i] < 1000000)
        {
            BestDelta = Delta;
            BestAxis = i;
        }

        Samples[i] = Raw;
    }

    if (BestAxis >= 0)
    {
        SampleAxis = BestAxis;
        MapAxis[SelectedAxis] = BestAxis;

        // Update label to the detected axis name:
        const Text T = Game::GetText(JoyAxisNames[BestAxis]);
        SetAxisLabel(SelectedAxis, UTF8_TO_TCHAR(T.data()));

        // Stop capture after a successful detect:
        SelectedAxis = -1;

        if (MessageText)
            MessageText->SetText(FText::FromString(TEXT("AXIS DETECTED. SELECT ANOTHER SLOT OR APPLY.")));
    }
}

void UJoyDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);
    RefreshAxisUIFromCurrentBindings();
    SetKeyboardFocus();
}

void UJoyDlg::Apply()
{
    CommitToKeyMap();
}

void UJoyDlg::Cancel()
{
    // Restore UI from current stored bindings:
    RefreshAxisUIFromCurrentBindings();
}

// ------------------------------------------------------------
// Click handlers
// ------------------------------------------------------------

void UJoyDlg::OnAxis0Clicked() { HandleAxisClicked(0); }
void UJoyDlg::OnAxis1Clicked() { HandleAxisClicked(1); }
void UJoyDlg::OnAxis2Clicked() { HandleAxisClicked(2); }
void UJoyDlg::OnAxis3Clicked() { HandleAxisClicked(3); }

void UJoyDlg::HandleAxisClicked(int32 AxisIndex)
{
    if (AxisIndex < 0 || AxisIndex >= 4)
        return;

    // Clicking the same slot toggles capture off:
    if (SelectedAxis == AxisIndex)
    {
        SelectedAxis = -1;
        RefreshAxisUIFromCurrentBindings();

        if (MessageText)
            MessageText->SetText(FText::FromString(TEXT("CAPTURE CANCELLED.")));

        return;
    }

    // Start capture on this slot:
    SelectedAxis = AxisIndex;
    SampleAxis = -1;

    // Refresh labels so we show "SELECT" on active slot:
    RefreshAxisUIFromCurrentBindings();
    SetAxisLabel(AxisIndex, TEXT("SELECT… THEN MOVE AXIS"));

    // Reset sampling window:
    for (int32 i = 0; i < 8; ++i)
        Samples[i] = 10000000;

    if (MessageText)
        MessageText->SetText(FText::FromString(TEXT("MOVE THE AXIS YOU WANT TO BIND…")));
}

void UJoyDlg::OnApplyClicked()
{
    // Let OptionsScreen orchestrate if present:
    if (OptionsManager)
        OptionsManager->ApplyOptions();
    else
        Apply();
}

void UJoyDlg::OnCancelClicked()
{
    if (OptionsManager)
        OptionsManager->CancelOptions();
    else
        Cancel();
}

// ------------------------------------------------------------
// UI helpers
// ------------------------------------------------------------

void UJoyDlg::SetAxisLabel(int32 AxisIndex, const TCHAR* Text)
{
    UTextBlock* Target = nullptr;
    switch (AxisIndex)
    {
    case 0: Target = AxisText0; break;
    case 1: Target = AxisText1; break;
    case 2: Target = AxisText2; break;
    case 3: Target = AxisText3; break;
    default: break;
    }

    if (Target)
        Target->SetText(FText::FromString(Text ? Text : TEXT("")));
}

void UJoyDlg::RefreshAxisUIFromCurrentBindings()
{
    // Pull from joystick legacy mapping:
    for (int32 i = 0; i < 4; ++i)
    {
        const int32 Map = Joystick::GetAxisMap(i) - KEY_JOY_AXIS_X;
        const int32 Inv = Joystick::GetAxisInv(i);

        if (Map >= 0 && Map < 8)
            MapAxis[i] = Map;
        else
            MapAxis[i] = -1;

        // Axis label:
        if (MapAxis[i] >= 0 && MapAxis[i] < 8)
        {
            const Text T = Game::GetText(JoyAxisNames[MapAxis[i]]);
            SetAxisLabel(i, UTF8_TO_TCHAR(T.data()));
        }
        else
        {
            SetAxisLabel(i, TEXT("UNMAPPED"));
        }

        // Invert:
        const bool bInv = (Inv != 0);
        if (i == 0 && Invert0) Invert0->SetIsChecked(bInv);
        if (i == 1 && Invert1) Invert1->SetIsChecked(bInv);
        if (i == 2 && Invert2) Invert2->SetIsChecked(bInv);
        if (i == 3 && Invert3) Invert3->SetIsChecked(bInv);
    }
}

// ------------------------------------------------------------
// Commit to legacy KeyMap
// ------------------------------------------------------------

void UJoyDlg::CommitToKeyMap()
{
    Starshatter* Stars = Starshatter::GetInstance();
    if (!Stars)
        return;

    KeyMap& KM = Stars->GetKeyMap();

    // Axis binds (slot->mapped axis)
    const int32 YawAxis = (MapAxis[0] >= 0) ? (MapAxis[0] + KEY_JOY_AXIS_X) : 0;
    const int32 PitchAxis = (MapAxis[1] >= 0) ? (MapAxis[1] + KEY_JOY_AXIS_X) : 0;
    const int32 RollAxis = (MapAxis[2] >= 0) ? (MapAxis[2] + KEY_JOY_AXIS_X) : 0;
    const int32 ThrottleAxis = (MapAxis[3] >= 0) ? (MapAxis[3] + KEY_JOY_AXIS_X) : 0;

    KM.Bind(KEY_AXIS_YAW, YawAxis, 0);
    KM.Bind(KEY_AXIS_PITCH, PitchAxis, 0);
    KM.Bind(KEY_AXIS_ROLL, RollAxis, 0);
    KM.Bind(KEY_AXIS_THROTTLE, ThrottleAxis, 0);

    const bool bInvYaw = Invert0 ? Invert0->IsChecked() : false;
    const bool bInvPitch = Invert1 ? Invert1->IsChecked() : false;
    const bool bInvRoll = Invert2 ? Invert2->IsChecked() : false;
    const bool bInvThrottle = Invert3 ? Invert3->IsChecked() : false;

    KM.Bind(KEY_AXIS_YAW_INVERT, bInvYaw ? 1 : 0, 0);
    KM.Bind(KEY_AXIS_PITCH_INVERT, bInvPitch ? 1 : 0, 0);
    KM.Bind(KEY_AXIS_ROLL_INVERT, bInvRoll ? 1 : 0, 0);
    KM.Bind(KEY_AXIS_THROTTLE_INVERT, bInvThrottle ? 1 : 0, 0);

    KM.SaveKeyMap("key.cfg", 256);
    Stars->MapKeys();

    UE_LOG(LogJoyDlg, Log, TEXT("[JoyDlg] Saved joystick bindings to key.cfg and remapped keys."));
}
