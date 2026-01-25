/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         CtlDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Control Options (keyboard/joystick) Dialog (Unreal UUserWidget)
*/

#pragma once

// Minimal Unreal includes required by project conventions:
#include "Math/Vector.h"                // FVector
#include "Math/Color.h"                 // FColor
#include "Math/UnrealMathUtility.h"     // Math

#include "BaseScreen.h"
#include "CtlDlg.generated.h"

// Forward declarations (keep header light):
class UButton;
class UComboBoxString;
class USlider;
class UListView;
class UTextBlock;
class UWidget;

UCLASS()
class STARSHATTERWARS_API UCtlDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UCtlDlg(const FObjectInitializer& ObjectInitializer);

    // UUserWidget overrides:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // Always focusable so Enter/Escape can work if you later add key handling:
    virtual bool IsInteractable() const override { return true; }
    virtual bool SupportsKeyboardFocus() const override { return true; }

public:
    // Operations (mirrors legacy Apply/Cancel):
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Apply();

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Cancel();

protected:
    // Internal helpers (legacy ShowCategory/UpdateCategory):
    void ShowCategory();
    void UpdateCategory();

    void OnCommandClicked(UObject* Item);
    void OnCommandSelectionChanged(UObject* Item);

protected:
    // Category buttons (4):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* Category0Btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* Category1Btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* Category2Btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* Category3Btn = nullptr;

    // Command list (legacy ListBox* commands):
    // If you want the exact old behavior (index + text), a UListView with entries is recommended.
    // For now, keep it as a UListView binding point.
    UPROPERTY(meta = (BindWidgetOptional)) UListView* CommandsList = nullptr;

    int command_index = -1;

    // Control model:
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* ControlModelCombo = nullptr;

    // Joystick:
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* JoySelectCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* JoyThrottleCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* JoyRudderCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USlider* JoySensitivitySlider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* JoyAxisBtn = nullptr;

    // Mouse:
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* MouseSelectCombo = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USlider* MouseSensitivitySlider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* MouseInvertCheckbox = nullptr;

    // Tabs / navigation:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* aud_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* vid_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* opt_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ctl_btn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* mod_btn = nullptr;

    // Action buttons (per your convention):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;

protected:
    // UI event handlers (mirrors legacy callbacks):
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

    UFUNCTION() void OnCategory0Clicked();
    UFUNCTION() void OnCategory1Clicked();
    UFUNCTION() void OnCategory2Clicked();
    UFUNCTION() void OnCategory3Clicked();

    // Combo/slider changed handlers:
    UFUNCTION() void OnControlModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void OnJoySelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnJoyThrottleChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnJoyRudderChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnJoySensitivityChanged(float Value);
    UFUNCTION() void OnJoyAxisClicked();

    UFUNCTION() void OnMouseSelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnMouseSensitivityChanged(float Value);
    UFUNCTION() void OnMouseInvertClicked();

protected:
    // State (mirrors old ints):
    int selected_category = 0;
    int control_model = 0;

    int joy_select = 0;
    int joy_throttle = 0;
    int joy_rudder = 0;
    int joy_sensitivity = 0;

    int mouse_select = 0;
    int mouse_sensitivity = 0;
    int mouse_invert = 0;

    bool closed = true;
};
