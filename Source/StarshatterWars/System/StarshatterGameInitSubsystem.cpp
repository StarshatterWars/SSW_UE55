/*
Project Starshatter Wars
Fractal Dev Studios
Copyright (C) 2025-2026.
All Rights Reserved.

SUBSYSTEM:    StarshatterWars (Unreal Engine)
FILE:         StarshatterGameInitSubsystem.h / .cpp
AUTHOR:       Carlos Bott

OVERVIEW
========
Post-boot game initialization coordinator.

This subsystem executes AFTER the StarshatterBootSubsystem
has completed all startup phases, including GameInstance
boot and first valid world initialization.

It is responsible for initializing runtime game systems
that require:
  - Settings to be loaded and applied
  - Core subsystems to be active
  - A valid UWorld
  - GameDataLoader to be present

This subsystem intentionally does NOT perform:
  - SaveGame loading
  - Audio, video, or input configuration
  - Boot-time service actor spawning

BOOT ORDER
==========
1) StarshatterBootSubsystem
   - GameInstance boot
   - World boot
   - GameDataLoader spawn
   - BootComplete broadcast

2) StarshatterGameInitSubsystem (this)
   - Subscribes to BootComplete
   - Executes once per game session
   - Performs runtime game initialization

DESIGN GOALS
============
- Clear separation between BOOT and GAME INIT
- Deterministic initialization order
- No startup logic in UI code
- No hidden dependencies or race conditions
- Centralized ownership of runtime initialization

OWNERSHIP RULES
===============
- BootSubsystem is the sole authority for startup
- GameInitSubsystem assumes Boot is complete
- Gameplay systems may rely on GameInit completion
- No other system may perform game initialization

This file is intentionally verbose in documentation.
Runtime initialization order is critical and must
remain explicit and well-defined.

*/
#include "StarshatterGameInitSubsystem.h"

#include "Engine/GameInstance.h"
#include "StarshatterBootSubsystem.h"

void UStarshatterGameInitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    BeginAfterBoot();
}

void UStarshatterGameInitSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UStarshatterGameInitSubsystem::BeginAfterBoot()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    UStarshatterBootSubsystem* BootSS = GI->GetSubsystem<UStarshatterBootSubsystem>();
    if (!BootSS)
        return;

    if (BootSS->IsBootComplete())
    {
        RunGameInit();
        return;
    }

    // Delay until boot completes
    BootSS->OnBootComplete.AddUObject(this, &UStarshatterGameInitSubsystem::RunGameInit);
}

void UStarshatterGameInitSubsystem::RunGameInit()
{
    if (bGameInitComplete)
        return;

    bGameInitComplete = true;

    // ----------------------------------------------------
    // GAME INIT STAGE
    // ----------------------------------------------------
    // Put any logic here that must occur AFTER:
    // - settings loaded/applied
    // - world exists
    // - GameDataLoader ensured/spawned
    //
    // Examples:
    // - Call into Starshatter core runtime init
    // - Build galaxy caches, mission lists, campaign registry
    // - Kick off async asset preload
    // - Register global UI routers that need runtime ready
    // - Validate config and report to log
    //
    // Keep this phase pure: do not duplicate Boot work.
    // ----------------------------------------------------
}
