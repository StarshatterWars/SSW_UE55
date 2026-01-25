/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         CtlDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Control Options (keyboard/joystick) dialog Unreal UUserWidget implementation.
    Manager is GameScreen (non-UObject). Use full include in this .cpp.
*/

#include "CtlDlg.h"

// IMPORTANT: full definition needed to call methods on manager:
#include "GameScreen.h"

// UMG:
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"

// Input:
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Starshatter:
#include "Starshatter.h"
#include "KeyMap.h"
#include "Ship.h"
#include "Game.h"

UCtlDlg::UCtlDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UCtlDlg::BindFormWidgets()
{
    // Categories:
    BindButton(101, category_101);
    BindButton(102, category_102);
    BindButton(103, category_103);
    BindButton(104, category_104);

    // Commands list:
    BindList(200, commands_list);

    // Combos:
    BindCombo(210, control_model_combo);
    BindCombo(211, joy_select_combo);
    BindCombo(212, joy_throttle_combo);
    BindCombo(213, joy_rudder_combo);
    BindCombo(511, mouse_select_combo);

    // Sliders:
    BindSlider(214, joy_sensitivity_slider);
    BindSlider(514, mouse_sensitivity_slider);

    // Buttons:
    BindButton(215, joy_axis_button);
    BindButton(515, mouse_invert_checkbox);

    // Tabs:
    BindButton(901, vid_btn);
    BindButton(902, aud_btn);
    BindButton(903, ctl_btn);
    BindButton(904, opt_btn);
    BindButton(905, mod_btn);

    // Apply/Cancel:
    BindButton(1, ApplyBtn);
    BindButton(2, CancelBtn);
}

void UCtlDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Apply/Cancel:
    if (ApplyBtn)  ApplyBtn->OnClicked.AddDynamic(this, &UCtlDlg::OnApplyClicked);
    if (CancelBtn) CancelBtn->OnClicked.AddDynamic(this, &UCtlDlg::OnCancelClicked);

    // Tabs:
    if (vid_btn) vid_btn->OnClicked.AddDynamic(this, &UCtlDlg::OnVideoClicked);
    if (aud_btn) aud_btn->OnClicked.AddDynamic(this, &UCtlDlg::OnAudioClicked);
    if (ctl_btn) ctl_btn->OnClicked.AddDynamic(this, &UCtlDlg::OnControlsClicked);
    if (opt_btn) opt_btn->OnClicked.AddDynamic(this, &UCtlDlg::OnOptionsClicked);
    if (mod_btn) mod_btn->OnClicked.AddDynamic(this, &UCtlDlg::OnModClicked);

    // Categories:
    if (category_101) category_101->OnClicked.AddDynamic(this, &UCtlDlg::OnCategory0Clicked);
    if (category_102) category_102->OnClicked.AddDynamic(this, &UCtlDlg::OnCategory1Clicked);
    if (category_103) category_103->OnClicked.AddDynamic(this, &UCtlDlg::OnCategory2Clicked);
    if (category_104) category_104->OnClicked.AddDynamic(this, &UCtlDlg::OnCategory3Clicked);

    // Combos:
    if (control_model_combo) control_model_combo->OnSelectionChanged.AddDynamic(this, &UCtlDlg::OnControlModelChanged);

    if (joy_select_combo)   joy_select_combo->OnSelectionChanged.AddDynamic(this, &UCtlDlg::OnJoySelectChanged);
    if (joy_throttle_combo) joy_throttle_combo->OnSelectionChanged.AddDynamic(this, &UCtlDlg::OnJoyThrottleChanged);
    if (joy_rudder_combo)   joy_rudder_combo->OnSelectionChanged.AddDynamic(this, &UCtlDlg::OnJoyRudderChanged);

    if (mouse_select_combo) mouse_select_combo->OnSelectionChanged.AddDynamic(this, &UCtlDlg::OnMouseSelectChanged);

    // Sliders:
    if (joy_sensitivity_slider)   joy_sensitivity_slider->OnValueChanged.AddDynamic(this, &UCtlDlg::OnJoySensitivityChanged);
    if (mouse_sensitivity_slider) mouse_sensitivity_slider->OnValueChanged.AddDynamic(this, &UCtlDlg::OnMouseSensitivityChanged);

    // Buttons:
    if (joy_axis_button)       joy_axis_button->OnClicked.AddDynamic(this, &UCtlDlg::OnJoyAxisClicked);
    if (mouse_invert_checkbox) mouse_invert_checkbox->OnClicked.AddDynamic(this, &UCtlDlg::OnMouseInvertClicked);

    // Commands list selection:
    if (commands_list)
        commands_list->OnItemClicked().AddUObject(this, &UCtlDlg::OnCommandSelected);

    // Open:
    if (closed)
        ShowCategory();
    else
        UpdateCategory();

    closed = false;
}

void UCtlDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

FReply UCtlDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
    {
        OnApplyClicked();
        return FReply::Handled();
    }

    if (Key == EKeys::Escape || Key == EKeys::Virtual_Back)
    {
        OnCancelClicked();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ---------------------------------------------------------------------
// Legacy behavior: ShowCategory / UpdateCategory
// ---------------------------------------------------------------------

void UCtlDlg::ShowCategory()
{
    Starshatter* Stars = Starshatter::GetInstance();
    if (!Stars)
        return;

    KeyMap& Keymap = Stars->GetKeyMap();

    // Pull special binds, mirror original CtlDlg:
    for (int32 i = 0; i < 256; i++)
    {
        KeyMapEntry* K = Keymap.GetKeyMap(i);
        if (!K)
            continue;

        switch (K->act)
        {
        case KEY_CONTROL_MODEL:
            control_model = K->key;
            if (control_model_combo)
                control_model_combo->SetSelectedIndex(control_model);
            break;

        case KEY_JOY_SELECT:
            joy_select = K->key;
            if (joy_select_combo)
                joy_select_combo->SetSelectedIndex(joy_select);
            break;

        case KEY_JOY_RUDDER:
            joy_rudder = K->key;
            if (joy_rudder_combo)
                joy_rudder_combo->SetSelectedIndex(joy_rudder);
            break;

        case KEY_JOY_THROTTLE:
            joy_throttle = K->key;
            if (joy_throttle_combo)
                joy_throttle_combo->SetSelectedIndex(joy_throttle);
            break;

        case KEY_JOY_SENSE:
            joy_sensitivity = K->key;
            if (joy_sensitivity_slider)
                joy_sensitivity_slider->SetValue(FMath::Clamp(joy_sensitivity / 10.0f, 0.0f, 1.0f));
            break;

        case KEY_MOUSE_SELECT:
            mouse_select = K->key;
            if (mouse_select_combo)
                mouse_select_combo->SetSelectedIndex(mouse_select);
            break;

        case KEY_MOUSE_SENSE:
            mouse_sensitivity = K->key;
            if (mouse_sensitivity_slider)
                mouse_sensitivity_slider->SetValue(FMath::Clamp(mouse_sensitivity / 50.0f, 0.0f, 1.0f));
            break;

        case KEY_MOUSE_INVERT:
            mouse_invert = (K->key > 0);
            break;

        default:
            break;
        }
    }

    // Populate command list:
    if (commands_list)
    {
        commands_list->ClearListItems();

        // Integration point:
        // for (int32 i=0; i<256; ++i)
        //   if (Keymap.GetCategory(i) == selected_category)
        //      Add your list item UObject (ActionIndex=i, ActionText=..., KeyText=...)
    }
}

void UCtlDlg::UpdateCategory()
{
    // Refresh list key text if needed after key changes.
}

// ---------------------------------------------------------------------
// Categories
// ---------------------------------------------------------------------

void UCtlDlg::OnCategory0Clicked() { selected_category = 0; ShowCategory(); }
void UCtlDlg::OnCategory1Clicked() { selected_category = 1; ShowCategory(); }
void UCtlDlg::OnCategory2Clicked() { selected_category = 2; ShowCategory(); }
void UCtlDlg::OnCategory3Clicked() { selected_category = 3; ShowCategory(); }

// ---------------------------------------------------------------------
// List selection
// ---------------------------------------------------------------------

void UCtlDlg::OnCommandSelected(UObject* Item)
{
    // Legacy behavior: double-click opens KeyDlg and sets keymap index.
    // You will forward Item->ActionIndex to manager->ShowKeyDlg() equivalent.
    (void)Item;
}

// ---------------------------------------------------------------------
// Control changes
// ---------------------------------------------------------------------

void UCtlDlg::OnControlModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    (void)SelectedItem;
    (void)SelectionType;

    if (control_model_combo)
        control_model = control_model_combo->GetSelectedIndex();
}

void UCtlDlg::OnJoySelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    (void)SelectedItem;
    (void)SelectionType;

    if (joy_select_combo)
        joy_select = joy_select_combo->GetSelectedIndex();
}

void UCtlDlg::OnJoyThrottleChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    (void)SelectedItem;
    (void)SelectionType;

    if (joy_throttle_combo)
        joy_throttle = joy_throttle_combo->GetSelectedIndex();
}

void UCtlDlg::OnJoyRudderChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    (void)SelectedItem;
    (void)SelectionType;

    if (joy_rudder_combo)
        joy_rudder = joy_rudder_combo->GetSelectedIndex();
}

void UCtlDlg::OnMouseSelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    (void)SelectedItem;
    (void)SelectionType;

    if (mouse_select_combo)
        mouse_select = mouse_select_combo->GetSelectedIndex();
}

void UCtlDlg::OnJoySensitivityChanged(float Value)
{
    joy_sensitivity = FMath::Clamp(FMath::RoundToInt(Value * 10.0f), 0, 10);
}

void UCtlDlg::OnMouseSensitivityChanged(float Value)
{
    mouse_sensitivity = FMath::Clamp(FMath::RoundToInt(Value * 50.0f), 0, 50);
}

void UCtlDlg::OnJoyAxisClicked()
{
    if (manager) manager->ShowJoyDlg();
    else UE_LOG(LogTemp, Warning, TEXT("CtlDlg: manager is null (OnJoyAxisClicked)."));
}

void UCtlDlg::OnMouseInvertClicked()
{
    mouse_invert = !mouse_invert;
}

// ---------------------------------------------------------------------
// Navigation (tabs)
// ---------------------------------------------------------------------

void UCtlDlg::OnAudioClicked()
{
    if (manager) manager->ShowAudDlg();
    else UE_LOG(LogTemp, Warning, TEXT("CtlDlg: manager is null (OnAudioClicked)."));
}

void UCtlDlg::OnVideoClicked()
{
    if (manager) manager->ShowVidDlg();
    else UE_LOG(LogTemp, Warning, TEXT("CtlDlg: manager is null (OnVideoClicked)."));
}

void UCtlDlg::OnOptionsClicked()
{
    if (manager) manager->ShowOptDlg();
    else UE_LOG(LogTemp, Warning, TEXT("CtlDlg: manager is null (OnOptionsClicked)."));
}

void UCtlDlg::OnControlsClicked()
{
    if (manager) manager->ShowCtlDlg();
    else UE_LOG(LogTemp, Warning, TEXT("CtlDlg: manager is null (OnControlsClicked)."));
}

void UCtlDlg::OnModClicked()
{
    if (manager) manager->ShowModDlg();
    else UE_LOG(LogTemp, Warning, TEXT("CtlDlg: manager is null (OnModClicked)."));
}

// ---------------------------------------------------------------------
// Apply / Cancel (mirrors legacy CtlDlg::OnApply/OnCancel forwarding)
// ---------------------------------------------------------------------

void UCtlDlg::OnApplyClicked()
{
    if (manager) manager->ApplyOptions();
    else UE_LOG(LogTemp, Warning, TEXT("CtlDlg: manager is null (OnApplyClicked)."));
}

void UCtlDlg::OnCancelClicked()
{
    if (manager) manager->CancelOptions();
    else UE_LOG(LogTemp, Warning, TEXT("CtlDlg: manager is null (OnCancelClicked)."));
}

// ---------------------------------------------------------------------
// Operations (mirrors legacy CtlDlg::Apply/Cancel)
// ---------------------------------------------------------------------

void UCtlDlg::Apply()
{
    if (closed)
        return;

    Starshatter* Stars = Starshatter::GetInstance();
    if (Stars)
    {
        KeyMap& Keymap = Stars->GetKeyMap();

        Keymap.Bind(KEY_CONTROL_MODEL, control_model, 0);

        Keymap.Bind(KEY_JOY_SELECT, joy_select, 0);
        Keymap.Bind(KEY_JOY_RUDDER, joy_rudder, 0);
        Keymap.Bind(KEY_JOY_THROTTLE, joy_throttle, 0);
        Keymap.Bind(KEY_JOY_SENSE, joy_sensitivity, 0);

        Keymap.Bind(KEY_MOUSE_SELECT, mouse_select, 0);
        Keymap.Bind(KEY_MOUSE_SENSE, mouse_sensitivity, 0);
        Keymap.Bind(KEY_MOUSE_INVERT, mouse_invert ? 1 : 0, 0);

        Keymap.SaveKeyMap("key.cfg", 256);

        Stars->MapKeys();
        Ship::SetControlModel(control_model);
    }

    closed = true;
}

void UCtlDlg::Cancel()
{
    closed = true;
}
