/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         VideoDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UVideoDlg
    - Refactored so the dialog only touches UStarshatterVideoSettings (config model).
    - No direct video.cfg writing from the dialog.
    - No subsystem calls from the dialog.
*/

#include "VideoDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"

// Model:
#include "StarshatterVideoSettings.h"

// Host/router:
#include "OptionsScreen.h"

UVideoDlg::UVideoDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetDialogInputEnabled(true);
}

void UVideoDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Buttons:
    if (ApplyBtn)
    {
        ApplyBtn->OnClicked.RemoveAll(this);
        ApplyBtn->OnClicked.AddDynamic(this, &UVideoDlg::OnApplyClicked);
    }

    if (CancelBtn)
    {
        CancelBtn->OnClicked.RemoveAll(this);
        CancelBtn->OnClicked.AddDynamic(this, &UVideoDlg::OnCancelClicked);
    }

    // Tabs:
    if (VidTabButton) { VidTabButton->OnClicked.RemoveAll(this); VidTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnVideoClicked); }
    if (AudTabButton) { AudTabButton->OnClicked.RemoveAll(this); AudTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnAudioClicked); }
    if (CtlTabButton) { CtlTabButton->OnClicked.RemoveAll(this); CtlTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnControlsClicked); }
    if (OptTabButton) { OptTabButton->OnClicked.RemoveAll(this); OptTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnOptionsClicked); }
    if (ModTabButton) { ModTabButton->OnClicked.RemoveAll(this); ModTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnModClicked); }

    // Combos:
    if (ModeCombo)
    {
        ModeCombo->OnSelectionChanged.RemoveAll(this);
        ModeCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnModeChanged);
    }

    if (TexSizeCombo)
    {
        TexSizeCombo->OnSelectionChanged.RemoveAll(this);
        TexSizeCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTexSizeChanged);
    }

    if (DetailCombo)
    {
        DetailCombo->OnSelectionChanged.RemoveAll(this);
        DetailCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnDetailChanged);
    }

    if (TextureCombo)
    {
        TextureCombo->OnSelectionChanged.RemoveAll(this);
        TextureCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTextureChanged);
    }

    if (LensFlareCombo)
    {
        LensFlareCombo->OnSelectionChanged.RemoveAll(this);
        LensFlareCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTextureChanged);
    }

    if (CoronaCombo)
    {
        CoronaCombo->OnSelectionChanged.RemoveAll(this);
        CoronaCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTextureChanged);
    }

    if (NebulaCombo)
    {
        NebulaCombo->OnSelectionChanged.RemoveAll(this);
        NebulaCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTextureChanged);
    }

    if (DustCombo)
    {
        DustCombo->OnSelectionChanged.RemoveAll(this);
        DustCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTextureChanged);
    }

    if (ShadowsCombo)
    {
        ShadowsCombo->OnSelectionChanged.RemoveAll(this);
        ShadowsCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTextureChanged);
    }

    if (SpecMapsCombo)
    {
        SpecMapsCombo->OnSelectionChanged.RemoveAll(this);
        SpecMapsCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTextureChanged);
    }

    if (BumpMapsCombo)
    {
        BumpMapsCombo->OnSelectionChanged.RemoveAll(this);
        BumpMapsCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTextureChanged);
    }

    // Slider:
    if (GammaSlider)
    {
        GammaSlider->OnValueChanged.RemoveAll(this);
        GammaSlider->OnValueChanged.AddDynamic(this, &UVideoDlg::OnGammaChanged);
    }

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }
}

void UVideoDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame((double)InDeltaTime);
}

void UVideoDlg::ExecFrame(double /*DeltaTime*/)
{
    // Optional per-frame UI logic
}

void UVideoDlg::BindFormWidgets() {}
FString UVideoDlg::GetLegacyFormText() const { return FString(); }

void UVideoDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }
}

void UVideoDlg::HandleAccept() { OnApplyClicked(); }
void UVideoDlg::HandleCancel() { OnCancelClicked(); }

// ---------------------------------------------------------------------
// Settings access
// ---------------------------------------------------------------------

UStarshatterVideoSettings* UVideoDlg::GetVideoSettings() const
{
    return GetMutableDefault<UStarshatterVideoSettings>();
}

// ---------------------------------------------------------------------
// Helpers (tex size mapping + gamma mapping)
// ---------------------------------------------------------------------

int32 UVideoDlg::TexSizePow2FromIndex(int32 Index)
{
    // index 0..7 => 64..8192
    const int32 Clamped = FMath::Clamp(Index, 0, 7);
    return (int32)FMath::Pow(2.0f, (float)(Clamped + 6));
}

int32 UVideoDlg::TexSizeIndexFromPow2(int32 Pow2)
{
    // 64..8192 => 0..7
    Pow2 = FMath::Clamp(Pow2, 64, 8192);
    int32 I = 0;
    int32 V = 64;
    while (V < Pow2 && I < 7)
    {
        V <<= 1;
        ++I;
    }
    return I;
}

float UVideoDlg::Gamma01FromLevel(int32 InGammaLevel)
{
    const float G = (float)FMath::Clamp(InGammaLevel, 32, 224);
    return (G - 32.0f) / (224.0f - 32.0f);
}

int32 UVideoDlg::GammaLevelFrom01(float V01)
{
    const float Clamped = FMath::Clamp(V01, 0.0f, 1.0f);
    const float Mapped = 32.0f + Clamped * (224.0f - 32.0f);
    int32 Snapped = (int32)(FMath::RoundToInt(Mapped / 16.0f) * 16);
    return FMath::Clamp(Snapped, 32, 224);
}

// ---------------------------------------------------------------------
// Mode list (pure UI list; selection stored in settings)
// ---------------------------------------------------------------------

void UVideoDlg::BuildModeList()
{
    if (!ModeCombo)
        return;

    ModeCombo->ClearOptions();
    SelectedMode = 0;

    // Keep your legacy curated list:
    struct FMode { int32 W; int32 H; };
    static const FMode Modes[] =
    {
        {1280, 720},
        {1366, 768},
        {1600, 900},
        {1920, 1080},
        {1920, 1200},
        {2560, 1440},
        {2560, 1600},
        {3440, 1440},
        {3840, 2160},
    };

    for (const FMode& M : Modes)
    {
        ModeCombo->AddOption(FString::Printf(TEXT("%d x %d"), M.W, M.H));
    }

    // Select current Width/Height from our model cache:
    const FString Current = FString::Printf(TEXT("%d x %d"), Width, Height);

    const int32 Count = ModeCombo->GetOptionCount();
    for (int32 i = 0; i < Count; ++i)
    {
        if (ModeCombo->GetOptionAtIndex(i) == Current)
        {
            SelectedMode = i;
            break;
        }
    }
}

// ---------------------------------------------------------------------
// Model -> UI
// ---------------------------------------------------------------------

void UVideoDlg::RefreshFromModel()
{
    UStarshatterVideoSettings* S = GetVideoSettings();
    if (!S) return;

    S->ReloadConfig();
    S->Sanitize();

    Width = S->GetWidth();
    Height = S->GetHeight();
    bFullscreen = S->GetFullscreen();

    SelectedTexSize = TexSizeIndexFromPow2(S->GetMaxTexSize());
    SelectedDetail = S->GetTerrainDetailLevel();
    SelectedTexture = S->GetTerrainTextureEnabled() ? 1 : 0;

    bShadows = S->GetShadows();
    bSpecMaps = S->GetSpecMaps();
    bBumpMaps = S->GetBumpMaps();

    bLensFlare = S->GetLensFlare();
    bCorona = S->GetCorona();
    bNebula = S->GetNebula();
    Dust = S->GetDust();

    GammaLevel = S->GetGammaLevel();
    DepthBias = S->GetDepthBias();

    // Push to widgets:
    if (ModeCombo)
    {
        BuildModeList();
        ModeCombo->SetSelectedIndex(SelectedMode);
        ModeCombo->SetIsEnabled(bFullscreen);
    }

    if (TexSizeCombo) TexSizeCombo->SetSelectedIndex(SelectedTexSize);
    if (DetailCombo)  DetailCombo->SetSelectedIndex(SelectedDetail);
    if (TextureCombo) TextureCombo->SetSelectedIndex(SelectedTexture);

    if (ShadowsCombo)  ShadowsCombo->SetSelectedIndex(bShadows ? 1 : 0);
    if (SpecMapsCombo) SpecMapsCombo->SetSelectedIndex(bSpecMaps ? 1 : 0);
    if (BumpMapsCombo) BumpMapsCombo->SetSelectedIndex(bBumpMaps ? 1 : 0);

    if (LensFlareCombo) LensFlareCombo->SetSelectedIndex(bLensFlare ? 1 : 0);
    if (CoronaCombo)    CoronaCombo->SetSelectedIndex(bCorona ? 1 : 0);
    if (NebulaCombo)    NebulaCombo->SetSelectedIndex(bNebula ? 1 : 0);
    if (DustCombo)      DustCombo->SetSelectedIndex(Dust);

    if (GammaSlider)
        GammaSlider->SetValue(Gamma01FromLevel(GammaLevel));
}

// ---------------------------------------------------------------------
// UI -> Model
// ---------------------------------------------------------------------

void UVideoDlg::PushToModel(bool bApplyRuntimeToo)
{
    UStarshatterVideoSettings* S = GetVideoSettings();
    if (!S) return;

    // Resolve mode selection to width/height:
    if (ModeCombo)
    {
        const FString ModeDesc = ModeCombo->GetSelectedOption();
        int32 XPos = INDEX_NONE;
        if (ModeDesc.FindChar(TEXT('x'), XPos))
        {
            const FString Left = ModeDesc.Left(XPos).TrimStartAndEnd();
            const FString Right = ModeDesc.Mid(XPos + 1).TrimStartAndEnd();
            Width = FCString::Atoi(*Left);
            Height = FCString::Atoi(*Right);
        }
    }

    S->SetWidth(Width);
    S->SetHeight(Height);
    S->SetFullscreen(bFullscreen);

    S->SetMaxTexSize(TexSizePow2FromIndex(SelectedTexSize));
    S->SetTerrainDetailLevel(SelectedDetail);
    S->SetTerrainTextureEnabled(SelectedTexture != 0);

    S->SetShadows(bShadows);
    S->SetSpecMaps(bSpecMaps);
    S->SetBumpMaps(bBumpMaps);

    S->SetLensFlare(bLensFlare);
    S->SetCorona(bCorona);
    S->SetNebula(bNebula);
    S->SetDust(Dust);

    S->SetGammaLevel(GammaLevel);
    S->SetDepthBias(DepthBias);

    S->Sanitize();
    S->Save();

    if (bApplyRuntimeToo)
        S->ApplyToRuntimeVideo(const_cast<UVideoDlg*>(this));
}

// ---------------------------------------------------------------------
// Apply / Cancel surface (for old callsites)
// ---------------------------------------------------------------------

void UVideoDlg::Apply() { ApplySettings(); }
void UVideoDlg::Cancel() { CancelSettings(); }

void UVideoDlg::ApplySettings()
{
    if (bClosed) return;
    PushToModel(true);
    bClosed = true;
}

void UVideoDlg::CancelSettings()
{
    RefreshFromModel();
    bClosed = true;
}

// ---------------------------------------------------------------------
// Selection handlers
// ---------------------------------------------------------------------

void UVideoDlg::OnModeChanged(FString, ESelectInfo::Type)
{
    if (ModeCombo)
        SelectedMode = ModeCombo->GetSelectedIndex();
}

void UVideoDlg::OnTexSizeChanged(FString, ESelectInfo::Type)
{
    if (TexSizeCombo)
        SelectedTexSize = TexSizeCombo->GetSelectedIndex();
}

void UVideoDlg::OnDetailChanged(FString, ESelectInfo::Type)
{
    if (DetailCombo)
        SelectedDetail = DetailCombo->GetSelectedIndex();
}

void UVideoDlg::OnTextureChanged(FString, ESelectInfo::Type)
{
    if (TextureCombo)
        SelectedTexture = TextureCombo->GetSelectedIndex();

    if (LensFlareCombo) bLensFlare = (LensFlareCombo->GetSelectedIndex() != 0);
    if (CoronaCombo)    bCorona = (CoronaCombo->GetSelectedIndex() != 0);
    if (NebulaCombo)    bNebula = (NebulaCombo->GetSelectedIndex() != 0);
    if (DustCombo)      Dust = DustCombo->GetSelectedIndex();

    if (ShadowsCombo)   bShadows = (ShadowsCombo->GetSelectedIndex() != 0);
    if (SpecMapsCombo)  bSpecMaps = (SpecMapsCombo->GetSelectedIndex() != 0);
    if (BumpMapsCombo)  bBumpMaps = (BumpMapsCombo->GetSelectedIndex() != 0);
}

void UVideoDlg::OnGammaChanged(float Value)
{
    GammaLevel = GammaLevelFrom01(Value);
    // IMPORTANT: dialog does NOT directly apply runtime gamma;
    // runtime apply occurs only on ApplySettings via settings->ApplyToRuntimeVideo().
}

// ---------------------------------------------------------------------
// Buttons
// ---------------------------------------------------------------------

void UVideoDlg::OnApplyClicked()
{
    if (Manager) Manager->ApplyOptions();
    else ApplySettings();
}

void UVideoDlg::OnCancelClicked()
{
    if (Manager) Manager->CancelOptions();
    else CancelSettings();
}

// ---------------------------------------------------------------------
// Tabs (route through host)
// ---------------------------------------------------------------------

void UVideoDlg::OnAudioClicked() { if (Manager) Manager->ShowAudDlg(); }
void UVideoDlg::OnVideoClicked() { if (Manager) Manager->ShowVidDlg(); }
void UVideoDlg::OnOptionsClicked() { if (Manager) Manager->ShowOptDlg(); }
void UVideoDlg::OnControlsClicked() { if (Manager) Manager->ShowCtlDlg(); }
void UVideoDlg::OnModClicked() { if (Manager) Manager->ShowModDlg(); }
