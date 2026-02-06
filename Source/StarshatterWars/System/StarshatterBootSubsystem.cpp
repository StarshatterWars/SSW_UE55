/*
    Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterBootSubsystem.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Boot sequence coordinator for GameInstance-scoped systems.

    No world boot is performed here. No actors are spawned.
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

// Game data subsystem (replaces AGameDataLoader actor)
#include "StarshatterGameDataSubsystem.h"

// --------------------------------------------------
// UGameInstanceSubsystem
// --------------------------------------------------

void UStarshatterBootSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FBootContext Ctx;
    if (BuildContext(Ctx))
    {
        BootFonts(Ctx);
        BootAudio(Ctx);
        BootVideo(Ctx);
        BootControls(Ctx);
        BootKeyboard(Ctx);
    }

    // Do NOT auto-load heavy content here unless you explicitly want boot-time generation.
    // Recommended: call BootGameDataLoader() from GameInitSubsystem during INIT.
    //
    // BootGameDataLoader(false);

    MarkBootComplete();
}

void UStarshatterBootSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

// --------------------------------------------------
// Optional loader hook
// --------------------------------------------------

void UStarshatterBootSubsystem::BootGameDataLoader(bool bFull)
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    UStarshatterGameDataSubsystem* DataSS =
        GI->GetSubsystem<UStarshatterGameDataSubsystem>();

    if (!DataSS)
        return;

    DataSS->LoadAll(bFull);
}

// --------------------------------------------------
// Boot stages
// --------------------------------------------------

bool UStarshatterBootSubsystem::BuildContext(FBootContext& OutCtx)
{
    OutCtx.GI = GetGameInstance();
    if (!OutCtx.GI)
        return false;

    OutCtx.SaveSS = OutCtx.GI->GetSubsystem<UStarshatterSettingsSaveSubsystem>();
    if (OutCtx.SaveSS)
    {
        OutCtx.SaveSS->LoadOrCreate();
        OutCtx.SG = OutCtx.SaveSS->GetSettings();
    }

    OutCtx.FontSS = OutCtx.GI->GetSubsystem<UFontManagerSubsystem>();
    OutCtx.AudioSS = OutCtx.GI->GetSubsystem<UStarshatterAudioSubsystem>();
    OutCtx.VideoSS = OutCtx.GI->GetSubsystem<UStarshatterVideoSubsystem>();
    OutCtx.ControlsSS = OutCtx.GI->GetSubsystem<UStarshatterControlsSubsystem>();
    OutCtx.KeyboardSS = OutCtx.GI->GetSubsystem<UStarshatterKeyboardSubsystem>();

    return true;
}

void UStarshatterBootSubsystem::BootFonts(const FBootContext& Ctx)
{
    if (!Ctx.FontSS)
        return;

    // Reserved for future font config hooks
}

void UStarshatterBootSubsystem::BootAudio(const FBootContext& Ctx)
{
    if (!Ctx.AudioSS)
        return;

    if (Ctx.SG)
        Ctx.AudioSS->LoadFromSaveGame(Ctx.SG);

    Ctx.AudioSS->ApplySettingsToRuntime();
}

void UStarshatterBootSubsystem::BootVideo(const FBootContext& Ctx)
{
    if (!Ctx.VideoSS)
        return;

    if (Ctx.SG)
        Ctx.VideoSS->LoadFromSaveGame(Ctx.SG);
    else
        Ctx.VideoSS->LoadVideoConfig(TEXT("video.cfg"), true);

    Ctx.VideoSS->ApplySettingsToRuntime();
}

void UStarshatterBootSubsystem::BootControls(const FBootContext& Ctx)
{
    if (!Ctx.ControlsSS)
        return;

    if (Ctx.SG)
        Ctx.ControlsSS->LoadFromSaveGame(Ctx.SG);

    Ctx.ControlsSS->ApplySettingsToRuntime(this);
}

void UStarshatterBootSubsystem::BootKeyboard(const FBootContext& Ctx)
{
    if (!Ctx.KeyboardSS)
        return;

    if (Ctx.SG)
        Ctx.KeyboardSS->LoadFromSaveGame(Ctx.SG);

    Ctx.KeyboardSS->ApplySettingsToRuntime(this);
}

// --------------------------------------------------
// Boot completion
// --------------------------------------------------

void UStarshatterBootSubsystem::MarkBootComplete()
{
    if (bBootComplete)
        return;

    bBootComplete = true;
    OnBootComplete.Broadcast();
}
