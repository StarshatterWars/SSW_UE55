#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "AudioDlg.generated.h"

class UComboBoxString;
class USlider;
class UButton;

class UOptionsScreen;

// NEW:
class UStarshatterAudioSubsystem;
class UStarshatterAudioSettings;

UCLASS()
class STARSHATTERWARS_API UAudioDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UAudioDlg(const FObjectInitializer& ObjectInitializer);

    void SetManager(UOptionsScreen* InManager) { Manager = InManager; }
    UOptionsScreen* GetManager() const { return Manager; }

    void Show();

    // IMPORTANT: Match base signature if you want polymorphism
    virtual void ExecFrame(double DeltaTime) override;

    void Apply();
    void Cancel();

    // Optional: keep your existing helper
    void PushToModel(bool bApplyRuntimeToo);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    virtual void BindFormWidgets() override;
    virtual FString GetLegacyFormText() const override;
    virtual void HandleAccept() override;
    virtual void HandleCancel() override;

private:
    // NEW: helpers
    UStarshatterAudioSubsystem* GetAudioSubsystem() const;
    UStarshatterAudioSettings* GetAudioSettings() const;

    void RefreshFromModel();

private:
    UPROPERTY(Transient)
    UOptionsScreen* Manager = nullptr;

    bool bClosed = true;

    float MasterVolume = 1.0f;
    float MusicVolume = 1.0f;
    float EffectsVolume = 1.0f;
    float VoiceVolume = 1.0f;
    int32 SoundQuality = 1;

protected:
    UPROPERTY(meta = (BindWidgetOptional)) USlider* MasterSlider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USlider* MusicSlider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USlider* EffectsSlider = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) USlider* VoiceSlider = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UComboBoxString* QualityCombo = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* ApplyBtn = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CancelBtn = nullptr;

    // Tabs...
    UPROPERTY(meta = (BindWidgetOptional)) UButton* VidTabButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* AudTabButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* CtlTabButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* OptTabButton = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* ModTabButton = nullptr;

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
    UFUNCTION() void OnOptionsClicked();
    UFUNCTION() void OnControlsClicked();
    UFUNCTION() void OnModClicked();
};
