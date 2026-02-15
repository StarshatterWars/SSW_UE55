/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.

    SUBSYSTEM:      Stars.exe
    FILE:           OptionsScreen.cpp
    AUTHOR:         Carlos Bott

=============================================================================*/

#include "OptionsScreen.h"

// UMG
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Blueprint/UserWidget.h"

// Input
#include "Input/Reply.h"
#include "InputCoreTypes.h"

#include "MenuScreen.h"

DEFINE_LOG_CATEGORY_STATIC(LogOptionsScreen, Log, All);

UOptionsScreen::UOptionsScreen(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsFocusable = true;
    SetDialogInputEnabled(true);
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

    // Default tab
    ShowAudDlg();

    if (TitleText) {
        TitleText->SetText(FText::FromString(TEXT("OPTIONS")));
    }

    // FORCE focus onto a real widget (prefer a button)
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetShowMouseCursor(true);
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;

        FInputModeUIOnly Mode;
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

        // Focus the first tab button if possible, otherwise the screen itself:
        if (BtnAudio)
        {
            Mode.SetWidgetToFocus(BtnAudio->TakeWidget());
        }
        else
        {
            Mode.SetWidgetToFocus(TakeWidget());
        }

        PC->SetInputMode(Mode);

        // Also set user focus (this avoids the “does not support focus” path)
        if (BtnAudio)
            BtnAudio->SetUserFocus(PC);
        else
            SetUserFocus(PC);
    }
}

void UOptionsScreen::BindDelegates()
{
    if (bDelegatesBound)
        return;

    // Tabs (alphabetical):
    if (BtnAudio)    BtnAudio->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnAudioClicked);
    if (BtnControls) BtnControls->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnControlsClicked);
    if (BtnGame)     BtnGame->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnGameClicked);
    if (BtnJoystick) BtnJoystick->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnJoystickClicked);
    if (BtnKeyboard) BtnKeyboard->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnKeyboardClicked);
    if (BtnMods)     BtnMods->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnModsClicked);
    if (BtnVideo)    BtnVideo->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnVideoClicked);

    // Bottom buttons (optional):
    if (BtnSave)   BtnSave->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnSaveClicked);
    if (BtnCancel) BtnCancel->OnClicked.AddUniqueDynamic(this, &UOptionsScreen::OnCancelClicked);

    const bool bAny =
        (BtnAudio || BtnControls || BtnGame || BtnJoystick || BtnKeyboard || BtnMods || BtnVideo || BtnSave || BtnCancel);

    if (!bAny)
    {
        UE_LOG(LogOptionsScreen, Error, TEXT("[Options] NO BUTTONS BOUND. Check WBP names (BtnAudio, BtnControls, ...) and IsVariable."));
    }

    bDelegatesBound = true;
}

void UOptionsScreen::HandleCancel()
{
    // BaseScreen ESC fallback
    CancelOptions();
}

FReply UOptionsScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    (void)InGeometry;

    const FKey Key = InKeyEvent.GetKey();

    // ESC -> return to menu
    if (Key == EKeys::Escape || Key == EKeys::Virtual_Back)
    {
        CancelOptions();
        return FReply::Handled();
    }

    // TAB cycles tabs (Shift+Tab reverses)
    if (Key == EKeys::Tab)
    {
        const bool bShift = InKeyEvent.IsShiftDown();
        FocusTabByDelta(bShift ? -1 : +1);
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ------------------------------------------------------------
// Switcher helpers (uses existing children in WBP)
// ------------------------------------------------------------

UUserWidget* UOptionsScreen::FindSwitcherChildByName(const FName& WidgetName) const
{
    if (!OptionsSwitcher)
        return nullptr;

    const int32 Count = OptionsSwitcher->GetNumWidgets();
    for (int32 i = 0; i < Count; ++i)
    {
        if (UWidget* W = OptionsSwitcher->GetWidgetAtIndex(i))
        {
            if (W->GetFName() == WidgetName)
            {
                return Cast<UUserWidget>(W);
            }
        }
    }

    return nullptr;
}

void UOptionsScreen::SwitchToNamedPage(const FName& PageWidgetName)
{
    if (!OptionsSwitcher)
        return;

    UUserWidget* Page = FindSwitcherChildByName(PageWidgetName);
    if (!Page)
    {
        UE_LOG(LogOptionsScreen, Warning, TEXT("[Options] Page '%s' not found in OptionsSwitcher. Check the child widget NAME in WBP."),
            *PageWidgetName.ToString());
        return;
    }

    OptionsSwitcher->SetActiveWidget(Page);
}

// ------------------------------------------------------------
// Tab visuals
// ------------------------------------------------------------

void UOptionsScreen::SetActiveTab(UButton* ActiveButton)
{
    // Radio behavior: selected tab disabled (lets Disabled style look “selected” if you want)
    if (BtnAudio)    BtnAudio->SetIsEnabled(BtnAudio != ActiveButton);
    if (BtnControls) BtnControls->SetIsEnabled(BtnControls != ActiveButton);
    if (BtnGame)     BtnGame->SetIsEnabled(BtnGame != ActiveButton);
    if (BtnJoystick) BtnJoystick->SetIsEnabled(BtnJoystick != ActiveButton);
    if (BtnKeyboard) BtnKeyboard->SetIsEnabled(BtnKeyboard != ActiveButton);
    if (BtnMods)     BtnMods->SetIsEnabled(BtnMods != ActiveButton);
    if (BtnVideo)    BtnVideo->SetIsEnabled(BtnVideo != ActiveButton);

    // OPTION B: border highlight
    UpdateTabBorders(ActiveButton);
}

void UOptionsScreen::SetBorderSelected(UBorder* Border, bool bSelected)
{
    if (!Border)
        return;

    // Selected: steel gray visible
    // Unselected: alpha = 0 (invisible)
    const FLinearColor SelectedColor(0.45f, 0.48f, 0.52f, 1.0f);
    const FLinearColor UnselectedColor(0.45f, 0.48f, 0.52f, 0.0f);

    Border->SetBrushColor(bSelected ? SelectedColor : UnselectedColor);
}

void UOptionsScreen::UpdateTabBorders(UButton* ActiveButton)
{
    SetBorderSelected(BorderAudio, ActiveButton == BtnAudio);
    SetBorderSelected(BorderControls, ActiveButton == BtnControls);
    SetBorderSelected(BorderGame, ActiveButton == BtnGame);
    SetBorderSelected(BorderJoystick, ActiveButton == BtnJoystick);
    SetBorderSelected(BorderKeyboard, ActiveButton == BtnKeyboard);
    SetBorderSelected(BorderMods, ActiveButton == BtnMods);
    SetBorderSelected(BorderVideo, ActiveButton == BtnVideo);
}

// ------------------------------------------------------------
// TAB navigation
// ------------------------------------------------------------

void UOptionsScreen::FocusTabByDelta(int32 Delta)
{
    // Order must match your “alphabetical” requirement:
    UButton* Tabs[] = { BtnAudio, BtnControls, BtnGame, BtnJoystick, BtnKeyboard, BtnMods, BtnVideo };

    // Find current focus index (fallback to selected tab / active widget)
    int32 Current = -1;

    // Prefer actual keyboard focus:
    for (int32 i = 0; i < UE_ARRAY_COUNT(Tabs); ++i)
    {
        if (Tabs[i] && Tabs[i]->HasKeyboardFocus())
        {
            Current = i;
            break;
        }
    }

    // If none focused, infer from which tab is currently “selected” (disabled)
    if (Current < 0)
    {
        for (int32 i = 0; i < UE_ARRAY_COUNT(Tabs); ++i)
        {
            if (Tabs[i] && !Tabs[i]->GetIsEnabled())
            {
                Current = i;
                break;
            }
        }
    }

    if (Current < 0)
        Current = 0;

    // Step until we find a non-null tab
    int32 Next = Current;
    for (int32 step = 0; step < UE_ARRAY_COUNT(Tabs); ++step)
    {
        Next = (Next + Delta + UE_ARRAY_COUNT(Tabs)) % UE_ARRAY_COUNT(Tabs);
        if (Tabs[Next])
        {
            Tabs[Next]->SetKeyboardFocus();
            return;
        }
    }
}

// ------------------------------------------------------------
// Click handlers
// ------------------------------------------------------------

void UOptionsScreen::OnAudioClicked() { ShowAudDlg(); }
void UOptionsScreen::OnControlsClicked() { ShowCtlDlg(); }
void UOptionsScreen::OnGameClicked() { ShowGameDlg(); }
void UOptionsScreen::OnJoystickClicked() { ShowJoyDlg(); }
void UOptionsScreen::OnKeyboardClicked() { ShowKeyDlg(); }
void UOptionsScreen::OnModsClicked() { ShowModDlg(); }
void UOptionsScreen::OnVideoClicked() { ShowVidDlg(); }

void UOptionsScreen::OnSaveClicked() { ApplyOptions(); }
void UOptionsScreen::OnCancelClicked() { CancelOptions(); }

// ------------------------------------------------------------
// Public routing (uses WBP switcher child NAMES)
// ------------------------------------------------------------
// THESE MUST MATCH THE CHILD WIDGET NAMES INSIDE YOUR SWITCHER.
// If your switcher children are named differently, change only these strings.

void UOptionsScreen::ShowAudDlg()
{
    SetActiveTab(BtnAudio);
    SwitchToNamedPage(TEXT("AudioDlg"));

    if (APlayerController* PC = GetOwningPlayer())
        if (BtnAudio) BtnAudio->SetUserFocus(PC);
}

void UOptionsScreen::ShowCtlDlg()
{
    SetActiveTab(BtnControls);
    SwitchToNamedPage(TEXT("ControlsDlg"));

    if (APlayerController* PC = GetOwningPlayer())
        if (BtnAudio) BtnAudio->SetUserFocus(PC);
}

void UOptionsScreen::ShowGameDlg()
{
    SetActiveTab(BtnGame);
    SwitchToNamedPage(TEXT("GameDlg"));

    if (APlayerController* PC = GetOwningPlayer())
        if (BtnAudio) BtnAudio->SetUserFocus(PC);
}

void UOptionsScreen::ShowJoyDlg()
{
    SetActiveTab(BtnJoystick);
    SwitchToNamedPage(TEXT("JoystickDlg"));

    if (APlayerController* PC = GetOwningPlayer())
        if (BtnAudio) BtnAudio->SetUserFocus(PC);
}

void UOptionsScreen::ShowKeyDlg()
{
    SetActiveTab(BtnKeyboard);
    SwitchToNamedPage(TEXT("KeyboardDlg"));

    if (APlayerController* PC = GetOwningPlayer())
        if (BtnAudio) BtnAudio->SetUserFocus(PC);
}

void UOptionsScreen::ShowModDlg()
{
    SetActiveTab(BtnMods);
    SwitchToNamedPage(TEXT("ModsDlg"));

    if (APlayerController* PC = GetOwningPlayer())
        if (BtnAudio) BtnAudio->SetUserFocus(PC);
}

void UOptionsScreen::ShowVidDlg()
{
    SetActiveTab(BtnVideo);
    SwitchToNamedPage(TEXT("VideoDlg"));

    if (APlayerController* PC = GetOwningPlayer())
        if (BtnAudio) BtnAudio->SetUserFocus(PC);
}

// ------------------------------------------------------------
// Apply / Cancel orchestration
// ------------------------------------------------------------

void UOptionsScreen::ApplyOptions()
{
    UE_LOG(LogOptionsScreen, Log, TEXT("[Options] ApplyOptions()"));

    // Keep this “dumb” and stable:
    // Each page applies itself (AudioDlg::Apply(), VideoDlg::Apply(), etc.)
    // If you want automatic apply on tab-switch later, we’ll call Apply on the page being left.

    // If you have direct pointers in BP you can call them here, but DO NOT CreateWidget them.
    // Safer: look at active widget and call Apply if it’s a BaseScreen-derived page.

    if (!OptionsSwitcher)
        return;

    if (UWidget* Active = OptionsSwitcher->GetActiveWidget())
    {
        if (UBaseScreen* Page = Cast<UBaseScreen>(Active))
        {
            // If the page exposes Apply() as a method, you can call a known function here.
            // For now, keep consistent with your existing dialogs:
            // - AudioDlg/VideoDlg/ControlsDlg already route Apply via Manager->ApplyOptions()
            // so calling ApplyOptions() is enough and they’ll push their models when they choose.
        }
    }

    // If you *do* still have a Save button, this is where it “commits everything”.
    // You already have AudioDlg/VideoDlg writing to config + SaveGame, so this can remain minimal.
}

void UOptionsScreen::CancelOptions()
{
    UE_LOG(LogOptionsScreen, Log, TEXT("[Options] CancelOptions()"));

    // Return to menu
    if (MenuManager)
    {
        MenuManager->ReturnFromOptions();
    }
    else
    {
        UE_LOG(LogOptionsScreen, Warning, TEXT("[Options] CancelOptions: MenuManager is NULL (ExposeOnSpawn?)"));
        RemoveFromParent(); // safe fallback so you don’t get stuck
    }
}

void UOptionsScreen::ReturnFromKeyDlg()
{
    // KeyDlg should just call back here (as you requested).
    // We return to Keyboard tab/page by default, or Controls if you prefer.
    ShowKeyDlg();
    SetKeyboardFocus();
}
