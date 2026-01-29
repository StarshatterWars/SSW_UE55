/*
    Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         CmpSceneDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UCmpSceneDlg
    - Campaign title card / load progress dialog
    - Cutscene camera host + optional subtitles scroller
    - UE version of legacy CmpSceneDlg (FormWindow) implemented as UBaseScreen
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "CmpSceneDlg.generated.h"

class UCanvasPanel;
class URichTextBlock;

class UCameraView;    // your ported camera view (UObject recommended)
class UDisplayView;   // your ported display view (UObject recommended)

UCLASS()
class STARSHATTERWARS_API UCmpSceneDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UCmpSceneDlg(const FObjectInitializer& ObjectInitializer);

    // UBaseScreen
    virtual void BindFormWidgets() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
    // Legacy operations equivalents:
    void ShowScreen();     // legacy Show()
    void HideScreen();     // legacy Hide()

    CameraView* GetCameraView() const { return CamView; }
    UDisplayView* GetDisplayView() const { return DispView; }

private:
    void EnsureViewObjects();
    void AttachViewsForCutscene();
    void DetachViews();
    void UpdateSubtitlesScroll(double NowSeconds);

    // FORM bindings:
    // id 101 = movie scene host (legacy label used as a view host)
    // id 102 = subtitles box (legacy RichTextBox)
    UCanvasPanel* MovSceneHost = nullptr;   // preferred as a panel host
    URichTextBlock* SubtitlesBox = nullptr;

private:
    // View objects:
    CameraView* CamView = nullptr;
    DisplayView* DispView = nullptr;

    // If your DisplayView still needs to remember old window/host:
    void* OldDispHost = nullptr;

    // Lens flare textures (pinned manually because you disallow UPROPERTY):
    UTexture2D* Flare1 = nullptr;
    UTexture2D* Flare2 = nullptr;
    UTexture2D* Flare3 = nullptr;
    UTexture2D* Flare4 = nullptr;

    // Subtitle timing:
    double SubtitlesDelay = 0.0;
    double SubtitlesNextTime = 0.0;

    bool bWasAttached = false;
};
