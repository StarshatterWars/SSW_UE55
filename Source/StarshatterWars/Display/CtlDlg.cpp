/*  Project Starshatter Wars
    Fractal Dev Games
    Copyright (C) 2024. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    System
    FILE:         CtlDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Control Options (keyboard/joystick) screen (Unreal UUserWidget port)
*/

#include "CtlDlg.h"

// Project:
#include "SSWGameInstance.h"

// NOTE:
// Replace these with your actual UI manager / menu screen equivalents.
// The legacy code called manager->ShowKeyDlg(), manager->ShowJoyDlg(), etc.
#include "BaseScreen.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"

#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// Optional: lightweight data object for ListView rows (COMMAND | KEY)

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

UCLASS()
class STARSHATTERWARS_API UCtlCommandRowData : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly) FString Command;
    UPROPERTY(BlueprintReadOnly) FString Key;
    UPROPERTY(BlueprintReadOnly) int32 KeyMapIndex = -1;
};

//////////////////////////////////////////////////////////////////////////
// Local constants for double-click window

static constexpr float SSW_CommandDoubleClickSeconds = 0.35f;

UCtlDlg::UCtlDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UCtlDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind category buttons:
    if (Category0Btn) Category0Btn->OnClicked.AddDynamic(this, &UCtlDlg::OnCategory0Clicked);
    if (Category1Btn) Category1Btn->OnClicked.AddDynamic(this, &UCtlDlg::OnCategory1Clicked);
    if (Category2Btn) Category2Btn->OnClicked.AddDynamic(this, &UCtlDlg::OnCategory2Clicked);
    if (Category3Btn) Category3Btn->OnClicked.AddDynamic(this, &UCtlDlg::OnCategory3Clicked);

    // Apply/Cancel:
    if (ApplyBtn)  ApplyBtn->OnClicked.AddDynamic(this, &UCtlDlg::OnApplyClicked);
    if (CancelBtn) CancelBtn->OnClicked.AddDynamic(this, &UCtlDlg::OnCancelClicked);

    // Tabs (Video/Audio/Options/Controls/Mod):
    if (vid_btn) vid_btn->OnClicked.AddDynamic(this, &UCtlDlg::OnVideoClicked);
    if (aud_btn) aud_btn->OnClicked.AddDynamic(this, &UCtlDlg::OnAudioClicked);
    if (opt_btn) opt_btn->OnClicked.AddDynamic(this, &UCtlDlg::OnOptionsClicked);
    if (ctl_btn) ctl_btn->OnClicked.AddDynamic(this, &UCtlDlg::OnControlsClicked);
    if (mod_btn) mod_btn->OnClicked.AddDynamic(this, &UCtlDlg::OnModClicked);

    // Combos:
    if (ControlModelCombo)  ControlModelCombo->OnSelectionChanged.AddDynamic(this, &UCtlDlg::OnControlModelChanged);
    if (JoySelectCombo)     JoySelectCombo->OnSelectionChanged.AddDynamic(this, &UCtlDlg::OnJoySelectChanged);
    if (JoyThrottleCombo)   JoyThrottleCombo->OnSelectionChanged.AddDynamic(this, &UCtlDlg::OnJoyThrottleChanged);
    if (JoyRudderCombo)     JoyRudderCombo->OnSelectionChanged.AddDynamic(this, &UCtlDlg::OnJoyRudderChanged);
    if (MouseSelectCombo)   MouseSelectCombo->OnSelectionChanged.AddDynamic(this, &UCtlDlg::OnMouseSelectChanged);

    // Sliders:
    if (JoySensitivitySlider)
    {
        JoySensitivitySlider->SetMinValue(0.0f);
        JoySensitivitySlider->SetMaxValue(10.0f);
        JoySensitivitySlider->SetStepSize(0.1f); // UI resolution; you can quantize to int in handler
        JoySensitivitySlider->OnValueChanged.AddDynamic(this, &UCtlDlg::OnJoySensitivityChanged);
    }

    if (MouseSensitivitySlider)
    {
        MouseSensitivitySlider->SetMinValue(0.0f);
        MouseSensitivitySlider->SetMaxValue(50.0f);
        MouseSensitivitySlider->SetStepSize(0.1f);
        MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &UCtlDlg::OnMouseSensitivityChanged);
    }

    // Buttons:
    if (JoyAxisBtn)          JoyAxisBtn->OnClicked.AddDynamic(this, &UCtlDlg::OnJoyAxisClicked);
    if (MouseInvertCheckbox) MouseInvertCheckbox->OnClicked.AddDynamic(this, &UCtlDlg::OnMouseInvertClicked);

    // List selection:
    if (CommandsList)
    {
        CommandsList->OnItemClicked().AddUObject(this, &UCtlDlg::OnCommandClicked); // implemented below (non-UFUNCTION)
        CommandsList->OnItemSelectionChanged().AddUObject(this, &UCtlDlg::OnCommandSelectionChanged);
    }

    // Initial draw:
    if (closed)
        ShowCategory();
    else
        UpdateCategory();

    // Tab state (Controls active):
    // Unreal buttons do not have "SetButtonState" like Starshatter,
    // so treat this as styling in BP or set IsEnabled / style classes.
    closed = false;
}

void UCtlDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Legacy behavior: Enter applies.
    // If you want key handling, implement NativeOnKeyDown and call Apply() there.
}

//////////////////////////////////////////////////////////////////////////
// Category logic (ported)

void UCtlDlg::ShowCategory()
{
    if (!CommandsList)
        return;

    // Clear:
    CommandsList->ClearListItems();

    // Pull from your UE-side keymap.
    // This assumes you have a keymap system in your GameInstance (or elsewhere).
    // Replace these calls with your actual implementations.
    USSWGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<USSWGameInstance>() : nullptr;
    if (!GI)
        return;

    // 1) Read current option bindings into our cached state.
    // The legacy code scanned 0..255 and updated:
    // control_model, joy_select, joy_throttle, joy_rudder, joy_sensitivity,
    // mouse_select, mouse_sensitivity, mouse_invert.
    //
    // Replace the stubs below with your real keymap query functions.

    // --- BEGIN STUB QUERIES ---
    control_model = GI->GetControlModel();        // expected 0..n
    joy_select = GI->GetJoySelectMode();       // matches combo indices
    joy_throttle = GI->GetJoyThrottleEnabled();  // matches combo indices
    joy_rudder = GI->GetJoyRudderEnabled();    // matches combo indices
    joy_sensitivity = GI->GetJoySensitivity();      // 0..10

    mouse_select = GI->GetMouseSelectMode();     // matches combo indices
    mouse_sensitivity = GI->GetMouseSensitivity();    // 0..50
    mouse_invert = GI->GetMouseInvert() ? 1 : 0; // bool -> 0/1
    // --- END STUB QUERIES ---

    // Push into widgets:
    if (ControlModelCombo)   ControlModelCombo->SetSelectedIndex(control_model);
    if (JoySelectCombo)      JoySelectCombo->SetSelectedIndex(joy_select);
    if (JoyThrottleCombo)    JoyThrottleCombo->SetSelectedIndex(joy_throttle);
    if (JoyRudderCombo)      JoyRudderCombo->SetSelectedIndex(joy_rudder);

    if (JoySensitivitySlider)  JoySensitivitySlider->SetValue((float)joy_sensitivity);

    if (MouseSelectCombo)      MouseSelectCombo->SetSelectedIndex(mouse_select);
    if (MouseSensitivitySlider) MouseSensitivitySlider->SetValue((float)mouse_sensitivity);

    // Checkbox button state: toggle visual in BP; here we just keep value
    // and rely on BP styling or a bound image.
    // (If you convert to UCheckBox, use SetIsChecked(mouse_invert > 0).)

    // 2) Populate the command list for the selected category:
    // Legacy: for each action in keymap, if category matches, show COMMAND + KEY.
    //
    // Replace this with your own keymap enumeration.
    TArray<FKeyMapRow> Rows;
    GI->GetKeyMapRowsForCategory(selected_category, Rows); // STUB

    for (const FKeyMapRow& Row : Rows)
    {
        UCtlCommandRowData* Item = NewObject<UCtlCommandRowData>(this);
        Item->Command = Row.ActionName;
        Item->Key = Row.KeyName;
        Item->KeyMapIndex = Row.MapIndex;

        CommandsList->AddItem(Item);
    }

    // Reset selection bookkeeping:
    command_index = -1;
}

void UCtlDlg::UpdateCategory()
{
    if (!CommandsList)
        return;

    USSWGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<USSWGameInstance>() : nullptr;
    if (!GI)
        return;

    // Re-resolve keys and refresh existing items in-place.
    // Easiest: rebuild the list.
    ShowCategory();
}

//////////////////////////////////////////////////////////////////////////
// Category click handlers

void UCtlDlg::OnCategory0Clicked() { selected_category = 0; ShowCategory(); }
void UCtlDlg::OnCategory1Clicked() { selected_category = 1; ShowCategory(); }
void UCtlDlg::OnCategory2Clicked() { selected_category = 2; ShowCategory(); }
void UCtlDlg::OnCategory3Clicked() { selected_category = 3; ShowCategory(); }

//////////////////////////////////////////////////////////////////////////
// ListView selection + “double click” emulation

// Store last click time / index (legacy used Game::RealTime and 350ms)
static double GCtl_LastClickTime = 0.0;
static int32  GCtl_LastClickIndex = -1;

void UCtlDlg::OnCommandSelectionChanged(UObject* Item)
{
    UCtlCommandRowData* Row = Cast<UCtlCommandRowData>(Item);
    if (!Row)
        return;

    command_index = CommandsList ? CommandsList->GetIndexForItem(Item) : -1;
}

void UCtlDlg::OnCommandClicked(UObject* Item)
{
    UCtlCommandRowData* Row = Cast<UCtlCommandRowData>(Item);
    if (!Row || !CommandsList)
        return;

    const int32 ClickedIndex = CommandsList->GetIndexForItem(Item);
    const double NowSeconds = FPlatformTime::Seconds();

    const bool bDoubleClick =
        (ClickedIndex == GCtl_LastClickIndex) &&
        ((NowSeconds - GCtl_LastClickTime) <= (double)SSW_CommandDoubleClickSeconds);

    GCtl_LastClickTime = NowSeconds;
    GCtl_LastClickIndex = ClickedIndex;
    command_index = ClickedIndex;

    if (!bDoubleClick)
        return;

    // Legacy: open KeyDlg and set keymap index.
    // Replace with your own flow (spawn widget, set index, etc.).
    USSWGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<USSWGameInstance>() : nullptr;
    if (GI)
    {
        GI->OpenKeyBindDialog(Row->KeyMapIndex); // STUB
    }
}

//////////////////////////////////////////////////////////////////////////
// Value change handlers (ported)

void UCtlDlg::OnControlModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (ControlModelCombo)
        control_model = ControlModelCombo->GetSelectedIndex();
}

void UCtlDlg::OnJoySelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (JoySelectCombo)
        joy_select = JoySelectCombo->GetSelectedIndex();
}

void UCtlDlg::OnJoyThrottleChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (JoyThrottleCombo)
        joy_throttle = JoyThrottleCombo->GetSelectedIndex();
}

void UCtlDlg::OnJoyRudderChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (JoyRudderCombo)
        joy_rudder = JoyRudderCombo->GetSelectedIndex();
}

void UCtlDlg::OnJoySensitivityChanged(float Value)
{
    // Quantize to int like the legacy slider:
    joy_sensitivity = FMath::Clamp((int)FMath::RoundToInt(Value), 0, 10);
}

void UCtlDlg::OnJoyAxisClicked()
{
    // Legacy: manager->ShowJoyDlg();
    USSWGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<USSWGameInstance>() : nullptr;
    if (GI)
    {
        GI->OpenJoyAxisDialog(); // STUB
    }
}

void UCtlDlg::OnMouseSelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (MouseSelectCombo)
        mouse_select = MouseSelectCombo->GetSelectedIndex();
}

void UCtlDlg::OnMouseSensitivityChanged(float Value)
{
    mouse_sensitivity = FMath::Clamp((int)FMath::RoundToInt(Value), 0, 50);
}

void UCtlDlg::OnMouseInvertClicked()
{
    // If you keep this as a UButton, treat it as a toggle:
    mouse_invert = mouse_invert ? 0 : 1;

    // Visual should be handled in BP (swap image/state).
    // If you switch to UCheckBox later, set checked state here.
}

//////////////////////////////////////////////////////////////////////////
// Tabs

void UCtlDlg::OnAudioClicked() { if (USSWGameInstance* GI = GetWorld()->GetGameInstance<USSWGameInstance>()) GI->ShowAudDlg(); }
void UCtlDlg::OnVideoClicked() { if (USSWGameInstance* GI = GetWorld()->GetGameInstance<USSWGameInstance>()) GI->ShowVidDlg(); }
void UCtlDlg::OnOptionsClicked() { if (USSWGameInstance* GI = GetWorld()->GetGameInstance<USSWGameInstance>()) GI->ShowOptDlg(); }
void UCtlDlg::OnControlsClicked() { if (USSWGameInstance* GI = GetWorld()->GetGameInstance<USSWGameInstance>()) GI->ShowCtlDlg(); }
void UCtlDlg::OnModClicked() { if (USSWGameInstance* GI = GetWorld()->GetGameInstance<USSWGameInstance>()) GI->ShowModDlg(); }

//////////////////////////////////////////////////////////////////////////
// Apply/Cancel

void UCtlDlg::OnApplyClicked()
{
    Apply();
}

void UCtlDlg::OnCancelClicked()
{
    Cancel();
}

void UCtlDlg::Apply()
{
    if (closed)
        return;

    USSWGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<USSWGameInstance>() : nullptr;
    if (!GI)
        return;

    // Legacy: keymap.Bind(...) + SaveKeyMap + MapKeys + Ship::SetControlModel(...)
    // Replace with your actual UE-side storage and mapping.

    GI->SetControlModel(control_model);

    GI->SetJoySelectMode(joy_select);
    GI->SetJoyRudderEnabled(joy_rudder);
    GI->SetJoyThrottleEnabled(joy_throttle);
    GI->SetJoySensitivity(joy_sensitivity);

    GI->SetMouseSelectMode(mouse_select);
    GI->SetMouseSensitivity(mouse_sensitivity);
    GI->SetMouseInvert(mouse_invert > 0);

    GI->SaveKeyConfig();     // STUB: writes key.cfg equivalent
    GI->RebuildInputMaps();  // STUB: equivalent to stars->MapKeys()

    closed = true;
}

void UCtlDlg::Cancel()
{
    closed = true;

    // If you want true legacy cancel (reload previous key.cfg / revert UI),
    // call GI->ReloadKeyConfig() and then ShowCategory().
}


