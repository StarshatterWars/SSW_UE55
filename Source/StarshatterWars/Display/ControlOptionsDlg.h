/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:      StarshatterWars (Unreal Engine)
    FILE:           ControlOptionsDlg.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UControlOptionsDlg
    - UE-only controls options dialog (OptionsScreen subpage).
    - UI reads/writes ONLY UStarshatterControlsSettings (config-backed CDO).
    - Runtime apply delegated to UStarshatterControlsSettings::ApplyToRuntimeControls(...)

    NOTES
    =====
    - Uses AddUniqueDynamic bindings (no RemoveAll) to prevent delegate ensure crashes.
    - Routes tabs through UOptionsScreen (Audio/Video/Game/Controls/Mods).
=============================================================================*/

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

    // Standardized across subscreens:
    void SetOptionsManager(UOptionsScreen* InManager) { OptionsManager = InManager; }
    UOptionsScreen* GetOptionsManager() const { return OptionsManager.Get(); }

    virtual void Show();
    virtual void ExecFrame(double DeltaTime) override;

    void Apply();
    void Cancel();

    bool IsDirty() const { return bDirty; }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;
    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

private:
    void BindDelegates();

    UStarshatterControlsSettings* GetControlsSettings() const;

    void RefreshFromModel();
    void PushToModel(bool bApplyRuntimeToo);

    static int32 SliderToInt(float Normalized, int32 MinV, int32 MaxV);
    static float IntToSlider(int32 V, int32 MinV, int32 MaxV);

private:
    // Model change handlers
    UFUNCTION() void OnControlModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void OnJoystickIndexChanged(float NormalizedValue);
    UFUNCTION() void OnThrottleAxisChanged(float NormalizedValue);
    UFUNCTION() void OnRudderAxisChanged(float NormalizedValue);
    UFUNCTION() void OnJoystickSensitivityChanged(float NormalizedValue);

    UFUNCTION() void OnMouseSensitivityChanged(float NormalizedValue);
    UFUNCTION() void OnMouseInvertChanged(bool bIsChecked);

    // Apply/Cancel
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // Tabs
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnGameClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

private:

    bool bClosed = true;
    bool bDelegatesBound = false;

    UPROPERTY(Transient)
    bool bDirty = false;

    // Snapshot (mirrors FStarshatterControlsConfig)
    int32 control_model = 1; // FlightSim default

    int32 joystick_index = 0;
    int32 throttle_axis = 0;
    int32 rudder_axis = 0;
    int32 joystick_sensitivity = 5;

    int32 mouse_sensitivity = 25;
    bool  b_mouse_invert = false;

protected:
    // ------------------------------------------------------------
    // UMG bindings (names must match BP variables)
    // ------------------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UComboBoxString> control_model_combo = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> joystick_index_slider = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> throttle_axis_slider = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> rudder_axis_slider = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> joystick_sensitivity_slider = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<USlider> mouse_sensitivity_slider = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> mouse_invert_checkbox = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ApplyBtn = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> CancelBtn = nullptr;

    // Tabs (optional if embedded in subpage BP)
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> vid_btn = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> aud_btn = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ctl_btn = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> opt_btn = nullptr;  // "GAME" tab in new layout

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> mod_btn = nullptr;
};
