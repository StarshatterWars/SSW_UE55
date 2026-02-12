/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2025–2026. All Rights Reserved.

    SUBSYSTEM:      UI / Options
    FILE:           OptionsScreen.cpp
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UOptionsScreen

    Unreal UMG Options router + page implementation.

    PORT NOTES
    ==========
    Legacy OptionsScreen used PlayerCharacter for profile persistence:
      - FlyingStart / AILevel / GridMode / Gunsight
      - Setter methods like SetAILevel(), SetGunsight(), etc.
      - PlayerCharacter::Save()

    Unreal port replaces PlayerCharacter with:
      - UStarshatterPlayerSubsystem (persistence manager)
      - FS_PlayerGameInfo (authoritative profile model)

    Runtime-apply still routes to:
      - Ship (flight model, landing model, friendly fire)
      - HUDView (arcade mode, hud colors)
      - Subsystems (audio/video/controls/keyboard)
=============================================================================*/

#include "OptionsScreen.h"

// UE:
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

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
#include "KeyDlg.h"

// Starshatter core (runtime state):
#include "Ship.h"
#include "HUDView.h"
#include "Starshatter.h"

// Subsystems:
#include "StarshatterAudioSubsystem.h"
#include "StarshatterVideoSubsystem.h"
#include "StarshatterControlsSubsystem.h"
#include "StarshatterKeyboardSubsystem.h"

// Player persistence:
#include "StarshatterPlayerSubsystem.h"
#include "GameStructs.h"

/* --------------------------------------------------------------------
   Construction
   -------------------------------------------------------------------- */

UOptionsScreen::UOptionsScreen(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bClosed = true;
    SetDialogInputEnabled(true);
}

/* --------------------------------------------------------------------
   NativeConstruct
   -------------------------------------------------------------------- */

void UOptionsScreen::NativeConstruct()
{
    Super::NativeConstruct();

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
    {
        flight_model->OnSelectionChanged.RemoveAll(this);
        flight_model->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnFlightModelChanged);
    }
    if (flying_start)
    {
        flying_start->OnSelectionChanged.RemoveAll(this);
        flying_start->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnFlyingStartChanged);
    }
    if (landings)
    {
        landings->OnSelectionChanged.RemoveAll(this);
        landings->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnLandingsChanged);
    }
    if (ai_difficulty)
    {
        ai_difficulty->OnSelectionChanged.RemoveAll(this);
        ai_difficulty->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnAIDifficultyChanged);
    }
    if (hud_mode)
    {
        hud_mode->OnSelectionChanged.RemoveAll(this);
        hud_mode->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnHudModeChanged);
    }
    if (hud_color)
    {
        hud_color->OnSelectionChanged.RemoveAll(this);
        hud_color->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnHudColorChanged);
    }
    if (ff_mode)
    {
        ff_mode->OnSelectionChanged.RemoveAll(this);
        ff_mode->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnFfModeChanged);
    }
    if (grid_mode)
    {
        grid_mode->OnSelectionChanged.RemoveAll(this);
        grid_mode->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnGridModeChanged);
    }
    if (gunsight)
    {
        gunsight->OnSelectionChanged.RemoveAll(this);
        gunsight->OnSelectionChanged.AddDynamic(this, &UOptionsScreen::OnGunsightChanged);
    }

    ShowOptDlg();
}

/* --------------------------------------------------------------------
   NativeTick
   -------------------------------------------------------------------- */

void UOptionsScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    (void)MyGeometry;
    (void)InDeltaTime;
}

/* --------------------------------------------------------------------
   Input
   -------------------------------------------------------------------- */

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
// Subdialog creation
// ---------------------------------------------------------------------

/* --------------------------------------------------------------------
   EnsureSubDialogs (OptionsScreen)
   -------------------------------------------------------------------- */

void UOptionsScreen::EnsureSubDialogs()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("OptionsScreen::EnsureSubDialogs: OwningPlayer is NULL"));
        return;
    }

    // ------------------------------------------------------------
    // Audio
    // ------------------------------------------------------------

    if (!AudDlg && AudioDlgClass)
    {
        AudDlg = CreateWidget<UAudioDlg>(PC, AudioDlgClass);
        if (AudDlg)
        {
            AudDlg->AddToViewport(0);
            AudDlg->SetVisibility(ESlateVisibility::Collapsed);
            AudDlg->SetIsEnabled(false);

            AudDlg->SetOptionsManager(this);
        }
    }

    // ------------------------------------------------------------
    // Video
    // ------------------------------------------------------------

    if (!VidDlg && VideoDlgClass)
    {
        VidDlg = CreateWidget<UVideoDlg>(PC, VideoDlgClass);
        if (VidDlg)
        {
            VidDlg->AddToViewport(0);
            VidDlg->SetVisibility(ESlateVisibility::Collapsed);
            VidDlg->SetIsEnabled(false);

            VidDlg->SetOptionsManager(this);
        }
    }

    // ------------------------------------------------------------
    // Controls
    // ------------------------------------------------------------

    if (!CtlDlg && ControlDlgClass)
    {
        CtlDlg = CreateWidget<UControlOptionsDlg>(PC, ControlDlgClass);
        if (CtlDlg)
        {
            CtlDlg->AddToViewport(0);
            CtlDlg->SetVisibility(ESlateVisibility::Collapsed);
            CtlDlg->SetIsEnabled(false);

            CtlDlg->SetOptionsManager(this);
        }
    }

    // ------------------------------------------------------------
    // Key Binding Dialog
    // ------------------------------------------------------------

    if (!KeyDlg && KeyDlgClass)
    {
        KeyDlg = CreateWidget<UKeyDlg>(PC, KeyDlgClass);
        if (KeyDlg)
        {
            KeyDlg->AddToViewport(0);
            KeyDlg->SetVisibility(ESlateVisibility::Collapsed);
            KeyDlg->SetIsEnabled(false);

            KeyDlg->SetOptionsManager(this);
        }
    }
}

/* --------------------------------------------------------------------
   HideAllPages
   -------------------------------------------------------------------- */

void UOptionsScreen::HideAllPages()
{
    if (AudDlg) AudDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (VidDlg) VidDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (CtlDlg) CtlDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (KeyDlg) KeyDlg->SetVisibility(ESlateVisibility::Collapsed);

    SetVisibility(ESlateVisibility::Collapsed);
}

// ---------------------------------------------------------------------
// Page routing
// ---------------------------------------------------------------------

/* --------------------------------------------------------------------
   ShowOptDlg
   -------------------------------------------------------------------- */

void UOptionsScreen::ShowOptDlg()
{
    EnsureSubDialogs();

    if (AudDlg) AudDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (VidDlg) VidDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (CtlDlg) CtlDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (KeyDlg) KeyDlg->SetVisibility(ESlateVisibility::Collapsed);

    SetVisibility(ESlateVisibility::Visible);
    SetDialogInputEnabled(true);

    if (bClosed)
    {
        // ------------------------------------------------------------
        // Runtime state (Ship/HUDView)
        // ------------------------------------------------------------

        if (flight_model)  flight_model->SetSelectedIndex(Ship::GetFlightModel());
        if (landings)      landings->SetSelectedIndex(Ship::GetLandingModel());
        if (hud_mode)      hud_mode->SetSelectedIndex(HUDView::IsArcade() ? 1 : 0);
        if (hud_color)     hud_color->SetSelectedIndex(HUDView::DefaultColorSet());
        if (ff_mode)       ff_mode->SetSelectedIndex((int32)(Ship::GetFriendlyFireLevel() * 4.0f));

        // ------------------------------------------------------------
        // Player profile state (PlayerSubsystem)
        // ------------------------------------------------------------

        if (UStarshatterPlayerSubsystem* PlayerSS = UStarshatterPlayerSubsystem::Get(GetWorld()))
        {
            if (PlayerSS->HasLoaded())
            {
                const FS_PlayerGameInfo& Info = PlayerSS->GetPlayerInfo();

                if (flying_start)
                    flying_start->SetSelectedIndex(Info.FlyingStart ? 1 : 0);

                if (ai_difficulty)
                {
                    const int32 OptionCount = ai_difficulty->GetOptionCount();
                    const int32 ClampedAI = FMath::Clamp(Info.AILevel, 0, FMath::Max(0, OptionCount - 1));
                    ai_difficulty->SetSelectedIndex(OptionCount - ClampedAI - 1);
                }

                if (grid_mode)
                    grid_mode->SetSelectedIndex(Info.GridMode ? 1 : 0);

                if (gunsight)
                    gunsight->SetSelectedIndex(Info.GunSightMode ? 1 : 0);
            }
        }

        bClosed = false;
    }
}

/* --------------------------------------------------------------------
   ShowAudDlg
   -------------------------------------------------------------------- */

void UOptionsScreen::ShowAudDlg()
{
    EnsureSubDialogs();
    if (!AudDlg)
        return;

    if (VidDlg) { VidDlg->SetVisibility(ESlateVisibility::Collapsed); VidDlg->SetIsEnabled(false); }
    if (CtlDlg) { CtlDlg->SetVisibility(ESlateVisibility::Collapsed); CtlDlg->SetIsEnabled(false); }
    if (KeyDlg) { KeyDlg->SetVisibility(ESlateVisibility::Collapsed); KeyDlg->SetIsEnabled(false); }

    SetVisibility(ESlateVisibility::Visible);
    SetIsEnabled(true);

    AudDlg->SetOptionsManager(this);
    AudDlg->SetVisibility(ESlateVisibility::Visible);
    AudDlg->SetIsEnabled(true);

    if (AudDlg->IsInViewport())
        AudDlg->RemoveFromParent();
    AudDlg->AddToViewport(500);
}

void UOptionsScreen::ShowVidDlg()
{
    EnsureSubDialogs();
    if (!VidDlg) return;

    SetVisibility(ESlateVisibility::Collapsed);
    if (KeyDlg) KeyDlg->SetVisibility(ESlateVisibility::Collapsed);

    VidDlg->SetManager(this);
    VidDlg->Show();
}

void UOptionsScreen::ShowCtlDlg()
{
    EnsureSubDialogs();
    if (!CtlDlg) return;

    SetVisibility(ESlateVisibility::Collapsed);
    if (KeyDlg) KeyDlg->SetVisibility(ESlateVisibility::Collapsed);

    CtlDlg->Show();
}

void UOptionsScreen::ShowModDlg()
{
    UE_LOG(LogTemp, Warning, TEXT("OptionsScreen: Mod page not implemented."));
}

/* --------------------------------------------------------------------
   ShowKeyDlg
   -------------------------------------------------------------------- */

void UOptionsScreen::ShowKeyDlg(EStarshatterInputAction Action)
{
    EnsureSubDialogs();
    if (!KeyDlg) return;

    SetVisibility(ESlateVisibility::Collapsed);
    if (AudDlg) AudDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (VidDlg) VidDlg->SetVisibility(ESlateVisibility::Collapsed);
    if (CtlDlg) CtlDlg->SetVisibility(ESlateVisibility::Collapsed);

    KeyDlg->SetManager(this);
    KeyDlg->SetEditingAction(Action);
    KeyDlg->SetVisibility(ESlateVisibility::Visible);
    KeyDlg->SetKeyboardFocus();
}

// ---------------------------------------------------------------------
// Apply / Cancel orchestration
// ---------------------------------------------------------------------

void UOptionsScreen::ApplyOptions()
{
    Apply();

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UStarshatterAudioSubsystem* AudioSS = GI->GetSubsystem<UStarshatterAudioSubsystem>())
            AudioSS->ApplySettingsToRuntime();

        if (UStarshatterVideoSubsystem* VideoSS = GI->GetSubsystem<UStarshatterVideoSubsystem>())
            VideoSS->ApplySettingsToRuntime();

        if (UStarshatterControlsSubsystem* CtlSS = GI->GetSubsystem<UStarshatterControlsSubsystem>())
            CtlSS->ApplySettingsToRuntime(this);

        if (UStarshatterKeyboardSubsystem* KeyboardSS = GI->GetSubsystem<UStarshatterKeyboardSubsystem>())
            KeyboardSS->ApplySettingsToRuntime(this);
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

    if (KeyDlg) KeyDlg->SetVisibility(ESlateVisibility::Collapsed);

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
// Combo handlers (optional UI feedback)
// ---------------------------------------------------------------------

void UOptionsScreen::OnFlightModelChanged(FString, ESelectInfo::Type)
{
    if (description) description->SetText(FText::FromString(TEXT("Flight model changed.")));
}

void UOptionsScreen::OnFlyingStartChanged(FString, ESelectInfo::Type)
{
    if (description) description->SetText(FText::FromString(TEXT("Flying start changed.")));
}

void UOptionsScreen::OnLandingsChanged(FString, ESelectInfo::Type)
{
    if (description) description->SetText(FText::FromString(TEXT("Landings changed.")));
}

void UOptionsScreen::OnAIDifficultyChanged(FString, ESelectInfo::Type)
{
    if (description) description->SetText(FText::FromString(TEXT("AI difficulty changed.")));
}

void UOptionsScreen::OnHudModeChanged(FString, ESelectInfo::Type)
{
    if (description) description->SetText(FText::FromString(TEXT("HUD mode changed.")));
}

void UOptionsScreen::OnHudColorChanged(FString, ESelectInfo::Type)
{
    if (description) description->SetText(FText::FromString(TEXT("HUD color changed.")));
}

void UOptionsScreen::OnFfModeChanged(FString, ESelectInfo::Type)
{
    if (description) description->SetText(FText::FromString(TEXT("Friendly fire changed.")));
}

void UOptionsScreen::OnGridModeChanged(FString, ESelectInfo::Type)
{
    if (description) description->SetText(FText::FromString(TEXT("Grid mode changed.")));
}

void UOptionsScreen::OnGunsightChanged(FString, ESelectInfo::Type)
{
    if (description) description->SetText(FText::FromString(TEXT("Gunsight changed.")));
}

// ---------------------------------------------------------------------
// Legacy Apply/Cancel for THIS page values
// ---------------------------------------------------------------------

void UOptionsScreen::Apply()
{
    if (bClosed)
        return;

    // ------------------------------------------------------------
    // Persist player-side options (PlayerSubsystem)
    // ------------------------------------------------------------

    if (UStarshatterPlayerSubsystem* PlayerSS = UStarshatterPlayerSubsystem::Get(GetWorld()))
    {
        if (PlayerSS->HasLoaded())
        {
            FS_PlayerGameInfo& Info = PlayerSS->GetMutablePlayerInfo();

            if (flight_model)  Info.FlightModel = flight_model->GetSelectedIndex();
            if (landings)      Info.LandingMode = landings->GetSelectedIndex();
            if (hud_mode)      Info.HudMode = hud_mode->GetSelectedIndex();
            if (hud_color)     Info.HudColor = hud_color->GetSelectedIndex();

            if (flying_start)  Info.FlyingStart = (flying_start->GetSelectedIndex() != 0);
            if (grid_mode)     Info.GridMode = (grid_mode->GetSelectedIndex() != 0);
            if (gunsight)      Info.GunSightMode = (gunsight->GetSelectedIndex() != 0);

            if (ai_difficulty)
            {
                const int32 OptionCount = ai_difficulty->GetOptionCount();
                const int32 Selected = ai_difficulty->GetSelectedIndex();
                const int32 NewAI = OptionCount - Selected - 1;
                Info.AILevel = FMath::Clamp(NewAI, 0, FMath::Max(0, OptionCount - 1));
            }

            // Keep alias fields consistent if you rely on them elsewhere:
            Info.SyncRosterAliasesFromPlayerStats();

            PlayerSS->MarkDirty();
            PlayerSS->SavePlayer(false);
        }
    }

    // ------------------------------------------------------------
    // Apply runtime settings (Ship/HUDView)
    // ------------------------------------------------------------

    if (flight_model) Ship::SetFlightModel(flight_model->GetSelectedIndex());
    if (landings)     Ship::SetLandingModel(landings->GetSelectedIndex());
    if (hud_mode)     HUDView::SetArcade(hud_mode->GetSelectedIndex() > 0);
    if (hud_color)    HUDView::SetDefaultColorSet(hud_color->GetSelectedIndex());

    if (ff_mode)
        Ship::SetFriendlyFireLevel((float)ff_mode->GetSelectedIndex() / 4.0f);

    if (HUDView* Hud = HUDView::GetInstance())
    {
        if (hud_color)
            Hud->SetHUDColorSet(hud_color->GetSelectedIndex());
    }

    bClosed = true;
}

void UOptionsScreen::Cancel()
{
    bClosed = true;
}
