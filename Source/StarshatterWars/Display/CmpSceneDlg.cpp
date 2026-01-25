/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         CmpSceneDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UCmpSceneDlg implementation.
    - Loads flare textures (legacy flare0+/flare2/flare3/flare4).
    - Attaches/detaches cutscene view to the UMG host container.
    - Advances subtitle lines using mission begin/end scene timing.
*/
#include "CmpSceneDlg.h"

#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/RichTextBlock.h"
#include "Engine/Texture2D.h"

UCmpSceneDlg::UCmpSceneDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UCmpSceneDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Make sure we can receive key input (Enter/Escape) if desired:
    bIsFocusable = true;
    SetDialogInputEnabled(true);

    // Load flare textures once (legacy did this in constructor after Init(def)):
    LoadFlareTextures();
}

void UCmpSceneDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bActiveCutscene)
        return;

    // Maintain a dialog-local time base:
    ElapsedSeconds += InDeltaTime;

    ExecFrame(InDeltaTime);
}

void UCmpSceneDlg::BindFormWidgets()
{
    // Legacy FORM had ctrl id 101 as a "label" acting as a host window.
    // In Unreal we are not binding id->panel yet; BaseScreen doesn't include Panel binding.
    // This dialog uses SceneHostBorder/SceneHostOverlay directly via BindWidgetOptional.
}

FString UCmpSceneDlg::GetLegacyFormText() const
{
    // Embedded legacy .frm (as provided). Note: it only defines ctrl 101.
    return TEXT(R"FORM(
form: {
   back_color: (0,0,0)

   layout: {
      x_mins:     (0)
      x_weights:  (1)

      y_mins:     (0, 200, 0)
      y_weights:  (1,   6, 1)
   }

   ctrl: {
      id:            101
      type:          label
      cells:         (0,1,1,1)
      hide_partial:  false
      fore_color:       (0,0,0)
      back_color:       (0,0,0)
      cell_insets:      (0,0,0,0)
   }
}
)FORM");
}

void UCmpSceneDlg::Show()
{
    bActiveCutscene = false;
    ElapsedSeconds = 0.0;
    SubtitlesDelay = 0.0;
    SubtitlesTime = 0.0;
    CurrentSubtitleLine = 0;
    SubtitleLines.Reset();

    if (!IsInCutscene())
    {
        RequestShowCmdDlg();
        return;
    }

    // Attach render view(s) to host:
    AttachCutsceneViewToHost();

    // Initialize subtitles:
    InitializeSubtitles();

    bActiveCutscene = true;
}

void UCmpSceneDlg::Hide()
{
    bActiveCutscene = false;

    DetachCutsceneViewFromHost();

    // Clear subtitles UI if you want:
    if (SubtitlesText)
    {
        SubtitlesText->SetText(FText::GetEmpty());
    }

    SubtitleLines.Reset();
    CurrentSubtitleLine = 0;
}

void UCmpSceneDlg::ExecFrame(float DeltaTime)
{
    (void)DeltaTime;

    // Legacy:
    // - disp_view->ExecFrame()
    // - subtitle auto-scroll based on begin/end scene events

    // If you have a UDisplayView equivalent, tick it here:
    // if (DispView) { DispView->ExecFrame(); }

    if (!SubtitlesText || SubtitleLines.Num() <= 0)
        return;

    // On first frame, compute delay from mission timing:
    if (SubtitlesDelay == 0.0)
    {
        const int32 NumLines = SubtitleLines.Num();

        double BeginT = 0.0;
        double EndT = 0.0;

        if (NumLines > 0 && GetCutsceneSceneTimeRange(BeginT, EndT))
        {
            const double TotalTime = EndT - BeginT;
            if (TotalTime > 0.0)
            {
                SubtitlesDelay = TotalTime / (double)NumLines;
                SubtitlesTime = ElapsedSeconds + SubtitlesDelay;
            }
            else
            {
                SubtitlesDelay = -1.0;
            }
        }
        else
        {
            // Legacy: subtitles_delay = -1
            SubtitlesDelay = -1.0;
        }
    }

    // Advance lines:
    if (SubtitlesDelay > 0.0 && ElapsedSeconds >= SubtitlesTime)
    {
        SubtitlesTime = ElapsedSeconds + SubtitlesDelay;
        AdvanceSubtitleLine();
    }
}

void UCmpSceneDlg::InitializeSubtitles()
{
    const FString SubText = GetCutsceneSubtitles();
    BuildSubtitleLines(SubText);

    // Show initial text:
    if (SubtitlesText)
    {
        SubtitlesText->SetText(FText::FromString(SubText));
    }

    SubtitlesDelay = 0.0;
    SubtitlesTime = 0.0;
    CurrentSubtitleLine = 0;
}

void UCmpSceneDlg::BuildSubtitleLines(const FString& InText)
{
    SubtitleLines.Reset();

    // Keep it simple: split on \n; trim \r.
    TArray<FString> Lines;
    InText.ParseIntoArrayLines(Lines, /*CullEmpty*/ false);

    for (FString& L : Lines)
    {
        L.ReplaceInline(TEXT("\r"), TEXT(""));
        SubtitleLines.Add(L);
    }
}

void UCmpSceneDlg::AdvanceSubtitleLine()
{
    // Legacy behavior: subtitles_box->Scroll(SCROLL_DOWN)
    // In Unreal RichTextBlock has no line-scroll API, so we emulate by dropping the first line.

    if (SubtitleLines.Num() <= 0)
        return;

    if (CurrentSubtitleLine < SubtitleLines.Num())
    {
        ++CurrentSubtitleLine;
    }

    if (CurrentSubtitleLine >= SubtitleLines.Num())
        return;

    // Display from current line to end:
    FString Remaining;
    for (int32 i = CurrentSubtitleLine; i < SubtitleLines.Num(); ++i)
    {
        Remaining += SubtitleLines[i];
        if (i < SubtitleLines.Num() - 1)
            Remaining += TEXT("\n");
    }

    if (SubtitlesText)
    {
        SubtitlesText->SetText(FText::FromString(Remaining));
    }
}

void UCmpSceneDlg::RequestShowCmdDlg()
{
    // Legacy: manager->ShowCmdDlg();
    // Here: call manager if you have it, otherwise raise BP event.
    if (Manager)
    {
        // If UCmpnScreen exposes ShowCmdDlg() as UFUNCTION, call it here.
        // Manager->ShowCmdDlg();
        OnRequestShowCmdDlg();
        return;
    }

    OnRequestShowCmdDlg();
}

// ---------------------------------------------------------------------
// Default hook implementations (override in your project)
// ---------------------------------------------------------------------

bool UCmpSceneDlg::IsInCutscene() const
{
    // Wire to your Starshatter singleton / subsystem.
    return false;
}

FString UCmpSceneDlg::GetCutsceneSubtitles() const
{
    // Wire to stars->GetSubtitles() equivalent.
    return FString();
}

bool UCmpSceneDlg::IsLensFlareEnabled() const
{
    return false;
}

bool UCmpSceneDlg::IsCoronaEnabled() const
{
    return false;
}

bool UCmpSceneDlg::GetCutsceneSceneTimeRange(double& OutBeginSeconds, double& OutEndSeconds) const
{
    // Wire to mission event timings:
    // begin_scene->Time(), end_scene->Time()
    // Return false if either missing.
    OutBeginSeconds = 0.0;
    OutEndSeconds = 0.0;
    return false;
}

void UCmpSceneDlg::AttachCutsceneViewToHost()
{
    // This is where you attach your 3D view to the host panel.
    // Typical Unreal patterns:
    // - Add a UViewport widget or CommonUI viewport
    // - Use a SceneCapture2D into a render target and show in an Image
    // - Or your existing DisplayView/CamView wrapper that renders into a widget

    // If you already have a render widget, you would do:
    // SceneHostOverlay->AddChild(RenderWidget);

    // Lens flare settings can be applied to your camera system here, using Flare1..4.
}

void UCmpSceneDlg::DetachCutsceneViewFromHost()
{
    // Remove your view widget and restore previous output routing (legacy old_disp_win).
}

void UCmpSceneDlg::LoadFlareTextures()
{
    // Legacy loaded:
    // flare0+.pcx, flare2.pcx, flare3.pcx, flare4.pcx
    // Here we load textures by name via hook. Override to use your DataLoader/PCX bridge.

    Flare1 = LoadFlareTextureByName(TEXT("flare0+"));
    Flare2 = LoadFlareTextureByName(TEXT("flare2"));
    Flare3 = LoadFlareTextureByName(TEXT("flare3"));
    Flare4 = LoadFlareTextureByName(TEXT("flare4"));
}

UTexture2D* UCmpSceneDlg::LoadFlareTextureByName(const FString& Name) const
{
    // Default: expects cooked assets in /Game/Textures/ (change to your real content path).
    // Example: /Game/Textures/flare2.flare2
    // If you are still using PCX on disk, override and use your DataLoader -> UTexture2D path.

    const FString AssetPath = FString::Printf(TEXT("/Game/Textures/%s.%s"), *Name, *Name);
    return LoadObject<UTexture2D>(nullptr, *AssetPath);
}
