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
      - Owns all options sub-screens (already placed in WBP)
      - Uses a WidgetSwitcher to swap sub-dialogs
      - Manages tab buttons (radio-style behavior + highlight)
      - Handles Apply / Cancel routing
      - ESC returns to MenuScreen
      - TAB / SHIFT+TAB cycles tabs

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
class UTextBlock;
class UWidgetSwitcher;
class UUserWidget;
class UMenuScreen;

UCLASS()
class STARSHATTERWARS_API UOptionsScreen : public UBaseScreen
{
    GENERATED_BODY()

public:
    UOptionsScreen(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
    // BaseScreen ESC fallback:
    virtual void HandleCancel() override;

    // Public API used by subdialogs:
    void ApplyOptions();
    void CancelOptions();

    // KeyDlg returns here:
    void ReturnFromKeyDlg();

    // Legacy-compatible routing names:
    void ShowAudDlg();
    void ShowCtlDlg();
    void ShowGameDlg();
    void ShowJoyDlg();
    void ShowKeyDlg();
    void ShowModDlg();
    void ShowVidDlg();

protected:
    // ------------------------------------------------------------
    // Core container (must exist in WBP)
    // ------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UWidgetSwitcher> OptionsSwitcher = nullptr;

    // ------------------------------------------------------------
    // Tab Buttons (alphabetical)
    // ------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnAudio = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnControls = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnGame = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnJoystick = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnKeyboard = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnMods = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnVideo = nullptr;

    // Optional bottom buttons (if you still have them in WBP)
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnSave = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> BtnCancel = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TitleText = nullptr;

    // ------------------------------------------------------------
    // Tab highlight borders (OPTION B)
    // Add these Borders in WBP wrapping each tab button.
    // Name them exactly: BorderAudio, BorderControls, etc.
    // ------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> BorderAudio = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> BorderControls = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> BorderGame = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> BorderJoystick = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> BorderKeyboard = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> BorderMods = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> BorderVideo = nullptr;

private:
    // ------------------------------------------------------------
    // Delegate binding
    // ------------------------------------------------------------
    void BindDelegates();
    bool bDelegatesBound = false;

    // ------------------------------------------------------------
    // Switcher helpers (CRITICAL FIX)
    // We do NOT CreateWidget pages. We use the instances already in the WBP switcher.
    // ------------------------------------------------------------
    UUserWidget* FindSwitcherChildByName(const FName& WidgetName) const;
    void SwitchToNamedPage(const FName& PageWidgetName);

    void SetActiveTab(UButton* ActiveButton);

    // OPTION B: border highlight (steel gray when selected, alpha 0 when not)
    void UpdateTabBorders(UButton* ActiveButton);
    static void SetBorderSelected(UBorder* Border, bool bSelected);

    // TAB navigation
    void FocusTabByDelta(int32 Delta);

private:
    // Button handlers:
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnGameClicked();
    UFUNCTION() void OnJoystickClicked();
    UFUNCTION() void OnKeyboardClicked();
    UFUNCTION() void OnModsClicked();
    UFUNCTION() void OnVideoClicked();

    UFUNCTION() void OnSaveClicked();
    UFUNCTION() void OnCancelClicked();
};
