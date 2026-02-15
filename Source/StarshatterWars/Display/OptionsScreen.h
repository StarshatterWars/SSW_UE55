/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.

    SUBSYSTEM:      Stars.exe
    FILE:           OptionsScreen.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UOptionsScreen

    Central Options Hub for Starshatter Wars.

    Responsibilities:
      - Owns all options sub-screens
      - Uses a WidgetSwitcher to swap sub-dialogs
      - Manages tab buttons (radio-style behavior)
      - Handles Save / Cancel
      - ESC returns to MenuScreen
      - No routing logic lives in MenuDlg anymore

    Tabs (alphabetical order):
      - Audio
      - Controls
      - Game
      - Joystick
      - Keyboard
      - Mods
      - Video

=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "OptionsScreen.generated.h"

class UButton;
class UBorder;
class UWidget;
class UWidgetSwitcher;

// Sub-widgets
class UAudioDlg;
class UVideoDlg;
class UControlOptionsDlg;
class UKeyDlg;
class UJoyDlg;
class UGameOptionsDlg;
class UModsDlg;

UCLASS()
class STARSHATTERWARS_API UOptionsScreen : public UBaseScreen
{
    GENERATED_BODY()

public:
    UOptionsScreen(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

public:
    // ESC should behave like Cancel (return to menu)
    virtual void HandleCancel() override;

    // Apply / Cancel orchestration
    void ApplyOptions();
    void CancelOptions();

    // KeyDlg returns here (KeyDlg does NOT route itself)
    void ReturnFromKeyDlg();

    // Sub-screen routing (legacy compatibility)
    void ShowAudDlg();
    void ShowCtlDlg();
    void ShowGameDlg();
    void ShowJoyDlg();
    void ShowKeyDlg();
    void ShowModDlg();
    void ShowVidDlg();

protected:
    // ------------------------------------------------------------
    // Core container
    // ------------------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UWidgetSwitcher> OptionsSwitcher;

    // ------------------------------------------------------------
    // Tab Buttons (alphabetical)
    // ------------------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnAudio;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnControls;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnGame;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnJoystick;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnKeyboard;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnMods;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnVideo;

    // ------------------------------------------------------------
    // Bottom Buttons
    // ------------------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnSave;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnCancel;

    // ------------------------------------------------------------
    // Sub Widgets (must be in the WidgetSwitcher and named exactly)
    // ------------------------------------------------------------

    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UAudioDlg>         AudioDlg;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UControlOptionsDlg> ControlsDlg;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UGameOptionsDlg>   GameDlg;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UJoyDlg>           JoystickDlg;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UKeyDlg>           KeyboardDlg;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UModsDlg>          ModsDlg;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UVideoDlg>         VideoDlg;

    // Borders (for highlight)
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> BorderAudio;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> BorderControls;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> BorderGame;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> BorderJoystick;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> BorderKeyboard;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> BorderMods;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UBorder> BorderVideo;

protected:
    void BindDelegates();
    bool bDelegatesBound = false;

    // Option B: selected tab = disabled state (BP Disabled style = STEEL GRAY)
    void SetActiveTab(UButton* ActiveButton);

    // FIX: take UWidget* (WidgetSwitcher uses UWidget)
    void SwitchToWidget(UWidget* Widget);

protected:
    // Tab handlers
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnGameClicked();
    UFUNCTION() void OnJoystickClicked();
    UFUNCTION() void OnKeyboardClicked();
    UFUNCTION() void OnModsClicked();
    UFUNCTION() void OnVideoClicked();

    // Bottom handlers
    UFUNCTION() void OnSaveClicked();
    UFUNCTION() void OnCancelClicked();
};
