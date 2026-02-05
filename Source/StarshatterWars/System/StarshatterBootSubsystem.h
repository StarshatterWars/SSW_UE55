#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StarshatterBootSubsystem.generated.h"

class UGameInstance;
class UWorld;

class UStarshatterSettingsSaveSubsystem;
class UStarshatterSettingsSaveGame;
class UFontManagerSubsystem;
class UStarshatterAudioSubsystem;
class UStarshatterVideoSubsystem;
class UStarshatterControlsSubsystem;
class UStarshatterKeyboardSubsystem;

/*
    Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterBootSubsystem.h / .cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Central bootstrap coordinator for Starshatter Wars.

    Responsible for deterministic initialization of all
    GameInstance-scoped systems and controlled boot of
    world-dependent services.

    BOOT PHASES
    ==========
    1) GameInstance Boot
       - Runs during Initialize()
       - No UWorld access
       - No Actor spawning

    2) World Boot
       - Triggered via OnPostWorldInitialization
       - Runs once per game session
       - Safe to spawn world-bound service Actors
*/

DECLARE_MULTICAST_DELEGATE(FOnStarshatterBootComplete);

UCLASS()
class STARSHATTERWARS_API UStarshatterBootSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    bool IsBootComplete() const { return bBootComplete; }
    FOnStarshatterBootComplete OnBootComplete;


private:
    // --------------------------------------------------
    // Boot context (shared startup state)
    // --------------------------------------------------
    struct FBootContext
    {
        UGameInstance* GI = nullptr;

        UStarshatterSettingsSaveSubsystem* SaveSS = nullptr;
        UStarshatterSettingsSaveGame* SG = nullptr;

        UFontManagerSubsystem* FontSS = nullptr;
        UStarshatterAudioSubsystem* AudioSS = nullptr;
        UStarshatterVideoSubsystem* VideoSS = nullptr;
        UStarshatterControlsSubsystem* ControlsSS = nullptr;
        UStarshatterKeyboardSubsystem* KeyboardSS = nullptr;
    };

    // --------------------------------------------------
    // GameInstance boot
    // --------------------------------------------------
    bool BuildContext(FBootContext& OutCtx);

    void BootFonts(const FBootContext& Ctx);
    void BootAudio(const FBootContext& Ctx);
    void BootVideo(const FBootContext& Ctx);
    void BootControls(const FBootContext& Ctx);
    void BootKeyboard(const FBootContext& Ctx);

    // --------------------------------------------------
    // World boot
    // --------------------------------------------------
    void HookWorldBoot();
    void OnPostWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
    void BootGameData(UWorld* World);

private:
    FDelegateHandle PostWorldInitHandle;
    bool bWorldBootDone = false;

    void MarkBootComplete();
    bool bBootComplete = false;   
};
