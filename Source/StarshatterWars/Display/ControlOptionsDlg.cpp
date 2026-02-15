/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:      StarshatterWars (Unreal Engine)
    FILE:           ControlOptionsDlg.cpp
    AUTHOR:         Carlos Bott

    IMPLEMENTATION
    ==============
    UControlOptionsDlg
=============================================================================*/

#include "ControlOptionsDlg.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"

#include "OptionsScreen.h"
#include "StarshatterControlsSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogControlOptionsDlg, Log, All);

UControlOptionsDlg::UControlOptionsDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetDialogInputEnabled(true);
}

void UControlOptionsDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BindDelegates();
}

void UControlOptionsDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // BaseScreen Enter/Escape routing (your pattern)
    ApplyButton = ApplyBtn;
    CancelButton = CancelBtn;

    // Safe even if BP reconstructs:
    BindDelegates();

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }
}

void UControlOptionsDlg::BindDelegates()
{
    if (bDelegatesBound)
        return;

    // Model controls
    if (control_model_combo)
        control_model_combo->OnSelectionChanged.AddUniqueDynamic(this, &UControlOptionsDlg::OnControlModelChanged);

    if (joystick_index_slider)
        joystick_index_slider->OnValueChanged.AddUniqueDynamic(this, &UControlOptionsDlg::OnJoystickIndexChanged);

    if (throttle_axis_slider)
        throttle_axis_slider->OnValueChanged.AddUniqueDynamic(this, &UControlOptionsDlg::OnThrottleAxisChanged);

    if (rudder_axis_slider)
        rudder_axis_slider->OnValueChanged.AddUniqueDynamic(this, &UControlOptionsDlg::OnRudderAxisChanged);

    if (joystick_sensitivity_slider)
        joystick_sensitivity_slider->OnValueChanged.AddUniqueDynamic(this, &UControlOptionsDlg::OnJoystickSensitivityChanged);

    if (mouse_sensitivity_slider)
        mouse_sensitivity_slider->OnValueChanged.AddUniqueDynamic(this, &UControlOptionsDlg::OnMouseSensitivityChanged);

    if (mouse_invert_checkbox)
        mouse_invert_checkbox->OnCheckStateChanged.AddUniqueDynamic(this, &UControlOptionsDlg::OnMouseInvertChanged);

    // Apply/Cancel
    if (ApplyBtn)
        ApplyBtn->OnClicked.AddUniqueDynamic(this, &UControlOptionsDlg::OnApplyClicked);

    if (CancelBtn)
        CancelBtn->OnClicked.AddUniqueDynamic(this, &UControlOptionsDlg::OnCancelClicked);

    // Tabs (route to OptionsScreen)
    if (vid_btn) vid_btn->OnClicked.AddUniqueDynamic(this, &UControlOptionsDlg::OnVideoClicked);
    if (aud_btn) aud_btn->OnClicked.AddUniqueDynamic(this, &UControlOptionsDlg::OnAudioClicked);
    if (ctl_btn) ctl_btn->OnClicked.AddUniqueDynamic(this, &UControlOptionsDlg::OnControlsClicked);
    if (opt_btn) opt_btn->OnClicked.AddUniqueDynamic(this, &UControlOptionsDlg::OnGameClicked);
    if (mod_btn) mod_btn->OnClicked.AddUniqueDynamic(this, &UControlOptionsDlg::OnModClicked);

    bDelegatesBound = true;
}

void UControlOptionsDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    (void)MyGeometry;
    ExecFrame((double)InDeltaTime);
}

// UBaseScreen overrides
void UControlOptionsDlg::BindFormWidgets() {}
FString UControlOptionsDlg::GetLegacyFormText() const { return FString(); }

void UControlOptionsDlg::HandleAccept() { OnApplyClicked(); }
void UControlOptionsDlg::HandleCancel() { OnCancelClicked(); }

UStarshatterControlsSettings* UControlOptionsDlg::GetControlsSettings() const
{
    return UStarshatterControlsSettings::Get();
}

void UControlOptionsDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }

    SetKeyboardFocus();
}

void UControlOptionsDlg::ExecFrame(double /*DeltaTime*/)
{
    // No polling. BaseScreen handles input routing.
}

void UControlOptionsDlg::RefreshFromModel()
{
    UStarshatterControlsSettings* S = GetControlsSettings();
    if (!S)
        return;

    S->Load();
    const FStarshatterControlsConfig& C = S->GetControlsConfig();

    control_model = (int32)C.ControlModel;

    joystick_index = C.JoystickIndex;
    throttle_axis = C.ThrottleAxis;
    rudder_axis = C.RudderAxis;
    joystick_sensitivity = C.JoystickSensitivity;

    mouse_sensitivity = C.MouseSensitivity;
    b_mouse_invert = C.bMouseInvert;

    if (control_model_combo)
        control_model_combo->SetSelectedIndex(FMath::Clamp(control_model, 0, 2));

    if (joystick_index_slider) joystick_index_slider->SetValue(IntToSlider(joystick_index, 0, 8));
    if (throttle_axis_slider) throttle_axis_slider->SetValue(IntToSlider(throttle_axis, 0, 16));
    if (rudder_axis_slider) rudder_axis_slider->SetValue(IntToSlider(rudder_axis, 0, 16));
    if (joystick_sensitivity_slider) joystick_sensitivity_slider->SetValue(IntToSlider(joystick_sensitivity, 0, 10));

    if (mouse_sensitivity_slider) mouse_sensitivity_slider->SetValue(IntToSlider(mouse_sensitivity, 0, 50));
    if (mouse_invert_checkbox) mouse_invert_checkbox->SetIsChecked(b_mouse_invert);
}

void UControlOptionsDlg::PushToModel(bool bApplyRuntimeToo)
{
    UStarshatterControlsSettings* S = GetControlsSettings();
    if (!S)
        return;

    FStarshatterControlsConfig C = S->GetControlsConfig();

    C.ControlModel = (EStarshatterControlModel)FMath::Clamp(control_model, 0, 2);

    C.JoystickIndex = FMath::Max(0, joystick_index);
    C.ThrottleAxis = FMath::Max(0, throttle_axis);
    C.RudderAxis = FMath::Max(0, rudder_axis);
    C.JoystickSensitivity = FMath::Clamp(joystick_sensitivity, 0, 10);

    C.MouseSensitivity = FMath::Clamp(mouse_sensitivity, 0, 50);
    C.bMouseInvert = b_mouse_invert;

    S->SetControlsConfig(C);
    S->Sanitize();
    S->Save();

    if (bApplyRuntimeToo)
        S->ApplyToRuntimeControls(this);
}

void UControlOptionsDlg::Apply()
{
    if (bClosed)
        return;

    PushToModel(true);
    bClosed = true;
}

void UControlOptionsDlg::Cancel()
{
    // Restore from model, but keep behavior consistent with AudioDlg:
    RefreshFromModel();
    bClosed = true;
}

// -------------------- Handlers --------------------

void UControlOptionsDlg::OnControlModelChanged(FString, ESelectInfo::Type)
{
    if (control_model_combo)
    {
        control_model = control_model_combo->GetSelectedIndex();
        bClosed = false;
    }
}

void UControlOptionsDlg::OnJoystickIndexChanged(float NormalizedValue)
{
    joystick_index = SliderToInt(NormalizedValue, 0, 8);
    bClosed = false;
}

void UControlOptionsDlg::OnThrottleAxisChanged(float NormalizedValue)
{
    throttle_axis = SliderToInt(NormalizedValue, 0, 16);
    bClosed = false;
}

void UControlOptionsDlg::OnRudderAxisChanged(float NormalizedValue)
{
    rudder_axis = SliderToInt(NormalizedValue, 0, 16);
    bClosed = false;
}

void UControlOptionsDlg::OnJoystickSensitivityChanged(float NormalizedValue)
{
    joystick_sensitivity = SliderToInt(NormalizedValue, 0, 10);
    bClosed = false;
}

void UControlOptionsDlg::OnMouseSensitivityChanged(float NormalizedValue)
{
    mouse_sensitivity = SliderToInt(NormalizedValue, 0, 50);
    bClosed = false;
}

void UControlOptionsDlg::OnMouseInvertChanged(bool bIsChecked)
{
    b_mouse_invert = bIsChecked;
    bClosed = false;
}

void UControlOptionsDlg::OnApplyClicked()
{
    if (OptionsManager) OptionsManager->ApplyOptions();
    else Apply();
}

void UControlOptionsDlg::OnCancelClicked()
{
    if (OptionsManager) OptionsManager->CancelOptions();
    else Cancel();
}

// Tabs (route to OptionsScreen)
void UControlOptionsDlg::OnAudioClicked() { if (OptionsManager) OptionsManager->ShowAudDlg(); }
void UControlOptionsDlg::OnVideoClicked() { if (OptionsManager) OptionsManager->ShowVidDlg(); }
void UControlOptionsDlg::OnGameClicked() { if (OptionsManager) OptionsManager->ShowGameDlg(); }
void UControlOptionsDlg::OnControlsClicked() { /* already here */ }
void UControlOptionsDlg::OnModClicked() { if (OptionsManager) OptionsManager->ShowModDlg(); }

// -------------------- Slider helpers --------------------

int32 UControlOptionsDlg::SliderToInt(float Normalized, int32 MinV, int32 MaxV)
{
    const float Clamped = FMath::Clamp(Normalized, 0.0f, 1.0f);
    const float V = (float)MinV + (float)(MaxV - MinV) * Clamped;
    return FMath::RoundToInt(V);
}

float UControlOptionsDlg::IntToSlider(int32 V, int32 MinV, int32 MaxV)
{
    if (MaxV <= MinV) return 0.0f;
    const int32 Clamped = FMath::Clamp(V, MinV, MaxV);
    return (float)(Clamped - MinV) / (float)(MaxV - MinV);
}
