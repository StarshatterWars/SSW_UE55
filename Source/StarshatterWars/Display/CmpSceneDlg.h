/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         CmpSceneDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UCmpSceneDlg
    - Unreal port of legacy CmpSceneDlg (cutscene/campaign scene dialog).
    - Uses UBaseScreen for unified key handling and optional FORM defaults.
    - Hosts a cutscene render widget inside SceneHost.
    - Displays subtitles (RichText) and auto-advances lines based on mission timing.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "CmpSceneDlg.generated.h"

class UBorder;
class UOverlay;
class UWidget;
class URichTextBlock;
class UTexture2D;

class UCmpnScreen;

// Forward declare your eventual Unreal ports (or wrappers):
class UDisplayView;
class UCameraView;

UCLASS()
class STARSHATTERWARS_API UCmpSceneDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UCmpSceneDlg(const FObjectInitializer& ObjectInitializer);

    // ----------------------------------------------------------------
    // UBaseScreen
    // ----------------------------------------------------------------
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

public:
    // ----------------------------------------------------------------
    // Legacy-equivalent API
    // ----------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "CmpSceneDlg")
    void Show();

    UFUNCTION(BlueprintCallable, Category = "CmpSceneDlg")
    void Hide();

    /** Legacy ExecFrame() equivalent (driven from Tick). */
    virtual void ExecFrame(float DeltaTime);

    UCameraView* GetCameraView() const { return CamView; }
    UDisplayView* GetDisplayView() const { return DispView; }

    void SetManager(UCmpnScreen* InManager) { Manager = InManager; }

protected:
    // ----------------------------------------------------------------
    // UUserWidget lifecycle
    // ----------------------------------------------------------------
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // ----------------------------------------------------------------
    // Widgets
    // ----------------------------------------------------------------

    /**
     * Scene host container (legacy mov_scene = FindControl(101)).
     * In UMG, make this a Border/Overlay/SizeBox that you can attach a viewport/render widget to.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UBorder* SceneHostBorder = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    UOverlay* SceneHostOverlay = nullptr;

    /**
     * Subtitles (legacy RichTextBox id 102).
     * Use a URichTextBlock. If you want a real scroll box, wrap it in UMG.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    URichTextBlock* SubtitlesText = nullptr;

protected:
    // ----------------------------------------------------------------
    // Routing hooks back to the campaign screen
    // ----------------------------------------------------------------
    void RequestShowCmdDlg();

    UFUNCTION(BlueprintImplementableEvent, Category = "CmpSceneDlg")
    void OnRequestShowCmdDlg();

protected:
    // ----------------------------------------------------------------
    // Cutscene integration hooks (override to wire to your real runtime)
    // ----------------------------------------------------------------

    /** True if the game is currently in a cutscene. */
    virtual bool IsInCutscene() const;

    /** Returns the full subtitles text (newline-separated). */
    virtual FString GetCutsceneSubtitles() const;

    /** True if lens flare is enabled. */
    virtual bool IsLensFlareEnabled() const;

    /** True if corona-only flare is enabled. */
    virtual bool IsCoronaEnabled() const;

    /**
     * Returns begin/end scene times in seconds.
     * Return false if unavailable (legacy would set subtitles_delay = -1).
     */
    virtual bool GetCutsceneSceneTimeRange(double& OutBeginSeconds, double& OutEndSeconds) const;

    /**
     * Attach your cutscene render widget to SceneHost here.
     * Called on Show() when in cutscene.
     */
    virtual void AttachCutsceneViewToHost();

    /**
     * Detach your cutscene view from host and restore previous view/window state.
     * Called on Hide().
     */
    virtual void DetachCutsceneViewFromHost();

protected:
    // ----------------------------------------------------------------
    // Flare texture loading hooks
    // ----------------------------------------------------------------
    virtual void LoadFlareTextures();

    /** Override if you need DataLoader/PCX bridge; default uses LoadObject with a soft path. */
    virtual UTexture2D* LoadFlareTextureByName(const FString& Name) const;

protected:
    // ----------------------------------------------------------------
    // Subtitle timing/scroll (legacy behavior)
    // ----------------------------------------------------------------
    void InitializeSubtitles();
    void AdvanceSubtitleLine();

    /** Splits subtitles into lines; preserves empty lines if desired. */
    void BuildSubtitleLines(const FString& InText);

protected:
    // ----------------------------------------------------------------
    // State (legacy mirrors)
    // ----------------------------------------------------------------
    UCmpnScreen* Manager = nullptr;
    UCameraView* CamView = nullptr;

    UDisplayView* DispView = nullptr;

    // flare textures:
    UTexture2D* Flare1 = nullptr;
    UTexture2D* Flare2 = nullptr;
    UTexture2D* Flare3 = nullptr;
    UTexture2D* Flare4 = nullptr;

    // subtitles timing:
    double SubtitlesDelay = 0.0;   // seconds per line
    double SubtitlesTime = 0.0;   // next advance timestamp (seconds)
    double ElapsedSeconds = 0.0;   // running time base while dialog active

    // parsed lines and current index:
    TArray<FString> SubtitleLines;
    int32 CurrentSubtitleLine = 0;

    // Whether we are currently showing an active cutscene:
    bool bActiveCutscene = false;
};
