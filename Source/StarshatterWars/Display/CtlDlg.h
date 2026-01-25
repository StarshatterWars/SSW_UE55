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
    Control Options (keyboard/joystick) dialog Unreal UUserWidget implementation.
    - Manager is GameScreen (non-UObject).
    - Uses UBaseScreen FORM-ID binding model for control mapping.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

// UMG:
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"

#include "CtlDlg.generated.h"

// Forward declaration (NON-UObject manager):
class GameScreen;

UCLASS()
class STARSHATTERWARS_API UCtlDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UCtlDlg(const FObjectInitializer& ObjectInitializer);

    /** Non-UObject manager setter */
    void SetManager(GameScreen* InManager) { manager = InManager; }

    // ---- UBaseScreen overrides --------------------------------------
    virtual void BindFormWidgets() override;

    // Optional: provide raw legacy .frm if you want BaseScreen to parse defaults
    virtual FString GetLegacyFormText() const override { return FString(); }

    // ---- Operations --------------------------------------------------
    void Apply();
    void Cancel();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
    // -----------------------------------------------------------------
    // Manager (non-UObject)
    // -----------------------------------------------------------------
    GameScreen* manager = nullptr;

    // -----------------------------------------------------------------
    // Widgets (BindWidgetOptional) – match your UMG widget names
    // -----------------------------------------------------------------

    // Category buttons (MenuDlg.frm: 101-104):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* category_101 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* category_102 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* category_103 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* category_104 = nullptr;

    // Commands list (MenuDlg.frm: 200):
    UPROPERTY(meta = (BindWidgetOptional)) UListView* commands_list = nullptr;

    // Combos:
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* control_model_combo = nullptr; // 210
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* joy_select_combo = nullptr;    // 211
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* joy_throttle_combo = nullptr;  // 212
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* joy_rudder_combo = nullptr;    // 213
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* mouse_select_combo = nullptr;  // 511

    // Sliders:
    UPROPERTY(meta = (BindWidgetOptional)) USlider* joy_sensitivity_slider = nullptr;      // 214
    UPROPERTY(meta = (BindWidgetOptional)) USlider* mouse_sensitivity_slider = nullptr;    // 514

    // Buttons:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* joy_axis_button = nullptr;             // 215
    UPROPERTY(meta = (BindWidgetOptional)) UButton* mouse_invert_checkbox = nullptr;       // 515

    // Tabs (MenuDlg.frm: 901-905):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* vid_btn = nullptr; // 901
    UPROPERTY(meta = (BindWidgetOptional)) UButton* aud_btn = nullptr; // 902
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ctl_btn = nullptr; // 903
    UPROPERTY(meta = (BindWidgetOptional)) UButton* opt_btn = nullptr; // 904
    UPROPERTY(meta = (BindWidgetOptional)) UButton* mod_btn = nullptr; // 905

    // Apply/Cancel (MenuDlg.frm: 1,2):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;  // 1
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr; // 2

protected:
    // -----------------------------------------------------------------
    // Legacy state mirrors (CtlDlg.cpp)
    // -----------------------------------------------------------------
    int32 selected_category = 0;
    int32 command_index = 0;

    int32 control_model = 0;

    int32 joy_select = 0;
    int32 joy_throttle = 0;
    int32 joy_rudder = 0;
    int32 joy_sensitivity = 0;

    int32 mouse_select = 0;
    int32 mouse_sensitivity = 0;
    bool  mouse_invert = false;

    bool  closed = true;

protected:
    // -----------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------
    void ShowCategory();
    void UpdateCategory();

protected:
    // -----------------------------------------------------------------
    // UMG handlers
    // -----------------------------------------------------------------
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

    UFUNCTION() void OnControlModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnJoySelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnJoyThrottleChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnJoyRudderChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnMouseSelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void OnJoySensitivityChanged(float Value);
    UFUNCTION() void OnMouseSensitivityChanged(float Value);

    UFUNCTION() void OnJoyAxisClicked();
    UFUNCTION() void OnMouseInvertClicked();

    UFUNCTION() void OnCommandSelected(UObject* Item);
};
