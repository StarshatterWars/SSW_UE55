/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         AudioDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UAudioDlg
    - Unreal-native Audio Options dialog.
    - Does NOT depend on legacy AudioSettings.h.
    - Reads/writes UStarshatterAudioSettings (model).
    - Routed through UOptionsScreen for tab switching + Apply/Cancel.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

// Needed for ESelectInfo::Type in UFUNCTION signature:
#include "Components/ComboBoxString.h"

#include "AudioDlg.generated.h"

// UMG:
class USlider;
class UButton;

// Host/router:
class UOptionsScreen;

// Model:
class UStarshatterAudioSettings;

UCLASS()
class STARSHATTERWARS_API UAudioDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UAudioDlg(const FObjectInitializer& ObjectInitializer);

    void SetManager(UOptionsScreen* InManager) { Manager = InManager; }
    UOptionsScreen* GetManager() const { return Manager; }

    // Legacy-ish surface (kept for compatibility with your routing):
    void Show();
    void ExecFrame(float DeltaTime);

    // Compatibility with old callsites:
    void Apply();
    void Cancel();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // UBaseScreen overrides:
    virtual void BindFormWidgets() override {}
    virtual FString GetLegacyFormText() const override { return FString(); }
    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

private:
    // Model access:
    UStarshatterAudioSettings* GetAudioSettings() const;
    void RefreshFromModel();
    void PushToModel(bool bApplyRuntimeToo);

private:
    // Slider handlers:
    UFUNCTION() void OnMasterVolumeChanged(float Value);
    UFUNCTION() void OnMusicVolumeChanged(float Value);
    UFUNCTION() void OnEffectsVolumeChanged(float Value);
    UFUNCTION() void OnVoiceVolumeChanged(float Value);

    // Combo handlers:
    UFUNCTION() void OnSoundQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    // Buttons:
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    // Tabs:
    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();

private:
    UPROPERTY(Transient) TObjectPtr<UOptionsScreen> Manager = nullptr;

    // UI state:
    bool  bClosed = true;

    float MasterVolume = 1.0f;
    float MusicVolume = 1.0f;
    float EffectsVolume = 1.0f;
    float VoiceVolume = 1.0f;
    int32 SoundQuality = 1;

protected:
    // ------------------------------------------------------------
    // UMG bindings
    // ------------------------------------------------------------
    UPROPERTY(meta = (BindWidgetOptional)) USlider* MasterSlider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USlider* MusicSlider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USlider* EffectsSlider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USlider* VoiceSlider = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* QualityCombo = nullptr;

    // Apply / Cancel:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;

    // Tabs:
    UPROPERTY(meta = (BindWidgetOptional)) UButton* VidTabButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* AudTabButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CtlTabButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* OptTabButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ModTabButton = nullptr;
};
