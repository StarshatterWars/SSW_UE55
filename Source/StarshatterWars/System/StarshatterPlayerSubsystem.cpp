/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024–2026. All Rights Reserved.

    SUBSYSTEM:      StarshatterPlayerSubsystem
    FILE:           StarshatterPlayerSubsystem.cpp
    AUTHOR:         Carlos Bott

    IMPLEMENTATION
    ==============
    Player profile persistence with SaveVersion and migration.

    Load behavior:
    - Records whether a save existed (bHadExistingSave)
    - Loads if present, migrates if needed
    - If absent, leaves defaults in memory and expects FirstRun to save

    Save behavior:
    - Writes UPlayerSaveGame { SaveVersion, PlayerInfo } to slot
=============================================================================*/

#include "StarshatterPlayerSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"

void UStarshatterPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    bLoaded = false;
    bDirty = false;
    bHadExistingSave = false;

    // PlayerInfo is default-constructed (FS_PlayerGameInfo ctor).
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


bool UStarshatterPlayerSubsystem::DoesSaveExistNow() const
{
    return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

bool UStarshatterPlayerSubsystem::LoadPlayer()
{
    // FIRST-RUN flag: did a save exist before we loaded?
    bHadExistingSave = UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);

    if (bHadExistingSave)
    {
        FS_PlayerGameInfo LoadedInfo;
        int32 LoadedVersion = 0;

        if (!LoadGameInternal(SlotName, UserIndex, LoadedInfo, LoadedVersion))
        {
            UE_LOG(LogTemp, Error, TEXT("[PlayerSubsystem] Failed to load save slot. Slot=%s UserIndex=%d"),
                *SlotName, UserIndex);
            bLoaded = false;
            return false;
        }

        // Version handling
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

            PlayerInfo = LoadedInfo;
            bLoaded = true;
            bDirty = true;

            // Repair slot immediately after migration
            SavePlayer(true);

            UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Migration complete; slot repaired."));
        }
        else if (LoadedVersion > CURRENT_SAVE_VERSION)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[PlayerSubsystem] SaveVersion v%d is newer than build supports (v%d). Loading anyway."),
                LoadedVersion, CURRENT_SAVE_VERSION);

            PlayerInfo = LoadedInfo;
            bLoaded = true;
            bDirty = false;
        }
        else
        {
            PlayerInfo = LoadedInfo;
            bLoaded = true;
            bDirty = false;
        }

        UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] Player loaded: %s (SaveVersion=%d)"),
            *PlayerInfo.Name, FMath::Max(LoadedVersion, CURRENT_SAVE_VERSION));

        return true;
    }

    // No save exists: keep default-constructed PlayerInfo in memory.
    // DO NOT auto-create a save here; FirstRun is expected to do that.
    bLoaded = true;
    bDirty = true;

    UE_LOG(LogTemp, Log, TEXT("[PlayerSubsystem] No player save found. FirstRun required."));
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
    // Stepwise migration framework
    if (FromVersion <= 0)
        FromVersion = 1;

    while (FromVersion < ToVersion)
    {
        switch (FromVersion)
        {
        case 1:
            // v1 -> v2 example (not active yet)
            // When CURRENT_SAVE_VERSION becomes 2, implement defaults/transform here.
            // Example:
            // if (InOutPlayerInfo.CampaignRowName.IsNone()) InOutPlayerInfo.CampaignRowName = TEXT("Operation Live Fire");
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
