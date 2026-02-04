/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         VideoDlg.cpp
    AUTHOR:       Carlos Bott
*/

#include "VideoDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"

// Starshatter core (ported):
#include "Game.h"
#include "Starshatter.h"
#include "Terrain.h"
#include "CameraManager.h"
#include "Video.h"
#include "VideoSettings.h"

// Host/router:
#include "OptionsScreen.h"   // <-- your renamed OptDlg host (router)

#include <stdio.h>

UVideoDlg::UVideoDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Enable unified input (Enter/Escape) if your UBaseScreen uses it:
    SetDialogInputEnabled(true);

    StarsInstance = Starshatter::GetInstance();
}

void UVideoDlg::NativeConstruct()
{
    Super::NativeConstruct();

    StarsInstance = Starshatter::GetInstance();
    OrigGamma = Game::GammaLevel();

    // Hook buttons:
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

    // Hook tabs:
    if (VidTabButton)
    {
        VidTabButton->OnClicked.RemoveAll(this);
        VidTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnVideoClicked);
    }

    if (AudTabButton)
    {
        AudTabButton->OnClicked.RemoveAll(this);
        AudTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnAudioClicked);
    }

    if (CtlTabButton)
    {
        CtlTabButton->OnClicked.RemoveAll(this);
        CtlTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnControlsClicked);
    }

    if (OptTabButton)
    {
        OptTabButton->OnClicked.RemoveAll(this);
        OptTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnOptionsClicked);
    }

    if (ModTabButton)
    {
        ModTabButton->OnClicked.RemoveAll(this);
        ModTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnModClicked);
    }

    // Hook combos/sliders:
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

    if (GammaSlider)
    {
        GammaSlider->OnValueChanged.RemoveAll(this);
        GammaSlider->OnValueChanged.AddDynamic(this, &UVideoDlg::OnGammaChanged);
    }

    // First open init:
    if (bClosed)
    {
        RefreshSelectionsFromRuntime();
        bClosed = false;
    }
}

void UVideoDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame(InDeltaTime);
}

void UVideoDlg::BindFormWidgets()
{
    // If you're using legacy id mapping:
    // BindButton(1, ApplyBtn);
    // BindButton(2, CancelBtn);
    //
    // BindButton(901, VidTabButton);
    // BindButton(902, AudTabButton);
    // BindButton(903, CtlTabButton);
    // BindButton(904, OptTabButton);
    // BindButton(905, ModTabButton);
    //
    // BindCombo(203, ModeCombo); etc.
}

FString UVideoDlg::GetLegacyFormText() const
{
    // If you embedded VidDlg.frm text, return it here.
    return FString();
}

void UVideoDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);

    if (bClosed)
    {
        RefreshSelectionsFromRuntime();
        bClosed = false;
    }
}

void UVideoDlg::ExecFrame(float DeltaTime)
{
    (void)DeltaTime;
    // Legacy VidDlg didn't need per-frame logic.
}

// ---------------------------------------------------------------------
// Centralized Enter/Escape via UBaseScreen

void UVideoDlg::HandleAccept()
{
    OnApplyClicked();
}

void UVideoDlg::HandleCancel()
{
    OnCancelClicked();
}

// ---------------------------------------------------------------------
// Modern modes (32-bit only) + current mode selection

void UVideoDlg::BuildModeList()
{
    if (!ModeCombo)
        return;

    ModeCombo->ClearOptions();
    SelectedMode = 0;

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

    // 32-bit only:
    for (const FMode& M : Modes)
    {
        if (Game::DisplayModeSupported(M.W, M.H, 32))
        {
            ModeCombo->AddOption(FString::Printf(TEXT("%d x %d"), M.W, M.H));
        }
    }

    Video* VideoObj = Game::GetVideo();
    if (!StarsInstance || !VideoObj)
        return;

    int32 W = VideoObj->Width();
    int32 H = VideoObj->Height();

    const FString Current = FString::Printf(TEXT("%d x %d"), W, H);

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

void UVideoDlg::RefreshSelectionsFromRuntime()
{
    bool bFullscreen = true;

    if (StarsInstance)
    {
        const int32 MaxTex = StarsInstance->MaxTexSize();

        SelectedTexSize = 0;
        for (int32 i = 0; i < 7; ++i)
        {
            const int32 Pow2 = (int32)FMath::Pow(2.0f, (float)(i + 6)); // 64..8192
            if (MaxTex <= Pow2)
            {
                SelectedTexSize = i;
                break;
            }
        }

        Video* VideoObj = Game::GetVideo();
        if (VideoObj)
        {
            if (ShadowsCombo)  ShadowsCombo->SetSelectedIndex(VideoObj->IsShadowEnabled() ? 1 : 0);
            if (SpecMapsCombo) SpecMapsCombo->SetSelectedIndex(VideoObj->IsSpecMapEnabled() ? 1 : 0);
            if (BumpMapsCombo) BumpMapsCombo->SetSelectedIndex(VideoObj->IsBumpMapEnabled() ? 1 : 0);

            bFullscreen = VideoObj->IsFullScreen();
        }

        if (LensFlareCombo) LensFlareCombo->SetSelectedIndex(StarsInstance->LensFlare() ? 1 : 0);
        if (CoronaCombo)    CoronaCombo->SetSelectedIndex(StarsInstance->Corona() ? 1 : 0);
        if (NebulaCombo)    NebulaCombo->SetSelectedIndex(StarsInstance->Nebula() ? 1 : 0);
        if (DustCombo)      DustCombo->SetSelectedIndex(StarsInstance->Dust());
    }

    SelectedDetail = Terrain::DetailLevel() - 2;
    SelectedTexture = 1;

    if (ModeCombo)
    {
        BuildModeList();
        ModeCombo->SetSelectedIndex(SelectedMode);
        ModeCombo->SetIsEnabled(bFullscreen);
    }

    if (TexSizeCombo) TexSizeCombo->SetSelectedIndex(SelectedTexSize);
    if (DetailCombo)  DetailCombo->SetSelectedIndex(SelectedDetail);
    if (TextureCombo) TextureCombo->SetSelectedIndex(SelectedTexture);

    if (GammaSlider)
    {
        OrigGamma = Game::GammaLevel();
        const float Clamped = (float)FMath::Clamp(OrigGamma, 32, 224);
        GammaSlider->SetValue((Clamped - 32.0f) / (224.0f - 32.0f));
    }
}

// ---------------------------------------------------------------------
// Apply / Cancel surface (for old callsites)

void UVideoDlg::Apply() { ApplySettings(); }
void UVideoDlg::Cancel() { CancelSettings(); }

// ---------------------------------------------------------------------
// Operations

void UVideoDlg::ApplySettings()
{
    if (bClosed)
        return;

    int32 W = 1920;
    int32 H = 1080;
    const int32 D = 32; // 32-bit only

    int32 G = 128;
    int32 T = 2048;
    float Bias = 0.0f;

    FString ModeDesc;
    if (ModeCombo)
        ModeDesc = ModeCombo->GetSelectedOption();

    // Parse "W x H"
    {
        int32 XPos = INDEX_NONE;
        if (ModeDesc.FindChar(TEXT('x'), XPos))
        {
            const FString Left = ModeDesc.Left(XPos).TrimStartAndEnd();
            const FString Right = ModeDesc.Mid(XPos + 1).TrimStartAndEnd();
            W = FCString::Atoi(*Left);
            H = FCString::Atoi(*Right);
        }
    }

    if (SelectedTexSize > 0)
        T = (int32)FMath::Pow(2.0f, (float)(SelectedTexSize + 6)); // 64..8192

    bool bVideoChange = false;

    Video* VideoObj = Game::GetVideo();
    if (VideoObj)
    {
        const VideoSettings* VS = VideoObj->GetVideoSettings();
        if (VS)
            Bias = VS->depth_bias;

        if (VideoObj->IsFullScreen())
        {
            if (VideoObj->Width() != W) bVideoChange = true;
            if (VideoObj->Height() != H) bVideoChange = true;
            if (VideoObj->Depth() != D) bVideoChange = true;
        }
        else if (VS)
        {
            // Windowed: keep legacy fullscreen_mode as the reference
            W = VS->fullscreen_mode.width;
            H = VS->fullscreen_mode.height;
        }

        if (Game::MaxTexSize() != T)
            bVideoChange = true;
    }

    if (GammaSlider)
    {
        const float V01 = FMath::Clamp(GammaSlider->GetValue(), 0.0f, 1.0f);
        const float Mapped = 32.0f + V01 * (224.0f - 32.0f);
        int32 Snapped = (int32)(FMath::RoundToInt(Mapped / 16.0f) * 16);
        Snapped = FMath::Clamp(Snapped, 32, 224);
        G = Snapped;
    }

    const char* FileName = bVideoChange ? "video2.cfg" : "video.cfg";
    FILE* F = nullptr;
    fopen_s(&F, FileName, "w");

    auto ComboEnabled = [](UComboBoxString* C) -> bool
        {
            if (!C) return false;
            return C->GetSelectedIndex() != 0;
        };

    if (F)
    {
        fprintf(F, "VIDEO\n\n");
        fprintf(F, "width:     %4d\n", W);
        fprintf(F, "height:    %4d\n", H);
        fprintf(F, "depth:     %4d\n", 32);
        fprintf(F, "\n");
        fprintf(F, "max_tex:   %d\n", (int)FMath::Pow(2.0f, (float)(6 + SelectedTexSize)));
        fprintf(F, "primary3D: true\n");
        fprintf(F, "gamma:     %4d\n", G);
        fprintf(F, "\n");
        fprintf(F, "terrain_detail_level:   %d\n", SelectedDetail + 2);
        fprintf(F, "terrain_texture_enable: %s\n", (SelectedTexture ? "true" : "false"));
        fprintf(F, "\n");
        fprintf(F, "shadows:   %s\n", ComboEnabled(ShadowsCombo) ? "true" : "false");
        fprintf(F, "spec_maps: %s\n", ComboEnabled(SpecMapsCombo) ? "true" : "false");
        fprintf(F, "bump_maps: %s\n", ComboEnabled(BumpMapsCombo) ? "true" : "false");
        fprintf(F, "bias:      %f\n", Bias);
        fprintf(F, "\n");
        fprintf(F, "flare:     %s\n", ComboEnabled(LensFlareCombo) ? "true" : "false");
        fprintf(F, "corona:    %s\n", ComboEnabled(CoronaCombo) ? "true" : "false");
        fprintf(F, "nebula:    %s\n", ComboEnabled(NebulaCombo) ? "true" : "false");
        fprintf(F, "dust:      %d\n", DustCombo ? DustCombo->GetSelectedIndex() : 0);

        if (CameraManager::GetRangeLimit() != 300e3f)
            fprintf(F, "   cam_range_max: %f,\n", CameraManager::GetRangeLimit());

        fclose(F);
    }

    if (StarsInstance)
    {
        if (bVideoChange)
            StarsInstance->RequestChangeVideo();
        else
            StarsInstance->LoadVideoConfig("video.cfg");
    }

    bClosed = true;
}

void UVideoDlg::CancelSettings()
{
    Game::SetGammaLevel(OrigGamma);
    bClosed = true;
}

// ---------------------------------------------------------------------
// Selection handlers

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
}

void UVideoDlg::OnGammaChanged(float Value)
{
    const float V01 = FMath::Clamp(Value, 0.0f, 1.0f);
    const float Mapped = 32.0f + V01 * (224.0f - 32.0f);
    int32 Snapped = (int32)(FMath::RoundToInt(Mapped / 16.0f) * 16);
    Snapped = FMath::Clamp(Snapped, 32, 224);

    Game::SetGammaLevel(Snapped);
}

// ---------------------------------------------------------------------
// Buttons

void UVideoDlg::OnApplyClicked()
{
    // Route through host:
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

void UVideoDlg::OnAudioClicked() { if (Manager) Manager->ShowAudDlg(); }
void UVideoDlg::OnVideoClicked() { if (Manager) Manager->ShowVidDlg(); }
void UVideoDlg::OnOptionsClicked() { if (Manager) Manager->ShowOptDlg(); }
void UVideoDlg::OnControlsClicked() { if (Manager) Manager->ShowCtlDlg(); }
void UVideoDlg::OnModClicked() { if (Manager) Manager->ShowModDlg(); }
