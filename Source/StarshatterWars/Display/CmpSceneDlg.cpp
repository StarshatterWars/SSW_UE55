/*
    Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         CmpSceneDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UCmpSceneDlg
    - Campaign title card / load progress dialog
    - Cutscene camera host + optional subtitles scroller
    - UE version of legacy CmpSceneDlg (FormWindow) implemented as UBaseScreen
*/

#include "CmpSceneDlg.h"

#include "Components/CanvasPanel.h"
#include "Components/RichTextBlock.h"

#include "Starshatter.h"
#include "Sim.h"
#include "CameraManager.h"
#include "Mission.h"
#include "MissionEvent.h"
#include "Game.h"
#include "GameStructs.h"

// Your UE ports:
#include "CameraView.h"
#include "DisplayView.h"

static UTexture2D* LoadPinnedTexture(const TCHAR* Path)
{
    UTexture2D* Tex = LoadObject<UTexture2D>(nullptr, Path);
    if (Tex)
    {
        // Pin it since we are not using UPROPERTY:
        Tex->AddToRoot();
    }
    return Tex;
}

UCmpSceneDlg::UCmpSceneDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Do not touch widgets here; constructor is defaults only.
}

void UCmpSceneDlg::BindFormWidgets()
{
    // IMPORTANT:
    // Your BaseScreen binding model is ID-based, so you bind by hand here.
    // If you are using a UMG widget tree, you should FindWidget by name and bind it.

    // Example (replace with your actual widget names):
    // MovSceneHost = Cast<UCanvasPanel>(GetWidgetFromName(TEXT("MovSceneHost")));
    // SubtitlesBox = Cast<URichTextBlock>(GetWidgetFromName(TEXT("SubtitlesBox")));

    // If you're doing FORM-id binding, wire it here using your own helper(s).
    // (You already have BindText/BindLabel/etc. in BaseScreen.)
    // Since id 101 is not a label in UE, we treat it as a host panel:
    //
    // MovSceneHost = ... (your lookup)
    // SubtitlesBox = ... (your lookup)
}

void UCmpSceneDlg::NativeConstruct()
{
    Super::NativeConstruct();

    EnsureViewObjects();

    // Load flare textures once (paths are examples; use your content paths):
    if (!Flare1) Flare1 = LoadPinnedTexture(TEXT("/Game/UI/Textures/flare0_plus.flare0_plus"));
    if (!Flare2) Flare2 = LoadPinnedTexture(TEXT("/Game/UI/Textures/flare2.flare2"));
    if (!Flare3) Flare3 = LoadPinnedTexture(TEXT("/Game/UI/Textures/flare3.flare3"));
    if (!Flare4) Flare4 = LoadPinnedTexture(TEXT("/Game/UI/Textures/flare4.flare4"));
}

void UCmpSceneDlg::NativeDestruct()
{
    DetachViews();

    // Unpin textures:
    if (Flare1) { Flare1->RemoveFromRoot(); Flare1 = nullptr; }
    if (Flare2) { Flare2->RemoveFromRoot(); Flare2 = nullptr; }
    if (Flare3) { Flare3->RemoveFromRoot(); Flare3 = nullptr; }
    if (Flare4) { Flare4->RemoveFromRoot(); Flare4 = nullptr; }

    Super::NativeDestruct();
}

void UCmpSceneDlg::ShowScreen()
{
    // Legacy Show() equivalent. Your Menu/Campaign screen manager should call this.
    SetVisibility(ESlateVisibility::Visible);

    AttachViewsForCutscene();

    // Reset subtitles timing when shown:
    SubtitlesDelay = 0.0;
    SubtitlesNextTime = 0.0;

    // Set subtitles text from Starshatter if available:
    if (SubtitlesBox)
    {
        if (Starshatter* Stars = Starshatter::GetInstance())
        {
            SubtitlesBox->SetText(Stars->GetSubtitlesAsText());
        }
    }
}

void UCmpSceneDlg::HideScreen()
{
    DetachViews();
    SetVisibility(ESlateVisibility::Hidden);
}

void UCmpSceneDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    Starshatter* Stars = Starshatter::GetInstance();
    if (!Stars)
        return;

    Mission* CutsceneMission = Stars->GetCutsceneMission();
    if (!CutsceneMission || !DispView)
    {
        // Legacy behavior: return to command dialog
        // manager->ShowCmdDlg();
        // In UE, your owning screen should do this; BaseScreen should not hard-reference manager objects.
        return;
    }

    // Drive the display view (legacy disp_view->ExecFrame()):
    DispView->ExecFrame(InDeltaTime);

    const double NowSeconds = FPlatformTime::Seconds();
    UpdateSubtitlesScroll(NowSeconds);
}

void UCmpSceneDlg::EnsureViewObjects()
{
    if (!CamView)
    {
        CamView = NewObject<UCameraView>(this);
    }

    if (!DispView)
    {
        DispView = DisplayView::GetInstance(); 
    }
}

void UCmpSceneDlg::AttachViewsForCutscene()
{
    if (bWasAttached)
        return;

    Starshatter* Stars = Starshatter::GetInstance();
    if (!Stars || !Stars->InCutscene())
        return;

    EnsureViewObjects();

    Sim* Sim = Sim::GetSim();
    if (Sim && CamView)
    {
        CameraManager* CamDir = CameraManager::GetInstance();
        CamView->UseCamera(CamDir ? CamDir->GetCamera() : nullptr);
        CamView->UseScene(Sim->GetScene());
    }

    // Lens flare / corona policy mirrors legacy:
    if (CamView)
    {
        if (Stars->LensFlare())
        {
            CamView->LensFlareElements(Flare1, Flare4, Flare2, Flare3);
            CamView->LensFlare(true);
        }
        else if (Stars->Corona())
        {
            CamView->LensFlareElements(Flare1, nullptr, nullptr, nullptr);
            CamView->LensFlare(true);
        }
        else
        {
            CamView->LensFlare(false);
        }
    }

    // Attach DisplayView to the host:
    if (DispView && MovSceneHost)
    {
        // Replace these calls with your actual DisplayView host API:
        OldDispHost = DispView->GetHost();
        DispView->SetHost(MovSceneHost);
        DispView->Attach();
    }

    bWasAttached = true;
}

void UCmpSceneDlg::DetachViews()
{
    if (!bWasAttached)
        return;

    if (DispView)
    {
        DispView->Detach();

        if (OldDispHost)
        {
            DispView->SetHost(OldDispHost);
            OldDispHost = nullptr;
        }
    }

    bWasAttached = false;
}

void UCmpSceneDlg::UpdateSubtitlesScroll(double NowSeconds)
{
    if (!SubtitlesBox)
        return;

    // You’ll need equivalent “line count” and “scroll down” in UE.
    // If your subtitles are lines separated by \n, a minimal approach is to
    // progressively reveal lines instead of physically scrolling a RichTextBlock.
    //
    // If you already implemented a “RichTextBox” port with ScrollDown(), call that here.

    Starshatter* Stars = Starshatter::Get();
    Mission* CutsceneMission = Stars ? Stars->GetCutsceneMission() : nullptr;
    if (!CutsceneMission)
        return;

    // Only compute delay once:
    if (SubtitlesDelay == 0.0)
    {
        const int32 LineCount = 0; // TODO: provide your own line-counting method

        MissionEvent* BeginScene = CutsceneMission->FindEvent(EMissionEvent::BEGIN_SCENE);
        MissionEvent* EndScene = CutsceneMission->FindEvent(EMissionEvent::END_SCENE);

        if (BeginScene && EndScene && LineCount > 0)
        {
            const double TotalTime = EndScene->Time() - BeginScene->Time();
            SubtitlesDelay = TotalTime / (double)LineCount;
            SubtitlesNextTime = NowSeconds + SubtitlesDelay;
        }
        else
        {
            SubtitlesDelay = -1.0;
        }
    }

    if (SubtitlesDelay > 0.0 && SubtitlesNextTime <= NowSeconds)
    {
        SubtitlesNextTime = NowSeconds + SubtitlesDelay;

        // TODO:
        // SubtitlesBox->ScrollDown();   // if you have your legacy port
        // OR progressively reveal the next line in the text.
    }
}
