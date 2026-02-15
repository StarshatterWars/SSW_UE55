/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         VideoDlg.cpp
    AUTHOR:       Carlos Bott

    IMPLEMENTATION
    ==============
    UVideoDlg
    - Video options subpage for OptionsScreen.
    - Routes tabs through OptionsScreen.
    - Implements OptionsPage so OptionsScreen can Apply/Cancel uniformly.
*/

#include "VideoDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"

// Model:
#include "StarshatterVideoSettings.h"

// Runtime apply:
#include "StarshatterVideoSubsystem.h"

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
    if (AudTabButton) { AudTabButton->OnClicked.RemoveAll(this); AudTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnAudioClicked); }
    if (VidTabButton) { VidTabButton->OnClicked.RemoveAll(this); VidTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnVideoClicked); }
    if (CtlTabButton) { CtlTabButton->OnClicked.RemoveAll(this); CtlTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnControlsClicked); }
    if (OptTabButton) { OptTabButton->OnClicked.RemoveAll(this); OptTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnGameClicked); } // NOW = GAME
    if (ModTabButton) { ModTabButton->OnClicked.RemoveAll(this); ModTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnModClicked); }

    // Combos:
    if (ModeCombo) { ModeCombo->OnSelectionChanged.RemoveAll(this);       ModeCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnModeChanged); }
    if (TexSizeCombo) { TexSizeCombo->OnSelectionChanged.RemoveAll(this);    TexSizeCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTexSizeChanged); }
    if (DetailCombo) { DetailCombo->OnSelectionChanged.RemoveAll(this);     DetailCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnDetailChanged); }
    if (TextureCombo) { TextureCombo->OnSelectionChanged.RemoveAll(this);    TextureCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTextureChanged); }

    if (LensFlareCombo) { LensFlareCombo->OnSelectionChanged.RemoveAll(this);  LensFlareCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnLensFlareChanged); }
    if (CoronaCombo) { CoronaCombo->OnSelectionChanged.RemoveAll(this);     CoronaCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnCoronaChanged); }
    if (NebulaCombo) { NebulaCombo->OnSelectionChanged.RemoveAll(this);     NebulaCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnNebulaChanged); }
    if (DustCombo) { DustCombo->OnSelectionChanged.RemoveAll(this);       DustCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnDustChanged); }

    if (ShadowsCombo) { ShadowsCombo->OnSelectionChanged.RemoveAll(this);    ShadowsCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnShadowsChanged); }
    if (SpecMapsCombo) { SpecMapsCombo->OnSelectionChanged.RemoveAll(this);   SpecMapsCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnSpecMapsChanged); }
    if (BumpMapsCombo) { BumpMapsCombo->OnSelectionChanged.RemoveAll(this);   BumpMapsCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnBumpMapsChanged); }

    // Slider:
    if (GammaSlider)
    {
        GammaSlider->OnValueChanged.RemoveAll(this);
        GammaSlider->OnValueChanged.AddDynamic(this, &UVideoDlg::OnGammaChanged);
    }

    BuildListsIfNeeded();

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

void UVideoDlg::HandleAccept() { OnApplyClicked(); }
void UVideoDlg::HandleCancel() { OnCancelClicked(); }

void UVideoDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);

    BuildListsIfNeeded();

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }
}

// ------------------------------------------------------------
// IOptionsPage (OptionsScreen orchestration)
// ------------------------------------------------------------

void UVideoDlg::LoadFromSettings_Implementation()
{
    BuildListsIfNeeded();
    RefreshFromModel();
    bClosed = false;
}

void UVideoDlg::ApplySettings_Implementation()
{
    // “Apply” meaning runtime apply too:
    if (!bClosed)
        Apply();
}

void UVideoDlg::SaveSettings_Implementation()
{
    // Settings already saved in Apply(). Keep as no-op or call Apply(false) if you split semantics later.
}

void UVideoDlg::CancelChanges_Implementation()
{
    Cancel();
}

// -------------------------
// Settings / subsystem access
// -------------------------

UStarshatterVideoSettings* UVideoDlg::GetVideoSettings() const
{
    return GetMutableDefault<UStarshatterVideoSettings>();
}

UStarshatterVideoSubsystem* UVideoDlg::GetVideoSubsystem() const
{
    return UStarshatterVideoSubsystem::Get(this);
}

// -------------------------
// Lists
// -------------------------

const TArray<int32>& UVideoDlg::GetTexSizeOptions()
{
    static TArray<int32> Options = { 256, 512, 1024, 2048, 4096, 8192, 16384 };
    return Options;
}

int32 UVideoDlg::TexSizeIndexFromPow2(int32 Pow2)
{
    const TArray<int32>& Opt = GetTexSizeOptions();
    for (int32 i = 0; i < Opt.Num(); ++i)
    {
        if (Opt[i] == Pow2) return i;
    }

    // Fallback: closest
    int32 BestIdx = 0;
    int32 BestDist = INT32_MAX;
    for (int32 i = 0; i < Opt.Num(); ++i)
    {
        const int32 Dist = FMath::Abs(Opt[i] - Pow2);
        if (Dist < BestDist) { BestDist = Dist; BestIdx = i; }
    }
    return BestIdx;
}

int32 UVideoDlg::TexSizePow2FromIndex(int32 Index)
{
    const TArray<int32>& Opt = GetTexSizeOptions();
    return Opt.IsValidIndex(Index) ? Opt[Index] : 2048;
}

float UVideoDlg::Gamma01FromLevel(int32 InGammaLevel)
{
    const int32 G = ClampGammaLevel(InGammaLevel);
    return (float)(G - 32) / (float)(224 - 32);
}

int32 UVideoDlg::GammaLevelFrom01(float V01)
{
    const float C = FMath::Clamp(V01, 0.0f, 1.0f);
    const float G = 32.0f + (224.0f - 32.0f) * C;
    return ClampGammaLevel(FMath::RoundToInt(G));
}

void UVideoDlg::BuildListsIfNeeded()
{
    if (bListsBuilt)
        return;

    // Tex sizes:
    if (TexSizeCombo)
    {
        TexSizeCombo->ClearOptions();
        for (int32 Pow2 : GetTexSizeOptions())
            TexSizeCombo->AddOption(FString::FromInt(Pow2));
    }

    // Detail levels:
    if (DetailCombo)
    {
        DetailCombo->ClearOptions();
        DetailCombo->AddOption(TEXT("LOW"));
        DetailCombo->AddOption(TEXT("MEDIUM"));
        DetailCombo->AddOption(TEXT("HIGH"));
        DetailCombo->AddOption(TEXT("ULTRA"));
    }

    // Terrain textures:
    if (TextureCombo)
    {
        TextureCombo->ClearOptions();
        TextureCombo->AddOption(TEXT("OFF"));
        TextureCombo->AddOption(TEXT("ON"));
    }

    auto FillOnOff = [](UComboBoxString* Combo)
        {
            if (!Combo) return;
            Combo->ClearOptions();
            Combo->AddOption(TEXT("OFF"));
            Combo->AddOption(TEXT("ON"));
        };

    FillOnOff(LensFlareCombo);
    FillOnOff(CoronaCombo);
    FillOnOff(NebulaCombo);
    FillOnOff(ShadowsCombo);
    FillOnOff(SpecMapsCombo);
    FillOnOff(BumpMapsCombo);

    // Dust:
    if (DustCombo)
    {
        DustCombo->ClearOptions();
        DustCombo->AddOption(TEXT("0"));
        DustCombo->AddOption(TEXT("1"));
        DustCombo->AddOption(TEXT("2"));
        DustCombo->AddOption(TEXT("3"));
    }

    // Mode list (keep minimal/stable):
    if (ModeCombo)
    {
        ModeCombo->ClearOptions();
        ModeCombo->AddOption(TEXT("1920x1080"));
        ModeCombo->AddOption(TEXT("2560x1440"));
        ModeCombo->AddOption(TEXT("3840x2160"));
    }

    bListsBuilt = true;
}

// -------------------------
// Model -> UI
// -------------------------

void UVideoDlg::RefreshFromModel()
{
    UStarshatterVideoSettings* S = GetVideoSettings();
    if (!S) return;

    S->Load();
    const FStarshatterVideoConfig& C = S->GetConfig();

    Width = C.Width;
    Height = C.Height;
    bFullscreen = C.bFullscreen;

    MaxTextureSize = C.MaxTextureSize;
    GammaLevel = C.GammaLevel;

    bShadows = C.bShadows;
    bSpecularMaps = C.bSpecularMaps;
    bBumpMaps = C.bBumpMaps;

    bLensFlare = C.bLensFlare;
    bCorona = C.bCorona;
    bNebula = C.bNebula;

    DustLevel = C.DustLevel;
    TerrainDetailIndex = C.TerrainDetailIndex;
    bTerrainTextures = C.bTerrainTextures;

    if (GammaSlider)
        GammaSlider->SetValue(Gamma01FromLevel(GammaLevel));

    if (TexSizeCombo)
        TexSizeCombo->SetSelectedIndex(TexSizeIndexFromPow2(MaxTextureSize));

    if (DetailCombo)
        DetailCombo->SetSelectedIndex(FMath::Clamp(TerrainDetailIndex, 0, 3));

    if (TextureCombo)
        TextureCombo->SetSelectedIndex(bTerrainTextures ? 1 : 0);

    auto SetOnOff = [](UComboBoxString* Combo, bool bOn)
        {
            if (Combo) Combo->SetSelectedIndex(bOn ? 1 : 0);
        };

    SetOnOff(ShadowsCombo, bShadows);
    SetOnOff(SpecMapsCombo, bSpecularMaps);
    SetOnOff(BumpMapsCombo, bBumpMaps);

    SetOnOff(LensFlareCombo, bLensFlare);
    SetOnOff(CoronaCombo, bCorona);
    SetOnOff(NebulaCombo, bNebula);

    if (DustCombo)
        DustCombo->SetSelectedIndex(FMath::Clamp(DustLevel, 0, 3));

    if (ModeCombo)
        ModeCombo->SetSelectedOption(FString::Printf(TEXT("%dx%d"), Width, Height));
}

// -------------------------
// UI -> Model
// -------------------------

void UVideoDlg::PushToModel(bool bApplyRuntimeToo)
{
    UStarshatterVideoSettings* S = GetVideoSettings();
    if (!S) return;

    FStarshatterVideoConfig C = S->GetConfig();

    C.Width = Width;
    C.Height = Height;
    C.bFullscreen = bFullscreen;

    C.MaxTextureSize = MaxTextureSize;
    C.GammaLevel = ClampGammaLevel(GammaLevel);

    C.bShadows = bShadows;
    C.bSpecularMaps = bSpecularMaps;
    C.bBumpMaps = bBumpMaps;

    C.bLensFlare = bLensFlare;
    C.bCorona = bCorona;
    C.bNebula = bNebula;

    C.DustLevel = FMath::Clamp(DustLevel, 0, 3);
    C.TerrainDetailIndex = FMath::Clamp(TerrainDetailIndex, 0, 3);
    C.bTerrainTextures = bTerrainTextures;

    S->SetConfig(C);
    S->Save();

    if (bApplyRuntimeToo)
    {
        if (UStarshatterVideoSubsystem* VideoSS = GetVideoSubsystem())
            VideoSS->ApplySettingsToRuntime();
    }
}

void UVideoDlg::Apply()
{
    if (bClosed) return;
    PushToModel(true);
    bClosed = true;
}

void UVideoDlg::Cancel()
{
    RefreshFromModel();
    bClosed = true;
}

// -------------------------
// Handlers
// -------------------------

void UVideoDlg::OnModeChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/)
{
    if (!ModeCombo) return;

    FString Opt = ModeCombo->GetSelectedOption();
    FString L, R;
    if (Opt.Split(TEXT("x"), &L, &R))
    {
        Width = FCString::Atoi(*L);
        Height = FCString::Atoi(*R);
    }
}

void UVideoDlg::OnTexSizeChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/)
{
    if (!TexSizeCombo) return;
    MaxTextureSize = TexSizePow2FromIndex(TexSizeCombo->GetSelectedIndex());
}

void UVideoDlg::OnDetailChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/)
{
    if (!DetailCombo) return;
    TerrainDetailIndex = DetailCombo->GetSelectedIndex();
}

void UVideoDlg::OnTextureChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/)
{
    if (!TextureCombo) return;
    bTerrainTextures = (TextureCombo->GetSelectedIndex() == 1);
}

void UVideoDlg::OnGammaChanged(float Value)
{
    GammaLevel = GammaLevelFrom01(Value);
}

static bool ComboOn(UComboBoxString* Combo)
{
    return Combo && Combo->GetSelectedIndex() == 1;
}

void UVideoDlg::OnLensFlareChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bLensFlare = ComboOn(LensFlareCombo); }
void UVideoDlg::OnCoronaChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bCorona = ComboOn(CoronaCombo); }
void UVideoDlg::OnNebulaChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bNebula = ComboOn(NebulaCombo); }
void UVideoDlg::OnShadowsChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bShadows = ComboOn(ShadowsCombo); }
void UVideoDlg::OnSpecMapsChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bSpecularMaps = ComboOn(SpecMapsCombo); }
void UVideoDlg::OnBumpMapsChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/) { bBumpMaps = ComboOn(BumpMapsCombo); }

void UVideoDlg::OnDustChanged(FString /*SelectedItem*/, ESelectInfo::Type /*SelectionType*/)
{
    if (!DustCombo) return;
    DustLevel = FMath::Clamp(DustCombo->GetSelectedIndex(), 0, 3);
}

// Buttons:
void UVideoDlg::OnApplyClicked()
{
    if (OptionsManager) OptionsManager->ApplyOptions();
    else Apply();
}

void UVideoDlg::OnCancelClicked()
{
    if (OptionsManager) OptionsManager->CancelOptions();
    else Cancel();
}

// Tabs (route to OptionsScreen):
void UVideoDlg::OnAudioClicked() { if (OptionsManager) OptionsManager->ShowAudDlg(); }
void UVideoDlg::OnVideoClicked() { if (OptionsManager) OptionsManager->ShowVidDlg(); }
void UVideoDlg::OnControlsClicked() { if (OptionsManager) OptionsManager->ShowCtlDlg(); }
void UVideoDlg::OnGameClicked() { if (OptionsManager) OptionsManager->ShowGameDlg(); } // NEW
void UVideoDlg::OnModClicked() { if (OptionsManager) OptionsManager->ShowModDlg(); }
