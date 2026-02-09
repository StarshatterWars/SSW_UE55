/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         AudioDlg.cpp
    AUTHOR:       Carlos Bott
*/

#include "AudioDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"

// Model:
#include "StarshatterAudioSettings.h"

// Unified settings save:
#include "StarshatterSettingsSaveSubsystem.h"
#include "StarshatterSettingsSaveGame.h"
#include "GameStructs.h"

// Host/router:
#include "OptionsScreen.h"

// UE:
#include "Engine/GameInstance.h"

UAudioDlg::UAudioDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetDialogInputEnabled(true);
}

void UAudioDlg::NativeConstruct()
{
    Super::NativeConstruct();

    if (ApplyBtn)
    {
        ApplyBtn->OnClicked.RemoveAll(this);
        ApplyBtn->OnClicked.AddDynamic(this, &UAudioDlg::OnApplyClicked);
    }

    if (CancelBtn)
    {
        CancelBtn->OnClicked.RemoveAll(this);
        CancelBtn->OnClicked.AddDynamic(this, &UAudioDlg::OnCancelClicked);
    }

    // Tabs:
    if (AudTabButton) { AudTabButton->OnClicked.RemoveAll(this); AudTabButton->OnClicked.AddDynamic(this, &UAudioDlg::OnAudioClicked); }
    if (VidTabButton) { VidTabButton->OnClicked.RemoveAll(this); VidTabButton->OnClicked.AddDynamic(this, &UAudioDlg::OnVideoClicked); }
    if (CtlTabButton) { CtlTabButton->OnClicked.RemoveAll(this); CtlTabButton->OnClicked.AddDynamic(this, &UAudioDlg::OnControlsClicked); }
    if (OptTabButton) { OptTabButton->OnClicked.RemoveAll(this); OptTabButton->OnClicked.AddDynamic(this, &UAudioDlg::OnOptionsClicked); }
    if (ModTabButton) { ModTabButton->OnClicked.RemoveAll(this); ModTabButton->OnClicked.AddDynamic(this, &UAudioDlg::OnModClicked); }

    // Sliders:
    if (MasterSlider) { MasterSlider->OnValueChanged.RemoveAll(this);  MasterSlider->OnValueChanged.AddDynamic(this, &UAudioDlg::OnMasterVolumeChanged); }
    if (MusicSlider) { MusicSlider->OnValueChanged.RemoveAll(this);   MusicSlider->OnValueChanged.AddDynamic(this, &UAudioDlg::OnMusicVolumeChanged); }
    if (EffectsSlider) { EffectsSlider->OnValueChanged.RemoveAll(this); EffectsSlider->OnValueChanged.AddDynamic(this, &UAudioDlg::OnEffectsVolumeChanged); }
    if (VoiceSlider) { VoiceSlider->OnValueChanged.RemoveAll(this);   VoiceSlider->OnValueChanged.AddDynamic(this, &UAudioDlg::OnVoiceVolumeChanged); }

    if (QualityCombo)
    {
        QualityCombo->OnSelectionChanged.RemoveAll(this);
        QualityCombo->OnSelectionChanged.AddDynamic(this, &UAudioDlg::OnSoundQualityChanged);
    }

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }
}

void UAudioDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame((double)InDeltaTime);
}

void UAudioDlg::ExecFrame(double /*DeltaTime*/)
{
    // Optional per-frame UI logic
}

void UAudioDlg::BindFormWidgets() {}
FString UAudioDlg::GetLegacyFormText() const { return FString(); }

void UAudioDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }
}

void UAudioDlg::HandleAccept() { OnApplyClicked(); }
void UAudioDlg::HandleCancel() { OnCancelClicked(); }

// -------------------------
// Settings access
// -------------------------

UStarshatterAudioSettings* UAudioDlg::GetAudioSettings() const
{
    // Config-backed UObject (CDO pattern)
    return GetMutableDefault<UStarshatterAudioSettings>();
}

UStarshatterSettingsSaveSubsystem* UAudioDlg::GetSettingsSaveSubsystem() const
{
    if (UGameInstance* GI = GetGameInstance())
    {
        return GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>();
    }
    return nullptr;
}

// -------------------------
// Model -> UI
// -------------------------

void UAudioDlg::RefreshFromModel()
{
    UStarshatterAudioSettings* S = GetAudioSettings();
    if (!S) return;

    // Your current model source = config CDO
    S->ReloadConfig();
    S->Sanitize();

    MasterVolume = S->GetMasterVolume();
    MusicVolume = S->GetMusicVolume();
    EffectsVolume = S->GetEffectsVolume();
    VoiceVolume = S->GetVoiceVolume();
    SoundQuality = S->GetSoundQuality();

    if (MasterSlider)  MasterSlider->SetValue(MasterVolume);
    if (MusicSlider)   MusicSlider->SetValue(MusicVolume);
    if (EffectsSlider) EffectsSlider->SetValue(EffectsVolume);
    if (VoiceSlider)   VoiceSlider->SetValue(VoiceVolume);

    if (QualityCombo)
        QualityCombo->SetSelectedIndex(SoundQuality);
}

// -------------------------
// UI -> Model (plus SaveGame)
// -------------------------

void UAudioDlg::SaveAudioToUnifiedSettings(UStarshatterAudioSettings* Settings)
{
    if (!Settings)
        return;

    UStarshatterSettingsSaveSubsystem* SaveSS = GetSettingsSaveSubsystem();
    if (!SaveSS)
        return;

    // Ensure settings object exists
    SaveSS->LoadOrCreate();

    UStarshatterSettingsSaveGame* SG = SaveSS->GetSettings();
    if (!SG)
        return;

    // Write Settings -> SaveGame payload
    SG->Audio.MasterVolume = Settings->GetMasterVolume();
    SG->Audio.MusicVolume = Settings->GetMusicVolume();
    SG->Audio.EffectsVolume = Settings->GetEffectsVolume();
    SG->Audio.VoiceVolume = Settings->GetVoiceVolume();
    SG->Audio.SoundQuality = Settings->GetSoundQuality();

    SG->Sanitize();

    SaveSS->MarkDirty();
    SaveSS->Save();
}

void UAudioDlg::PushToModel(bool bApplyRuntimeToo)
{
    UStarshatterAudioSettings* S = GetAudioSettings();
    if (!S) return;

    S->SetMasterVolume(MasterVolume);
    S->SetMusicVolume(MusicVolume);
    S->SetEffectsVolume(EffectsVolume);
    S->SetVoiceVolume(VoiceVolume);
    S->SetSoundQuality(SoundQuality);

    S->Sanitize();

    // Persist to ini (your current path)
    S->Save();

    // ALSO persist to unified SaveGame
    SaveAudioToUnifiedSettings(S);

    if (bApplyRuntimeToo)
    {
        // Runtime apply stays owned by settings class
        S->ApplyToRuntimeAudio(const_cast<UAudioDlg*>(this));
    }
}

void UAudioDlg::Apply()
{
    if (bClosed) return;
    PushToModel(true);
    bClosed = true;
}

void UAudioDlg::Cancel()
{
    RefreshFromModel();
    bClosed = true;
}

// Slider handlers:
void UAudioDlg::OnMasterVolumeChanged(float V) { MasterVolume = V; }
void UAudioDlg::OnMusicVolumeChanged(float V) { MusicVolume = V; }
void UAudioDlg::OnEffectsVolumeChanged(float V) { EffectsVolume = V; }
void UAudioDlg::OnVoiceVolumeChanged(float V) { VoiceVolume = V; }

void UAudioDlg::OnSoundQualityChanged(FString, ESelectInfo::Type)
{
    if (QualityCombo)
        SoundQuality = QualityCombo->GetSelectedIndex();
}

// Buttons:
void UAudioDlg::OnApplyClicked()
{
    if (OptionsManager) OptionsManager->ApplyOptions();
    else Apply();
}

void UAudioDlg::OnCancelClicked()
{
    if (OptionsManager) OptionsManager->CancelOptions();
    else Cancel();
}

// Tabs:
void UAudioDlg::OnAudioClicked() { if (OptionsManager) OptionsManager->ShowAudDlg(); }
void UAudioDlg::OnVideoClicked() { if (OptionsManager) OptionsManager->ShowVidDlg(); }
void UAudioDlg::OnOptionsClicked() { if (OptionsManager) OptionsManager->ShowOptDlg(); }
void UAudioDlg::OnControlsClicked() { if (OptionsManager) OptionsManager->ShowCtlDlg(); }
void UAudioDlg::OnModClicked() { if (OptionsManager) OptionsManager->ShowModDlg(); }
