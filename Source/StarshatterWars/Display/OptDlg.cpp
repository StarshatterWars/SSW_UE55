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
        // Build the legacy FORM as a UMG widget tree:
        // (Implemented inline below as BuildFromLegacyForm-equivalent)
        if (!WidgetTree)
            return;

        // Root: Canvas
        UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
        WidgetTree->RootWidget = RootCanvas;

        // Frame1 background (Frame1.pcx):
        UImage* Frame1 = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Frame1"));
        {
            UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(Frame1);
            Slot->SetAnchors(FAnchors(0, 0, 1, 1));
            Slot->SetOffsets(FMargin(0, 0, 0, 0));
            // TODO: assign brush from "Frame1.pcx"
        }

        // Overlay for Frame2a/Frame2b + UI:
        UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
        {
            UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(RootOverlay);
            Slot->SetAnchors(FAnchors(0, 0, 1, 1));
            Slot->SetOffsets(FMargin(0, 0, 0, 0));
        }

        // Frame2a:
        UImage* Frame2a = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Frame2a"));
        {
            UOverlaySlot* Slot = RootOverlay->AddChildToOverlay(Frame2a);
            Slot->SetHorizontalAlignment(HAlign_Fill);
            Slot->SetVerticalAlignment(VAlign_Fill);
            // TODO: assign brush "Frame2a"
        }

        // Frame2b:
        UImage* Frame2b = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Frame2b"));
        {
            UOverlaySlot* Slot = RootOverlay->AddChildToOverlay(Frame2b);
            Slot->SetHorizontalAlignment(HAlign_Fill);
            Slot->SetVerticalAlignment(VAlign_Fill);
            // TODO: assign brush "Frame2b"
        }

        // Main Grid (approximates legacy layout + margins):
        UGridPanel* MainGrid = WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass(), TEXT("MainGrid"));
        {
            UOverlaySlot* Slot = RootOverlay->AddChildToOverlay(MainGrid);
            Slot->SetHorizontalAlignment(HAlign_Fill);
            Slot->SetVerticalAlignment(VAlign_Fill);
            MainGrid->SetPadding(FMargin(10, 10, 64, 8));
        }

        // Title label "Options":
        UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title_Options"));
        TitleText->SetText(FText::FromString(TEXT("Options")));
        {
            UGridSlot* Slot = MainGrid->AddChildToGrid(TitleText, 0, 0);
            Slot->SetColumnSpan(6);
            Slot->SetPadding(FMargin(0, 0, 0, 8));
            Slot->SetHorizontalAlignment(HAlign_Left);
            Slot->SetVerticalAlignment(VAlign_Top);
        }

        // Tabs row:
        UHorizontalBox* TabsRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TabsRow"));
        {
            UGridSlot* Slot = MainGrid->AddChildToGrid(TabsRow, 1, 0);
            Slot->SetColumnSpan(6);
            Slot->SetPadding(FMargin(0, 8, 0, 8));
            Slot->SetHorizontalAlignment(HAlign_Fill);
            Slot->SetVerticalAlignment(VAlign_Top);
        }

        auto MakeTab = [&](const TCHAR* Name, const TCHAR* Label) -> UButton*
            {
                UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
                UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *(FString(Name) + TEXT("_Text")));
                Txt->SetText(FText::FromString(Label));
                Btn->AddChild(Txt);
                return Btn;
            };

        vid_btn = MakeTab(TEXT("Tab_Video"), TEXT("Video"));
        aud_btn = MakeTab(TEXT("Tab_Audio"), TEXT("Audio"));
        ctl_btn = MakeTab(TEXT("Tab_Controls"), TEXT("Controls"));
        opt_btn = MakeTab(TEXT("Tab_Gameplay"), TEXT("Gameplay"));
        mod_btn = MakeTab(TEXT("Tab_Mod"), TEXT("Mod Config"));

        auto AddTab = [&](UButton* Btn)
            {
                UHorizontalBoxSlot* Slot = TabsRow->AddChildToHorizontalBox(Btn);
                Slot->SetPadding(FMargin(4, 0, 4, 0));
                Slot->SetSize(ESlateSizeRule::Automatic);
                Slot->SetHorizontalAlignment(HAlign_Left);
                Slot->SetVerticalAlignment(VAlign_Fill);
            };

        AddTab(vid_btn);
        AddTab(aud_btn);
        AddTab(ctl_btn);
        AddTab(opt_btn);
        AddTab(mod_btn);

        // Main panel:
        UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MainPanelBorder"));
        {
            UGridSlot* Slot = MainGrid->AddChildToGrid(PanelBorder, 2, 0);
            Slot->SetColumnSpan(6);
            Slot->SetPadding(FMargin(12, 12, 12, 0));
            Slot->SetHorizontalAlignment(HAlign_Fill);
            Slot->SetVerticalAlignment(VAlign_Fill);
            // TODO: Panel brush
        }

        UGridPanel* PanelGrid = WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass(), TEXT("PanelGrid"));
        PanelBorder->SetContent(PanelGrid);

        auto MakeLabel = [&](const TCHAR* Name, const TCHAR* Text) -> UTextBlock*
            {
                UTextBlock* L = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
                L->SetText(FText::FromString(Text));
                return L;
            };

        auto MakeCombo = [&](const TCHAR* Name) -> UComboBoxString*
            {
                UComboBoxString* C = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), Name);
                return C;
            };

        // Left side labels (rows):
        struct FRowDef { const TCHAR* LabelName; const TCHAR* LabelText; const TCHAR* ComboName; int32 Row; };

        const FRowDef Rows[] =
        {
            { TEXT("Lbl_FlightModel"),  TEXT("Flight Model:"),   TEXT("Cmb_FlightModel"),  0 },
            { TEXT("Lbl_FlyingStart"),  TEXT("Flying Start:"),   TEXT("Cmb_FlyingStart"),  1 },
            { TEXT("Lbl_Landings"),     TEXT("Landings:"),       TEXT("Cmb_Landings"),     2 },
            { TEXT("Lbl_AIDifficulty"), TEXT("AI Difficulty:"),  TEXT("Cmb_AIDifficulty"), 3 },
            { TEXT("Lbl_HudMode"),      TEXT("HUD Mode:"),       TEXT("Cmb_HudMode"),      4 },
            { TEXT("Lbl_HudColor"),     TEXT("HUD Color:"),      TEXT("Cmb_HudColor"),     5 },
            { TEXT("Lbl_FriendlyFire"), TEXT("Friendly Fire:"),  TEXT("Cmb_FriendlyFire"), 6 },
            { TEXT("Lbl_GridMode"),     TEXT("Reference Grid:"), TEXT("Cmb_GridMode"),     7 },
            { TEXT("Lbl_Gunsight"),     TEXT("Gunsight:"),       TEXT("Cmb_Gunsight"),     8 },
        };

        for (const FRowDef& R : Rows)
        {
            UTextBlock* L = MakeLabel(R.LabelName, R.LabelText);
            UComboBoxString* C = MakeCombo(R.ComboName);

            UGridSlot* LSlot = PanelGrid->AddChildToGrid(L, R.Row, 0);
            LSlot->SetPadding(FMargin(0, 4, 10, 4));
            LSlot->SetHorizontalAlignment(HAlign_Left);
            LSlot->SetVerticalAlignment(VAlign_Center);

            UGridSlot* CSlot = PanelGrid->AddChildToGrid(C, R.Row, 1);
            CSlot->SetPadding(FMargin(0, 4, 0, 4));
            CSlot->SetHorizontalAlignment(HAlign_Fill);
            CSlot->SetVerticalAlignment(VAlign_Center);
        }

        // Right side description (legacy id 500):
        description = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Description"));
        {
            UGridSlot* Slot = PanelGrid->AddChildToGrid(description, 0, 2);
            Slot->SetRowSpan(9);
            Slot->SetPadding(FMargin(20, 0, 0, 0));
            Slot->SetHorizontalAlignment(HAlign_Fill);
            Slot->SetVerticalAlignment(VAlign_Fill);
        }

        // Assign combo refs by name:
        flight_model = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_FlightModel")));
        flying_start = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_FlyingStart")));
        landings = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_Landings")));
        ai_difficulty = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_AIDifficulty")));
        hud_mode = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_HudMode")));
        hud_color = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_HudColor")));
        ff_mode = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_FriendlyFire")));
        grid_mode = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_GridMode")));
        gunsight = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_Gunsight")));

        // Populate items (from legacy FORM):
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

        // Bottom buttons:
        UHorizontalBox* BottomRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BottomRow"));
        {
            UGridSlot* Slot = MainGrid->AddChildToGrid(BottomRow, 3, 0);
            Slot->SetColumnSpan(6);
            Slot->SetPadding(FMargin(0, 12, 0, 0));
            Slot->SetHorizontalAlignment(HAlign_Right);
            Slot->SetVerticalAlignment(VAlign_Bottom);
        }

        auto MakeAction = [&](const TCHAR* Name, const TCHAR* Text) -> UButton*
            {
                UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
                UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *(FString(Name) + TEXT("_Text")));
                Txt->SetText(FText::FromString(Text));
                Btn->AddChild(Txt);
                return Btn;
            };

        ApplyBtn = MakeAction(TEXT("ApplyBtn"), TEXT("Apply"));
        CancelBtn = MakeAction(TEXT("CancelBtn"), TEXT("Cancel"));

        {
            UHorizontalBoxSlot* SlotA = BottomRow->AddChildToHorizontalBox(ApplyBtn);
            SlotA->SetPadding(FMargin(8, 0, 0, 0));
            SlotA->SetSize(ESlateSizeRule::Automatic);

            UHorizontalBoxSlot* SlotC = BottomRow->AddChildToHorizontalBox(CancelBtn);
            SlotC->SetPadding(FMargin(8, 0, 0, 0));
            SlotC->SetSize(ESlateSizeRule::Automatic);
        }
    }

    // Bind action buttons:
    if (ApplyBtn)
        ApplyBtn->OnClicked.AddDynamic(this, &UOptDlg::OnApplyClicked);

    if (CancelBtn)
        CancelBtn->OnClicked.AddDynamic(this, &UOptDlg::OnCancelClicked);

    // Bind tabs:
    if (vid_btn)
        vid_btn->OnClicked.AddDynamic(this, &UOptDlg::OnVideoClicked);

    if (aud_btn)
        aud_btn->OnClicked.AddDynamic(this, &UOptDlg::OnAudioClicked);

    if (ctl_btn)
        ctl_btn->OnClicked.AddDynamic(this, &UOptDlg::OnControlsClicked);

    if (opt_btn)
        opt_btn->OnClicked.AddDynamic(this, &UOptDlg::OnOptionsClicked);

    if (mod_btn)
        mod_btn->OnClicked.AddDynamic(this, &UOptDlg::OnModClicked);

    // Bind combo changes (optional):
    if (flight_model)  flight_model->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnFlightModelChanged);
    if (flying_start)  flying_start->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnFlyingStartChanged);
    if (landings)      landings->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnLandingsChanged);
    if (ai_difficulty) ai_difficulty->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnAIDifficultyChanged);
    if (hud_mode)      hud_mode->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnHudModeChanged);
    if (hud_color)     hud_color->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnHudColorChanged);
    if (ff_mode)       ff_mode->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnFfModeChanged);
    if (grid_mode)     grid_mode->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnGridModeChanged);
    if (gunsight)      gunsight->OnSelectionChanged.AddDynamic(this, &UOptDlg::OnGunsightChanged);

    // Mirror legacy Show() initialization:
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
void UOptDlg::OnModClicked() { if (manager) manager->ShowModDlg();  else UE_LOG(LogTemp, Warning, TEXT("OptDlg: manager null (Mod).")); }

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

        Player::Save();
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
