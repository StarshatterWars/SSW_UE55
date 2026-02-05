/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterBootSubsystem.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Central bootstrapper for global subsystems that must initialize early.
*/

#include "StarshatterBootSubsystem.h"

#include "Engine/GameInstance.h"

// Subsystems
#include "FontManagerSubsystem.h"
#include "StarshatterAudioSubsystem.h"
#include "StarshatterVideoSubsystem.h"
#include "StarshatterControlsSubsystem.h"
#include "StarshatterKeyboardSubsystem.h"
#include "StarshatterSettingsSaveSubsystem.h"

// SaveGame
#include "StarshatterSettingsSaveGame.h"

// ------------------------------------------------------------
// UGameInstanceSubsystem
// ------------------------------------------------------------

void UStarshatterBootSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    BootSettings();
    BootFonts();
    BootAudio();
    BootVideo();
    BootControls();
    BootKeyboard();
}

void UStarshatterBootSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

// ------------------------------------------------------------
// Boot stages
// ------------------------------------------------------------

void UStarshatterBootSubsystem::BootSettings()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    if (UStarshatterSettingsSaveSubsystem* SaveSS = GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>())
    {
        SaveSS->LoadOrCreate();
    }
}

void UStarshatterBootSubsystem::BootFonts()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    if (UFontManagerSubsystem* FontSS = GI->GetSubsystem<UFontManagerSubsystem>())
    {
        // Optional future hooks:
        // FontSS->LoadFontConfig();
        // FontSS->ApplyToRuntimeFonts();
        (void)FontSS;
    }
}

void UStarshatterBootSubsystem::BootAudio()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    UStarshatterSettingsSaveSubsystem* SaveSS = GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>();
    UStarshatterAudioSubsystem* AudioSS = GI->GetSubsystem<UStarshatterAudioSubsystem>();

    if (!SaveSS || !AudioSS)
        return;

    SaveSS->LoadOrCreate();

    if (UStarshatterSettingsSaveGame* SG = SaveSS->GetSettings())
    {
        AudioSS->LoadFromSaveGame(SG);
        AudioSS->ApplySettingsToRuntime();
    }
    else
    {
        AudioSS->ApplySettingsToRuntime();
    }
}

void UStarshatterBootSubsystem::BootVideo()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    UStarshatterSettingsSaveSubsystem* SaveSS = GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>();
    UStarshatterVideoSubsystem* VideoSS = GI->GetSubsystem<UStarshatterVideoSubsystem>();

    if (!SaveSS || !VideoSS)
        return;

    SaveSS->LoadOrCreate();

    if (UStarshatterSettingsSaveGame* SG = SaveSS->GetSettings())
    {
        VideoSS->LoadFromSaveGame(SG);
        VideoSS->ApplySettingsToRuntime();
    }
    else
    {
        VideoSS->LoadVideoConfig(TEXT("video.cfg"), true);
        VideoSS->ApplySettingsToRuntime();
    }
}

void UStarshatterBootSubsystem::BootControls()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    UStarshatterSettingsSaveSubsystem* SaveSS = GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>();
    UStarshatterControlsSubsystem* ControlsSS = GI->GetSubsystem<UStarshatterControlsSubsystem>();
    if (!SaveSS || !ControlsSS)
        return;

    SaveSS->LoadOrCreate();

    if (UStarshatterSettingsSaveGame* SG = SaveSS->GetSettings())
    {
        ControlsSS->LoadFromSaveGame(SG);
        ControlsSS->ApplySettingsToRuntime(this);
    }
    else
    {
        ControlsSS->ApplySettingsToRuntime(this);
    }
}

void UStarshatterBootSubsystem::BootKeyboard()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    UStarshatterSettingsSaveSubsystem* SaveSS = GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>();
    UStarshatterKeyboardSubsystem* KeyboardSS = GI->GetSubsystem<UStarshatterKeyboardSubsystem>();
    if (!SaveSS || !KeyboardSS)
        return;

    SaveSS->LoadOrCreate();

    if (UStarshatterSettingsSaveGame* SG = SaveSS->GetSettings())
    {
        KeyboardSS->LoadFromSaveGame(SG);
        KeyboardSS->ApplySettingsToRuntime(this);
    }
    else
    {
        KeyboardSS->ApplySettingsToRuntime(this);
    }
}
