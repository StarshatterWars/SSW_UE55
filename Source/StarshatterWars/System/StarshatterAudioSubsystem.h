/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         StarshatterAudioSubsystem.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UStarshatterAudioSubsystem
    - GameInstanceSubsystem responsible for applying audio settings at runtime.
    - Bridges UI (AudioDlg) <-> Config object (UStarshatterAudioSettings) <-> UE Audio (SoundMix/SoundClass).
    - Safe if no assets are assigned: all Apply operations become no-ops.
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StarshatterAudioSubsystem.generated.h"

class UStarshatterAudioSettings;
class USoundMix;
class USoundClass;

UCLASS()
class STARSHATTERWARS_API UStarshatterAudioSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // UGameInstanceSubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Convenient accessor (optional)
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio", meta = (WorldContext = "WorldContextObject"))
    static UStarshatterAudioSubsystem* Get(UObject* WorldContextObject);

public:
    // --------------------------------------------
    // Settings access (CDO config object)
    // --------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    UStarshatterAudioSettings* GetSettings() const;

    // Loads config into settings object (ReloadConfig) + sanitizes
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    void LoadSettings();

    // Saves current settings to config
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    void SaveSettings();

public:
    // --------------------------------------------
    // Runtime apply
    // --------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio", meta = (WorldContext = "WorldContextObject"))
    void ApplySettingsToRuntime(UObject* WorldContextObject);

    // Optional: reset any pushed mix (if you want a clean slate)
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio", meta = (WorldContext = "WorldContextObject"))
    void ClearRuntimeMix(UObject* WorldContextObject);

public:
    // --------------------------------------------
    // Convenience getters for UI (AudioDlg)
    // --------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    float GetMasterVolume() const;

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    float GetMusicVolume() const;

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    float GetEffectsVolume() const;

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    float GetVoiceVolume() const;

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    int32 GetSoundQuality() const;

public:
    // --------------------------------------------
    // Convenience setters for UI (AudioDlg)
    // (Does NOT save automatically; UI should call Save on Apply)
    // --------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    void SetMasterVolume(float V);

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    void SetMusicVolume(float V);

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    void SetEffectsVolume(float V);

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    void SetVoiceVolume(float V);

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|Audio")
    void SetSoundQuality(int32 Index);

private:
    // Helper: apply a single SoundClass volume override if assets exist
    void ApplySoundClassVolume(UObject* WorldContextObject, USoundMix* Mix, USoundClass* SoundClass, float Volume) const;

    // Helper: ensure the mix is pushed
    void EnsureMixPushed(UObject* WorldContextObject);

private:
    // --------------------------------------------
    // Optional UE audio assets (assign in BP defaults)
    // --------------------------------------------

    // A SoundMix to push when applying settings
    UPROPERTY(EditDefaultsOnly, Category = "Audio|UE Assets")
    TObjectPtr<USoundMix> MasterMix = nullptr;

    // SoundClasses to control
    UPROPERTY(EditDefaultsOnly, Category = "Audio|UE Assets")
    TObjectPtr<USoundClass> MasterClass = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Audio|UE Assets")
    TObjectPtr<USoundClass> MusicClass = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Audio|UE Assets")
    TObjectPtr<USoundClass> EffectsClass = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Audio|UE Assets")
    TObjectPtr<USoundClass> VoiceClass = nullptr;

    // Track whether we pushed the mix
    bool bMixPushed = false;
};
