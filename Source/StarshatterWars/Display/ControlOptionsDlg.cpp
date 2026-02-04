/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         ControlOptionsDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UControlOptionsDlg wired to the NEW controls system:
      - UStarshatterControlsSettings (config-backed CDO) is the ONLY persistence model.
      - UStarshatterControlsSubsystem performs runtime apply (legacy KeyMap/Ship wiring).
      - Legacy KeyMap is used READ-ONLY for action/key descriptions and categories.
*/

#include "ControlOptionsDlg.h"

// UE:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Engine/World.h"

// Legacy (read-only describe/category):
#include "Starshatter.h"
#include "KeyMap.h"
#include "Game.h"
#include "Keyboard.h"

// Routing/overlays:
#include "OptionsScreen.h"
#include "JoyDlg.h"
#include "KeyDlg.h"

// NEW:
#include "StarshatterControlsSubsystem.h"
#include "StarshatterControlsSettings.h"

UControlOptionsDlg::UControlOptionsDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetDialogInputEnabled(true);
}

void UControlOptionsDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    stars = Starshatter::GetInstance();
}

void UControlOptionsDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // BaseScreen Enter/Escape routing:
    ApplyButton = ApplyBtn;
    CancelButton = CancelBtn;

    // Categories:
    if (category_0) { category_0->OnClicked.RemoveAll(this); category_0->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory0); }
    if (category_1) { category_1->OnClicked.RemoveAll(this); category_1->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory1); }
    if (category_2) { category_2->OnClicked.RemoveAll(this); category_2->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory2); }
    if (category_3) { category_3->OnClicked.RemoveAll(this); category_3->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory3); }

    // Commands list:
    if (commands)
    {
        commands->OnItemSelectionChanged().RemoveAll(this);
        commands->OnItemSelectionChanged().AddUObject(this, &UControlOptionsDlg::HandleCommandSelectionChanged);
    }

    // Combos:
    if (control_model_combo) { control_model_combo->OnSelectionChanged.RemoveAll(this); control_model_combo->OnSelectionChanged.AddDynamic(this, &UControlOptionsDlg::OnControlModelChanged); }
    if (joy_select_combo) { joy_select_combo->OnSelectionChanged.RemoveAll(this);    joy_select_combo->OnSelectionChanged.AddDynamic(this, &UControlOptionsDlg::OnJoySelectChanged); }
    if (joy_throttle_combo) { joy_throttle_combo->OnSelectionChanged.RemoveAll(this);  joy_throttle_combo->OnSelectionChanged.AddDynamic(this, &UControlOptionsDlg::OnJoyThrottleChanged); }
    if (joy_rudder_combo) { joy_rudder_combo->OnSelectionChanged.RemoveAll(this);    joy_rudder_combo->OnSelectionChanged.AddDynamic(this, &UControlOptionsDlg::OnJoyRudderChanged); }
    if (mouse_select_combo) { mouse_select_combo->OnSelectionChanged.RemoveAll(this);  mouse_select_combo->OnSelectionChanged.AddDynamic(this, &UControlOptionsDlg::OnMouseSelectChanged); }

    // Sliders:
    if (joy_sensitivity_slider) { joy_sensitivity_slider->OnValueChanged.RemoveAll(this);   joy_sensitivity_slider->OnValueChanged.AddDynamic(this, &UControlOptionsDlg::OnJoySensitivityChanged); }
    if (mouse_sensitivity_slider) { mouse_sensitivity_slider->OnValueChanged.RemoveAll(this); mouse_sensitivity_slider->OnValueChanged.AddDynamic(this, &UControlOptionsDlg::OnMouseSensitivityChanged); }

    // Checkbox:
    if (mouse_invert_checkbox) { mouse_invert_checkbox->OnCheckStateChanged.RemoveAll(this); mouse_invert_checkbox->OnCheckStateChanged.AddDynamic(this, &UControlOptionsDlg::OnMouseInvertChanged); }

    // Buttons:
    if (joy_axis_button) { joy_axis_button->OnClicked.RemoveAll(this); joy_axis_button->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnJoyAxis); }
    if (ApplyBtn) { ApplyBtn->OnClicked.RemoveAll(this);        ApplyBtn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnApplyClicked); }
    if (CancelBtn) { CancelBtn->OnClicked.RemoveAll(this);       CancelBtn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCancelClicked); }

    // Tabs:
    if (vid_btn) { vid_btn->OnClicked.RemoveAll(this); vid_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnVideoClicked); }
    if (aud_btn) { aud_btn->OnClicked.RemoveAll(this); aud_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnAudioClicked); }
    if (ctl_btn) { ctl_btn->OnClicked.RemoveAll(this); ctl_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnControlsClicked); }
    if (opt_btn) { opt_btn->OnClicked.RemoveAll(this); opt_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnOptionsClicked); }
    if (mod_btn) { mod_btn->OnClicked.RemoveAll(this); mod_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnModClicked); }

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }
}

void UControlOptionsDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    (void)MyGeometry;
    (void)InDeltaTime;
    ExecFrame();
}

// ---------------------------------------------------------------------
// UBaseScreen overrides
// ---------------------------------------------------------------------

void UControlOptionsDlg::BindFormWidgets() {}
FString UControlOptionsDlg::GetLegacyFormText() const { return FString(); }

void UControlOptionsDlg::HandleAccept() { OnApplyClicked(); }
void UControlOptionsDlg::HandleCancel() { OnCancelClicked(); }

// ---------------------------------------------------------------------
// NEW: accessors
// ---------------------------------------------------------------------

UStarshatterControlsSubsystem* UControlOptionsDlg::GetControlsSubsystem() const
{
    // Matches the pattern you used for AudioSubsystem::Get(this)
    return UStarshatterControlsSubsystem::Get(const_cast<UControlOptionsDlg*>(this));
}

UStarshatterControlsSettings* UControlOptionsDlg::GetControlsSettings() const
{
    return UStarshatterControlsSettings::Get();
}

KeyMap* UControlOptionsDlg::GetLegacyKeyMap() const
{
    if (!stars) return nullptr;
    return &stars->GetKeyMap();
}

// ---------------------------------------------------------------------
// Show / ExecFrame
// ---------------------------------------------------------------------

void UControlOptionsDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }

    // Build list from current category each show
    RebuildCommandList();
    SetKeyboardFocus();
}

void UControlOptionsDlg::ExecFrame()
{
    // Optional: retain legacy Enter-to-apply behavior:
    if (Keyboard::KeyDown(VK_RETURN))
        OnApplyClicked();
}

// ---------------------------------------------------------------------
// Model -> UI
// ---------------------------------------------------------------------

void UControlOptionsDlg::RefreshFromModel()
{
    UStarshatterControlsSettings* S = GetControlsSettings();
    if (!S) return;

    S->Load(); // ReloadConfig + Sanitize

    control_model = S->GetControlModel();
    joy_select = S->GetJoySelect();
    joy_throttle = S->GetJoyThrottle();
    joy_rudder = S->GetJoyRudder();
    joy_sensitivity = S->GetJoySensitivity();

    mouse_select = S->GetMouseSelect();
    mouse_sensitivity = S->GetMouseSensitivity();
    mouse_invert = S->GetMouseInvert();

    // Paint UI:
    if (control_model_combo) control_model_combo->SetSelectedIndex(control_model);

    if (joy_select_combo)    joy_select_combo->SetSelectedIndex(joy_select);
    if (joy_throttle_combo)  joy_throttle_combo->SetSelectedIndex(joy_throttle);
    if (joy_rudder_combo)    joy_rudder_combo->SetSelectedIndex(joy_rudder);
    if (joy_sensitivity_slider) joy_sensitivity_slider->SetValue(IntToSlider(joy_sensitivity, 0, 10));

    if (mouse_select_combo)  mouse_select_combo->SetSelectedIndex(mouse_select);
    if (mouse_sensitivity_slider) mouse_sensitivity_slider->SetValue(IntToSlider(mouse_sensitivity, 0, 50));
    if (mouse_invert_checkbox) mouse_invert_checkbox->SetIsChecked(mouse_invert != 0);
}

// ---------------------------------------------------------------------
// UI -> Model (+ optional runtime apply)
// ---------------------------------------------------------------------

void UControlOptionsDlg::PushToModel(bool bApplyRuntimeToo)
{
    UStarshatterControlsSettings* S = GetControlsSettings();
    if (!S) return;

    S->SetControlModel(control_model);

    S->SetJoySelect(joy_select);
    S->SetJoyThrottle(joy_throttle);
    S->SetJoyRudder(joy_rudder);
    S->SetJoySensitivity(joy_sensitivity);

    S->SetMouseSelect(mouse_select);
    S->SetMouseSensitivity(mouse_sensitivity);
    S->SetMouseInvert(mouse_invert);

    // NOTE: key bindings (ActionKeys[0..255]) are typically edited by KeyDlg/JoyDlg.
    // This dialog only edits the “specials”.

    S->Sanitize();
    S->Save();

    if (bApplyRuntimeToo)
    {
        if (UStarshatterControlsSubsystem* ControlsSS = GetControlsSubsystem())
        {
            // Subsystem owns applying the settings into legacy runtime (KeyMap/Ship/etc.)
            ControlsSS->ApplySettingsToRuntime(const_cast<UControlOptionsDlg*>(this));
        }
    }
}

// ---------------------------------------------------------------------
// Apply / Cancel (called by OptionsScreen::ApplyOptions)
// ---------------------------------------------------------------------

void UControlOptionsDlg::Apply()
{
    if (bClosed) return;
    PushToModel(true);
    bClosed = true;
}

void UControlOptionsDlg::Cancel()
{
    bClosed = true;
}

// ---------------------------------------------------------------------
// Category + list (READ-ONLY legacy for names/categories)
// ---------------------------------------------------------------------

void UControlOptionsDlg::SelectCategory(int32 InCategory)
{
    selected_category = FMath::Clamp(InCategory, 0, 3);
    RebuildCommandList();
}

void UControlOptionsDlg::OnCategory0() { SelectCategory(0); }
void UControlOptionsDlg::OnCategory1() { SelectCategory(1); }
void UControlOptionsDlg::OnCategory2() { SelectCategory(2); }
void UControlOptionsDlg::OnCategory3() { SelectCategory(3); }

FString UControlOptionsDlg::DescribeAction(int32 ActionIndex) const
{
    if (KeyMap* KM = GetLegacyKeyMap())
        return UTF8_TO_TCHAR(KM->DescribeAction(ActionIndex));
    return FString::Printf(TEXT("ACTION %d"), ActionIndex);
}

FString UControlOptionsDlg::DescribeKeyFromAction(int32 ActionIndex) const
{
    // For now we show the legacy description (it will match once subsystem applied),
    // but we *prefer* what’s stored in the Settings ActionKeys.
    UStarshatterControlsSettings* S = GetControlsSettings();
    if (!S) return FString();

    const int32 KeyCode = S->GetActionKey(ActionIndex);

    if (KeyMap* KM = GetLegacyKeyMap())
        return UTF8_TO_TCHAR(KM->DescribeKey(KeyCode));

    return FString::FromInt(KeyCode);
}

void UControlOptionsDlg::RebuildCommandList()
{
    if (!commands) return;

    CommandRows.Reset();
    commands->ClearListItems();

    KeyMap* KM = GetLegacyKeyMap();
    if (!KM) return;

    // Build from legacy category table, but show keys from Settings
    for (int32 i = 0; i < 256; ++i)
    {
        // Skip “specials” that are edited via combos/sliders:
        // (Keep this list consistent with your KeyMap constants.)
        KeyMapEntry* Entry = KM->GetKeyMap(i);
        if (!Entry) continue;

        // Legacy “specials” are handled by combos/slider/checkbox:
        switch (Entry->act)
        {
        case KEY_CONTROL_MODEL:
        case KEY_JOY_SELECT:
        case KEY_JOY_RUDDER:
        case KEY_JOY_THROTTLE:
        case KEY_JOY_SENSE:
        case KEY_MOUSE_SELECT:
        case KEY_MOUSE_SENSE:
        case KEY_MOUSE_INVERT:
            continue;

        default:
            break;
        }

        if (KM->GetCategory(i) != selected_category)
            continue;

        UControlBindingRow* Row = NewObject<UControlBindingRow>(this);
        Row->ActionIndex = i;
        Row->Command = DescribeAction(i);
        Row->Key = DescribeKeyFromAction(i);

        CommandRows.Add(Row);
        commands->AddItem(Row);
    }

    commands->RequestRefresh();
}

void UControlOptionsDlg::RefreshCommandKeysOnly()
{
    for (UControlBindingRow* Row : CommandRows)
    {
        if (!Row) continue;
        Row->Key = DescribeKeyFromAction(Row->ActionIndex);
    }

    if (commands)
        commands->RequestRefresh();
}

// ---------------------------------------------------------------------
// List selection / double-click -> KeyDlg (KeyDlg should write into Settings)
// ---------------------------------------------------------------------

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
        ShowKeyDlg(Row->ActionIndex);
    }

    command_click_time_sec = NowSec;
    command_index = ListIndex;
}

// ---------------------------------------------------------------------
// Overlays
// ---------------------------------------------------------------------

void UControlOptionsDlg::ShowJoyDlg()
{
    if (!JoyDlg)
    {
        if (UWorld* World = GetWorld())
        {
            JoyDlg = CreateWidget<UJoyDlg>(World, UJoyDlg::StaticClass());
            if (JoyDlg)
            {
                JoyDlg->AddToViewport(0);
                JoyDlg->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }

    if (!JoyDlg)
        return;

    JoyDlg->SetManager(this);

    SetVisibility(ESlateVisibility::Collapsed);
    JoyDlg->SetVisibility(ESlateVisibility::Visible);
    JoyDlg->SetKeyboardFocus();
}

void UControlOptionsDlg::ShowKeyDlg(int32 ActionIndex)
{
    if (!KeyDlg)
    {
        if (UWorld* World = GetWorld())
        {
            KeyDlg = CreateWidget<UKeyDlg>(World, UKeyDlg::StaticClass());
            if (KeyDlg)
            {
                KeyDlg->AddToViewport(0);
                KeyDlg->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }

    if (!KeyDlg)
        return;

    KeyDlg->SetManager(this);
    KeyDlg->SetKeyMapIndex(ActionIndex);

    SetVisibility(ESlateVisibility::Collapsed);
    KeyDlg->SetVisibility(ESlateVisibility::Visible);
    KeyDlg->SetKeyboardFocus();
}

// ---------------------------------------------------------------------
// Combos/sliders/checkbox (update snapshot only)
// ---------------------------------------------------------------------

void UControlOptionsDlg::OnControlModelChanged(FString, ESelectInfo::Type)
{
    if (control_model_combo)
        control_model = control_model_combo->GetSelectedIndex();
}

void UControlOptionsDlg::OnJoySelectChanged(FString, ESelectInfo::Type)
{
    if (joy_select_combo)
        joy_select = joy_select_combo->GetSelectedIndex();
}

void UControlOptionsDlg::OnJoyThrottleChanged(FString, ESelectInfo::Type)
{
    if (joy_throttle_combo)
        joy_throttle = joy_throttle_combo->GetSelectedIndex();
}

void UControlOptionsDlg::OnJoyRudderChanged(FString, ESelectInfo::Type)
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
    ShowJoyDlg();
}

void UControlOptionsDlg::OnMouseSelectChanged(FString, ESelectInfo::Type)
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

// ---------------------------------------------------------------------
// Apply/Cancel (route to OptionsScreen)
// ---------------------------------------------------------------------

void UControlOptionsDlg::OnApplyClicked()
{
    if (Manager) Manager->ApplyOptions();
    else Apply();
}

void UControlOptionsDlg::OnCancelClicked()
{
    if (Manager) Manager->CancelOptions();
    else Cancel();
}

// ---------------------------------------------------------------------
// Tabs (route to OptionsScreen)
// ---------------------------------------------------------------------

void UControlOptionsDlg::OnAudioClicked() { if (Manager) Manager->ShowAudDlg(); }
void UControlOptionsDlg::OnVideoClicked() { if (Manager) Manager->ShowVidDlg(); }
void UControlOptionsDlg::OnOptionsClicked() { if (Manager) Manager->ShowOptDlg(); }
void UControlOptionsDlg::OnControlsClicked() { /* already here */ }
void UControlOptionsDlg::OnModClicked() { if (Manager) Manager->ShowModDlg(); }

// ---------------------------------------------------------------------
// Slider mapping helpers
// ---------------------------------------------------------------------

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
