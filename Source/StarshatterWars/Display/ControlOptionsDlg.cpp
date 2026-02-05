/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         ControlOptionsDlg.cpp
    AUTHOR:       Carlos Bott
*/

#include "ControlOptionsDlg.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"

#include "OptionsScreen.h"
#include "StarshatterControlsSettings.h"

UControlOptionsDlg::UControlOptionsDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetDialogInputEnabled(true);
}

void UControlOptionsDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // BaseScreen Enter/Escape routing:
    ApplyButton = ApplyBtn;
    CancelButton = CancelBtn;

    if (control_model_combo)
    {
        control_model_combo->OnSelectionChanged.RemoveAll(this);
        control_model_combo->OnSelectionChanged.AddDynamic(this, &UControlOptionsDlg::OnControlModelChanged);
    }

    if (joystick_index_slider)
    {
        joystick_index_slider->OnValueChanged.RemoveAll(this);
        joystick_index_slider->OnValueChanged.AddDynamic(this, &UControlOptionsDlg::OnJoystickIndexChanged);
    }

    if (throttle_axis_slider)
    {
        throttle_axis_slider->OnValueChanged.RemoveAll(this);
        throttle_axis_slider->OnValueChanged.AddDynamic(this, &UControlOptionsDlg::OnThrottleAxisChanged);
    }

    if (rudder_axis_slider)
    {
        rudder_axis_slider->OnValueChanged.RemoveAll(this);
        rudder_axis_slider->OnValueChanged.AddDynamic(this, &UControlOptionsDlg::OnRudderAxisChanged);
    }

    if (joystick_sensitivity_slider)
    {
        joystick_sensitivity_slider->OnValueChanged.RemoveAll(this);
        joystick_sensitivity_slider->OnValueChanged.AddDynamic(this, &UControlOptionsDlg::OnJoystickSensitivityChanged);
    }

    if (mouse_sensitivity_slider)
    {
        mouse_sensitivity_slider->OnValueChanged.RemoveAll(this);
        mouse_sensitivity_slider->OnValueChanged.AddDynamic(this, &UControlOptionsDlg::OnMouseSensitivityChanged);
    }

    if (mouse_invert_checkbox)
    {
        mouse_invert_checkbox->OnCheckStateChanged.RemoveAll(this);
        mouse_invert_checkbox->OnCheckStateChanged.AddDynamic(this, &UControlOptionsDlg::OnMouseInvertChanged);
    }

    if (ApplyBtn)
    {
        ApplyBtn->OnClicked.RemoveAll(this);
        ApplyBtn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnApplyClicked);
    }

    if (CancelBtn)
    {
        CancelBtn->OnClicked.RemoveAll(this);
        CancelBtn->OnClicked.AddDynamic(this, &UControlOptionsDlg::OnCancelClicked);
    }

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

void UControlOptionsDlg::ExecFrame()
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

    if (control_model_combo) control_model_combo->SetSelectedIndex(FMath::Clamp(control_model, 0, 2));

    // Axis index range defaults:
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
    {
        S->ApplyToRuntimeControls(const_cast<UControlOptionsDlg*>(this));
    }
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
    bClosed = true;
}

void UControlOptionsDlg::OnControlModelChanged(FString, ESelectInfo::Type)
{
    if (control_model_combo)
        control_model = control_model_combo->GetSelectedIndex();
}

void UControlOptionsDlg::OnJoystickIndexChanged(float NormalizedValue)
{
    joystick_index = SliderToInt(NormalizedValue, 0, 8);
}

void UControlOptionsDlg::OnThrottleAxisChanged(float NormalizedValue)
{
    throttle_axis = SliderToInt(NormalizedValue, 0, 16);
}

void UControlOptionsDlg::OnRudderAxisChanged(float NormalizedValue)
{
    rudder_axis = SliderToInt(NormalizedValue, 0, 16);
}

void UControlOptionsDlg::OnJoystickSensitivityChanged(float NormalizedValue)
{
    joystick_sensitivity = SliderToInt(NormalizedValue, 0, 10);
}

void UControlOptionsDlg::OnMouseSensitivityChanged(float NormalizedValue)
{
    mouse_sensitivity = SliderToInt(NormalizedValue, 0, 50);
}

void UControlOptionsDlg::OnMouseInvertChanged(bool bIsChecked)
{
    b_mouse_invert = bIsChecked;
}

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

// Tabs (route to OptionsScreen)
void UControlOptionsDlg::OnAudioClicked() { if (Manager) Manager->ShowAudDlg(); }
void UControlOptionsDlg::OnVideoClicked() { if (Manager) Manager->ShowVidDlg(); }
void UControlOptionsDlg::OnOptionsClicked() { if (Manager) Manager->ShowOptDlg(); }
void UControlOptionsDlg::OnControlsClicked() {}
void UControlOptionsDlg::OnModClicked() { if (Manager) Manager->ShowModDlg(); }

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
