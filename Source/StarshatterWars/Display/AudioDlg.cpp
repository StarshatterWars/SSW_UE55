#include "AudioDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"

// Model:
#include "StarshatterAudioSettings.h"

// Host/router:
#include "OptionsScreen.h"

// UE:
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogAudioDlg, Log, All);

UAudioDlg::UAudioDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetDialogInputEnabled(true);
}

void UAudioDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Buttons:
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
    if (AudTabButton)
    {
        AudTabButton->OnClicked.RemoveAll(this);
        AudTabButton->OnClicked.AddDynamic(this, &UAudioDlg::OnAudioClicked);
    }

    if (VidTabButton)
    {
        VidTabButton->OnClicked.RemoveAll(this);
        VidTabButton->OnClicked.AddDynamic(this, &UAudioDlg::OnVideoClicked);
    }

    if (CtlTabButton)
    {
        CtlTabButton->OnClicked.RemoveAll(this);
        CtlTabButton->OnClicked.AddDynamic(this, &UAudioDlg::OnControlsClicked);
    }

    if (OptTabButton)
    {
        OptTabButton->OnClicked.RemoveAll(this);
        OptTabButton->OnClicked.AddDynamic(this, &UAudioDlg::OnOptionsClicked);
    }

    if (ModTabButton)
    {
        ModTabButton->OnClicked.RemoveAll(this);
        ModTabButton->OnClicked.AddDynamic(this, &UAudioDlg::OnModClicked);
    }

    // Sliders:
    if (MasterSlider)
    {
        MasterSlider->OnValueChanged.RemoveAll(this);
        MasterSlider->OnValueChanged.AddDynamic(this, &UAudioDlg::OnMasterVolumeChanged);
    }

    if (MusicSlider)
    {
        MusicSlider->OnValueChanged.RemoveAll(this);
        MusicSlider->OnValueChanged.AddDynamic(this, &UAudioDlg::OnMusicVolumeChanged);
    }

    if (EffectsSlider)
    {
        EffectsSlider->OnValueChanged.RemoveAll(this);
        EffectsSlider->OnValueChanged.AddDynamic(this, &UAudioDlg::OnEffectsVolumeChanged);
    }

    if (VoiceSlider)
    {
        VoiceSlider->OnValueChanged.RemoveAll(this);
        VoiceSlider->OnValueChanged.AddDynamic(this, &UAudioDlg::OnVoiceVolumeChanged);
    }

    // Combo:
    if (QualityCombo)
    {
        QualityCombo->OnSelectionChanged.RemoveAll(this);
        QualityCombo->OnSelectionChanged.AddDynamic(this, &UAudioDlg::OnSoundQualityChanged);
    }

    // Initial load:
    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }
}

void UAudioDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame(InDeltaTime);
}

void UAudioDlg::ExecFrame(float)
{
    // Optional: live preview hooks go here.
}

void UAudioDlg::HandleAccept()
{
    OnApplyClicked();
}

void UAudioDlg::HandleCancel()
{
    OnCancelClicked();
}

// ------------------------------------------------------------
// Model access / sync
// ------------------------------------------------------------

UStarshatterAudioSettings* UAudioDlg::GetAudioSettings() const
{
    if (UGameInstance* GI = GetGameInstance())
        return GI->GetSubsystem<UStarshatterAudioSettings>();

    return nullptr;
}

void UAudioDlg::RefreshFromModel()
{
    UStarshatterAudioSettings* S = GetAudioSettings();
    if (!S)
    {
        UE_LOG(LogAudioDlg, Warning, TEXT("AudioDlg: AudioSettings subsystem not available."));
        return;
    }

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

void UAudioDlg::PushToModel(bool bApplyRuntimeToo)
{
    UStarshatterAudioSettings* S = GetAudioSettings();
    if (!S)
        return;

    S->SetMasterVolume(MasterVolume);
    S->SetMusicVolume(MusicVolume);
    S->SetEffectsVolume(EffectsVolume);
    S->SetVoiceVolume(VoiceVolume);
    S->SetSoundQuality(SoundQuality);

    // Save to config:
    S->Save();

    // Apply to UE audio runtime (optional but usually desired on Apply):
    if (bApplyRuntimeToo)
        S->ApplyToRuntimeAudio();
}

// ------------------------------------------------------------
// Show / Apply / Cancel (compat with your routing)
// ------------------------------------------------------------

void UAudioDlg::Show()
{
    SetVisibility(ESlateVisibility::Visible);

    if (bClosed)
    {
        RefreshFromModel();
        bClosed = false;
    }
}

void UAudioDlg::Apply()
{
    if (bClosed)
        return;

    PushToModel(true);
    bClosed = true;
}

void UAudioDlg::Cancel()
{
    // Revert UI to model values:
    RefreshFromModel();
    bClosed = true;
}

// ------------------------------------------------------------
// Handlers
// ------------------------------------------------------------

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
    // If routed through OptionsScreen, let it coordinate Apply across dialogs.
    if (Manager)
        Manager->ApplyOptions();
    else
        Apply();
}

void UAudioDlg::OnCancelClicked()
{
    if (Manager)
        Manager->CancelOptions();
    else
        Cancel();
}

// Tabs:
void UAudioDlg::OnAudioClicked() { if (Manager) Manager->ShowAudDlg(); }
void UAudioDlg::OnVideoClicked() { if (Manager) Manager->ShowVidDlg(); }
void UAudioDlg::OnOptionsClicked() { if (Manager) Manager->ShowOptDlg(); }
void UAudioDlg::OnControlsClicked() { if (Manager) Manager->ShowCtlDlg(); }
void UAudioDlg::OnModClicked() { if (Manager) Manager->ShowModDlg(); }
