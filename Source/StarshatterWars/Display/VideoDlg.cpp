#include "VideoDlg.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"

#include "Game.h"
#include "Starshatter.h"
#include "Terrain.h"
#include "CameraManager.h"
#include "Video.h"
#include "VideoSettings.h"

UVideoDlg::UVideoDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    StarsInstance = Starshatter::GetInstance();
}

void UVideoDlg::InitializeVideoDlg(UBaseScreen* InManager)
{
    Manager = InManager;
}

void UVideoDlg::NativeConstruct()
{
    Super::NativeConstruct();

    StarsInstance = Starshatter::GetInstance();
    OrigGamma = Game::GammaLevel();

    // Optional: if you still want to support FORM-id lookup fallback,
    // keep BindFormWidgets() and have it fill anything still null.
    BindFormWidgets();

    if (ApplyButton)
    {
        ApplyButton->OnClicked.Clear();
        ApplyButton->OnClicked.AddDynamic(this, &UVideoDlg::OnApplyClicked);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.Clear();
        CancelButton->OnClicked.AddDynamic(this, &UVideoDlg::OnCancelClicked);
    }

    if (VidTabButton)
    {
        VidTabButton->OnClicked.Clear();
        VidTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnVideoClicked);
    }

    if (AudTabButton)
    {
        AudTabButton->OnClicked.Clear();
        AudTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnAudioClicked);
    }

    if (CtlTabButton)
    {
        CtlTabButton->OnClicked.Clear();
        CtlTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnControlsClicked);
    }

    if (OptTabButton)
    {
        OptTabButton->OnClicked.Clear();
        OptTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnOptionsClicked);
    }

    if (ModTabButton)
    {
        ModTabButton->OnClicked.Clear();
        ModTabButton->OnClicked.AddDynamic(this, &UVideoDlg::OnModClicked);
    }

    if (ModeCombo)
    {
        ModeCombo->OnSelectionChanged.Clear();
        ModeCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnModeChanged);
    }

    if (TexSizeCombo)
    {
        TexSizeCombo->OnSelectionChanged.Clear();
        TexSizeCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTexSizeChanged);
    }

    if (DetailCombo)
    {
        DetailCombo->OnSelectionChanged.Clear();
        DetailCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnDetailChanged);
    }

    if (TextureCombo)
    {
        TextureCombo->OnSelectionChanged.Clear();
        TextureCombo->OnSelectionChanged.AddDynamic(this, &UVideoDlg::OnTextureChanged);
    }

    if (GammaSlider)
    {
        GammaSlider->OnValueChanged.Clear();
        GammaSlider->OnValueChanged.AddDynamic(this, &UVideoDlg::OnGammaChanged);
    }
}

void UVideoDlg::BindFormWidgets()
{
    // You requested standard widget bindings; keep this OPTIONAL.
    // If you do not want FORM-id fallback, you can leave this empty.

    // Example fallback pattern (only if still null):
    // if (!ApplyButton)  ApplyButton  = Cast<UButton>(FindWidgetById(1));
    // if (!CancelButton) CancelButton = Cast<UButton>(FindWidgetById(2));
    //
    // if (!VidTabButton) VidTabButton = Cast<UButton>(FindWidgetById(901));
    // ... etc ...
}

void
UVideoDlg::ExecFrame(float DeltaTime)
{
   
}

FString UVideoDlg::GetLegacyFormText() const
{
    return LegacyFormText;
}

void UVideoDlg::ShowVideoDlg()
{
    SetVisibility(ESlateVisibility::Visible);

    if (bClosed)
    {
        RefreshSelectionsFromRuntime();
        bClosed = false;
    }
}

void UVideoDlg::RefreshSelectionsFromRuntime()
{
    bool bFullscreen = true;

    SelectedRender = 9;
    SelectedCard = 0;

    if (StarsInstance)
    {
        const int32 MaxTex = StarsInstance->MaxTexSize();

        SelectedTexSize = 0;
        for (int32 i = 0; i < 7; ++i)
        {
            const int32 Pow2 = (int32)FMath::Pow(2.0f, (float)(i + 6));
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

void UVideoDlg::BuildModeList()
{
    if (!ModeCombo)
        return;

    ModeCombo->ClearOptions();
    SelectedMode = 0;

    auto AddIfSupported = [this](int32 W, int32 H, int32 D)
        {
            if (Game::DisplayModeSupported(W, H, D))
            {
                const FString S = FString::Printf(TEXT("%d x %d x %d"), W, H, D);
                ModeCombo->AddOption(S);
            }
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

    Video* VideoObj = Game::GetVideo();
    if (!StarsInstance || !VideoObj)
        return;

    int32 W = VideoObj->Width();
    int32 H = VideoObj->Height();
    int32 D = VideoObj->Depth();
    if (D != 16 && D != 32) D = 32;

    const FString Current = FString::Printf(TEXT("%d x %d x %d"), W, H, D);

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

void UVideoDlg::ApplySettings()
{
    if (bClosed)
        return;

    int32 W = 800;
    int32 H = 600;
    int32 D = 32;
    int32 A = 1;
    int32 G = 128;
    int32 T = 2048;
    float Bias = 0.0f;

    FString ModeDesc;
    if (ModeCombo)
        ModeDesc = ModeCombo->GetSelectedOption();

    if (ModeDesc.Contains(TEXT("800 x 600"))) { W = 800; H = 600; }
    else if (ModeDesc.Contains(TEXT("1024 x 768"))) { W = 1024; H = 768; }
    else if (ModeDesc.Contains(TEXT("1152 x 864"))) { W = 1152; H = 864; }
    else if (ModeDesc.Contains(TEXT("1280 x 800"))) { W = 1280; H = 800; }
    else if (ModeDesc.Contains(TEXT("1280 x 960"))) { W = 1280; H = 960; }
    else if (ModeDesc.Contains(TEXT("1280 x 1024"))) { W = 1280; H = 1024; }
    else if (ModeDesc.Contains(TEXT("1440 x 900"))) { W = 1440; H = 900; }
    else if (ModeDesc.Contains(TEXT("1600 x 900"))) { W = 1600; H = 900; }
    else if (ModeDesc.Contains(TEXT("1600 x 1200"))) { W = 1600; H = 1200; }

    if (ModeDesc.Contains(TEXT("x 16"))) D = 16;
    else if (ModeDesc.Contains(TEXT("x 32"))) D = 32;

    if (SelectedTexSize > 0)
        T = (int32)FMath::Pow(2.0f, (float)(SelectedTexSize + 6));

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
            W = VS->fullscreen_mode.width;
            H = VS->fullscreen_mode.height;

            if (VS->fullscreen_mode.format == VideoMode::FMT_R5G6B5 ||
                VS->fullscreen_mode.format == VideoMode::FMT_R5G5B5)
                D = 16;
            else
                D = 32;
        }

        if (Game::MaxTexSize() != T)
            bVideoChange = true;
    }

    // Update toggles from combos (if present):
    // Shadows/spec/bump flags are written to config file only, as legacy did.

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
        fprintf(F, "depth:     %4d\n", D);
        fprintf(F, "\n");
        fprintf(F, "max_tex:   %d\n", (int)FMath::Pow(2.0f, (float)(6 + SelectedTexSize)));
        fprintf(F, "primary3D: %s\n", (A > 0) ? "true" : "false");
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

void UVideoDlg::OnModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (ModeCombo)
        SelectedMode = ModeCombo->GetSelectedIndex();
}

void UVideoDlg::OnTexSizeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (TexSizeCombo)
        SelectedTexSize = TexSizeCombo->GetSelectedIndex();
}

void UVideoDlg::OnDetailChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (DetailCombo)
        SelectedDetail = DetailCombo->GetSelectedIndex();
}

void UVideoDlg::OnTextureChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
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

void UVideoDlg::OnApplyClicked()
{
    ApplySettings();
}

void UVideoDlg::OnCancelClicked()
{
    CancelSettings();
}

void UVideoDlg::OnAudioClicked()
{
    if (Manager)
    {
        // Manager->ShowAudDlg();
    }
}

void UVideoDlg::OnVideoClicked()
{
    if (Manager)
    {
        // Manager->ShowVidDlg();
    }
    else
    {
        ShowVideoDlg();
    }
}

void UVideoDlg::OnOptionsClicked()
{
    if (Manager)
    {
        // Manager->ShowOptDlg();
    }
}

void UVideoDlg::OnControlsClicked()
{
    if (Manager)
    {
        // Manager->ShowCtlDlg();
    }
}

void UVideoDlg::OnModClicked()
{
    if (Manager)
    {
        // Manager->ShowModDlg();
    }
}
