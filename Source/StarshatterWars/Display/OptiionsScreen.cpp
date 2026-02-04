#include "OptionsScreen.h"

// UE:
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"

// Input:
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Subdialogs:
#include "AudioDlg.h"
#include "VideoDlg.h"
#include "ControlOptionsDlg.h"

// Starshatter core:
#include "Ship.h"
#include "HUDView.h"
#include "PlayerCharacter.h"
#include "Starshatter.h"

#include "StarshatterAudioSubsystem.h"
#include "StarshatterVideoSubsystem.h"
#include "Engine/GameInstance.h"

UOptionsScreen::UOptionsScreen(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bClosed = true;
    SetDialogInputEnabled(true);
}

void UOptionsScreen::NativeConstruct()
{
    Super::NativeConstruct();

    // Create children once:
    EnsureSubDialogs();

    // Apply/Cancel
    if (ApplyBtn)
    {
        ApplyBtn->OnClicked.RemoveAll(this);
        ApplyBtn->OnClicked.AddDynamic(this, &UOptionsScreen::OnApplyClicked);
    }
    if (CancelBtn)
    {
        CancelBtn->OnClicked.RemoveAll(this);
        CancelBtn->OnClicked.AddDynamic(this, &UOptionsScreen::OnCancelClicked);
    }

    // Tabs
    if (vid_btn)
    {
        vid_btn->OnClicked.RemoveAll(this);
        vid_btn->OnClicked.AddDynamic(this, &UOptionsScreen::OnVideoClicked);
    }
    if (aud_btn)
    {
        aud_btn->OnClicked.RemoveAll(this);
        aud_btn->OnClicked.AddDynamic(this, &UOptionsScreen::OnAudioClicked);
    }
    if (ctl_btn)
    {
        ctl_btn->OnClicked.RemoveAll(this);
        ctl_btn->OnClicked.AddDynamic(this, &UOptionsScreen::OnControlsClicked);
    }
    if (opt_btn)
    {
        opt_btn->OnClicked.RemoveAll(this);
        opt_btn->OnClicked.AddDynamic(this, &UOptionsScreen::OnOptionsClicked);
    }
    if (mod_btn)
    {
        mod_btn->OnClicked.RemoveAll(this);
        mod_btn->OnClicked.AddDynamic(this, &UOptionsScreen::OnModClicked);
    }

    // Combo handlers (optional)
    if (flight_model)
        flight_model->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnFlightModelChanged);
    if (flying_start)
        flying_start->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnFlyingStartChanged);
    if (landings)
        landings->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnLandingsChanged);
    if (ai_difficulty)
        ai_difficulty->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnAIDifficultyChanged);
    if (hud_mode)
        hud_mode->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnHudModeChanged);
    if (hud_color)
        hud_color->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnHudColorChanged);
    if (ff_mode)
        ff_mode->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnFfModeChanged);
    if (grid_mode)
        grid_mode->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnGridModeChanged);
    if (gunsight)
        gunsight->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnGunsightChanged);

    // Default to options page
    ShowOptDlg();
}

void UOptionsScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

FReply UOptionsScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
    {
        OnApplyClicked();
        return FReply::Handled();
    }

    if (Key == EKeys::Escape || Key == EKeys::Virtual_Back)
    {
        OnCancelClicked();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ---------------------------------------------------------------------
// Internal helpers (these are the missing symbols from your errors)
// ---------------------------------------------------------------------

void UOptionsScreen::EnsureSubDialogs()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    if (!AudDlg && AudioDlgClass)
    {
        AudDlg = CreateWidget<UAudioDlg>(World, AudioDlgClass);
        if (AudDlg)
        {
            AudDlg->AddToViewport(0);
            AudDlg->SetVisibility(ESlateVisibility::Collapsed);
            AudDlg->SetManager(this); // AudioDlg manager should be UOptionsScreen
        }
    }

    if (!VidDlg && VideoDlgClass)
    {
        VidDlg = CreateWidget<UVideoDlg>(World, VideoDlgClass);
        if (VidDlg)
        {
            VidDlg->AddToViewport(0);
            VidDlg->SetVisibility(ESlateVisibility::Collapsed);
            VidDlg->SetManager(this);
        }
    }

    if (!CtlDlg && ControlDlgClass)
    {
        CtlDlg = CreateWidget<UControlOptionsDlg>(World, ControlDlgClass);
        if (CtlDlg)
        {
            CtlDlg->AddToViewport(0);
            CtlDlg->SetVisibility(ESlateVisibility::Collapsed);

            // ControlOptionsDlg manager should be UMenuScreen (for JoyDlg/KeyDlg routing),
            // but it also needs to be able to return to OptionsScreen if you built it that way.
            // For now: don’t set anything here unless your ControlOptionsDlg has a setter.
        }
    }
}

void UOptionsScreen::HideAllPages()
{
    if (AudDlg) AudDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (VidDlg) VidDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (CtlDlg) CtlDlg->SetVisibility(ESlateVisibility::Collapsed);

    // Show this page by default when “not hidden”:
    SetVisibility(ESlateVisibility::Collapsed);
}

// ---------------------------------------------------------------------
// Page routing
// ---------------------------------------------------------------------

void UOptionsScreen::ShowOptDlg()
{
    EnsureSubDialogs();

    if (AudDlg) AudDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (VidDlg) VidDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (CtlDlg) CtlDlg->SetVisibility(ESlateVisibility::Collapsed);

    SetVisibility(ESlateVisibility::Visible);
    SetDialogInputEnabled(true);

    if (bClosed)
    {
        if (flight_model)  flight_model->SetSelectedIndex(Ship::GetFlightModel());
        if (landings)      landings->SetSelectedIndex(Ship::GetLandingModel());
        if (hud_mode)      hud_mode->SetSelectedIndex(HUDView::IsArcade() ? 1 : 0);
        if (hud_color)     hud_color->SetSelectedIndex(HUDView::DefaultColorSet());
        if (ff_mode)       ff_mode->SetSelectedIndex((int32)(Ship::GetFriendlyFireLevel() * 4.0f));

        if (PlayerCharacter* Player = PlayerCharacter::GetCurrentPlayer())
        {
            if (flying_start)  flying_start->SetSelectedIndex(Player->FlyingStart());
            if (ai_difficulty) ai_difficulty->SetSelectedIndex(ai_difficulty->GetOptionCount() - Player->AILevel() - 1);
            if (grid_mode)     grid_mode->SetSelectedIndex(Player->GridMode());
            if (gunsight)      gunsight->SetSelectedIndex(Player->Gunsight());
        }

        bClosed = false;
    }
}

void UOptionsScreen::ShowAudDlg()
{
    EnsureSubDialogs();
    if (!AudDlg) return;

    SetVisibility(ESlateVisibility::Collapsed);
    AudDlg->SetManager(this);
    AudDlg->Show();
}

void UOptionsScreen::ShowVidDlg()
{
    EnsureSubDialogs();
    if (!VidDlg) return;

    SetVisibility(ESlateVisibility::Collapsed);
    VidDlg->SetManager(this);
    VidDlg->Show();
}

void UOptionsScreen::ShowCtlDlg()
{
    EnsureSubDialogs();
    if (!CtlDlg) return;

    // Control options is special: JoyDlg/KeyDlg route through it to MenuScreen.
    // So *show it* but do not make it depend on OptionsScreen.
    SetVisibility(ESlateVisibility::Collapsed);
    CtlDlg->Show();
}

void UOptionsScreen::ShowModDlg()
{
    UE_LOG(LogTemp, Warning, TEXT("OptionsScreen: Mod page not implemented."));
}

void UOptionsScreen::ApplyOptions()
{
    Apply();
    // Now apply to runtime ONCE, centrally:
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UStarshatterAudioSubsystem* AudioSS = GI->GetSubsystem<UStarshatterAudioSubsystem>())
        {
            AudioSS->ApplySettingsToRuntime();   // no args
        }

        if (UStarshatterVideoSubsystem* VideoSS = GI->GetSubsystem<UStarshatterVideoSubsystem>())
        {
            VideoSS->ApplySettingsToRuntime();   // no args
        }

        // If you add a controls subsystem later, do it here too.
        // if (UStarshatterControlsSubsystem* CtlSS = GI->GetSubsystem<UStarshatterControlsSubsystem>())
        // {
        //     CtlSS->ApplySettingsToRuntime();
        // }
    }
    if (CtlDlg) CtlDlg->Apply();

    bClosed = true;
    ShowOptDlg();
}

void UOptionsScreen::CancelOptions()
{
    Cancel();
    if (AudDlg) AudDlg->Cancel();
    if (VidDlg) VidDlg->Cancel();
    if (CtlDlg) CtlDlg->Cancel();

    bClosed = true;
    ShowOptDlg();
}

// ---------------------------------------------------------------------
// Tabs
// ---------------------------------------------------------------------

void UOptionsScreen::OnAudioClicked() { ShowAudDlg(); }
void UOptionsScreen::OnVideoClicked() { ShowVidDlg(); }
void UOptionsScreen::OnOptionsClicked() { ShowOptDlg(); }
void UOptionsScreen::OnControlsClicked() { ShowCtlDlg(); }
void UOptionsScreen::OnModClicked() { ShowModDlg(); }

// ---------------------------------------------------------------------
// Apply/Cancel buttons
// ---------------------------------------------------------------------

void UOptionsScreen::OnApplyClicked() { ApplyOptions(); }
void UOptionsScreen::OnCancelClicked() { CancelOptions(); }

// ---------------------------------------------------------------------
// Combo handlers (optional)
 // ---------------------------------------------------------------------

void UOptionsScreen::OnFlightModelChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Flight model changed."))); }
void UOptionsScreen::OnFlyingStartChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Flying start changed."))); }
void UOptionsScreen::OnLandingsChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Landings changed."))); }
void UOptionsScreen::OnAIDifficultyChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("AI difficulty changed."))); }
void UOptionsScreen::OnHudModeChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("HUD mode changed."))); }
void UOptionsScreen::OnHudColorChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("HUD color changed."))); }
void UOptionsScreen::OnFfModeChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Friendly fire changed."))); }
void UOptionsScreen::OnGridModeChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Grid mode changed."))); }
void UOptionsScreen::OnGunsightChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Gunsight changed."))); }

// ---------------------------------------------------------------------
// Legacy Apply/Cancel for THIS page values
// ---------------------------------------------------------------------

void UOptionsScreen::Apply()
{
    if (bClosed)
        return;

    if (PlayerCharacter* Player = PlayerCharacter::GetCurrentPlayer())
    {
        if (flight_model)  Player->SetFlightModel(flight_model->GetSelectedIndex());
        if (flying_start)  Player->SetFlyingStart(flying_start->GetSelectedIndex());
        if (landings)      Player->SetLandingModel(landings->GetSelectedIndex());

        if (ai_difficulty)
            Player->SetAILevel(ai_difficulty->GetOptionCount() - ai_difficulty->GetSelectedIndex() - 1);

        if (hud_mode)      Player->SetHUDMode(hud_mode->GetSelectedIndex());
        if (hud_color)     Player->SetHUDColor(hud_color->GetSelectedIndex());
        if (ff_mode)       Player->SetFriendlyFire(ff_mode->GetSelectedIndex());
        if (grid_mode)     Player->SetGridMode(grid_mode->GetSelectedIndex());
        if (gunsight)      Player->SetGunsight(gunsight->GetSelectedIndex());

        PlayerCharacter::Save();
    }

    if (flight_model) Ship::SetFlightModel(flight_model->GetSelectedIndex());
    if (landings)     Ship::SetLandingModel(landings->GetSelectedIndex());
    if (hud_mode)     HUDView::SetArcade(hud_mode->GetSelectedIndex() > 0);
    if (hud_color)    HUDView::SetDefaultColorSet(hud_color->GetSelectedIndex());

    if (ff_mode)
        Ship::SetFriendlyFireLevel((float)ff_mode->GetSelectedIndex() / 4.0f);

    if (HUDView* Hud = HUDView::GetInstance())
        if (hud_color) Hud->SetHUDColorSet(hud_color->GetSelectedIndex());

    bClosed = true;
}

void UOptionsScreen::Cancel()
{
    bClosed = true;
}
