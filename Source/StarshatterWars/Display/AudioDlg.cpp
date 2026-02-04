#include "AudioDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"

// NEW:
#include "StarshatterAudioSubsystem.h"
#include "StarshatterAudioSettings.h"

// Host/router:
#include "OptionsScreen.h"

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

// IMPORTANT: matches UBaseScreen::ExecFrame(double)
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
// NEW: Correct accessors
// -------------------------

UStarshatterAudioSubsystem* UAudioDlg::GetAudioSubsystem() const
{
    return UStarshatterAudioSubsystem::Get(const_cast<UAudioDlg*>(this));
}

UStarshatterAudioSettings* UAudioDlg::GetAudioSettings() const
{
    // Settings is a config-backed UObject (CDO pattern), not a subsystem:
    return GetMutableDefault<UStarshatterAudioSettings>();
}

// -------------------------
// Model -> UI
// -------------------------

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

    if (QualityCombo)
        QualityCombo->SetSelectedIndex(SoundQuality);
}

// -------------------------
// UI -> Model
// -------------------------

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

    if (bApplyRuntimeToo)
    {
        if (UStarshatterAudioSubsystem* AudioSub = GetAudioSubsystem())
        {
            AudioSub->ApplySettingsToRuntime(const_cast<UAudioDlg*>(this));
        }
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
    if (Manager) Manager->ApplyOptions();
    else Apply();
}

void UAudioDlg::OnCancelClicked()
{
    if (Manager) Manager->CancelOptions();
    else Cancel();
}

// Tabs:
void UAudioDlg::OnAudioClicked() { if (Manager) Manager->ShowAudDlg(); }
void UAudioDlg::OnVideoClicked() { if (Manager) Manager->ShowVidDlg(); }
void UAudioDlg::OnOptionsClicked() { if (Manager) Manager->ShowOptDlg(); }
void UAudioDlg::OnControlsClicked() { if (Manager) Manager->ShowCtlDlg(); }
void UAudioDlg::OnModClicked() { if (Manager) Manager->ShowModDlg(); }
