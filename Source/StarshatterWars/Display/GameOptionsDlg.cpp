/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:      StarshatterWars (Unreal Engine)
    FILE:           GameOptionsDlg.cpp
    AUTHOR:         Carlos Bott
=============================================================================*/

#include "GameOptionsDlg.h"

// UMG
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"

// Router
#include "OptionsScreen.h"

// Legacy
#include "Ship.h"
#include "HUDView.h"
#include "PlayerCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogGameOptionsDlg, Log, All);

UGameOptionsDlg::UGameOptionsDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetDialogInputEnabled(true);
}

void UGameOptionsDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BindDelegates();
}

void UGameOptionsDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // BaseScreen Enter/Escape routing:
    ApplyButton = ApplyBtn;
    CancelButton = CancelBtn;

    BindDelegates();

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
        bDirty = false;
    }
}

void UGameOptionsDlg::BindDelegates()
{
    if (bDelegatesBound)
        return;

    if (ApplyBtn)  ApplyBtn->OnClicked.AddUniqueDynamic(this, &UGameOptionsDlg::OnApplyClicked);
    if (CancelBtn) CancelBtn->OnClicked.AddUniqueDynamic(this, &UGameOptionsDlg::OnCancelClicked);

    // Tabs
    if (aud_btn) aud_btn->OnClicked.AddUniqueDynamic(this, &UGameOptionsDlg::OnAudioClicked);
    if (vid_btn) vid_btn->OnClicked.AddUniqueDynamic(this, &UGameOptionsDlg::OnVideoClicked);
    if (ctl_btn) ctl_btn->OnClicked.AddUniqueDynamic(this, &UGameOptionsDlg::OnControlsClicked);
    if (opt_btn) opt_btn->OnClicked.AddUniqueDynamic(this, &UGameOptionsDlg::OnGameClicked);
    if (mod_btn) mod_btn->OnClicked.AddUniqueDynamic(this, &UGameOptionsDlg::OnModClicked);

    // Combos
    if (flight_model)  flight_model->OnSelectionChanged.AddUniqueDynamic(this, &UGameOptionsDlg::OnFlightModelChanged);
    if (flying_start)  flying_start->OnSelectionChanged.AddUniqueDynamic(this, &UGameOptionsDlg::OnFlyingStartChanged);
    if (landings)      landings->OnSelectionChanged.AddUniqueDynamic(this, &UGameOptionsDlg::OnLandingsChanged);
    if (ai_difficulty) ai_difficulty->OnSelectionChanged.AddUniqueDynamic(this, &UGameOptionsDlg::OnAIDifficultyChanged);
    if (hud_mode)      hud_mode->OnSelectionChanged.AddUniqueDynamic(this, &UGameOptionsDlg::OnHudModeChanged);
    if (hud_color)     hud_color->OnSelectionChanged.AddUniqueDynamic(this, &UGameOptionsDlg::OnHudColorChanged);
    if (ff_mode)       ff_mode->OnSelectionChanged.AddUniqueDynamic(this, &UGameOptionsDlg::OnFfModeChanged);
    if (grid_mode)     grid_mode->OnSelectionChanged.AddUniqueDynamic(this, &UGameOptionsDlg::OnGridModeChanged);
    if (gunsight)      gunsight->OnSelectionChanged.AddUniqueDynamic(this, &UGameOptionsDlg::OnGunsightChanged);

    bDelegatesBound = true;
}

void UGameOptionsDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
        bDirty = false;
    }

    SetKeyboardFocus();
}

void UGameOptionsDlg::ExecFrame(double /*DeltaTime*/)
{
}

void UGameOptionsDlg::BindFormWidgets() {}
FString UGameOptionsDlg::GetLegacyFormText() const { return FString(); }

void UGameOptionsDlg::HandleAccept() { OnApplyClicked(); }
void UGameOptionsDlg::HandleCancel() { OnCancelClicked(); }

// ------------------------------------------------------------
// Model -> UI
// ------------------------------------------------------------

void UGameOptionsDlg::RefreshFromModel()
{
    // Pull from legacy globals
    FlightModel = Ship::GetFlightModel();
    LandingModel = Ship::GetLandingModel();
    HudModeIndex = HUDView::IsArcade() ? 1 : 0;
    HudColorIndex = HUDView::DefaultColorSet();
    FriendlyFireIndex = (int32)(Ship::GetFriendlyFireLevel() * 4.0f);

    if (PlayerCharacter* Player = PlayerCharacter::GetCurrentPlayer())
    {
        FlyingStart = Player->FlyingStart();

        if (ai_difficulty)
            AIDifficultyIndex = ai_difficulty->GetOptionCount() - Player->AILevel() - 1;

        GridModeIndex = Player->GridMode();
        GunsightIndex = Player->Gunsight();
    }

    if (flight_model)  flight_model->SetSelectedIndex(FlightModel);
    if (landings)      landings->SetSelectedIndex(LandingModel);
    if (hud_mode)      hud_mode->SetSelectedIndex(HudModeIndex);
    if (hud_color)     hud_color->SetSelectedIndex(HudColorIndex);
    if (ff_mode)       ff_mode->SetSelectedIndex(FriendlyFireIndex);

    if (flying_start)  flying_start->SetSelectedIndex(FlyingStart);
    if (ai_difficulty) ai_difficulty->SetSelectedIndex(AIDifficultyIndex);
    if (grid_mode)     grid_mode->SetSelectedIndex(GridModeIndex);
    if (gunsight)      gunsight->SetSelectedIndex(GunsightIndex);
}

// ------------------------------------------------------------
// UI -> Model
// ------------------------------------------------------------

void UGameOptionsDlg::PushToModel()
{
    // PlayerCharacter
    if (PlayerCharacter* Player = PlayerCharacter::GetCurrentPlayer())
    {
        if (flight_model)  Player->SetFlightModel(flight_model->GetSelectedIndex());
        if (flying_start)  Player->SetFlyingStart(flying_start->GetSelectedIndex());
        if (landings)      Player->SetLandingModel(landings->GetSelectedIndex());

        if (ai_difficulty)
            Player->SetAILevel(ai_difficulty->GetOptionCount() - ai_difficulty->GetSelectedIndex() - 1);

        if (hud_mode)  Player->SetHUDMode(hud_mode->GetSelectedIndex());
        if (hud_color) Player->SetHUDColor(hud_color->GetSelectedIndex());
        if (ff_mode)   Player->SetFriendlyFire(ff_mode->GetSelectedIndex());
        if (grid_mode) Player->SetGridMode(grid_mode->GetSelectedIndex());
        if (gunsight)  Player->SetGunsight(gunsight->GetSelectedIndex());

        PlayerCharacter::Save();
    }

    // Ship + HUDView globals
    if (flight_model) Ship::SetFlightModel(flight_model->GetSelectedIndex());
    if (landings)     Ship::SetLandingModel(landings->GetSelectedIndex());

    if (hud_mode)  HUDView::SetArcade(hud_mode->GetSelectedIndex() > 0);
    if (hud_color) HUDView::SetDefaultColorSet(hud_color->GetSelectedIndex());

    if (ff_mode)
        Ship::SetFriendlyFireLevel((float)ff_mode->GetSelectedIndex() / 4.0f);

    if (HUDView* Hud = HUDView::GetInstance())
        if (hud_color) Hud->SetHUDColorSet(hud_color->GetSelectedIndex());

    bDirty = false;
}

void UGameOptionsDlg::Apply()
{
    if (!bDirty)
        return;

    PushToModel();
    bClosed = true;
}

void UGameOptionsDlg::Cancel()
{
    RefreshFromModel();
    bClosed = true;
    bDirty = false;
}

// ------------------------------------------------------------
// Apply / Cancel
// ------------------------------------------------------------

void UGameOptionsDlg::OnApplyClicked()
{
    if (OptionsManager) OptionsManager->ApplyOptions();
    else Apply();
}

void UGameOptionsDlg::OnCancelClicked()
{
    if (OptionsManager) OptionsManager->CancelOptions();
    else Cancel();
}

// ------------------------------------------------------------
// Tabs
// ------------------------------------------------------------

void UGameOptionsDlg::OnAudioClicked() { if (OptionsManager) OptionsManager->ShowAudDlg(); }
void UGameOptionsDlg::OnVideoClicked() { if (OptionsManager) OptionsManager->ShowVidDlg(); }
void UGameOptionsDlg::OnControlsClicked() { if (OptionsManager) OptionsManager->ShowCtlDlg(); }
void UGameOptionsDlg::OnGameClicked() { /* already here */ }
void UGameOptionsDlg::OnModClicked() { if (OptionsManager) OptionsManager->ShowModDlg(); }

// ------------------------------------------------------------
// Combo handlers (mark dirty)
// ------------------------------------------------------------

void UGameOptionsDlg::OnFlightModelChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bDirty = true; bClosed = false; }
void UGameOptionsDlg::OnFlyingStartChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bDirty = true; bClosed = false; }
void UGameOptionsDlg::OnLandingsChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bDirty = true; bClosed = false; }
void UGameOptionsDlg::OnAIDifficultyChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bDirty = true; bClosed = false; }
void UGameOptionsDlg::OnHudModeChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bDirty = true; bClosed = false; }
void UGameOptionsDlg::OnHudColorChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bDirty = true; bClosed = false; }
void UGameOptionsDlg::OnFfModeChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bDirty = true; bClosed = false; }
void UGameOptionsDlg::OnGridModeChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bDirty = true; bClosed = false; }
void UGameOptionsDlg::OnGunsightChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bDirty = true; bClosed = false; }
