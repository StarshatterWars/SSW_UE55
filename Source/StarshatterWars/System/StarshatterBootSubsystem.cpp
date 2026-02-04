/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025–2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterBootSubsystem.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Implements early startup orchestration for Starshatter subsystems.

    IMPORTANT
    =========
    Do NOT forward-declare subsystem classes here if you need to call methods.
    Always include the subsystem headers so the compiler sees full declarations.
*/

#include "StarshatterBootSubsystem.h"

#include "Engine/GameInstance.h"

// IMPORTANT: include the actual subsystem headers (not forward declarations):
#include "FontManagerSubsystem.h"
#include "StarshatterAudioSubsystem.h"
#include "StarshatterVideoSubsystem.h"

void UStarshatterBootSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    BootFonts();
    BootAudio();
    BootVideo();
}

void UStarshatterBootSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UStarshatterBootSubsystem::BootFonts()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    if (UFontManagerSubsystem* FontSS = GI->GetSubsystem<UFontManagerSubsystem>())
    {
        // If your font subsystem has a load/apply method, call it here.
        // FontSS->LoadFontConfig();
        // FontSS->ApplyToRuntimeFonts();
    }
}

void UStarshatterBootSubsystem::BootAudio()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    if (UStarshatterAudioSubsystem* AudioSS = GI->GetSubsystem<UStarshatterAudioSubsystem>())
    {
        // This is the call that was failing for you:
        //AudioSS->LoadAudioConfig();

        // Optional (LoadAudioConfig already calls ApplyToRuntimeAudio in my implementation):
        // AudioSS->ApplyToRuntimeAudio();
    }
}

void UStarshatterBootSubsystem::BootVideo()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    if (UStarshatterVideoSubsystem* VideoSS = GI->GetSubsystem<UStarshatterVideoSubsystem>())
    {
        // If your video subsystem has a load/apply method, call it here.
        // VideoSS->LoadVideoConfig();
        // VideoSS->ApplyToRuntimeVideo();
    }
}
