/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         OptDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Main Menu Options Dialog (Unreal UUserWidget)
    Integrated legacy OptDlg.frm as a pure C++ WidgetTree build.
*/



#include "OptDlg.h"

// UMG:
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"

// Input:
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Manager screen (dialog switching + apply/cancel options):
#include "MenuScreen.h"
#include "GameStructs.h"

// Starshatter core (legacy types kept in game logic):
#include "Ship.h"
#include "HUDView.h"
#include "PlayerCharacter.h"
#include "Starshatter.h"

// +--------------------------------------------------------------------+

UOptDlg::UOptDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

static void AddComboItem(UComboBoxString* Combo, const TCHAR* Item)
{
    if (Combo)
        Combo->AddOption(FString(Item));
}

static void ClearAndSetSelection(UComboBoxString* Combo, int32 Index)
{
    if (!Combo)
        return;

    const int32 Num = Combo->GetOptionCount();
    if (Num <= 0)
        return;

    const int32 Clamped = FMath::Clamp(Index, 0, Num - 1);
    Combo->SetSelectedIndex(Clamped);
}

void UOptDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // If authored in C++ only, build the widget tree now.
    // If you authored it in UMG, BindWidgetOptional pointers will already be set.
    const bool bNeedsBuild =
        (!ApplyBtn && !CancelBtn && !flight_model && WidgetTree && !WidgetTree->RootWidget);

    if (bNeedsBuild)
    {
        if (!WidgetTree)
            return;

        // Root: Canvas
        UCanvasPanel* RootCanvas =
            WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
        WidgetTree->RootWidget = RootCanvas;

        // Frame1 background (Frame1.pcx)
        UImage* Frame1Image =
            WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Frame1"));
        {
            UCanvasPanelSlot* Frame1CanvasSlot = RootCanvas->AddChildToCanvas(Frame1Image);
            Frame1CanvasSlot->SetAnchors(FAnchors(0, 0, 1, 1));
            Frame1CanvasSlot->SetOffsets(FMargin(0, 0, 0, 0));
            // TODO: assign brush from "Frame1.pcx"
        }

        // Overlay for Frame2a/Frame2b + UI
        UOverlay* RootOverlay =
            WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
        {
            UCanvasPanelSlot* RootOverlayCanvasSlot = RootCanvas->AddChildToCanvas(RootOverlay);
            RootOverlayCanvasSlot->SetAnchors(FAnchors(0, 0, 1, 1));
            RootOverlayCanvasSlot->SetOffsets(FMargin(0, 0, 0, 0));
        }

        // Frame2a
        UImage* Frame2aImage =
            WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Frame2a"));
        {
            UOverlaySlot* Frame2aOverlaySlot = RootOverlay->AddChildToOverlay(Frame2aImage);
            Frame2aOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
            Frame2aOverlaySlot->SetVerticalAlignment(VAlign_Fill);
            // TODO: assign brush "Frame2a"
        }

        // Frame2b
        UImage* Frame2bImage =
            WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Frame2b"));
        {
            UOverlaySlot* Frame2bOverlaySlot = RootOverlay->AddChildToOverlay(Frame2bImage);
            Frame2bOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
            Frame2bOverlaySlot->SetVerticalAlignment(VAlign_Fill);
            // TODO: assign brush "Frame2b"
        }

        // Main Grid (approximates legacy layout + margins)
        UGridPanel* MainGrid =
            WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass(), TEXT("MainGrid"));
        {
            UOverlaySlot* MainGridOverlaySlot = RootOverlay->AddChildToOverlay(MainGrid);
            MainGridOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
            MainGridOverlaySlot->SetVerticalAlignment(VAlign_Fill);

            // IMPORTANT: Padding belongs to the SLOT, not UGridPanel (no SetPadding on UGridPanel)
            MainGridOverlaySlot->SetPadding(FMargin(10.f, 10.f, 64.f, 8.f));
        }

        // Title label "Options"
        UTextBlock* TitleText =
            WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title_Options"));
        TitleText->SetText(FText::FromString(TEXT("Options")));
        {
            UGridSlot* TitleGridSlot = MainGrid->AddChildToGrid(TitleText, 0, 0);
            TitleGridSlot->SetColumnSpan(6);
            TitleGridSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
            TitleGridSlot->SetHorizontalAlignment(HAlign_Left);
            TitleGridSlot->SetVerticalAlignment(VAlign_Top);
        }

        // Tabs row
        UHorizontalBox* TabsRow =
            WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TabsRow"));
        {
            UGridSlot* TabsRowGridSlot = MainGrid->AddChildToGrid(TabsRow, 1, 0);
            TabsRowGridSlot->SetColumnSpan(6);
            TabsRowGridSlot->SetPadding(FMargin(0.f, 8.f, 0.f, 8.f));
            TabsRowGridSlot->SetHorizontalAlignment(HAlign_Fill);
            TabsRowGridSlot->SetVerticalAlignment(VAlign_Top);
        }

        auto MakeTab = [&](const TCHAR* InName, const TCHAR* InLabel) -> UButton*
            {
                UButton* TabButton =
                    WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), InName);

                const FString TabTextName = FString(InName) + TEXT("_Text");
                UTextBlock* TabText =
                    WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *TabTextName);

                TabText->SetText(FText::FromString(InLabel));
                TabButton->AddChild(TabText);
                return TabButton;
            };

        vid_btn = MakeTab(TEXT("Tab_Video"), TEXT("Video"));
        aud_btn = MakeTab(TEXT("Tab_Audio"), TEXT("Audio"));
        ctl_btn = MakeTab(TEXT("Tab_Controls"), TEXT("Controls"));
        opt_btn = MakeTab(TEXT("Tab_Gameplay"), TEXT("Gameplay"));
        mod_btn = MakeTab(TEXT("Tab_Mod"), TEXT("Mod Config"));

        auto AddTab = [&](UButton* InBtn)
            {
                UHorizontalBoxSlot* TabHBoxSlot = TabsRow->AddChildToHorizontalBox(InBtn);
                TabHBoxSlot->SetPadding(FMargin(4.f, 0.f, 4.f, 0.f));
                TabHBoxSlot->SetSize(ESlateSizeRule::Automatic);
                TabHBoxSlot->SetHorizontalAlignment(HAlign_Left);
                TabHBoxSlot->SetVerticalAlignment(VAlign_Fill);
            };

        AddTab(vid_btn);
        AddTab(aud_btn);
        AddTab(ctl_btn);
        AddTab(opt_btn);
        AddTab(mod_btn);

        // Main panel
        UBorder* PanelBorder =
            WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MainPanelBorder"));
        {
            UGridSlot* PanelBorderGridSlot = MainGrid->AddChildToGrid(PanelBorder, 2, 0);
            PanelBorderGridSlot->SetColumnSpan(6);
            PanelBorderGridSlot->SetPadding(FMargin(12.f, 12.f, 12.f, 0.f));
            PanelBorderGridSlot->SetHorizontalAlignment(HAlign_Fill);
            PanelBorderGridSlot->SetVerticalAlignment(VAlign_Fill);
            // TODO: panel brush
        }

        UGridPanel* PanelGrid =
            WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass(), TEXT("PanelGrid"));
        PanelBorder->SetContent(PanelGrid);

        auto MakeLabel = [&](const TCHAR* InName, const TCHAR* InText) -> UTextBlock*
            {
                UTextBlock* Label =
                    WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), InName);
                Label->SetText(FText::FromString(InText));
                return Label;
            };

        auto MakeCombo = [&](const TCHAR* InName) -> UComboBoxString*
            {
                return WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), InName);
            };

        // Left side labels (rows)
        struct FRowDef
        {
            const TCHAR* LabelName;
            const TCHAR* LabelText;
            const TCHAR* ComboName;
            int32 Row;
        };

        const FRowDef Rows[] =
        {
            { TEXT("Lbl_FlightModel"),  TEXT("Flight Model:"),   TEXT("Cmb_FlightModel"),   0 },
            { TEXT("Lbl_FlyingStart"),  TEXT("Flying Start:"),   TEXT("Cmb_FlyingStart"),   1 },
            { TEXT("Lbl_Landings"),     TEXT("Landings:"),       TEXT("Cmb_Landings"),      2 },
            { TEXT("Lbl_AIDifficulty"), TEXT("AI Difficulty:"),  TEXT("Cmb_AIDifficulty"),  3 },
            { TEXT("Lbl_HudMode"),      TEXT("HUD Mode:"),       TEXT("Cmb_HudMode"),       4 },
            { TEXT("Lbl_HudColor"),     TEXT("HUD Color:"),      TEXT("Cmb_HudColor"),      5 },
            { TEXT("Lbl_FriendlyFire"), TEXT("Friendly Fire:"),  TEXT("Cmb_FriendlyFire"),  6 },
            { TEXT("Lbl_GridMode"),     TEXT("Reference Grid:"), TEXT("Cmb_GridMode"),      7 },
            { TEXT("Lbl_Gunsight"),     TEXT("Gunsight:"),       TEXT("Cmb_Gunsight"),      8 },
        };

        for (const FRowDef& RowDef : Rows)
        {
            UTextBlock* RowLabel = MakeLabel(RowDef.LabelName, RowDef.LabelText);
            UComboBoxString* RowCombo = MakeCombo(RowDef.ComboName);

            UGridSlot* RowLabelGridSlot = PanelGrid->AddChildToGrid(RowLabel, RowDef.Row, 0);
            RowLabelGridSlot->SetPadding(FMargin(0.f, 4.f, 10.f, 4.f));
            RowLabelGridSlot->SetHorizontalAlignment(HAlign_Left);
            RowLabelGridSlot->SetVerticalAlignment(VAlign_Center);

            UGridSlot* RowComboGridSlot = PanelGrid->AddChildToGrid(RowCombo, RowDef.Row, 1);
            RowComboGridSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 4.f));
            RowComboGridSlot->SetHorizontalAlignment(HAlign_Fill);
            RowComboGridSlot->SetVerticalAlignment(VAlign_Center);
        }

        // Right side description (legacy id 500)
        description =
            WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Description"));
        {
            UGridSlot* DescriptionGridSlot = PanelGrid->AddChildToGrid(description, 0, 2);
            DescriptionGridSlot->SetRowSpan(9);
            DescriptionGridSlot->SetPadding(FMargin(20.f, 0.f, 0.f, 0.f));
            DescriptionGridSlot->SetHorizontalAlignment(HAlign_Fill);
            DescriptionGridSlot->SetVerticalAlignment(VAlign_Fill);
        }

        // Assign combo refs by name
        flight_model = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_FlightModel")));
        flying_start = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_FlyingStart")));
        landings = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_Landings")));
        ai_difficulty = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_AIDifficulty")));
        hud_mode = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_HudMode")));
        hud_color = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_HudColor")));
        ff_mode = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_FriendlyFire")));
        grid_mode = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_GridMode")));
        gunsight = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_Gunsight")));

        // Populate items (from legacy FORM)
        if (flight_model)
        {
            AddComboItem(flight_model, TEXT("Standard"));
            AddComboItem(flight_model, TEXT("Relaxed"));
            AddComboItem(flight_model, TEXT("Arcade"));
        }

        if (flying_start)
        {
            AddComboItem(flying_start, TEXT("Disabled"));
            AddComboItem(flying_start, TEXT("Enabled"));
        }

        if (landings)
        {
            AddComboItem(landings, TEXT("Standard"));
            AddComboItem(landings, TEXT("Easier"));
        }

        if (ai_difficulty)
        {
            AddComboItem(ai_difficulty, TEXT("Ace"));
            AddComboItem(ai_difficulty, TEXT("Veteran"));
            AddComboItem(ai_difficulty, TEXT("Rookie"));
        }

        if (hud_mode)
        {
            AddComboItem(hud_mode, TEXT("Standard"));
            AddComboItem(hud_mode, TEXT("Simplified"));
        }

        if (hud_color)
        {
            AddComboItem(hud_color, TEXT("Green"));
            AddComboItem(hud_color, TEXT("Blue"));
            AddComboItem(hud_color, TEXT("Orange"));
            AddComboItem(hud_color, TEXT("Black"));
        }

        if (ff_mode)
        {
            AddComboItem(ff_mode, TEXT("None"));
            AddComboItem(ff_mode, TEXT("25% Damage"));
            AddComboItem(ff_mode, TEXT("50% Damage"));
            AddComboItem(ff_mode, TEXT("75% Damage"));
            AddComboItem(ff_mode, TEXT("Full Damage"));
        }

        if (grid_mode)
        {
            AddComboItem(grid_mode, TEXT("Disabled"));
            AddComboItem(grid_mode, TEXT("Enabled"));
        }

        if (gunsight)
        {
            AddComboItem(gunsight, TEXT("Standard LCOS"));
            AddComboItem(gunsight, TEXT("Lead Indicator"));
        }

        // Bottom buttons
        UHorizontalBox* BottomRow =
            WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BottomRow"));
        {
            UGridSlot* BottomRowGridSlot = MainGrid->AddChildToGrid(BottomRow, 3, 0);
            BottomRowGridSlot->SetColumnSpan(6);
            BottomRowGridSlot->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));
            BottomRowGridSlot->SetHorizontalAlignment(HAlign_Right);
            BottomRowGridSlot->SetVerticalAlignment(VAlign_Bottom);
        }

        auto MakeAction = [&](const TCHAR* InName, const TCHAR* InText) -> UButton*
            {
                UButton* ActionButton =
                    WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), InName);

                const FString ActionTextName = FString(InName) + TEXT("_Text");
                UTextBlock* ActionText =
                    WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *ActionTextName);

                ActionText->SetText(FText::FromString(InText));
                ActionButton->AddChild(ActionText);
                return ActionButton;
            };

        ApplyBtn = MakeAction(TEXT("ApplyBtn"), TEXT("Apply"));
        CancelBtn = MakeAction(TEXT("CancelBtn"), TEXT("Cancel"));

        {
            UHorizontalBoxSlot* ApplyBtnHBoxSlot = BottomRow->AddChildToHorizontalBox(ApplyBtn);
            ApplyBtnHBoxSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));
            ApplyBtnHBoxSlot->SetSize(ESlateSizeRule::Automatic);

            UHorizontalBoxSlot* CancelBtnHBoxSlot = BottomRow->AddChildToHorizontalBox(CancelBtn);
            CancelBtnHBoxSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));
            CancelBtnHBoxSlot->SetSize(ESlateSizeRule::Automatic);
        }
    }

    // -----------------------------------------------------------------
    // Bind action buttons
    // -----------------------------------------------------------------

    if (ApplyBtn)
    {
        ApplyBtn->OnClicked.Clear();
        ApplyBtn->OnClicked.AddDynamic(this, &UOptDlg::OnApplyClicked);
    }

    if (CancelBtn)
    {
        CancelBtn->OnClicked.Clear();
        CancelBtn->OnClicked.AddDynamic(this, &UOptDlg::OnCancelClicked);
    }

    // -----------------------------------------------------------------
    // Bind tabs
    // -----------------------------------------------------------------

    if (vid_btn)
    {
        vid_btn->OnClicked.Clear();
        vid_btn->OnClicked.AddDynamic(this, &UOptDlg::OnVideoClicked);
    }

    if (aud_btn)
    {
        aud_btn->OnClicked.Clear();
        aud_btn->OnClicked.AddDynamic(this, &UOptDlg::OnAudioClicked);
    }

    if (ctl_btn)
    {
        ctl_btn->OnClicked.Clear();
        ctl_btn->OnClicked.AddDynamic(this, &UOptDlg::OnControlsClicked);
    }

    if (opt_btn)
    {
        opt_btn->OnClicked.Clear();
        opt_btn->OnClicked.AddDynamic(this, &UOptDlg::OnOptionsClicked);
    }

    if (mod_btn)
    {
        mod_btn->OnClicked.Clear();
        mod_btn->OnClicked.AddDynamic(this, &UOptDlg::OnModClicked);
    }

    // -----------------------------------------------------------------
    // Bind combo changes (optional)
    // -----------------------------------------------------------------

    if (flight_model)
    {
        flight_model->OnSelectionChanged.Clear();
        flight_model->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnFlightModelChanged);
    }

    if (flying_start)
    {
        flying_start->OnSelectionChanged.Clear();
        flying_start->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnFlyingStartChanged);
    }

    if (landings)
    {
        landings->OnSelectionChanged.Clear();
        landings->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnLandingsChanged);
    }

    if (ai_difficulty)
    {
        ai_difficulty->OnSelectionChanged.Clear();
        ai_difficulty->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnAIDifficultyChanged);
    }

    if (hud_mode)
    {
        hud_mode->OnSelectionChanged.Clear();
        hud_mode->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnHudModeChanged);
    }

    if (hud_color)
    {
        hud_color->OnSelectionChanged.Clear();
        hud_color->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnHudColorChanged);
    }

    if (ff_mode)
    {
        ff_mode->OnSelectionChanged.Clear();
        ff_mode->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnFfModeChanged);
    }

    if (grid_mode)
    {
        grid_mode->OnSelectionChanged.Clear();
        grid_mode->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnGridModeChanged);
    }

    if (gunsight)
    {
        gunsight->OnSelectionChanged.Clear();
        gunsight->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnGunsightChanged);
    }

    // -----------------------------------------------------------------
    // Mirror legacy Show() initialization
    // -----------------------------------------------------------------

    if (closed)
    {
        Starshatter* stars = Starshatter::GetInstance();
        if (stars)
        {
            if (flight_model)  ClearAndSetSelection(flight_model, Ship::GetFlightModel());
            if (landings)      ClearAndSetSelection(landings, Ship::GetLandingModel());
            if (hud_mode)      ClearAndSetSelection(hud_mode, HUDView::IsArcade() ? 1 : 0);
            if (hud_color)     ClearAndSetSelection(hud_color, HUDView::DefaultColorSet());
            if (ff_mode)       ClearAndSetSelection(ff_mode, (int32)(Ship::GetFriendlyFireLevel() * 4.0f));
        }

        PlayerCharacter* player = PlayerCharacter::GetCurrentPlayer();
        if (player)
        {
            if (flying_start)  ClearAndSetSelection(flying_start, player->FlyingStart());
            if (ai_difficulty) ClearAndSetSelection(ai_difficulty, ai_difficulty->GetOptionCount() - player->AILevel() - 1);
            if (grid_mode)     ClearAndSetSelection(grid_mode, player->GridMode());
            if (gunsight)      ClearAndSetSelection(gunsight, player->Gunsight());
        }
    }

    closed = false;
}

void UOptDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

FReply UOptDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
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
// Tab navigation:

void UOptDlg::OnAudioClicked() { if (manager) manager->ShowAudDlg();  else UE_LOG(LogTemp, Warning, TEXT("OptDlg: manager null (Audio).")); }
void UOptDlg::OnVideoClicked() { if (manager) manager->ShowVidDlg();  else UE_LOG(LogTemp, Warning, TEXT("OptDlg: manager null (Video).")); }
void UOptDlg::OnOptionsClicked() { if (manager) manager->ShowOptDlg();  else UE_LOG(LogTemp, Warning, TEXT("OptDlg: manager null (Options).")); }
void UOptDlg::OnControlsClicked() { if (manager) manager->ShowCtlDlg();  else UE_LOG(LogTemp, Warning, TEXT("OptDlg: manager null (Controls).")); }

// ---------------------------------------------------------------------
// Actions:

void UOptDlg::OnApplyClicked()
{
    if (manager) manager->ApplyOptions();
    else UE_LOG(LogTemp, Warning, TEXT("OptDlg: manager null (Apply)."));
}

void UOptDlg::OnCancelClicked()
{
    if (manager) manager->CancelOptions();
    else UE_LOG(LogTemp, Warning, TEXT("OptDlg: manager null (Cancel)."));
}

// ---------------------------------------------------------------------
// Optional combo changed handlers (currently just updates description text if desired):

void UOptDlg::OnFlightModelChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Flight model selection changed."))); }
void UOptDlg::OnFlyingStartChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Flying start selection changed."))); }
void UOptDlg::OnLandingsChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Landing model selection changed."))); }
void UOptDlg::OnAIDifficultyChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("AI difficulty selection changed."))); }
void UOptDlg::OnHudModeChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("HUD mode selection changed."))); }
void UOptDlg::OnHudColorChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("HUD color selection changed."))); }
void UOptDlg::OnJoyModeChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Joystick mode selection changed."))); }
void UOptDlg::OnFfModeChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Friendly fire selection changed."))); }
void UOptDlg::OnGridModeChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Grid mode selection changed."))); }
void UOptDlg::OnGunsightChanged(FString, ESelectInfo::Type) { if (description) description->SetText(FText::FromString(TEXT("Gunsight selection changed."))); }

// ---------------------------------------------------------------------
// Apply / Cancel (legacy behavior):

void UOptDlg::Apply()
{
    if (closed)
        return;

    PlayerCharacter* player = PlayerCharacter::GetCurrentPlayer();
    if (player)
    {
        if (flight_model)  player->SetFlightModel(flight_model->GetSelectedIndex());
        if (flying_start)  player->SetFlyingStart(flying_start->GetSelectedIndex());
        if (landings)      player->SetLandingModel(landings->GetSelectedIndex());

        if (ai_difficulty)
            player->SetAILevel(ai_difficulty->GetOptionCount() - ai_difficulty->GetSelectedIndex() - 1);

        if (hud_mode)      player->SetHUDMode(hud_mode->GetSelectedIndex());
        if (hud_color)     player->SetHUDColor(hud_color->GetSelectedIndex());
        if (ff_mode)       player->SetFriendlyFire(ff_mode->GetSelectedIndex());
        if (grid_mode)     player->SetGridMode(grid_mode->GetSelectedIndex());
        if (gunsight)      player->SetGunsight(gunsight->GetSelectedIndex());

        PlayerCharacter::Save();
    }

    if (flight_model) Ship::SetFlightModel(flight_model->GetSelectedIndex());
    if (landings)     Ship::SetLandingModel(landings->GetSelectedIndex());
    if (hud_mode)     HUDView::SetArcade(hud_mode->GetSelectedIndex() > 0);
    if (hud_color)    HUDView::SetDefaultColorSet(hud_color->GetSelectedIndex());

    if (ff_mode)
        Ship::SetFriendlyFireLevel((float)ff_mode->GetSelectedIndex() / 4.0f);

    HUDView* hud = HUDView::GetInstance();
    if (hud && hud_color)
        hud->SetHUDColorSet(hud_color->GetSelectedIndex());

    closed = true;
}

void UOptDlg::Cancel()
{
    closed = true;
}
