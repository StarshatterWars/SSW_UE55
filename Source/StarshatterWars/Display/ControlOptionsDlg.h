/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         ControlOptionsDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    UControlOptionsDlg (Controls Options Dialog)
    - Refactored to the NEW settings pipeline:
        * UI reads/writes ONLY UStarshatterControlsSettings (config-backed CDO)
        * Runtime apply is delegated to UStarshatterControlsSubsystem
    - Legacy KeyMap is used READ-ONLY for:
        * DescribeAction(i)
        * DescribeKey(i)
        * GetCategory(i)
      (So the UI list remains consistent with legacy naming during migration.)

    IMPORTANT
    =========
    - No direct calls to KeyMap::Bind(), SaveKeyMap(), stars->MapKeys(), or Ship::SetControlModel()
      from the dialog anymore. Those are the subsystem’s job.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "ControlOptionsDlg.generated.h"

// UMG:
class UButton;
class UComboBoxString;
class UListView;
class USlider;
class UCheckBox;

// Starshatter (read-only legacy describe/category):
class Starshatter;
class KeyMap;

// Router:
class UOptionsScreen;

// Overlays:
class UJoyDlg;
class UKeyDlg;

// NEW:
class UStarshatterControlsSubsystem;
class UStarshatterControlsSettings;

// ------------------------------------------------------------
// Row object for the two-column command list
// ------------------------------------------------------------

UCLASS()
class STARSHATTERWARS_API UControlBindingRow : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly) FString Command;
    UPROPERTY(BlueprintReadOnly) FString Key;
    UPROPERTY(BlueprintReadOnly) int32   ActionIndex = 0; // legacy keymap action index
};

// ------------------------------------------------------------
// Control Options Dialog
// ------------------------------------------------------------

UCLASS()
class STARSHATTERWARS_API UControlOptionsDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UControlOptionsDlg(const FObjectInitializer& ObjectInitializer);

    // Host/router (ONLY OptionsScreen):
    void SetManager(UOptionsScreen* InManager) { Manager = InManager; }
    UOptionsScreen* GetManager() const { return Manager; }

    // Surface used by OptionsScreen:
    virtual void Show();
    virtual void ExecFrame();
    virtual void Apply();
    virtual void Cancel();

    // Child overlays:
    void ShowJoyDlg();
    void ShowKeyDlg(int32 ActionIndex);

protected:
    // UUserWidget:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // UBaseScreen:
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;
    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

private:
    // NEW: accessors
    UStarshatterControlsSubsystem* GetControlsSubsystem() const;
    UStarshatterControlsSettings* GetControlsSettings() const;

    // Legacy read-only helper:
    KeyMap* GetLegacyKeyMap() const;

    // Model <-> UI
    void RefreshFromModel();
    void PushToModel(bool bApplyRuntimeToo);

    // Category + list rebuild
    void SelectCategory(int32 InCategory);
    void RebuildCommandList();
    void RefreshCommandKeysOnly();

    // Describe helper (read-only legacy; safe during migration)
    FString DescribeAction(int32 ActionIndex) const;
    FString DescribeKeyFromAction(int32 ActionIndex) const;

    // Slider mapping:
    static int32 SliderToInt(float Normalized, int32 MinV, int32 MaxV);
    static float IntToSlider(int32 V, int32 MinV, int32 MaxV);

private:
    // Category buttons
    UFUNCTION() void OnCategory0();
    UFUNCTION() void OnCategory1();
    UFUNCTION() void OnCategory2();
    UFUNCTION() void OnCategory3();

    // Combos / sliders / checkbox
    UFUNCTION() void OnControlModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void OnJoySelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnJoyThrottleChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnJoyRudderChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void OnJoySensitivityChanged(float NormalizedValue);
    UFUNCTION() void OnJoyAxis();

    UFUNCTION() void OnMouseSelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnMouseSensitivityChanged(float NormalizedValue);
    UFUNCTION() void OnMouseInvertChanged(bool bIsChecked);

    // Apply/Cancel
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // Tabs
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

    // List selection
    UFUNCTION() void HandleCommandSelectionChanged(UObject* SelectedItem);

private:
    // Router:
    UPROPERTY(Transient) TObjectPtr<UOptionsScreen> Manager = nullptr;

    // Overlays:
    UPROPERTY(Transient) TObjectPtr<UJoyDlg> JoyDlg = nullptr;
    UPROPERTY(Transient) TObjectPtr<UKeyDlg> KeyDlg = nullptr;

    // Legacy read-only:
    Starshatter* stars = nullptr;

    // State:
    bool bClosed = true;

    int32 selected_category = 0;
    int32 command_index = -1;
    double command_click_time_sec = 0.0;

    // Cached rows:
    UPROPERTY(Transient) TArray<TObjectPtr<UControlBindingRow>> CommandRows;

    // Editable settings snapshot (mirrors ControlsSettings):
    int32 control_model = 0;

    int32 joy_select = 0;
    int32 joy_throttle = 0;
    int32 joy_rudder = 0;
    int32 joy_sensitivity = 0;

    int32 mouse_select = 0;
    int32 mouse_sensitivity = 0;
    int32 mouse_invert = 0;

protected:
    // ------------------------------------------------------------
    // UMG widget bindings (OPTIONAL) — must match UMG widget names
    // ------------------------------------------------------------

    // Category buttons (101-104):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* category_0 = nullptr; // 101 Flight
    UPROPERTY(meta = (BindWidgetOptional)) UButton* category_1 = nullptr; // 102 Wep
    UPROPERTY(meta = (BindWidgetOptional)) UButton* category_2 = nullptr; // 103 View
    UPROPERTY(meta = (BindWidgetOptional)) UButton* category_3 = nullptr; // 104 Misc

    // Commands list (legacy ListBox id 200):
    UPROPERTY(meta = (BindWidgetOptional)) UListView* commands = nullptr; // 200

    // Combos:
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* control_model_combo = nullptr; // 210
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* joy_select_combo = nullptr;    // 211
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* joy_throttle_combo = nullptr;  // 212
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* joy_rudder_combo = nullptr;    // 213
    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* mouse_select_combo = nullptr;  // 511

    // Sliders:
    UPROPERTY(meta = (BindWidgetOptional)) USlider* joy_sensitivity_slider = nullptr;   // 214 (0..10)
    UPROPERTY(meta = (BindWidgetOptional)) USlider* mouse_sensitivity_slider = nullptr; // 514 (0..50)

    // Buttons:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* joy_axis_button = nullptr; // 215 Setup...
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;        // 1
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;       // 2

    // Tabs (901-905):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* vid_btn = nullptr; // 901
    UPROPERTY(meta = (BindWidgetOptional)) UButton* aud_btn = nullptr; // 902
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ctl_btn = nullptr; // 903
    UPROPERTY(meta = (BindWidgetOptional)) UButton* opt_btn = nullptr; // 904
    UPROPERTY(meta = (BindWidgetOptional)) UButton* mod_btn = nullptr; // 905

    // Mouse invert checkbox (515):
    UPROPERTY(meta = (BindWidgetOptional)) UCheckBox* mouse_invert_checkbox = nullptr; // 515
};
