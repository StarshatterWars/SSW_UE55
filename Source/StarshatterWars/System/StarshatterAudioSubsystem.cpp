/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         StarshatterAudioSubsystem.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Implements UStarshatterAudioSubsystem:
    - Boot() loads config + applies runtime.
    - LoadAudioConfig() delegates to UStarshatterAudioSettings::Load().
    - ApplySettingsToRuntime() delegates to UStarshatterAudioSettings::ApplyToRuntimeAudio().
*/

#include "StarshatterAudioSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include "StarshatterAudioSettings.h"

void UStarshatterAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Intentionally not auto-booting here unless you want it.
    // Boot is usually orchestrated by StarshatterBootSubsystem.
}

void UStarshatterAudioSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UStarshatterAudioSubsystem::Boot()
{
    LoadAudioConfig();
    ApplySettingsToRuntime();
}

UStarshatterAudioSubsystem* UStarshatterAudioSubsystem::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
        return nullptr;

    const UWorld* World = WorldContextObject->GetWorld();
    if (!World)
        return nullptr;

    UGameInstance* GI = World->GetGameInstance();
    return Get(GI);
}

UStarshatterAudioSubsystem* UStarshatterAudioSubsystem::Get(UGameInstance* GameInstance)
{
    return GameInstance ? GameInstance->GetSubsystem<UStarshatterAudioSubsystem>() : nullptr;
}

UStarshatterAudioSettings* UStarshatterAudioSubsystem::GetSettings() const
{
    return UStarshatterAudioSettings::Get();
}

void UStarshatterAudioSubsystem::LoadAudioConfig()
{
    if (UStarshatterAudioSettings* Settings = UStarshatterAudioSettings::Get())
    {
        Settings->Load();
    }
}

void UStarshatterAudioSubsystem::ApplySettingsToRuntime()
{
    if (UStarshatterAudioSettings* Settings = UStarshatterAudioSettings::Get())
    {
        // Pass something that can resolve a World (the subsystem can).
        Settings->ApplyToRuntimeAudio(this);
    }
}
