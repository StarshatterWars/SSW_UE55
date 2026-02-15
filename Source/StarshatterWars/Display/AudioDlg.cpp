#include "AudioDlg.h"

// UMG
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"

// Model
#include "StarshatterAudioSettings.h"

// Save system
#include "StarshatterSettingsSaveSubsystem.h"
#include "StarshatterSettingsSaveGame.h"

// Router
#include "OptionsScreen.h"

#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogAudioDlg, Log, All);

UAudioDlg::UAudioDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetDialogInputEnabled(true);
}

void UAudioDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BindDelegates();
}

void UAudioDlg::NativeConstruct()
{
    Super::NativeConstruct();
    BindDelegates();

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }
}

void UAudioDlg::BindDelegates()
{
    if (bDelegatesBound)
        return;

    if (ApplyBtn)  ApplyBtn->OnClicked.AddUniqueDynamic(this, &UAudioDlg::OnApplyClicked);
    if (CancelBtn) CancelBtn->OnClicked.AddUniqueDynamic(this, &UAudioDlg::OnCancelClicked);

    if (AudTabButton) AudTabButton->OnClicked.AddUniqueDynamic(this, &UAudioDlg::OnAudioClicked);
    if (VidTabButton) VidTabButton->OnClicked.AddUniqueDynamic(this, &UAudioDlg::OnVideoClicked);
    if (CtlTabButton) CtlTabButton->OnClicked.AddUniqueDynamic(this, &UAudioDlg::OnControlsClicked);
    if (OptTabButton) OptTabButton->OnClicked.AddUniqueDynamic(this, &UAudioDlg::OnOptionsClicked);
    if (ModTabButton) ModTabButton->OnClicked.AddUniqueDynamic(this, &UAudioDlg::OnModClicked);

    if (MasterSlider)  MasterSlider->OnValueChanged.AddUniqueDynamic(this, &UAudioDlg::OnMasterVolumeChanged);
    if (MusicSlider)   MusicSlider->OnValueChanged.AddUniqueDynamic(this, &UAudioDlg::OnMusicVolumeChanged);
    if (EffectsSlider) EffectsSlider->OnValueChanged.AddUniqueDynamic(this, &UAudioDlg::OnEffectsVolumeChanged);
    if (VoiceSlider)   VoiceSlider->OnValueChanged.AddUniqueDynamic(this, &UAudioDlg::OnVoiceVolumeChanged);

    if (QualityCombo)
        QualityCombo->OnSelectionChanged.AddUniqueDynamic(this, &UAudioDlg::OnSoundQualityChanged);

    bDelegatesBound = true;
}

void UAudioDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame((double)InDeltaTime);
}

void UAudioDlg::ExecFrame(double) {}

void UAudioDlg::BindFormWidgets() {}
FString UAudioDlg::GetLegacyFormText() const { return FString(); }

void UAudioDlg::HandleAccept() { OnApplyClicked(); }
void UAudioDlg::HandleCancel() { OnCancelClicked(); }

void UAudioDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }
}

// ---------------- Model ----------------

UStarshatterAudioSettings* UAudioDlg::GetAudioSettings() const
{
    return GetMutableDefault<UStarshatterAudioSettings>();
}

UStarshatterSettingsSaveSubsystem* UAudioDlg::GetSettingsSaveSubsystem() const
{
    if (UGameInstance* GI = GetGameInstance())
        return GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>();
    return nullptr;
}

void UAudioDlg::RefreshFromModel()
{
    UStarshatterAudioSettings* S = GetAudioSettings();
    if (!S) return;

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
    if (QualityCombo)  QualityCombo->SetSelectedIndex(SoundQuality);
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
    S->Save();

    SaveAudioToUnifiedSettings(S);

    if (bApplyRuntimeToo)
        S->ApplyToRuntimeAudio(this);
}

void UAudioDlg::SaveAudioToUnifiedSettings(UStarshatterAudioSettings* Settings)
{
    if (!Settings) return;

    UStarshatterSettingsSaveSubsystem* SaveSS = GetSettingsSaveSubsystem();
    if (!SaveSS) return;

    SaveSS->LoadOrCreate();

    if (UStarshatterSettingsSaveGame* SG = SaveSS->GetSettings())
    {
        SG->Audio.MasterVolume = Settings->GetMasterVolume();
        SG->Audio.MusicVolume = Settings->GetMusicVolume();
        SG->Audio.EffectsVolume = Settings->GetEffectsVolume();
        SG->Audio.VoiceVolume = Settings->GetVoiceVolume();
        SG->Audio.SoundQuality = Settings->GetSoundQuality();

        SG->Sanitize();
        SaveSS->MarkDirty();
        SaveSS->Save();
    }
}

// ---------------- Apply / Cancel ----------------

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

// ---------------- Handlers ----------------

void UAudioDlg::OnMasterVolumeChanged(float V) { MasterVolume = V;  bClosed = false; }
void UAudioDlg::OnMusicVolumeChanged(float V) { MusicVolume = V;   bClosed = false; }
void UAudioDlg::OnEffectsVolumeChanged(float V) { EffectsVolume = V; bClosed = false; }
void UAudioDlg::OnVoiceVolumeChanged(float V) { VoiceVolume = V;   bClosed = false; }

void UAudioDlg::OnSoundQualityChanged(FString, ESelectInfo::Type)
{
    if (QualityCombo)
        SoundQuality = QualityCombo->GetSelectedIndex();
    bClosed = false;
}

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

// Tabs route to new OptionsScreen hub

void UAudioDlg::OnAudioClicked() { if (OptionsManager) OptionsManager->ShowAudDlg(); }
void UAudioDlg::OnVideoClicked() { if (OptionsManager) OptionsManager->ShowVidDlg(); }
void UAudioDlg::OnControlsClicked() { if (OptionsManager) OptionsManager->ShowCtlDlg(); }
void UAudioDlg::OnOptionsClicked() { if (OptionsManager) OptionsManager->ShowGameDlg(); }
void UAudioDlg::OnModClicked() { if (OptionsManager) OptionsManager->ShowModDlg(); }
