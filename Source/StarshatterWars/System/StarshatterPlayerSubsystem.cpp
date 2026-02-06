/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024–2026. All Rights Reserved.

    SUBSYSTEM:      StarshatterPlayerSubsystem
    FILE:           StarshatterPlayerSubsystem.cpp
    AUTHOR:         Carlos Bott

    IMPLEMENTATION
    ==============
    Centralized player save/load manager with SaveVersion support.

    - Loads UPlayerSaveGame
    - Extracts SaveVersion + PlayerInfo
    - Migrates older saves forward (in memory)
    - Optionally resaves after migration to repair the slot
=============================================================================*/

#include "StarshatterPlayerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"

void UStarshatterPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    bLoaded = false;
    bDirty = false;
}

void UStarshatterPlayerSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UStarshatterPlayerSubsystem::SetSaveSlot(const FString& InSlotName, int32 InUserIndex)
{
    SlotName = InSlotName;
    UserIndex = InUserIndex;
}

bool UStarshatterPlayerSubsystem::LoadFromBoot()
{
    const bool bOk = LoadPlayer();
    OnPlayerSaveLoaded.Broadcast(bOk);
    return bOk;
}

bool UStarshatterPlayerSubsystem::LoadPlayer()
{
    if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        FS_PlayerGameInfo LoadedInfo;
        int32 LoadedVersion = 0;

        if (!LoadGameInternal(SlotName, UserIndex, LoadedInfo, LoadedVersion))
        {
            UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Failed to load save slot."));
            bLoaded = false;
            return false;
        }

        if (LoadedVersion < CURRENT_SAVE_VERSION)
        {
            UE_LOG(LogTemp, Warning, TEXT("[PlayerSubsystem] Migrating save from v%d to v%d..."),
                LoadedVersion, CURRENT_SAVE_VERSION);

            if (!MigratePlayerSave(LoadedVersion, CURRENT_SAVE_VERSION, LoadedInfo))
            {
                UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Migration failed."));
                bLoaded = false;
                return false;
            }

            // Adopt migrated data
            PlayerInfo = LoadedInfo;
            bLoaded = true;
            bDirty = true; // mark dirty so we can write repaired save

            // Repair the slot immediately
            SavePlayer(true);

            UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Migration complete; slot repaired."));
        }
        else if (LoadedVersion > CURRENT_SAVE_VERSION)
        {
            // Save from a newer build; accept but warn
            UE_LOG(LogTemp, Warning,
                TEXT("[PlayerSubsystem] SaveVersion v%d is newer than build supports (v%d). Attempting to load anyway."),
                LoadedVersion, CURRENT_SAVE_VERSION);

            PlayerInfo = LoadedInfo;
            bLoaded = true;
            bDirty = false;
        }
        else
        {
            // Normal load
            PlayerInfo = LoadedInfo;
            bLoaded = true;
            bDirty = false;
        }

        UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Player loaded: %s (SaveVersion=%d)"),
            *PlayerInfo.Name, FMath::Max(LoadedVersion, CURRENT_SAVE_VERSION));

        return true;
    }

    // No save exists — keep defaults (FS_PlayerGameInfo ctor) and create slot
    bLoaded = true;
    bDirty = true;

    SavePlayer(true);

    UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] No save found. Created default player (SaveVersion=%d)."),
        CURRENT_SAVE_VERSION);

    return true;
}

bool UStarshatterPlayerSubsystem::SavePlayer(bool bForce)
{
    if (!bForce && !bDirty)
        return true;

    const bool bOk = SaveGameInternal(SlotName, UserIndex, PlayerInfo, CURRENT_SAVE_VERSION);
    if (bOk)
        bDirty = false;

    return bOk;
}

const FS_PlayerGameInfo& UStarshatterPlayerSubsystem::GetPlayerInfo() const
{
    return PlayerInfo;
}

FS_PlayerGameInfo& UStarshatterPlayerSubsystem::GetMutablePlayerInfo()
{
    bDirty = true;
    return PlayerInfo;
}

bool UStarshatterPlayerSubsystem::SaveGameInternal(
    const FString& InSlotName,
    int32 InUserIndex,
    const FS_PlayerGameInfo& InPlayerData,
    int32 InSaveVersion)
{
    UPlayerSaveGame* SaveInstance =
        Cast<UPlayerSaveGame>(UGameplayStatics::CreateSaveGameObject(UPlayerSaveGame::StaticClass()));

    if (!SaveInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] CreateSaveGameObject failed."));
        return false;
    }

    SaveInstance->SaveVersion = InSaveVersion;
    SaveInstance->PlayerInfo = InPlayerData;

    const bool bOk = UGameplayStatics::SaveGameToSlot(SaveInstance, InSlotName, InUserIndex);
    if (!bOk)
    {
        UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] SaveGameToSlot failed. Slot=%s UserIndex=%d"),
            *InSlotName, InUserIndex);
    }

    return bOk;
}

bool UStarshatterPlayerSubsystem::LoadGameInternal(
    const FString& InSlotName,
    int32 InUserIndex,
    FS_PlayerGameInfo& OutPlayerData,
    int32& OutSaveVersion)
{
    UPlayerSaveGame* LoadedGame =
        Cast<UPlayerSaveGame>(UGameplayStatics::LoadGameFromSlot(InSlotName, InUserIndex));

    if (!LoadedGame)
    {
        UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] LoadGameFromSlot returned null/wrong class. Slot=%s UserIndex=%d"),
            *InSlotName, InUserIndex);
        return false;
    }

    OutSaveVersion = LoadedGame->SaveVersion;
    OutPlayerData = LoadedGame->PlayerInfo;
    return true;
}

bool UStarshatterPlayerSubsystem::MigratePlayerSave(int32 FromVersion, int32 ToVersion, FS_PlayerGameInfo& InOutPlayerInfo)
{
    // IMPORTANT:
    // This is a stub designed for incremental migrations.
    // Add cases as you bump CURRENT_SAVE_VERSION.

    if (FromVersion <= 0)
    {
        // If you ever had “unversioned” saves, normalize here:
        FromVersion = 1;
    }

    // Stepwise migrate:
    while (FromVersion < ToVersion)
    {
        switch (FromVersion)
        {
        case 1:
            // v1 -> v2 example (not currently used since CURRENT_SAVE_VERSION is 1)
            // Add new fields defaults here when you bump to v2.
            // Example:
            // if (InOutPlayerInfo.NewField == 0) InOutPlayerInfo.NewField = 123;
            FromVersion = 2;
            break;

        default:
            UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] No migration path from v%d to v%d."),
                FromVersion, ToVersion);
            return false;
        }
    }

    return true;
}
