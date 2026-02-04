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
    - ControlOptionsDlg routes under UOptionsScreen (NOT GameScreen / MenuScreen).
    - JoyDlg + KeyDlg are child overlays routed through ControlOptionsDlg.
*/

#include "ControlOptionsDlg.h"

// UE:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Engine/World.h"

// Starshatter:
#include "Starshatter.h"
#include "Ship.h"
#include "Game.h"
#include "Keyboard.h"
#include "Joystick.h"
#include "KeyMap.h"

// Routing:
#include "OptionsScreen.h"
#include "JoyDlg.h"
#include "KeyDlg.h"

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
    // If you are using BaseScreen numeric binding helpers, map them here.
    // Otherwise BindWidgetOptional is sufficient.
}

FString UControlOptionsDlg::GetLegacyFormText() const
{
    return FString();
}

// --------------------------------------------------------------------
// Wiring
// --------------------------------------------------------------------

void UControlOptionsDlg::RegisterControls()
{
    // Hook BaseScreen Enter/Escape routing:
    ApplyButton = ApplyBtn;
    CancelButton = CancelBtn;

    // Categories:
    if (category_0) { category_0->OnClicked.RemoveAll(this); category_0->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory0); }
    if (category_1) { category_1->OnClicked.RemoveAll(this); category_1->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory1); }
    if (category_2) { category_2->OnClicked.RemoveAll(this); category_2->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory2); }
    if (category_3) { category_3->OnClicked.RemoveAll(this); category_3->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCategory3); }

    // Commands list:
    // IMPORTANT: UListView::OnItemSelectionChanged() is NOT a dynamic multicast; use AddUObject.
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

    if (ApplyBtn) { ApplyBtn->OnClicked.RemoveAll(this);  ApplyBtn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnApplyClicked); }
    if (CancelBtn) { CancelBtn->OnClicked.RemoveAll(this); CancelBtn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCancelClicked); }

    // Tabs:
    if (vid_btn) { vid_btn->OnClicked.RemoveAll(this); vid_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnVideoClicked); }
    if (aud_btn) { aud_btn->OnClicked.RemoveAll(this); aud_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnAudioClicked); }
    if (ctl_btn) { ctl_btn->OnClicked.RemoveAll(this); ctl_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnControlsClicked); }
    if (opt_btn) { opt_btn->OnClicked.RemoveAll(this); opt_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnOptionsClicked); }
    if (mod_btn) { mod_btn->OnClicked.RemoveAll(this); mod_btn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnModClicked); }

    // Initial paint:
    Show();
}

// --------------------------------------------------------------------
// Show / Tick
// --------------------------------------------------------------------

void UControlOptionsDlg::Show()
{
    if (bClosed)
        ShowCategory();
    else
        UpdateCategory();

    bClosed = false;
    SetVisibility(ESlateVisibility::Visible);
    SetKeyboardFocus();
}

void UControlOptionsDlg::ExecFrame()
{
    // Classic: Enter applies
    if (Keyboard::KeyDown(VK_RETURN))
        OnApplyClicked();
}

// --------------------------------------------------------------------
// Child overlays (Joy/Key routed through ControlOptionsDlg)
// --------------------------------------------------------------------

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
    {
        UE_LOG(LogTemp, Warning, TEXT("ControlOptionsDlg: JoyDlg is null."));
        return;
    }

    JoyDlg->SetManager(this);

    SetVisibility(ESlateVisibility::Collapsed);
    JoyDlg->SetVisibility(ESlateVisibility::Visible);
    JoyDlg->SetKeyboardFocus();
}

void UControlOptionsDlg::ShowKeyDlg(int32 KeyMapIndex)
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
    {
        UE_LOG(LogTemp, Warning, TEXT("ControlOptionsDlg: KeyDlg is null."));
        return;
    }

    KeyDlg->SetManager(this);
    KeyDlg->SetKeyMapIndex(KeyMapIndex);

    SetVisibility(ESlateVisibility::Collapsed);
    KeyDlg->SetVisibility(ESlateVisibility::Visible);
    KeyDlg->SetKeyboardFocus();
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

    CommandRows.Reset();
    commands->ClearListItems();

    KeyMap& keymap = stars->GetKeyMap();

    // Map core options from KeyMap:
    for (int i = 0; i < 256; i++)
    {
        KeyMapEntry* k = keymap.GetKeyMap(i);
        if (!k) continue;

        switch (k->act)
        {
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
            if (keymap.GetCategory(i) == selected_category)
            {
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

    commands->RequestRefresh();
}

// --------------------------------------------------------------------
// Command selection
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
        // Open KeyDlg via ControlOptionsDlg routing:
        ShowKeyDlg(Row->ActionIndex);
    }

    command_click_time_sec = NowSec;
    command_index = ListIndex;
}

// --------------------------------------------------------------------
// Combo handlers
// --------------------------------------------------------------------

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

// --------------------------------------------------------------------
// Tabs (route UP to OptionsScreen)
// --------------------------------------------------------------------

void UControlOptionsDlg::OnAudioClicked() { if (Manager) Manager->ShowAudDlg(); }
void UControlOptionsDlg::OnVideoClicked() { if (Manager) Manager->ShowVidDlg(); }
void UControlOptionsDlg::OnOptionsClicked() { if (Manager) Manager->ShowOptDlg(); }
void UControlOptionsDlg::OnControlsClicked() { /* already here */ }
void UControlOptionsDlg::OnModClicked() { if (Manager) Manager->ShowModDlg(); }

// --------------------------------------------------------------------
// Apply/Cancel callbacks (route UP to OptionsScreen)
// --------------------------------------------------------------------

void UControlOptionsDlg::OnApplyClicked()
{
    if (Manager) Manager->ApplyOptions();
}

void UControlOptionsDlg::OnCancelClicked()
{
    if (Manager) Manager->CancelOptions();
}

// --------------------------------------------------------------------
// Legacy Apply / Cancel (called by OptionsScreen::ApplyOptions)
// --------------------------------------------------------------------

void UControlOptionsDlg::Apply()
{
    if (bClosed || !stars)
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

    bClosed = true;
}

void UControlOptionsDlg::Cancel()
{
    bClosed = true;
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
