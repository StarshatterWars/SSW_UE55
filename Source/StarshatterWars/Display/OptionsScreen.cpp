/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.

    SUBSYSTEM:      Stars.exe
    FILE:           OptionsScreen.cpp
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UOptionsScreen

    Central Options Hub for Starshatter Wars.

    - Owns all options sub-screens (Audio/Video/Controls/Keyboard/Joystick/Game/Mods)
    - Uses a WidgetSwitcher to swap sub-dialogs
    - Tab buttons behave like radio buttons (selected tab stays highlighted)
    - Save applies settings; Cancel/ESC returns to menu
    - Uses AddUniqueDynamic for safe delegate binding

    Option B (recommended):
      - Selected tab == disabled state.
      - Style in BP:
          * Normal (unselected): background alpha = 0
          * Disabled (selected): STEEL GRAY background

=============================================================================*/

#include "OptionsScreen.h"

// UMG
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/WidgetSwitcher.h"
#include "Blueprint/UserWidget.h"

// IMPORTANT: include subwidget headers so C++ knows inheritance
#include "AudioDlg.h"
#include "VideoDlg.h"
#include "ControlOptionsDlg.h"
#include "KeyDlg.h"
#include "JoyDlg.h"
#include "GameOptionsDlg.h"
#include "ModsDlg.h"

// Menu routing
#include "MenuScreen.h"

DEFINE_LOG_CATEGORY_STATIC(LogOptionsScreen, Log, All);

UOptionsScreen::UOptionsScreen(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UOptionsScreen::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BindDelegates();
}

void UOptionsScreen::NativeConstruct()
{
    Super::NativeConstruct();

    bIsFocusable = true;
    BindDelegates();

    // Wire managers so subpages route back through OptionsScreen:
    if (AudioDlg)    AudioDlg->SetOptionsManager(this);      // your AudioDlg uses OptionsManager
    if (VideoDlg)    VideoDlg->SetOptionsManager(this);
    if (ControlsDlg) ControlsDlg->SetOptionsManager(this);
    if (KeyboardDlg) KeyboardDlg->SetOptionsManager(this);
    if (JoystickDlg) JoystickDlg->SetOptionsManager(this);
    if (GameDlg) GameDlg->SetOptionsManager(this);
    //if (ModsDlg) ModsDlg->SetOptionsManager(this);
    // JoystickDlg/GameDlg/ModsDlg can be wired when you add Manager APIs.

    // Default landing page:
    if (AudioDlg)        ShowAudDlg();
    else if (ControlsDlg)ShowCtlDlg();
    else if (GameDlg)    ShowGameDlg();
    else if (VideoDlg)   ShowVidDlg();
    else
    {
        UE_LOG(LogOptionsScreen, Warning,
            TEXT("[Options] No subwidgets bound. Check WBP variable names match (AudioDlg, ControlsDlg, etc) and are in the WidgetSwitcher."));
    }
}

void UOptionsScreen::BindDelegates()
{
    if (bDelegatesBound)
        return;

    // Tabs (alphabetical)
    if (BtnAudio)    BtnAudio->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnAudioClicked);
    if (BtnControls) BtnControls->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnControlsClicked);
    if (BtnGame)     BtnGame->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnGameClicked);
    if (BtnJoystick) BtnJoystick->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnJoystickClicked);
    if (BtnKeyboard) BtnKeyboard->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnKeyboardClicked);
    if (BtnMods)     BtnMods->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnModsClicked);
    if (BtnVideo)    BtnVideo->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnVideoClicked);

    // Bottom buttons
    if (BtnSave)   BtnSave->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnSaveClicked);
    if (BtnCancel) BtnCancel->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnCancelClicked);

    bDelegatesBound = true;
}

void UOptionsScreen::HandleCancel()
{
    // ESC routed through BaseScreen -> CancelOptions
    CancelOptions();
}

// ------------------------------------------------------------
// Tab visual state (Option B)
// ------------------------------------------------------------

void UOptionsScreen::SetActiveTab(UButton* ActiveButton)
{
    const FLinearColor SelectedColor = FLinearColor(0.45f, 0.45f, 0.5f, 1.0f); // Steel gray
    const FLinearColor NotSelectedColor = FLinearColor(1, 1, 1, 0.0f);        // Transparent

    auto SetBorder = [&](UBorder* Border, UButton* Button)
        {
            if (!Border || !Button) return;

            Border->SetBrushColor(Button == ActiveButton ? SelectedColor : NotSelectedColor);
        };

    SetBorder(BorderAudio, BtnAudio);
    SetBorder(BorderControls, BtnControls);
    SetBorder(BorderGame, BtnGame);
    SetBorder(BorderJoystick, BtnJoystick);
    SetBorder(BorderKeyboard, BtnKeyboard);
    SetBorder(BorderMods, BtnMods);
    SetBorder(BorderVideo, BtnVideo);
}

void UOptionsScreen::SwitchToWidget(UWidget* Widget)
{
    if (!OptionsSwitcher)
    {
        UE_LOG(LogOptionsScreen, Error, TEXT("[Options] OptionsSwitcher is NULL (BindWidgetOptional missing in WBP)."));
        return;
    }

    if (!Widget)
    {
        UE_LOG(LogOptionsScreen, Warning, TEXT("[Options] SwitchToWidget: Widget is NULL."));
        return;
    }

    OptionsSwitcher->SetActiveWidget(Widget);
}

// ------------------------------------------------------------
// Tab click handlers
// ------------------------------------------------------------

void UOptionsScreen::OnAudioClicked() { ShowAudDlg(); }
void UOptionsScreen::OnControlsClicked() { ShowCtlDlg(); }
void UOptionsScreen::OnGameClicked() { ShowGameDlg(); }
void UOptionsScreen::OnJoystickClicked() { ShowJoyDlg(); }
void UOptionsScreen::OnKeyboardClicked() { ShowKeyDlg(); }
void UOptionsScreen::OnModsClicked() { ShowModDlg(); }
void UOptionsScreen::OnVideoClicked() { ShowVidDlg(); }

// ------------------------------------------------------------
// Public routing
// ------------------------------------------------------------

void UOptionsScreen::ShowAudDlg()
{
    SetActiveTab(BtnAudio.Get());
    SwitchToWidget(AudioDlg.Get());
}

void UOptionsScreen::ShowCtlDlg()
{
    SetActiveTab(BtnControls.Get());
    SwitchToWidget(ControlsDlg.Get());
}

void UOptionsScreen::ShowGameDlg()
{
    SetActiveTab(BtnGame.Get());
    SwitchToWidget(GameDlg.Get());
}

void UOptionsScreen::ShowJoyDlg()
{
    SetActiveTab(BtnJoystick.Get());
    SwitchToWidget(JoystickDlg.Get());
}

void UOptionsScreen::ShowKeyDlg()
{
    SetActiveTab(BtnKeyboard.Get());
    SwitchToWidget(KeyboardDlg.Get());

    // If KeyDlg is a capture screen, ensure focus:
    if (KeyboardDlg)
    {
        KeyboardDlg->SetKeyboardFocus();
        KeyboardDlg->BeginCapture();
    }
}

void UOptionsScreen::ShowModDlg()
{
    SetActiveTab(BtnMods.Get());
    SwitchToWidget(ModsDlg.Get());
}

void UOptionsScreen::ShowVidDlg()
{
    SetActiveTab(BtnVideo.Get());
    SwitchToWidget(VideoDlg.Get());
}

// ------------------------------------------------------------
// Save / Cancel
// ------------------------------------------------------------

void UOptionsScreen::OnSaveClicked()
{
    ApplyOptions();
}

void UOptionsScreen::OnCancelClicked()
{
    CancelOptions();
}

void UOptionsScreen::ApplyOptions()
{
    UE_LOG(LogOptionsScreen, Log, TEXT("[Options] ApplyOptions()"));

    // Centralized apply pattern:
    if (AudioDlg)    AudioDlg->Apply();
    if (VideoDlg)    VideoDlg->Apply();
    if (ControlsDlg) ControlsDlg->Apply();
    // GameDlg/JoystickDlg/ModsDlg as you implement them.

    // Stay on current tab after save (legacy-friendly)
}

void UOptionsScreen::CancelOptions()
{
    UE_LOG(LogOptionsScreen, Log, TEXT("[Options] CancelOptions()"));

    // Optional revert:
    if (AudioDlg)    AudioDlg->Cancel();
    if (VideoDlg)    VideoDlg->Cancel();
    if (ControlsDlg) ControlsDlg->Cancel();

    // Return to menu (MenuScreen owns showing MenuDlg)
    if (UMenuScreen* Menu = GetMenuManager())
    {
        Menu->ReturnFromOptions();
    }
    else
    {
        UE_LOG(LogOptionsScreen, Warning, TEXT("[Options] CancelOptions: MenuManager is NULL"));
        SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UOptionsScreen::ReturnFromKeyDlg()
{
    // KeyDlg asked to return: route back to the Keyboard page and keep it highlighted
    ShowKeyDlg();
}
