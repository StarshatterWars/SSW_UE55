/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         StarshatterAudioSubsystem.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UStarshatterAudioSubsystem
    - GameInstanceSubsystem wrapper around UStarshatterAudioSettings (config-backed CDO).
    - Provides stable entrypoints used by UI dialogs and boot sequencing:
        * Get(...)
        * LoadAudioConfig()
        * ApplySettingsToRuntime()
    - Keeps the rest of the codebase from directly poking config objects everywhere.
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StarshatterAudioSubsystem.generated.h"

class UStarshatterAudioSettings;

UCLASS()
class STARSHATTERWARS_API UStarshatterAudioSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // -----------------------------
    // UGameInstanceSubsystem
    // -----------------------------
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // -----------------------------
    // Boot entrypoint
    // -----------------------------
    UFUNCTION()
    void Boot();

    // -----------------------------
    // Stable API used by Boot + UI
    // -----------------------------

    // Load settings from config (reload ini) and sanitize.
    UFUNCTION()
    void LoadAudioConfig();

    // Apply current settings to runtime audio.
    UFUNCTION()
    void ApplySettingsToRuntime();

    // Optional legacy alias if any old code expects this name:
    UFUNCTION()
    void ApplySettingsToRuntimeAudio() { ApplySettingsToRuntime(); }

    // Access underlying settings object (config-backed CDO).
    UFUNCTION()
    UStarshatterAudioSettings* GetSettings() const;

    // -----------------------------
    // Convenience accessors
    // -----------------------------
    static UStarshatterAudioSubsystem* Get(const UObject* WorldContextObject);
    static UStarshatterAudioSubsystem* Get(UGameInstance* GameInstance);
};
