/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         VidDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Main Menu Video Dialog (Unreal UUserWidget)
    Integrated legacy VidDlg.frm as a pure C++ WidgetTree build (no networking).
*/

#include "GameStructs.h"

#include "VidDlg.h"

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
#include "Components/Slider.h"

// Input:
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Manager screen (dialog switching + apply/cancel options):
#include "MenuScreen.h"

// Starshatter core/game-side logic (kept as-is):
#include "Starshatter.h"
#include "Ship.h"
#include "Terrain.h"
#include "CameraManager.h"   // renamed from CameraDirector (per project rule)
#include "Video.h"
#include "VideoSettings.h"
#include "Game.h"

// +--------------------------------------------------------------------+

UVidDlg::UVidDlg(const FObjectInitializer& ObjectInitializer)
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

static FString MakeModeString(int32 W, int32 H, int32 D)
{
    return FString::Printf(TEXT("%d x %d x %d"), W, H, D);
}

void UVidDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache stars instance (legacy behavior):
    Starshatter* Stars = Starshatter::GetInstance();
    if (Stars && orig_gamma == 0)
    {
        orig_gamma = Game::GammaLevel();
    }

    // Build widget tree if not authored in UMG:
    const bool bNeedsBuild =
        (!ApplyBtn && !CancelBtn && !mode && WidgetTree && !WidgetTree->RootWidget);

    if (bNeedsBuild)
    {
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

        // Main Grid (approximates legacy margins/layout):
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
                return WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), Name);
            };

        // Left column labels + combos (legacy ids preserved conceptually):
        struct FRowDef { const TCHAR* LabelName; const TCHAR* LabelText; const TCHAR* ComboName; int32 Row; };

        // We map to a simpler grid: col0 label, col1 control, col2 label, col3 control.
        // Rows align to legacy: video mode, max tex, shadows, spec, bump, (gap), terrain detail, terrain texture
        const FRowDef LeftRows[] =
        {
            { TEXT("Lbl_Mode"),        TEXT("Video Mode:"),       TEXT("Cmb_Mode"),        0 },
            { TEXT("Lbl_MaxTex"),      TEXT("Max Texture Size:"), TEXT("Cmb_TexSize"),     1 },
            { TEXT("Lbl_Shadows"),     TEXT("Shadows:"),          TEXT("Cmb_Shadows"),     2 },
            { TEXT("Lbl_SpecMaps"),    TEXT("Spec Maps:"),        TEXT("Cmb_SpecMaps"),    3 },
            { TEXT("Lbl_BumpMaps"),    TEXT("Bump Maps:"),        TEXT("Cmb_BumpMaps"),    4 },
            { TEXT("Lbl_TerrainDet"),  TEXT("Terrain Detail:"),   TEXT("Cmb_Detail"),      6 },
            { TEXT("Lbl_TerrainTex"),  TEXT("Terrain Texture:"),  TEXT("Cmb_Texture"),     7 },
        };

        for (const FRowDef& R : LeftRows)
        {
            UTextBlock* L = MakeLabel(R.LabelName, R.LabelText);
            UComboBoxString* C = MakeCombo(R.ComboName);

            UGridSlot* LSlot = PanelGrid->AddChildToGrid(L, R.Row, 0);
            LSlot->SetPadding(FMargin(0, 4, 10, 4));
            LSlot->SetHorizontalAlignment(HAlign_Left);
            LSlot->SetVerticalAlignment(VAlign_Center);

            UGridSlot* CSlot = PanelGrid->AddChildToGrid(C, R.Row, 1);
            CSlot->SetPadding(FMargin(0, 4, 20, 4));
            CSlot->SetHorizontalAlignment(HAlign_Fill);
            CSlot->SetVerticalAlignment(VAlign_Center);
        }

        // Right column labels + combos + gamma:
        struct FRightRowDef { const TCHAR* LabelName; const TCHAR* LabelText; const TCHAR* ControlName; int32 Row; bool bSlider; };
        const FRightRowDef RightRows[] =
        {
            { TEXT("Lbl_Flare"),   TEXT("Lens Flare:"), TEXT("Cmb_LensFlare"), 0, false },
            { TEXT("Lbl_Corona"),  TEXT("Corona:"),     TEXT("Cmb_Corona"),    1, false },
            { TEXT("Lbl_Nebula"),  TEXT("Nebula:"),     TEXT("Cmb_Nebula"),    2, false },
            { TEXT("Lbl_Dust"),    TEXT("Space Dust:"), TEXT("Cmb_Dust"),      3, false },
            { TEXT("Lbl_Gamma"),   TEXT("Gamma Level:"),TEXT("Sld_Gamma"),     6, true  },
        };

        for (const FRightRowDef& R : RightRows)
        {
            UTextBlock* L = MakeLabel(R.LabelName, R.LabelText);

            UGridSlot* LSlot = PanelGrid->AddChildToGrid(L, R.Row, 2);
            LSlot->SetPadding(FMargin(0, 4, 10, 4));
            LSlot->SetHorizontalAlignment(HAlign_Left);
            LSlot->SetVerticalAlignment(VAlign_Center);

            if (R.bSlider)
            {
                gamma = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), R.ControlName);
                UGridSlot* SSlot = PanelGrid->AddChildToGrid(gamma, R.Row, 3);
                SSlot->SetPadding(FMargin(0, 4, 0, 4));
                SSlot->SetHorizontalAlignment(HAlign_Fill);
                SSlot->SetVerticalAlignment(VAlign_Center);
            }
            else
            {
                UComboBoxString* C = MakeCombo(R.ControlName);
                UGridSlot* CSlot = PanelGrid->AddChildToGrid(C, R.Row, 3);
                CSlot->SetPadding(FMargin(0, 4, 0, 4));
                CSlot->SetHorizontalAlignment(HAlign_Fill);
                CSlot->SetVerticalAlignment(VAlign_Center);
            }
        }

        // Gamma test image placeholder:
        UImage* GammaTest = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("GammaTest"));
        {
            UGridSlot* Slot = PanelGrid->AddChildToGrid(GammaTest, 7, 3);
            Slot->SetPadding(FMargin(0, 4, 0, 4));
            Slot->SetHorizontalAlignment(HAlign_Fill);
            Slot->SetVerticalAlignment(VAlign_Fill);
            // TODO: assign brush "gamma_test"
        }

        // Assign widget refs:
        mode = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_Mode")));
        tex_size = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_TexSize")));
        shadows = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_Shadows")));
        spec_maps = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_SpecMaps")));
        bump_maps = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_BumpMaps")));
        detail = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_Detail")));
        texture = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_Texture")));

        lens_flare = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_LensFlare")));
        corona = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_Corona")));
        nebula = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_Nebula")));
        dust = Cast<UComboBoxString>(WidgetTree->FindWidget(TEXT("Cmb_Dust")));

        // Populate static combos from legacy FORM:
        if (tex_size)
        {
            AddComboItem(tex_size, TEXT("64 x 64"));
            AddComboItem(tex_size, TEXT("128 x 128"));
            AddComboItem(tex_size, TEXT("256 x 256"));
            AddComboItem(tex_size, TEXT("512 x 512"));
            AddComboItem(tex_size, TEXT("1024 x 1024"));
            AddComboItem(tex_size, TEXT("2048 x 2048"));
            AddComboItem(tex_size, TEXT("4096 x 4096"));
        }

        auto FillDisableEnable = [&](UComboBoxString* C)
            {
                if (!C) return;
                AddComboItem(C, TEXT("Disable"));
                AddComboItem(C, TEXT("Enable"));
            };

        FillDisableEnable(shadows);
        FillDisableEnable(spec_maps);
        FillDisableEnable(bump_maps);

        FillDisableEnable(lens_flare);
        FillDisableEnable(corona);
        FillDisableEnable(nebula);

        if (dust)
        {
            AddComboItem(dust, TEXT("None"));
            AddComboItem(dust, TEXT("Some"));
            AddComboItem(dust, TEXT("Lots"));
        }

        if (detail)
        {
            AddComboItem(detail, TEXT("Low"));
            AddComboItem(detail, TEXT("Medium"));
            AddComboItem(detail, TEXT("High"));
        }

        if (texture)
        {
            AddComboItem(texture, TEXT("Disable"));
            AddComboItem(texture, TEXT("Enable"));
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

    // Bind buttons:
    if (ApplyBtn)
        ApplyBtn->OnClicked.AddDynamic(this, &UVidDlg::OnApplyClicked);

    if (CancelBtn)
        CancelBtn->OnClicked.AddDynamic(this, &UVidDlg::OnCancelClicked);

    // Bind tabs:
    if (vid_btn)
        vid_btn->OnClicked.AddDynamic(this, &UVidDlg::OnVideoClicked);

    if (aud_btn)
        aud_btn->OnClicked.AddDynamic(this, &UVidDlg::OnAudioClicked);

    if (ctl_btn)
        ctl_btn->OnClicked.AddDynamic(this, &UVidDlg::OnControlsClicked);

    if (opt_btn)
        opt_btn->OnClicked.AddDynamic(this, &UVidDlg::OnOptionsClicked);

    if (mod_btn)
        mod_btn->OnClicked.AddDynamic(this, &UVidDlg::OnModClicked);

    // Bind selection handlers:
    if (mode)      mode->OnSelectionChanged.AddDynamic(this, &UVidDlg::OnModeChanged);
    if (tex_size)  tex_size->OnSelectionChanged.AddDynamic(this, &UVidDlg::OnTexSizeChanged);
    if (detail)    detail->OnSelectionChanged.AddDynamic(this, &UVidDlg::OnDetailChanged);
    if (texture)   texture->OnSelectionChanged.AddDynamic(this, &UVidDlg::OnTextureChanged);

    if (lens_flare) lens_flare->OnSelectionChanged.AddDynamic(this, &UVidDlg::OnTextureChanged);
    if (corona)     corona->OnSelectionChanged.AddDynamic(this, &UVidDlg::OnTextureChanged);
    if (nebula)     nebula->OnSelectionChanged.AddDynamic(this, &UVidDlg::OnTextureChanged);
    if (dust)       dust->OnSelectionChanged.AddDynamic(this, &UVidDlg::OnTextureChanged);

    if (shadows)   shadows->OnSelectionChanged.AddDynamic(this, &UVidDlg::OnTextureChanged);
    if (spec_maps) spec_maps->OnSelectionChanged.AddDynamic(this, &UVidDlg::OnTextureChanged);
    if (bump_maps) bump_maps->OnSelectionChanged.AddDynamic(this, &UVidDlg::OnTextureChanged);

    if (gamma)
    {
        gamma->SetMinValue(0.0f);
        gamma->SetMaxValue(1.0f);
        gamma->OnValueChanged.AddDynamic(this, &UVidDlg::OnGammaChanged);
    }

    // Mirror legacy Show() initialization:
    if (closed)
    {
        Starshatter* Stars = Starshatter::GetInstance();

        bool bFullscreen = true;

        if (Stars)
        {
            selected_render = 9;
            selected_card = 0;

            const int32 MaxTex = Stars->MaxTexSize();
            selected_tex_size = 0;

            // Legacy: choose i where MaxTex <= 2^(i+6)
            for (int32 i = 0; i < 7; i++)
            {
                const int32 Candidate = (int32)FMath::Pow(2.0f, (float)(i + 6));
                if (MaxTex <= Candidate)
                {
                    selected_tex_size = i;
                    break;
                }
            }

            Video* video = Game::GetVideo();
            if (video)
            {
                if (shadows)   ClearAndSetSelection(shadows, video->IsShadowEnabled() ? 1 : 0);
                if (spec_maps) ClearAndSetSelection(spec_maps, video->IsSpecMapEnabled() ? 1 : 0);
                if (bump_maps) ClearAndSetSelection(bump_maps, video->IsBumpMapEnabled() ? 1 : 0);

                bFullscreen = video->IsFullScreen();
            }

            if (lens_flare) ClearAndSetSelection(lens_flare, Stars->LensFlare() ? 1 : 0);
            if (corona)     ClearAndSetSelection(corona, Stars->Corona() ? 1 : 0);
            if (nebula)     ClearAndSetSelection(nebula, Stars->Nebula() ? 1 : 0);
            if (dust)       ClearAndSetSelection(dust, Stars->Dust());
        }

        selected_detail = Terrain::DetailLevel() - 2;
        selected_texture = 1;

        if (mode)
        {
            BuildModeList();
            ClearAndSetSelection(mode, selected_mode);

            // Legacy: disable mode switching in windowed
            mode->SetIsEnabled(bFullscreen);
        }

        if (tex_size)  ClearAndSetSelection(tex_size, selected_tex_size);
        if (detail)    ClearAndSetSelection(detail, selected_detail);
        if (texture)   ClearAndSetSelection(texture, selected_texture);

        if (gamma)
        {
            orig_gamma = Game::GammaLevel();

            // Map 0..255 gamma into slider 0..1
            gamma->SetValue(FMath::Clamp((float)orig_gamma / 255.0f, 0.0f, 1.0f));
        }
    }

    closed = false;
}

void UVidDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

FReply UVidDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
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
// Selection changed handlers:

void UVidDlg::OnModeChanged(FString, ESelectInfo::Type)
{
    if (mode)
        selected_mode = mode->GetSelectedIndex();
}

void UVidDlg::OnTexSizeChanged(FString, ESelectInfo::Type)
{
    if (tex_size)
        selected_tex_size = tex_size->GetSelectedIndex();
}

void UVidDlg::OnDetailChanged(FString, ESelectInfo::Type)
{
    if (detail)
        selected_detail = detail->GetSelectedIndex();
}

void UVidDlg::OnTextureChanged(FString, ESelectInfo::Type)
{
    if (texture)
        selected_texture = texture->GetSelectedIndex();
}

void UVidDlg::OnGammaChanged(float Value)
{
    // Legacy slider used 32..224 stepping 16, stored in 0..255 range.
    // Here we keep a simple linear mapping 0..1 -> 0..255.
    const int32 G = FMath::Clamp((int32)FMath::RoundToInt(Value * 255.0f), 0, 255);
    Game::SetGammaLevel(G);
}

// ---------------------------------------------------------------------
// Tabs:

void UVidDlg::OnAudioClicked() { if (manager) manager->ShowAudDlg(); else UE_LOG(LogTemp, Warning, TEXT("VidDlg: manager null (Audio).")); }
void UVidDlg::OnVideoClicked() { if (manager) manager->ShowVidDlg(); else UE_LOG(LogTemp, Warning, TEXT("VidDlg: manager null (Video).")); }
void UVidDlg::OnOptionsClicked() { if (manager) manager->ShowOptDlg(); else UE_LOG(LogTemp, Warning, TEXT("VidDlg: manager null (Options).")); }
void UVidDlg::OnControlsClicked() { if (manager) manager->ShowCtlDlg(); else UE_LOG(LogTemp, Warning, TEXT("VidDlg: manager null (Controls).")); }
void UVidDlg::OnModClicked() { if (manager) manager->ShowModDlg(); else UE_LOG(LogTemp, Warning, TEXT("VidDlg: manager null (Mod).")); }

// ---------------------------------------------------------------------
// Apply/Cancel click:

void UVidDlg::OnApplyClicked()
{
    if (manager) manager->ApplyOptions();
    else UE_LOG(LogTemp, Warning, TEXT("VidDlg: manager null (Apply)."));
}

void UVidDlg::OnCancelClicked()
{
    if (manager) manager->CancelOptions();
    else UE_LOG(LogTemp, Warning, TEXT("VidDlg: manager null (Cancel)."));
}

// ---------------------------------------------------------------------
// Apply / Cancel (legacy behavior; file write replaced with UE_LOG + TODO config save):

void UVidDlg::Apply()
{
    if (closed)
        return;

    int32 W = 800;
    int32 H = 600;
    int32 D = 32;
    int32 G = 128;
    int32 T = 2048;
    float Bias = 0.0f;

    const FString ModeDesc = (mode && mode->GetSelectedOption().Len() > 0)
        ? mode->GetSelectedOption()
        : FString(TEXT("800 x 600 x 32"));

    // Parse "W x H x D":
    {
        int32 OutW = 0, OutH = 0, OutD = 0;
        if (FString(ModeDesc).Replace(TEXT(" "), TEXT("")).Split(TEXT("x"), nullptr, nullptr))
        {
            // Robust parse:
            TArray<FString> Parts;
            ModeDesc.ParseIntoArray(Parts, TEXT("x"), true);
            if (Parts.Num() >= 3)
            {
                OutW = FCString::Atoi(*Parts[0].TrimStartAndEnd());
                OutH = FCString::Atoi(*Parts[1].TrimStartAndEnd());
                OutD = FCString::Atoi(*Parts[2].TrimStartAndEnd());
            }
        }

        if (OutW > 0 && OutH > 0)
        {
            W = OutW;
            H = OutH;
        }

        if (OutD == 16 || OutD == 32)
            D = OutD;
    }

    if (selected_tex_size > 0)
        T = (int32)FMath::Pow(2.0f, (float)(selected_tex_size + 6));

    bool bVideoChange = false;

    Video* video = Game::GetVideo();
    if (video)
    {
        const VideoSettings* vs = video->GetVideoSettings();
        if (vs)
            Bias = vs->depth_bias;

        if (video->IsFullScreen())
        {
            if (video->Width() != W) bVideoChange = true;
            if (video->Height() != H) bVideoChange = true;
            if (video->Depth() != D) bVideoChange = true;
        }
        else if (vs)
        {
            // Mirror legacy fallback:
            W = vs->fullscreen_mode.width;
            H = vs->fullscreen_mode.height;

            // NOTE: We keep D as requested unless you expose VideoMode formats here.
        }

        if (Game::MaxTexSize() != T)
            bVideoChange = true;
    }

    if (gamma)
        G = FMath::Clamp((int32)FMath::RoundToInt(gamma->GetValue() * 255.0f), 0, 255);

    UE_LOG(LogTemp, Log, TEXT("VidDlg Apply: Mode=%s (W=%d H=%d D=%d) MaxTex=%d Gamma=%d"), *ModeDesc, W, H, D, T, G);
    UE_LOG(LogTemp, Log, TEXT("VidDlg Apply: TerrainDetail=%d TerrainTex=%s Shadows=%d Spec=%d Bump=%d Bias=%f Flare=%d Corona=%d Nebula=%d Dust=%d"),
        selected_detail + 2,
        selected_texture ? TEXT("true") : TEXT("false"),
        (shadows ? shadows->GetSelectedIndex() : 0),
        (spec_maps ? spec_maps->GetSelectedIndex() : 0),
        (bump_maps ? bump_maps->GetSelectedIndex() : 0),
        Bias,
        (lens_flare ? lens_flare->GetSelectedIndex() : 0),
        (corona ? corona->GetSelectedIndex() : 0),
        (nebula ? nebula->GetSelectedIndex() : 0),
        (dust ? dust->GetSelectedIndex() : 0)
    );

    // TODO: Replace legacy video.cfg file output with your Unreal config system.
    // Example: USSWGameInstance / SaveSubsystem / Config asset.

    Starshatter* Stars = Starshatter::GetInstance();
    if (Stars)
    {
        if (bVideoChange)
            Stars->RequestChangeVideo();
        else
            Stars->LoadVideoConfig("video.cfg");
    }

    closed = true;
}

void UVidDlg::Cancel()
{
    Game::SetGammaLevel(orig_gamma);
    closed = true;
}

// ---------------------------------------------------------------------
// BuildModeList (ported from legacy):

void UVidDlg::BuildModeList()
{
    if (!mode)
        return;

    mode->ClearOptions();
    selected_mode = 0;

    auto AddIfSupported = [&](int32 W, int32 H, int32 D)
        {
            if (Game::DisplayModeSupported(W, H, D))
                mode->AddOption(MakeModeString(W, H, D));
        };

    AddIfSupported(800, 600, 16);
    AddIfSupported(800, 600, 32);

    AddIfSupported(1024, 768, 16);
    AddIfSupported(1024, 768, 32);

    AddIfSupported(1152, 864, 16);
    AddIfSupported(1152, 864, 32);

    AddIfSupported(1280, 800, 16);
    AddIfSupported(1280, 800, 32);

    AddIfSupported(1280, 960, 16);
    AddIfSupported(1280, 960, 32);

    AddIfSupported(1280, 1024, 16);
    AddIfSupported(1280, 1024, 32);

    AddIfSupported(1440, 900, 16);
    AddIfSupported(1440, 900, 32);

    AddIfSupported(1600, 900, 16);
    AddIfSupported(1600, 900, 32);

    AddIfSupported(1600, 1200, 16);
    AddIfSupported(1600, 1200, 32);

    AddIfSupported(1680, 1050, 16);
    AddIfSupported(1680, 1050, 32);

    Video* video = Game::GetVideo();
    Starshatter* Stars = Starshatter::GetInstance();

    if (Stars && video)
    {
        int32 VW = video->Width();
        int32 VH = video->Height();
        int32 VD = video->Depth();

        // Rebuild mode_desc similarly to legacy branching:
        FString ModeDesc;
        switch (VW)
        {
        case 800:  ModeDesc = MakeModeString(800, 600, VD); break;

        default:
        case 1024: ModeDesc = MakeModeString(1024, 768, VD); break;

        case 1152: ModeDesc = MakeModeString(1152, 864, VD); break;

        case 1280:
            if (VH < 900)      ModeDesc = MakeModeString(1280, 800, VD);
            else if (VH < 1000)ModeDesc = MakeModeString(1280, 960, VD);
            else               ModeDesc = MakeModeString(1280, 1024, VD);
            break;

        case 1440: ModeDesc = MakeModeString(1440, 900, VD); break;

        case 1600:
            if (VH < 1000) ModeDesc = MakeModeString(1600, 900, VD);
            else           ModeDesc = MakeModeString(1600, 1200, VD);
            break;
        }

        const int32 Count = mode->GetOptionCount();
        for (int32 i = 0; i < Count; i++)
        {
            if (mode->GetOptionAtIndex(i) == ModeDesc)
            {
                selected_mode = i;
                break;
            }
        }
    }
}
