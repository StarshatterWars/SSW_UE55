/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         StarshatterAudioSettings.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UStarshatterAudioSettings
    - UE-native audio settings container
    - Replaces legacy AudioSettings.cfg logic
    - Stored via Unreal config system (GameUserSettings-style)
    - Used by AudioDlg and AudioSubsystem
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StarshatterAudioSettings.generated.h"

UCLASS(Config = Game, DefaultConfig)
class STARSHATTERWARS_API UStarshatterAudioSettings : public UObject
{
    GENERATED_BODY()

public:
    // ------------------------------------------------------------
    // Singleton-style access (explicit, not static data abuse)
    // ------------------------------------------------------------
    static UStarshatterAudioSettings* Get();

    // ------------------------------------------------------------
    // Volume accessors
    // ------------------------------------------------------------
    float GetMasterVolume() const { return MasterVolume; }
    float GetMusicVolume()  const { return MusicVolume; }
    float GetSfxVolume()    const { return SfxVolume; }
    float GetVoiceVolume()  const { return VoiceVolume; }

    void SetMasterVolume(float V);
    void SetMusicVolume(float V);
    void SetSfxVolume(float V);
    void SetVoiceVolume(float V);

    void ApplyToRuntimeAudio();

    // ------------------------------------------------------------
    // Quality
    // ------------------------------------------------------------
    int32 GetQualityIndex() const { return QualityIndex; }
    void  SetQualityIndex(int32 Index);

    // ------------------------------------------------------------
    // Legacy compatibility (temporary)
    // ------------------------------------------------------------
    float GetEffectsVolume() const { return GetSfxVolume(); }
    void  SetEffectsVolume(float V) { SetSfxVolume(V); }

    int32 GetSoundQuality() const { return GetQualityIndex(); }
    void  SetSoundQuality(int32 I) { SetQualityIndex(I); }

    // ------------------------------------------------------------
    // Persistence
    // ------------------------------------------------------------
    void Save();

private:
    // ------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------
    static float Clamp01(float V);

private:
    // ------------------------------------------------------------
    // Stored config values
    // ------------------------------------------------------------
    UPROPERTY(Config, EditAnywhere, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MusicVolume = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SfxVolume = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VoiceVolume = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Audio", meta = (ClampMin = "0", ClampMax = "3"))
    int32 QualityIndex = 1;
};
