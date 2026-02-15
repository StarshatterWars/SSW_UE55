/*=============================================================================
    Project:        Starshatter Wars
    Studio:         Fractal Dev Studios
    Copyright:      (c) 2025-2026.

    SUBSYSTEM:      Stars.exe (Unreal Port)
    FILE:           AudioDlg.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    Audio settings subpage used inside OptionsScreen hub.
    - Config-backed model (UStarshatterAudioSettings CDO)
    - Unified SaveGame persistence
    - Runtime apply via AudioSettings
    - Routed through OptionsScreen (Apply/Cancel/Tabs)
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "AudioDlg.generated.h"

class UComboBoxString;
class USlider;
class UButton;
class UOptionsScreen;

class UStarshatterAudioSettings;
class UStarshatterSettingsSaveSubsystem;
class UStarshatterSettingsSaveGame;

UCLASS()
class STARSHATTERWARS_API UAudioDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UAudioDlg(const FObjectInitializer& ObjectInitializer);

    void Show();
    virtual void ExecFrame(double DeltaTime) override;

    void Apply();
    void Cancel();
    void PushToModel(bool bApplyRuntimeToo);

    void SetOptionsManager(UOptionsScreen* InManager) { OptionsManager = InManager; }

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;
    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

private:
    void BindDelegates();

    UStarshatterAudioSettings* GetAudioSettings() const;
    UStarshatterSettingsSaveSubsystem* GetSettingsSaveSubsystem() const;
    void SaveAudioToUnifiedSettings(UStarshatterAudioSettings* Settings);
    void RefreshFromModel();

private:

    bool bClosed = true;
    bool bDelegatesBound = false;

    float MasterVolume = 1.0f;
    float MusicVolume = 1.0f;
    float EffectsVolume = 1.0f;
    float VoiceVolume = 1.0f;
    int32 SoundQuality = 1;

protected:
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> MasterSlider;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> MusicSlider;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> EffectsSlider;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> VoiceSlider;

    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UComboBoxString> QualityCombo;

    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ApplyBtn;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> CancelBtn;

    // Optional legacy tab buttons (safe if present in BP)
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> VidTabButton;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> AudTabButton;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> CtlTabButton;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> OptTabButton;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ModTabButton;

private:
    UFUNCTION() void OnMasterVolumeChanged(float V);
    UFUNCTION() void OnMusicVolumeChanged(float V);
    UFUNCTION() void OnEffectsVolumeChanged(float V);
    UFUNCTION() void OnVoiceVolumeChanged(float V);
    UFUNCTION() void OnSoundQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();

    UFUNCTION() void OnAudioClicked();
    UFUNCTION() void OnVideoClicked();
    UFUNCTION() void OnOptionsClicked();   // maps to Game tab now
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();
};
