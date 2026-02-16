/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024–2026. All Rights Reserved.

    SUBSYSTEM:      StarshatterPlayerSubsystem
    FILE:           StarshatterPlayerSubsystem.cpp
    AUTHOR:         Carlos Bott

    IMPLEMENTATION
    ==============
    Player profile persistence with SaveVersion + migration.

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

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterPlayerSubsystem, Log, All);

void UStarshatterPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    bLoaded = false;
    bDirty = false;
    bHadExistingSave = false;

    // PlayerInfo stays default-constructed (FS_PlayerGameInfo default ctor).
    UE_LOG(LogStarshatterPlayerSubsystem, Log, TEXT("[PlayerSubsystem] Initialize. Slot=%s UserIndex=%d"),
        *SlotName, UserIndex);
}

void UStarshatterPlayerSubsystem::Deinitialize()
{
    UE_LOG(LogStarshatterPlayerSubsystem, Log, TEXT("[PlayerSubsystem] Deinitialize."));
    Super::Deinitialize();
}

void UStarshatterPlayerSubsystem::SetSaveSlot(const FString& InSlotName, int32 InUserIndex)
{
    SlotName = InSlotName;
    UserIndex = InUserIndex;

    UE_LOG(LogStarshatterPlayerSubsystem, Log, TEXT("[PlayerSubsystem] SetSaveSlot. Slot=%s UserIndex=%d"),
        *SlotName, UserIndex);
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

    UE_LOG(LogStarshatterPlayerSubsystem, Log, TEXT("[PlayerSubsystem] LoadPlayer. Slot=%s UserIndex=%d Exists=%s"),
        *SlotName, UserIndex, bHadExistingSave ? TEXT("YES") : TEXT("NO"));

    if (bHadExistingSave)
    {
        FS_PlayerGameInfo LoadedPlayerInfo;
        int32 LoadedSaveVersion = 0;

        if (!LoadGameInternal(SlotName, UserIndex, LoadedPlayerInfo, LoadedSaveVersion))
        {
            UE_LOG(LogStarshatterPlayerSubsystem, Error,
                TEXT("[PlayerSubsystem] LoadGameInternal FAILED. Slot=%s UserIndex=%d"),
                *SlotName, UserIndex);

            bLoaded = false;
            return false;
        }

        // Version handling
        if (LoadedSaveVersion < CURRENT_SAVE_VERSION)
        {
            UE_LOG(LogStarshatterPlayerSubsystem, Warning,
                TEXT("[PlayerSubsystem] Migrating save from v%d to v%d..."),
                LoadedSaveVersion, CURRENT_SAVE_VERSION);

            if (!MigratePlayerSave(LoadedSaveVersion, CURRENT_SAVE_VERSION, LoadedPlayerInfo))
            {
                UE_LOG(LogStarshatterPlayerSubsystem, Error,
                    TEXT("[PlayerSubsystem] Migration FAILED. From=%d To=%d"),
                    LoadedSaveVersion, CURRENT_SAVE_VERSION);

                bLoaded = false;
                return false;
            }

            PlayerInfo = LoadedPlayerInfo;
            bLoaded = true;
            bDirty = true;

            // Repair slot immediately after migration
            const bool bRepaired = SavePlayer(true);

            UE_LOG(LogStarshatterPlayerSubsystem, Log,
                TEXT("[PlayerSubsystem] Migration complete. Slot repaired=%s"),
                bRepaired ? TEXT("YES") : TEXT("NO"));
        }
        else if (LoadedSaveVersion > CURRENT_SAVE_VERSION)
        {
            // Allow loading newer saves (best-effort forward compatibility)
            UE_LOG(LogStarshatterPlayerSubsystem, Warning,
                TEXT("[PlayerSubsystem] SaveVersion v%d is newer than build supports (v%d). Loading anyway."),
                LoadedSaveVersion, CURRENT_SAVE_VERSION);

            PlayerInfo = LoadedPlayerInfo;
            bLoaded = true;
            bDirty = false;
        }
        else
        {
            PlayerInfo = LoadedPlayerInfo;
            bLoaded = true;
            bDirty = false;
        }

        UE_LOG(LogStarshatterPlayerSubsystem, Log,
            TEXT("[PlayerSubsystem] Player loaded: Name='%s' (SaveVersion=%d)"),
            *PlayerInfo.Name,
            FMath::Max(LoadedSaveVersion, CURRENT_SAVE_VERSION));

        return true;
    }

    // No save exists: keep default-constructed PlayerInfo in memory.
    // DO NOT auto-create a save here; FirstRun is expected to do that.
    bLoaded = true;
    bDirty = true;

    UE_LOG(LogStarshatterPlayerSubsystem, Log,
        TEXT("[PlayerSubsystem] No player save found. FirstRun required. Slot=%s UserIndex=%d"),
        *SlotName, UserIndex);

    return true;
}

bool UStarshatterPlayerSubsystem::SavePlayer(bool bForce)
{
    if (!bForce && !bDirty)
    {
        // Nothing to do
        return true;
    }

    const bool bOk = SaveGameInternal(SlotName, UserIndex, PlayerInfo, CURRENT_SAVE_VERSION);
    if (bOk)
    {
        bDirty = false;

        UE_LOG(LogStarshatterPlayerSubsystem, Log,
            TEXT("[PlayerSubsystem] SavePlayer OK. Slot=%s UserIndex=%d Version=%d"),
            *SlotName, UserIndex, CURRENT_SAVE_VERSION);
    }
    else
    {
        UE_LOG(LogStarshatterPlayerSubsystem, Error,
            TEXT("[PlayerSubsystem] SavePlayer FAILED. Slot=%s UserIndex=%d Version=%d"),
            *SlotName, UserIndex, CURRENT_SAVE_VERSION);
    }

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

// ------------------------------------------------------------------
// Internal helpers
// ------------------------------------------------------------------

bool UStarshatterPlayerSubsystem::SaveGameInternal(
    const FString& InSlotName,
    int32 InUserIndex,
    const FS_PlayerGameInfo& InPlayerData,
    int32 InSaveVersion)
{
    UPlayerSaveGame* SaveObject =
        Cast<UPlayerSaveGame>(UGameplayStatics::CreateSaveGameObject(UPlayerSaveGame::StaticClass()));

    if (!SaveObject)
    {
        UE_LOG(LogStarshatterPlayerSubsystem, Error,
            TEXT("[PlayerSubsystem] CreateSaveGameObject FAILED. Slot=%s UserIndex=%d"),
            *InSlotName, InUserIndex);
        return false;
    }

    SaveObject->SaveVersion = InSaveVersion;
    SaveObject->PlayerInfo = InPlayerData;

    const bool bOk = UGameplayStatics::SaveGameToSlot(SaveObject, InSlotName, InUserIndex);
    if (!bOk)
    {
        UE_LOG(LogStarshatterPlayerSubsystem, Error,
            TEXT("[PlayerSubsystem] SaveGameToSlot FAILED. Slot=%s UserIndex=%d"),
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
    USaveGame* LoadedSaveGameBase = UGameplayStatics::LoadGameFromSlot(InSlotName, InUserIndex);
    UPlayerSaveGame* LoadedSaveObject = Cast<UPlayerSaveGame>(LoadedSaveGameBase);

    if (!LoadedSaveObject)
    {
        UE_LOG(LogStarshatterPlayerSubsystem, Error,
            TEXT("[PlayerSubsystem] LoadGameFromSlot returned null or wrong class. Slot=%s UserIndex=%d"),
            *InSlotName, InUserIndex);
        return false;
    }

    OutSaveVersion = LoadedSaveObject->SaveVersion;
    OutPlayerData = LoadedSaveObject->PlayerInfo;

    return true;
}

bool UStarshatterPlayerSubsystem::MigratePlayerSave(
    int32 FromVersion,
    int32 ToVersion,
    FS_PlayerGameInfo& InOutPlayerInfo)
{
    // Stepwise migration framework:
    // Implement each step as CURRENT_SAVE_VERSION increments.
    if (FromVersion <= 0)
        FromVersion = 1;

    while (FromVersion < ToVersion)
    {
        switch (FromVersion)
        {
        case 1:
            // v1 -> v2 example (inactive until you set CURRENT_SAVE_VERSION=2):
            // - Add defaults for new fields
            // - Convert deprecated fields
            // Example:
            // if (InOutPlayerInfo.CampaignRowName.IsNone())
            //     InOutPlayerInfo.CampaignRowName = FName(TEXT("Operation Live Fire"));
            FromVersion = 2;
            break;

        default:
            UE_LOG(LogStarshatterPlayerSubsystem, Error,
                TEXT("[PlayerSubsystem] No migration path from v%d to v%d."),
                FromVersion, ToVersion);
            return false;
        }
    }

    return true;
}
