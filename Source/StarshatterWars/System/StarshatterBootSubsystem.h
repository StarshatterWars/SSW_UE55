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
    GameInstance-scoped systems at startup.

    This subsystem performs early boot tasks only:
      - Settings load
      - Audio apply
      - Video apply
      - Controls apply
      - Keyboard apply
      - Fonts (optional)

    IMPORTANT
    =========
    This subsystem does NOT spawn Actors and does NOT require UWorld.
    Any heavy game data parsing/generation should be triggered after
    boot (typically during EGameMode::INIT) via the GameInit subsystem.

    GAME MODE OWNERSHIP
    ===================
    BootSubsystem sets EGameMode::BOOT at the start of Initialize().
    It does not advance beyond BOOT.

    BOOT COMPLETE
    =============
    BootComplete is broadcast once the boot sequence finishes.
    Systems that require a completed boot (ex: GameInitSubsystem)
    should subscribe to OnBootComplete.
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StarshatterBootSubsystem.generated.h"

class UGameInstance;

class UStarshatterSettingsSaveSubsystem;
class UStarshatterSettingsSaveGame;
class UFontManagerSubsystem;
class UStarshatterAudioSubsystem;
class UStarshatterVideoSubsystem;
class UStarshatterControlsSubsystem;
class UStarshatterKeyboardSubsystem;
class UStarshatterGameDataSubsystem;

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

    // Optional helper: triggers the GameData subsystem loader.
    // Recommended usage: call this from GameInitSubsystem during EGameMode::INIT.
    void BootGameDataLoader(bool bFull = false);

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

    bool BuildContext(FBootContext& OutCtx);

    void BootFonts(const FBootContext& Ctx);
    void BootAudio(const FBootContext& Ctx);
    void BootVideo(const FBootContext& Ctx);
    void BootControls(const FBootContext& Ctx);
    void BootKeyboard(const FBootContext& Ctx);

    void MarkBootComplete();

private:
    bool bBootComplete = false;
};
