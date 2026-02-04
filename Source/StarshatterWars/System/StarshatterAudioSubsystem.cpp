/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         StarshatterAudioSubsystem.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UStarshatterAudioSubsystem
    - GameInstanceSubsystem responsible for applying audio settings at runtime.
    - Bridges UI (AudioDlg) <-> Config object (UStarshatterAudioSettings) <-> UE Audio (SoundMix/SoundClass).
    - Safe if no assets are assigned: all Apply operations become no-ops.
*/

#include "StarshatterAudioSubsystem.h"

// UE:
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

// Audio:
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"

// Your settings object:
#include "StarshatterAudioSettings.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterAudioSubsystem, Log, All);

void UStarshatterAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Optional: load settings at boot of game instance:
    LoadSettings();
}

void UStarshatterAudioSubsystem::Deinitialize()
{
    // Optional: clear mix so we don't leave overrides hanging around:
    // (We need a world context for this; Deinitialize doesn't provide one,
    //  so just mark not pushed. Runtime will resolve next time Apply is called.)
    bMixPushed = false;

    Super::Deinitialize();
}

UStarshatterAudioSubsystem* UStarshatterAudioSubsystem::Get(UObject* WorldContextObject)
{
    if (!WorldContextObject)
        return nullptr;

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
        return nullptr;

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
        return nullptr;

    return GI->GetSubsystem<UStarshatterAudioSubsystem>();
}

// ------------------------------------------------------------
// Settings access
// ------------------------------------------------------------

UStarshatterAudioSettings* UStarshatterAudioSubsystem::GetSettings() const
{
    // Uses the CDO as a config-backed object:
    return GetMutableDefault<UStarshatterAudioSettings>();
}

void UStarshatterAudioSubsystem::LoadSettings()
{
    UStarshatterAudioSettings* S = GetSettings();
    if (!S)
        return;

    // Reload config from .ini then sanitize values:
    S->ReloadConfig();
    S->Sanitize();
}

void UStarshatterAudioSubsystem::SaveSettings()
{
    UStarshatterAudioSettings* S = GetSettings();
    if (!S)
        return;

    S->Sanitize();
    S->Save(); // your settings class should implement this (SaveConfig wrapper)
}

// ------------------------------------------------------------
// Runtime apply
// ------------------------------------------------------------

void UStarshatterAudioSubsystem::EnsureMixPushed(UObject* WorldContextObject)
{
    if (bMixPushed)
        return;

    if (!WorldContextObject || !MasterMix)
        return;

    // Push the mix once:
    UGameplayStatics::PushSoundMixModifier(WorldContextObject, MasterMix);
    bMixPushed = true;
}

void UStarshatterAudioSubsystem::ApplySoundClassVolume(
    UObject* WorldContextObject,
    USoundMix* Mix,
    USoundClass* SoundClass,
    float Volume) const
{
    if (!WorldContextObject || !Mix || !SoundClass)
        return;

    // Clamp to sane range:
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);

    // Apply a mix override:
    // FadeInTime=0, ApplyToChildren=true, but tweak if desired:
    UGameplayStatics::SetSoundMixClassOverride(
        WorldContextObject,
        Mix,
        SoundClass,
        Volume,   // Volume
        1.0f,     // Pitch
        0.0f,     // Fade in time
        true      // Apply to children
    );
}

void UStarshatterAudioSubsystem::ApplySettingsToRuntime(UObject* WorldContextObject)
{
    UStarshatterAudioSettings* S = GetSettings();
    if (!S)
        return;

    // If no assets assigned, treat as no-op:
    if (!MasterMix && !MasterClass && !MusicClass && !EffectsClass && !VoiceClass)
        return;

    EnsureMixPushed(WorldContextObject);

    // If we have a mix, we can apply overrides:
    if (MasterMix)
    {
        ApplySoundClassVolume(WorldContextObject, MasterMix, MasterClass, S->GetMasterVolume());
        ApplySoundClassVolume(WorldContextObject, MasterMix, MusicClass, S->GetMusicVolume());
        ApplySoundClassVolume(WorldContextObject, MasterMix, EffectsClass, S->GetEffectsVolume());
        ApplySoundClassVolume(WorldContextObject, MasterMix, VoiceClass, S->GetVoiceVolume());

        // Re-push to ensure audio device uses latest overrides immediately:
        if (WorldContextObject)
        {
            UGameplayStatics::PushSoundMixModifier(WorldContextObject, MasterMix);
        }
    }

    // SoundQuality is game-specific; you can use it to select quality tiers,
    // concurrency rules, or feature toggles later. For now it is stored and
    // available for logic elsewhere.
}

void UStarshatterAudioSubsystem::ClearRuntimeMix(UObject* WorldContextObject)
{
    if (!WorldContextObject || !MasterMix)
        return;

    // Pop the mix, clearing overrides:
    UGameplayStatics::PopSoundMixModifier(WorldContextObject, MasterMix);
    bMixPushed = false;
}

// ------------------------------------------------------------
// Convenience getters
// ------------------------------------------------------------

float UStarshatterAudioSubsystem::GetMasterVolume() const
{
    const UStarshatterAudioSettings* S = GetDefault<UStarshatterAudioSettings>();
    return S ? S->GetMasterVolume() : 1.0f;
}

float UStarshatterAudioSubsystem::GetMusicVolume() const
{
    const UStarshatterAudioSettings* S = GetDefault<UStarshatterAudioSettings>();
    return S ? S->GetMusicVolume() : 1.0f;
}

float UStarshatterAudioSubsystem::GetEffectsVolume() const
{
    const UStarshatterAudioSettings* S = GetDefault<UStarshatterAudioSettings>();
    return S ? S->GetEffectsVolume() : 1.0f;
}

float UStarshatterAudioSubsystem::GetVoiceVolume() const
{
    const UStarshatterAudioSettings* S = GetDefault<UStarshatterAudioSettings>();
    return S ? S->GetVoiceVolume() : 1.0f;
}

int32 UStarshatterAudioSubsystem::GetSoundQuality() const
{
    const UStarshatterAudioSettings* S = GetDefault<UStarshatterAudioSettings>();
    return S ? S->GetSoundQuality() : 1;
}

// ------------------------------------------------------------
// Convenience setters (does not auto-save)
// ------------------------------------------------------------

void UStarshatterAudioSubsystem::SetMasterVolume(float V)
{
    if (UStarshatterAudioSettings* S = GetSettings())
    {
        S->SetMasterVolume(V);
    }
}

void UStarshatterAudioSubsystem::SetMusicVolume(float V)
{
    if (UStarshatterAudioSettings* S = GetSettings())
    {
        S->SetMusicVolume(V);
    }
}

void UStarshatterAudioSubsystem::SetEffectsVolume(float V)
{
    if (UStarshatterAudioSettings* S = GetSettings())
    {
        S->SetEffectsVolume(V);
    }
}

void UStarshatterAudioSubsystem::SetVoiceVolume(float V)
{
    if (UStarshatterAudioSettings* S = GetSettings())
    {
        S->SetVoiceVolume(V);
    }
}

void UStarshatterAudioSubsystem::SetSoundQuality(int32 Index)
{
    if (UStarshatterAudioSettings* S = GetSettings())
    {
        S->SetSoundQuality(Index);
    }
}
