/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         ControlOptionsDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UControlOptionsDlg
    - UE-only controls options dialog.
    - UI reads/writes ONLY UStarshatterControlsSettings (config-backed CDO).
    - Runtime apply is delegated to UStarshatterControlsSettings::ApplyToRuntimeControls(...)
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "ControlOptionsDlg.generated.h"

class UButton;
class UComboBoxString;
class USlider;
class UCheckBox;

class UOptionsScreen;
class UStarshatterControlsSettings;

UCLASS()
class STARSHATTERWARS_API UControlOptionsDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UControlOptionsDlg(const FObjectInitializer& ObjectInitializer);

    void SetManager(UOptionsScreen* InManager) { Manager = InManager; }
    UOptionsScreen* GetManager() const { return Manager; }

    virtual void Show();
    virtual void ExecFrame();
    virtual void Apply();
    virtual void Cancel();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;
    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

private:
    UStarshatterControlsSettings* GetControlsSettings() const;

    void RefreshFromModel();
    void PushToModel(bool bApplyRuntimeToo);

    static int32 SliderToInt(float Normalized, int32 MinV, int32 MaxV);
    static float IntToSlider(int32 V, int32 MinV, int32 MaxV);

private:
    UFUNCTION() void OnControlModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void OnJoystickIndexChanged(float NormalizedValue);
    UFUNCTION() void OnThrottleAxisChanged(float NormalizedValue);
    UFUNCTION() void OnRudderAxisChanged(float NormalizedValue);
    UFUNCTION() void OnJoystickSensitivityChanged(float NormalizedValue);

    UFUNCTION() void OnMouseSensitivityChanged(float NormalizedValue);
    UFUNCTION() void OnMouseInvertChanged(bool bIsChecked);

    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // Tabs
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

private:
    UPROPERTY(Transient) TObjectPtr<UOptionsScreen> Manager = nullptr;

    bool bClosed = true;

    // Snapshot (mirrors FStarshatterControlsConfig)
    int32 control_model = 1; // FlightSim default

    int32 joystick_index = 0;
    int32 throttle_axis = 0;
    int32 rudder_axis = 0;
    int32 joystick_sensitivity = 5;

    int32 mouse_sensitivity = 25;
    bool  b_mouse_invert = false;

protected:
    // UMG bindings
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* control_model_combo = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) USlider* joystick_index_slider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USlider* throttle_axis_slider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USlider* rudder_axis_slider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USlider* joystick_sensitivity_slider = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) USlider* mouse_sensitivity_slider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UCheckBox* mouse_invert_checkbox = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;

    // Tabs
    UPROPERTY(meta = (BindWidgetOptional)) UButton* vid_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* aud_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ctl_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* opt_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* mod_btn = nullptr;
};
