/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterBootSubsystem.cpp
    AUTHOR:       Carlos Bott
*/

#include "StarshatterBootSubsystem.h"

#include "FontManager.h"          // your dual-path manager
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterBoot, Log, All);

void UStarshatterBootSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (bBooted)
        return;

    bBooted = true;

    UE_LOG(LogStarshatterBoot, Log, TEXT("Starshatter boot subsystem initializing..."));

    // Fonts: register once (feeds legacy SystemFont* + UE FSlateFontInfo registry)
    FontManager::RegisterAllFonts(this);

    // TODO: add other one-time init here:
    // - ensure music controller exists
    // - load Galaxy.def into UGalaxyManager
    // - register input mappings
    // - etc.
}

void UStarshatterBootSubsystem::Deinitialize()
{
    UE_LOG(LogStarshatterBoot, Log, TEXT("Starshatter boot subsystem deinitializing..."));

    // Optional: if you want to clear registries on shutdown
    // FontManager::Close();
    // FontManager::CloseUE();

    Super::Deinitialize();
}
