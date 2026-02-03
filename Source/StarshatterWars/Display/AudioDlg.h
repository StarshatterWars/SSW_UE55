/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         AudioDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UAudioDlg
    - Unreal replacement for legacy AudDlg (FormWindow)
    - Uses UBaseScreen for FORM-style ID binding + Enter/Escape handling
    - Applies legacy .frm defaults via GetLegacyFormText()
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "GameScreen.h"
#include "AudioDlg.generated.h"

class UButton;
class USlider;
class UTextBlock;

UCLASS()
class STARSHATTERWARS_API UAudioDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UAudioDlg(const FObjectInitializer& ObjectInitializer);

    // Legacy API (maintained names):
    virtual void RegisterControls();
    virtual void Show();
    virtual void ExecFrame(float DeltaTime);

    // Operations:
    virtual void Apply();
    virtual void Cancel();

    // Legacy semantic handlers:
    virtual void OnApply();
    virtual void OnCancel();

    virtual void OnAudio();
    virtual void OnVideo();
    virtual void OnOptions();
    virtual void OnControls();

    void SetManager(UGameScreen* InManager);

protected:
    // ------------------------------------------------------------
    // UBaseScreen overrides
    // ------------------------------------------------------------
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;

    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Centralized Enter/Escape from UBaseScreen:
    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

protected:
    // ------------------------------------------------------------
    // UMG click handlers (NO lambdas)
    // ------------------------------------------------------------
    UFUNCTION() void HandleApplyClicked();
    UFUNCTION() void HandleCancelClicked();

    UFUNCTION() void HandleAudioClicked();
    UFUNCTION() void HandleVideoClicked();
    UFUNCTION() void HandleOptionsClicked();
    UFUNCTION() void HandleControlsClicked();

protected:
    // ------------------------------------------------------------
    // Manager
    // ------------------------------------------------------------
    UGameScreen* Manager = nullptr;

    // ------------------------------------------------------------
    // Widgets (BindWidgetOptional; must match UMG widget names)
    // ------------------------------------------------------------

    // Title label (FORM id 10):
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TitleLabel = nullptr;

    // Nav tab buttons (FORM ids 901..905):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* vid_btn = nullptr; // 901
    UPROPERTY(meta = (BindWidgetOptional)) UButton* aud_btn = nullptr; // 902
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ctl_btn = nullptr; // 903
    UPROPERTY(meta = (BindWidgetOptional)) UButton* opt_btn = nullptr; // 904
    UPROPERTY(meta = (BindWidgetOptional)) UButton* mod_btn = nullptr; // 905

    // Sliders (FORM ids 201..206):
    UPROPERTY(meta = (BindWidgetOptional)) USlider* efx_volume_slider = nullptr; // 201
    UPROPERTY(meta = (BindWidgetOptional)) USlider* gui_volume_slider = nullptr; // 202
    UPROPERTY(meta = (BindWidgetOptional)) USlider* wrn_volume_slider = nullptr; // 203
    UPROPERTY(meta = (BindWidgetOptional)) USlider* vox_volume_slider = nullptr; // 204
    UPROPERTY(meta = (BindWidgetOptional)) USlider* menu_music_slider = nullptr; // 205
    UPROPERTY(meta = (BindWidgetOptional)) USlider* game_music_slider = nullptr; // 206

    // Apply/Cancel (FORM ids 1/2):
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr; // 1
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr; // 2

protected:
    bool bClosed = true;

private:
    void LoadFromConfig();
    void SaveToConfig();
};
