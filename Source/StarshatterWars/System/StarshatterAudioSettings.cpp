#include "StarshatterAudioSettings.h"

#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
// ------------------------------------------------------------
// Singleton accessor
// ------------------------------------------------------------
UStarshatterAudioSettings* UStarshatterAudioSettings::Get()
{
    static UStarshatterAudioSettings* Instance = nullptr;

    if (!Instance)
    {
        Instance = GetMutableDefault<UStarshatterAudioSettings>();
    }

    return Instance;
}

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
float UStarshatterAudioSettings::Clamp01(float V)
{
    return FMath::Clamp(V, 0.0f, 1.0f);
}

// ------------------------------------------------------------
// Setters
// ------------------------------------------------------------
void UStarshatterAudioSettings::SetMasterVolume(float V)
{
    MasterVolume = Clamp01(V);
}

void UStarshatterAudioSettings::SetMusicVolume(float V)
{
    MusicVolume = Clamp01(V);
}

void UStarshatterAudioSettings::SetSfxVolume(float V)
{
    SfxVolume = Clamp01(V);
}

void UStarshatterAudioSettings::SetVoiceVolume(float V)
{
    VoiceVolume = Clamp01(V);
}

void UStarshatterAudioSettings::SetQualityIndex(int32 Index)
{
    QualityIndex = FMath::Clamp(Index, 0, 3);
}

// ------------------------------------------------------------
// Save
// ------------------------------------------------------------
void UStarshatterAudioSettings::Save()
{
    SaveConfig();
}

void UStarshatterAudioSettings::ApplyToRuntimeAudio()
{
    // NOTE:
    // This is a safe stub that compiles even if you haven't wired SoundMix/SoundClass assets yet.
    // When you add assets, you can do:
    // - Push a SoundMix modifier
    // - Set SoundClass volumes (Master/Music/SFX/Voice)
    //
    // For now, no-op to unblock build.
}

