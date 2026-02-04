/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         ControlOptionsDlg.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    UNREAL PORT:
    - Converted from FormWindow/AWEvent mapping to UBaseScreen (UUserWidget-derived).
    - Routed under UOptionsScreen (NOT GameScreen / MenuScreen).
    - JoyDlg + KeyDlg route through ControlOptionsDlg (child overlays).
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "ControlOptionsDlg.generated.h"

// UMG fwd:
class UButton;
class UComboBoxString;
class UListView;
class USlider;
class UTextBlock;
class UCheckBox;

// Starshatter fwd:
class Starshatter;
class Ship;

// New routing:
class UOptionsScreen;
class UJoyDlg;
class UKeyDlg;

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
    UPROPERTY(BlueprintReadOnly) int32   ActionIndex = 0;   // legacy keymap index / action id
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

    // ----------------------------------------------------------------
    // UUserWidget lifecycle
    // ----------------------------------------------------------------
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ----------------------------------------------------------------
    // Legacy dialog surface (ported)
    // ----------------------------------------------------------------
    virtual void RegisterControls();
    virtual void Show();
    virtual void ExecFrame();

    // ----------------------------------------------------------------
    // Manager (ONLY OptionsScreen)
    // ----------------------------------------------------------------
    void SetManager(UOptionsScreen* InManager) { Manager = InManager; }
    UOptionsScreen* GetManager() const { return Manager; }

    // ----------------------------------------------------------------
    // Child overlays (Joy/Key) routed through THIS dialog
    // ----------------------------------------------------------------
    void ShowJoyDlg();
    void ShowKeyDlg(int32 KeyMapIndex);

    // ----------------------------------------------------------------
    // Category buttons
    // ----------------------------------------------------------------
    UFUNCTION() void OnCategory0();
    UFUNCTION() void OnCategory1();
    UFUNCTION() void OnCategory2();
    UFUNCTION() void OnCategory3();

    // ----------------------------------------------------------------
    // Combo handlers
    // ----------------------------------------------------------------
    UFUNCTION() void OnControlModelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void OnJoySelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnJoyThrottleChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnJoyRudderChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void OnJoySensitivityChanged(float NormalizedValue);
    UFUNCTION() void OnJoyAxis(); // opens JoyDlg (child)

    UFUNCTION() void OnMouseSelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    UFUNCTION() void OnMouseSensitivityChanged(float NormalizedValue);
    UFUNCTION() void OnMouseInvertChanged(bool bIsChecked);

    // ----------------------------------------------------------------
    // Apply/Cancel buttons (route up to OptionsScreen)
    // ----------------------------------------------------------------
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // ----------------------------------------------------------------
    // Tabs (route up to OptionsScreen)
    // ----------------------------------------------------------------
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

    // ----------------------------------------------------------------
    // Legacy apply/cancel actions (used by OptionsScreen::ApplyOptions)
    // ----------------------------------------------------------------
    virtual void Apply();
    virtual void Cancel();

protected:
    void ShowCategory();
    void UpdateCategory();

    void SelectCategory(int32 InCategory);

    // List selection handling:
    UFUNCTION() void HandleCommandSelectionChanged(UObject* SelectedItem);

    // Helpers for slider integer mapping:
    static int32 SliderToInt(float Normalized, int32 MinV, int32 MaxV);
    static float IntToSlider(int32 V, int32 MinV, int32 MaxV);

protected:
    // ----------------------------------------------------------------
    // UBaseScreen overrides
    // ----------------------------------------------------------------
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

protected:
    // ----------------------------------------------------------------
    // Bound UMG controls (match FORM ids)
    // ----------------------------------------------------------------

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

protected:
    // ----------------------------------------------------------------
    // Legacy state (ported)
    // ----------------------------------------------------------------
    Starshatter* stars = nullptr;

    int32 selected_category = 0;
    int32 command_index = -1;

    int32 control_model = 0;

    int32 joy_select = 0;
    int32 joy_throttle = 0;
    int32 joy_rudder = 0;
    int32 joy_sensitivity = 0;

    int32 mouse_select = 0;
    int32 mouse_sensitivity = 0;
    int32 mouse_invert = 0;

    bool bClosed = true;

    // Double click timing (ported):
    double command_click_time_sec = 0.0;

    // Cached row list (to keep indices stable):
    UPROPERTY(Transient) TArray<TObjectPtr<UControlBindingRow>> CommandRows;

protected:
    // ----------------------------------------------------------------
    // New routing state
    // ----------------------------------------------------------------
    UPROPERTY(Transient) TObjectPtr<UOptionsScreen> Manager = nullptr;

    // Child dialogs (ONLY know about ControlOptionsDlg):
    UPROPERTY(Transient) TObjectPtr<UJoyDlg> JoyDlg = nullptr;
    UPROPERTY(Transient) TObjectPtr<UKeyDlg> KeyDlg = nullptr;
};
