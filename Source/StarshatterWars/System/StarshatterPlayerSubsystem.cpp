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
#include "Engine/World.h"
#include "AwardInfoRegistry.h"
#include "PlayerProgression.h"
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
            PlayerProgression::SetCurrentRankId(PlayerInfo.Rank)
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

UStarshatterPlayerSubsystem* UStarshatterPlayerSubsystem::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
        return nullptr;

    const UWorld* World = WorldContextObject->GetWorld();
    return Get(World);
}

UStarshatterPlayerSubsystem* UStarshatterPlayerSubsystem::Get(const UWorld* World)
{
    if (!World)
        return nullptr;

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
        return nullptr;

    return GI->GetSubsystem<UStarshatterPlayerSubsystem>();
}
void UStarshatterPlayerSubsystem::MarkDirty()
{
    bDirty = true;
}

int32 UStarshatterPlayerSubsystem::GetPlayerIdSafe(const UObject* WorldContextObject, int32 DefaultId)
{
    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
        return SS->GetPlayerInfo().Id;

    return DefaultId;
}

int32 UStarshatterPlayerSubsystem::GetRankIdSafe(const UObject* WorldContextObject, int32 DefaultRankId)
{
    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
        return SS->GetPlayerInfo().Rank;

    return DefaultRankId;
}

FString UStarshatterPlayerSubsystem::GetPlayerNameSafe(const UObject* WorldContextObject)
{
    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
        return SS->GetPlayerInfo().Name;

    return FString();
}

FString UStarshatterPlayerSubsystem::GetRankNameSafe(const UObject* WorldContextObject, int32 RankId)
{
    // WorldContextObject is unused right now, but keeping it consistent
    // lets you later validate initialization, log warnings, etc.
    (void)WorldContextObject;

    return UAwardInfoRegistry::RankName(RankId);
}

FString UStarshatterPlayerSubsystem::GetRankDescSafe(const UObject* WorldContextObject, int32 RankId)
{
    (void)WorldContextObject;
    return UAwardInfoRegistry::RankDescription(RankId);
}

// ------------------------------------------------------------
// Training helpers
// FS_PlayerGameInfo has: TrainingMask, HighestTrainingMission, Trained
// ------------------------------------------------------------

bool UStarshatterPlayerSubsystem::HasTrained(int32 TrainingMissionId) const
{
    if (TrainingMissionId <= 0 || TrainingMissionId > 63)
        return false;

    const int64 Bit = (int64)1 << TrainingMissionId;
    return (PlayerInfo.TrainingMask & Bit) != 0;
}

void UStarshatterPlayerSubsystem::SetTrained(int32 TrainingMissionId, bool bTrained)
{
    if (TrainingMissionId <= 0 || TrainingMissionId > 63)
        return;

    const int64 Bit = (int64)1 << TrainingMissionId;

    if (bTrained)
    {
        PlayerInfo.TrainingMask |= Bit;
        PlayerInfo.HighestTrainingMission = FMath::Max(PlayerInfo.HighestTrainingMission, TrainingMissionId);
        PlayerInfo.Trained = FMath::Max(PlayerInfo.Trained, TrainingMissionId);
    }
    else
    {
        PlayerInfo.TrainingMask &= ~Bit;
        // do not try to recompute HighestTrainingMission here (not worth it)
    }

    bDirty = true;
}

bool UStarshatterPlayerSubsystem::HasTrainedSafe(const UObject* WorldContextObject, int32 TrainingMissionId)
{
    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
        return SS->HasTrained(TrainingMissionId);

    return false;
}

// ------------------------------------------------------------
// Campaign completion (bit index 0..63)
// ------------------------------------------------------------

bool UStarshatterPlayerSubsystem::HasCompletedCampaign(int32 CampaignBitIndex) const
{
    if (CampaignBitIndex < 0 || CampaignBitIndex > 63)
        return false;

    const int64 Bit = (int64)1 << CampaignBitIndex;
    return (PlayerInfo.CampaignCompleteMask & Bit) != 0;
}

void UStarshatterPlayerSubsystem::SetCampaignComplete(int32 CampaignBitIndex, bool bComplete)
{
    if (CampaignBitIndex < 0 || CampaignBitIndex > 63)
        return;

    const int64 Bit = (int64)1 << CampaignBitIndex;

    if (bComplete)
        PlayerInfo.CampaignCompleteMask |= Bit;
    else
        PlayerInfo.CampaignCompleteMask &= ~Bit;

    bDirty = true;
}

bool UStarshatterPlayerSubsystem::HasCompletedCampaignSafe(const UObject* WorldContextObject, int32 CampaignBitIndex)
{
    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
        return SS->HasCompletedCampaign(CampaignBitIndex);

    return false;
}

void UStarshatterPlayerSubsystem::SetCampaignCompleteSafe(
    const UObject* WorldContextObject,
    int32 CampaignBitIndex,
    bool bComplete,
    bool bSave)
{
    UStarshatterPlayerSubsystem* SS = Get(WorldContextObject);
    if (!SS)
        return;

    SS->SetCampaignComplete(CampaignBitIndex, bComplete);

    if (bSave)
        SS->SavePlayer(true);
}

// ------------------------------------------------------------
// Command eligibility
// Uses rank row GrantedShipClasses mask.
// CmdClass should map to a bit position (legacy behavior).
// ------------------------------------------------------------

bool UStarshatterPlayerSubsystem::CanCommand(int32 CmdClass) const
{
    const int32 RankId = PlayerInfo.Rank;
    const FRankInfo* RankRow = UAwardInfoRegistry::FindRank(RankId);
    if (!RankRow)
        return false;

    // Legacy pattern was "grant" bitmask check.
    // You are already storing GrantedShipClasses on FRankInfo.
    const int32 Mask = RankRow->GrantedShipClasses;

    if (CmdClass < 0 || CmdClass >= 31)
        return false;

    const int32 Bit = 1 << CmdClass;
    return (Mask & Bit) != 0;
}

bool UStarshatterPlayerSubsystem::CanCommandSafe(const UObject* WorldContextObject, int32 CmdClass)
{
    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
        return SS->CanCommand(CmdClass);

    return false;
}
int32 UStarshatterPlayerSubsystem::GetPlayerId(const UObject* WorldContextObject)
{
    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
        return SS->GetPlayerInfo().Id;
    return 0;
}

FString UStarshatterPlayerSubsystem::GetPlayerName(const UObject* WorldContextObject)
{
    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
        return SS->GetPlayerInfo().Name;
    return TEXT("");
}

int32 UStarshatterPlayerSubsystem::GetPlayerRankId(const UObject* WorldContextObject)
{
    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
        return SS->GetPlayerInfo().Rank;
    return 0;
}

bool UStarshatterPlayerSubsystem::HasTrainedMission(const UObject* WorldContextObject, int32 MissionId)
{
    if (MissionId <= 0)
        return false;

    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
    {
        const FS_PlayerGameInfo& Info = SS->GetPlayerInfo();

        // Legacy style: treat TrainingMask as bitset (MissionId assumed 0..63 or 1..64)
        // If your mission ids are 1-based, convert here:
        const int32 BitIndex = MissionId; // change to (MissionId - 1) if needed

        if (BitIndex < 0 || BitIndex >= 64)
            return false;

        const int64 Mask = (1ll << BitIndex);
        return (Info.TrainingMask & Mask) != 0;
    }

    return false;
}

void UStarshatterPlayerSubsystem::MarkTrainedMission(const UObject* WorldContextObject, int32 MissionId)
{
    if (MissionId <= 0)
        return;

    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
    {
        FS_PlayerGameInfo& Info = SS->GetMutablePlayerInfo();

        const int32 BitIndex = MissionId; // change to (MissionId - 1) if needed
        if (BitIndex < 0 || BitIndex >= 64)
            return;

        const int64 Mask = (1ll << BitIndex);
        Info.TrainingMask |= Mask;

        Info.HighestTrainingMission = FMath::Max(Info.HighestTrainingMission, MissionId);
        Info.Trained = Info.HighestTrainingMission; // keep legacy field aligned

        SS->SavePlayer(false);
        SS->SyncLegacySnapshot();
    }
}

bool UStarshatterPlayerSubsystem::IsCampaignComplete(const UObject* WorldContextObject, int32 CampaignIndex0Based)
{
    if (CampaignIndex0Based < 0 || CampaignIndex0Based >= 64)
        return false;

    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
    {
        const FS_PlayerGameInfo& Info = SS->GetPlayerInfo();
        const int64 Mask = (1ll << CampaignIndex0Based);
        return (Info.CampaignCompleteMask & Mask) != 0;
    }

    return false;
}

void UStarshatterPlayerSubsystem::SetCampaignComplete(
    const UObject* WorldContextObject, int32 CampaignIndex0Based, bool bComplete)
{
    if (CampaignIndex0Based < 0 || CampaignIndex0Based >= 64)
        return;

    if (UStarshatterPlayerSubsystem* SS = Get(WorldContextObject))
    {
        FS_PlayerGameInfo& Info = SS->GetMutablePlayerInfo();

        const int64 Mask = (1ll << CampaignIndex0Based);
        if (bComplete) Info.CampaignCompleteMask |= Mask;
        else           Info.CampaignCompleteMask &= ~Mask;

        SS->SavePlayer(false);
        SS->SyncLegacySnapshot();
    }
}