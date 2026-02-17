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

    // PlayerInfo stays default-constructed.
    UE_LOG(LogStarshatterPlayerSubsystem, Log, TEXT("[PlayerSubsystem] Initialize. Slot=%s UserIndex=%d"),
        *SlotName, UserIndex);

    // Build PlayerObject early so UI can bind even before load.
    RebuildPlayerObject();
    SyncPlayerObjectFromInfo();
}

void UStarshatterPlayerSubsystem::Deinitialize()
{
    UE_LOG(LogStarshatterPlayerSubsystem, Log, TEXT("[PlayerSubsystem] Deinitialize."));
    PlayerObject = nullptr;
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

void UStarshatterPlayerSubsystem::ResetToDefaults()
{
    PlayerInfo = FS_PlayerGameInfo();
    bDirty = true;

    RebuildPlayerObject();
    SyncPlayerObjectFromInfo();

    UE_LOG(LogStarshatterPlayerSubsystem, Log, TEXT("[PlayerSubsystem] ResetToDefaults (in-memory only)."));
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
            UE_LOG(LogStarshatterPlayerSubsystem, Warning,
                TEXT("[PlayerSubsystem] SaveVersion v%d newer than build supports (v%d). Loading anyway."),
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

        // Ensure PlayerObject exists + is synced
        RebuildPlayerObject();
        SyncPlayerObjectFromInfo();

        UE_LOG(LogStarshatterPlayerSubsystem, Log,
            TEXT("[PlayerSubsystem] Player loaded: Name='%s' Id=%d Rank=%d (SaveVersion=%d)"),
            *PlayerInfo.Name,
            PlayerInfo.Id,
            PlayerInfo.Rank,
            FMath::Max(LoadedSaveVersion, CURRENT_SAVE_VERSION));

        return true;
    }

    // No save exists: keep defaults in memory; FirstRun is expected to save.
    bLoaded = true;
    bDirty = true;

    RebuildPlayerObject();
    SyncPlayerObjectFromInfo();

    UE_LOG(LogStarshatterPlayerSubsystem, Log,
        TEXT("[PlayerSubsystem] No player save found. FirstRun required. Slot=%s UserIndex=%d"),
        *SlotName, UserIndex);

    return true;
}

bool UStarshatterPlayerSubsystem::SavePlayer(bool bForce)
{
    // If you allow UI to edit PlayerObject directly, pull it back before saving.
    // If you never edit PlayerObject, this is harmless.
    SyncInfoFromPlayerObject();

    if (!bForce && !bDirty)
        return true;

    const bool bOk = SaveGameInternal(SlotName, UserIndex, PlayerInfo, CURRENT_SAVE_VERSION);
    if (bOk)
    {
        bDirty = false;

        UE_LOG(LogStarshatterPlayerSubsystem, Log,
            TEXT("[PlayerSubsystem] SavePlayer OK. Slot=%s UserIndex=%d Version=%d Name='%s'"),
            *SlotName, UserIndex, CURRENT_SAVE_VERSION, *PlayerInfo.Name);
    }
    else
    {
        UE_LOG(LogStarshatterPlayerSubsystem, Error,
            TEXT("[PlayerSubsystem] SavePlayer FAILED. Slot=%s UserIndex=%d Version=%d"),
            *SlotName, UserIndex, CURRENT_SAVE_VERSION);
    }

    return bOk;
}

// ------------------------------------------------------------------
// PlayerObject sync
// ------------------------------------------------------------------

void UStarshatterPlayerSubsystem::RebuildPlayerObject()
{
    if (PlayerObject)
        return;

    // Outer = subsystem (safe; transient)
    PlayerObject = NewObject<UStarshatterPlayerCharacter>(this, UStarshatterPlayerCharacter::StaticClass(), TEXT("PlayerObject"));
    if (!PlayerObject)
    {
        UE_LOG(LogStarshatterPlayerSubsystem, Error, TEXT("[PlayerSubsystem] RebuildPlayerObject FAILED (NewObject returned null)."));
        return;
    }

    // So Commit() can call back into the subsystem
    PlayerObject->Initialize(this);

    UE_LOG(LogStarshatterPlayerSubsystem, Log, TEXT("[PlayerSubsystem] PlayerObject created."));
}

void UStarshatterPlayerSubsystem::SyncPlayerObjectFromInfo()
{
    if (!PlayerObject)
        return;

    // Full-field mirror if you implement it inside PlayerCharacter.
    // If your PlayerCharacter currently only mirrors a subset, that’s fine too,
    // but for “logbook correctness” you want ALL the fields.
    PlayerObject->FromPlayerInfo(PlayerInfo);
}

void UStarshatterPlayerSubsystem::SyncInfoFromPlayerObject()
{
    if (!PlayerObject)
        return;

    // If PlayerCharacter is used as a view-model with edits,
    // mirror it back into PlayerInfo.
    PlayerObject->ToPlayerInfo(PlayerInfo);

    // If ToPlayerInfo modifies, we consider it dirty
    // (PlayerCharacter can also track its own bDirty, but subsystem owns persistence)
    bDirty = true;
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
    if (FromVersion <= 0)
        FromVersion = 1;

    while (FromVersion < ToVersion)
    {
        switch (FromVersion)
        {
        case 1:
            // v1 -> v2 example (inactive until you set CURRENT_SAVE_VERSION=2)
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
