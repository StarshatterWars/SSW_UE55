/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         ControlOptionsDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    UNREAL PORT:
    - Converted from FormWindow/AWEvent mapping to UBaseScreen (UUserWidget-derived).
*/

#include "ControlOptionsDlg.h"

// Unreal:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

// Starshatter (ported core):
#include "Starshatter.h"
#include "Ship.h"
#include "Game.h"
#include "Keyboard.h"
#include "Joystick.h"
#include "KeyDlg.h"

// NOTE: you referenced KeyMap / KEY_* in original.
// Include whatever header defines KeyMap, KeyMapEntry, and KEY_* constants in your port:
#include "KeyMap.h"

UControlOptionsDlg::UControlOptionsDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UControlOptionsDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    stars = Starshatter::GetInstance();

    RegisterControls();
}

void UControlOptionsDlg::NativeConstruct()
{
    Super::NativeConstruct();
}

void UControlOptionsDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

// --------------------------------------------------------------------
// UBaseScreen overrides
// --------------------------------------------------------------------

void UControlOptionsDlg::BindFormWidgets()
{
    // Optional: if you are binding by legacy numeric IDs via BaseScreen’s mapping system,
    // you can BindButton/BindList/etc here. This class uses BindWidgetOptional directly.
}

FString UControlOptionsDlg::GetLegacyFormText() const
{
    // If you want BaseScreen to auto-apply form defaults for labels/buttons/colors/fonts,
    // return the legacy FORM text here. You pasted MenuDlg.frm above; you can embed it
    // verbatim as a raw string literal.
    return FString();
}

// --------------------------------------------------------------------
// Wiring
// --------------------------------------------------------------------

void UControlOptionsDlg::RegisterControls()
{
    // Category tabs:
    if (category_0) category_0->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory0);
    if (category_1) category_1->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory1);
    if (category_2) category_2->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory2);
    if (category_3) category_3->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory3);

    // Commands list:
    if (commands)
    {
        commands->OnItemSelectionChanged().AddUObject(this, &UControlOptionsDlg::HandleCommandSelectionChanged);
        // True "double click" is usually implemented in the entry widget; we emulate legacy
        // by timing consecutive selection of the same item.
    }

    // Combos:
    if (control_model_combo)
        control_model_combo->OnSelectionChanged.AddDynamic(this, &UControlOptionsDlg::OnControlModelChanged);

    if (joy_select_combo)
        joy_select_combo->OnSelectionChanged.AddDynamic(this, &UControlOptionsDlg::OnJoySelectChanged);

    if (joy_throttle_combo)
        joy_throttle_combo->OnSelectionChanged.AddDynamic(this, &UControlOptionsDlg::OnJoyThrottleChanged);

    if (joy_rudder_combo)
        joy_rudder_combo->OnSelectionChanged.AddDynamic(this, &UControlOptionsDlg::OnJoyRudderChanged);

    if (mouse_select_combo)
        mouse_select_combo->OnSelectionChanged.AddDynamic(this, &UControlOptionsDlg::OnMouseSelectChanged);

    // Sliders:
    if (joy_sensitivity_slider)
        joy_sensitivity_slider->OnValueChanged.AddDynamic(this, &UControlOptionsDlg::OnJoySensitivityChanged);

    if (mouse_sensitivity_slider)
        mouse_sensitivity_slider->OnValueChanged.AddDynamic(this, &UControlOptionsDlg::OnMouseSensitivityChanged);

    // Checkbox:
    if (mouse_invert_checkbox)
        mouse_invert_checkbox->OnCheckStateChanged.AddDynamic(this, &UControlOptionsDlg::OnMouseInvertChanged);

    // Buttons:
    if (joy_axis_button)
        joy_axis_button->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnJoyAxis);

    if (apply_btn)
        apply_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnApply);

    if (cancel_btn)
        cancel_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCancel);

    // Top tabs:
    if (vid_btn) vid_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnVideo);
    if (aud_btn) aud_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnAudio);
    if (ctl_btn) ctl_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnControls);
    if (opt_btn) opt_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnOptions);
    if (mod_btn) mod_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnMod);

    // Initialize:
    Show();
}

// --------------------------------------------------------------------
// Show / Tick
// --------------------------------------------------------------------

void UControlOptionsDlg::Show()
{
    // Classic: if (!IsShown()) FormWindow::Show();
    // In UE, this is typically AddToViewport + SetVisibility elsewhere.
    // This method refreshes the content/state.

    if (closed)
        ShowCategory();
    else
        UpdateCategory();

    // Tab highlight state is style-driven in UMG; keep logical intent:
    // (Controls tab active)
    // You can expose "selected" states in BP if you want.
    closed = false;
}

void UControlOptionsDlg::ExecFrame()
{
    // Classic: Enter applies
    if (Keyboard::KeyDown(VK_RETURN)) {
        OnApply();
    }
}

// --------------------------------------------------------------------
// Category handling
// --------------------------------------------------------------------

void UControlOptionsDlg::SelectCategory(int32 InCategory)
{
    selected_category = FMath::Clamp(InCategory, 0, 3);
    ShowCategory();
}

void UControlOptionsDlg::OnCategory0() { SelectCategory(0); }
void UControlOptionsDlg::OnCategory1() { SelectCategory(1); }
void UControlOptionsDlg::OnCategory2() { SelectCategory(2); }
void UControlOptionsDlg::OnCategory3() { SelectCategory(3); }

void UControlOptionsDlg::ShowCategory()
{
    if (!commands || !stars)
        return;

    // Visual "sticky" button state should be done in UMG; here we just refresh data.

    CommandRows.Reset();
    commands->ClearListItems();

    Starshatter* S = stars;
    KeyMap& keymap = S->GetKeyMap();

    // Map core options from KeyMap:
    for (int i = 0; i < 256; i++) {
        KeyMapEntry* k = keymap.GetKeyMap(i);
        if (!k) continue;

        switch (k->act) {
        case 0:
            break;

        case KEY_CONTROL_MODEL:
            control_model = k->key;
            if (control_model_combo)
                control_model_combo->SetSelectedIndex(control_model);
            break;

        case KEY_JOY_SELECT:
            joy_select = k->key;
            if (joy_select_combo)
                joy_select_combo->SetSelectedIndex(joy_select);
            break;

        case KEY_JOY_RUDDER:
            joy_rudder = k->key;
            if (joy_rudder_combo)
                joy_rudder_combo->SetSelectedIndex(joy_rudder);
            break;

        case KEY_JOY_THROTTLE:
            joy_throttle = k->key;
            if (joy_throttle_combo)
                joy_throttle_combo->SetSelectedIndex(joy_throttle);
            break;

        case KEY_JOY_SENSE:
            joy_sensitivity = k->key;
            if (joy_sensitivity_slider)
                joy_sensitivity_slider->SetValue(IntToSlider(joy_sensitivity, 0, 10));
            break;

        case KEY_MOUSE_SELECT:
            mouse_select = k->key;
            if (mouse_select_combo)
                mouse_select_combo->SetSelectedIndex(mouse_select);
            break;

        case KEY_MOUSE_SENSE:
            mouse_sensitivity = k->key;
            if (mouse_sensitivity_slider)
                mouse_sensitivity_slider->SetValue(IntToSlider(mouse_sensitivity, 0, 50));
            break;

        case KEY_MOUSE_INVERT:
            mouse_invert = (k->key > 0) ? 1 : 0;
            if (mouse_invert_checkbox)
                mouse_invert_checkbox->SetIsChecked(mouse_invert > 0);
            break;

        default:
            // Fill the command list by category:
            if (keymap.GetCategory(i) == selected_category) {
                UControlBindingRow* Row = NewObject<UControlBindingRow>(this);
                Row->ActionIndex = i;
                Row->Command = UTF8_TO_TCHAR(keymap.DescribeAction(i));
                Row->Key = UTF8_TO_TCHAR(keymap.DescribeKey(i));

                CommandRows.Add(Row);
                commands->AddItem(Row);
            }
            break;
        }
    }
}

void UControlOptionsDlg::UpdateCategory()
{
    if (!commands || !stars)
        return;

    KeyMap& keymap = stars->GetKeyMap();

    for (UControlBindingRow* Row : CommandRows)
    {
        if (!Row) continue;
        Row->Key = UTF8_TO_TCHAR(keymap.DescribeKey(Row->ActionIndex));
    }

    // Force list to refresh (UMG sometimes needs a nudge):
    commands->RequestRefresh();
}

// --------------------------------------------------------------------
// Command list selection (legacy OnCommand)
// --------------------------------------------------------------------

void UControlOptionsDlg::HandleCommandSelectionChanged(UObject* SelectedItem)
{
    if (!commands || !SelectedItem)
        return;

    UControlBindingRow* Row = Cast<UControlBindingRow>(SelectedItem);
    if (!Row)
        return;

    const int32 ListIndex = commands->GetIndexForItem(SelectedItem);
    const double NowSec = Game::RealTime() / 1000.0;

    // Legacy double-click: same item selected again within 350ms
    if (ListIndex == command_index && (NowSec - command_click_time_sec) < 0.35)
    {
        HandleCommandDoubleClick(Row->ActionIndex);
    }

    command_click_time_sec = NowSec;
    command_index = ListIndex;
}

void UControlOptionsDlg::HandleCommandDoubleClick(int32 ActionIndex)
{
    // Legacy:
    // KeyDlg* key_dlg = manager->GetKeyDlg(); key_dlg->SetKeyMapIndex(ActionIndex); manager->ShowKeyDlg();

    // Keep loose: your "manager" in UE will be some screen widget/controller class.
    // Implement these calls on your manager and cast here.
    UE_LOG(LogTemp, Verbose, TEXT("ControlOptionsDlg: DoubleClick ActionIndex=%d"), ActionIndex);

    // Example pattern:
    // if (UYourOptionsScreen* M = Cast<UYourOptionsScreen>(manager)) { ... }
}

// --------------------------------------------------------------------
// Combo handlers (legacy OnControlModel / OnJoy* / OnMouse*)
// --------------------------------------------------------------------

void UControlOptionsDlg::OnControlModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (control_model_combo)
        control_model = control_model_combo->GetSelectedIndex();
}

void UControlOptionsDlg::OnJoySelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (joy_select_combo)
        joy_select = joy_select_combo->GetSelectedIndex();
}

void UControlOptionsDlg::OnJoyThrottleChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (joy_throttle_combo)
        joy_throttle = joy_throttle_combo->GetSelectedIndex();
}

void UControlOptionsDlg::OnJoyRudderChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (joy_rudder_combo)
        joy_rudder = joy_rudder_combo->GetSelectedIndex();
}

void UControlOptionsDlg::OnJoySensitivityChanged(float NormalizedValue)
{
    joy_sensitivity = SliderToInt(NormalizedValue, 0, 10);
}

void UControlOptionsDlg::OnJoyAxis()
{
    // Legacy: manager->ShowJoyDlg();
    UE_LOG(LogTemp, Verbose, TEXT("ControlOptionsDlg: JoyAxis setup requested"));
}

void UControlOptionsDlg::OnMouseSelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (mouse_select_combo)
        mouse_select = mouse_select_combo->GetSelectedIndex();
}

void UControlOptionsDlg::OnMouseSensitivityChanged(float NormalizedValue)
{
    mouse_sensitivity = SliderToInt(NormalizedValue, 0, 50);
}

void UControlOptionsDlg::OnMouseInvertChanged(bool bIsChecked)
{
    mouse_invert = bIsChecked ? 1 : 0;
}

// --------------------------------------------------------------------
// Navigation tabs (legacy)
// --------------------------------------------------------------------

void UControlOptionsDlg::OnAudio() { UE_LOG(LogTemp, Verbose, TEXT("ControlOptionsDlg: Audio tab")); }
void UControlOptionsDlg::OnVideo() { UE_LOG(LogTemp, Verbose, TEXT("ControlOptionsDlg: Video tab")); }
void UControlOptionsDlg::OnOptions() { UE_LOG(LogTemp, Verbose, TEXT("ControlOptionsDlg: Options tab")); }
void UControlOptionsDlg::OnControls() { UE_LOG(LogTemp, Verbose, TEXT("ControlOptionsDlg: Controls tab")); }
void UControlOptionsDlg::OnMod() { UE_LOG(LogTemp, Verbose, TEXT("ControlOptionsDlg: Mod tab")); }

// --------------------------------------------------------------------
// Apply / Cancel (legacy manager->ApplyOptions / CancelOptions)
// --------------------------------------------------------------------

void UControlOptionsDlg::OnApply()
{
    Apply();

    // Then notify manager in your UE architecture, if required:
    UE_LOG(LogTemp, Verbose, TEXT("ControlOptionsDlg: Apply"));
}

void UControlOptionsDlg::OnCancel()
{
    Cancel();
    UE_LOG(LogTemp, Verbose, TEXT("ControlOptionsDlg: Cancel"));
}

void UControlOptionsDlg::Apply()
{
    if (closed || !stars)
        return;

    KeyMap& keymap = stars->GetKeyMap();

    keymap.Bind(KEY_CONTROL_MODEL, control_model, 0);

    keymap.Bind(KEY_JOY_SELECT, joy_select, 0);
    keymap.Bind(KEY_JOY_RUDDER, joy_rudder, 0);
    keymap.Bind(KEY_JOY_THROTTLE, joy_throttle, 0);
    keymap.Bind(KEY_JOY_SENSE, joy_sensitivity, 0);

    keymap.Bind(KEY_MOUSE_SELECT, mouse_select, 0);
    keymap.Bind(KEY_MOUSE_SENSE, mouse_sensitivity, 0);
    keymap.Bind(KEY_MOUSE_INVERT, mouse_invert, 0);

    keymap.SaveKeyMap("key.cfg", 256);

    stars->MapKeys();
    Ship::SetControlModel(control_model);

    closed = true;
}

void UControlOptionsDlg::Cancel()
{
    closed = true;
}

// --------------------------------------------------------------------
// Slider mapping helpers
// --------------------------------------------------------------------

int32 UControlOptionsDlg::SliderToInt(float Normalized, int32 MinV, int32 MaxV)
{
    const float Clamped = FMath::Clamp(Normalized, 0.0f, 1.0f);
    const float V = MinV + (MaxV - MinV) * Clamped;
    return FMath::RoundToInt(V);
}

float UControlOptionsDlg::IntToSlider(int32 V, int32 MinV, int32 MaxV)
{
    if (MaxV <= MinV) return 0.0f;
    const int32 Clamped = FMath::Clamp(V, MinV, MaxV);
    return (float)(Clamped - MinV) / (float)(MaxV - MinV);
}
