/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024–2026. All Rights Reserved.

    SUBSYSTEM:      StarshatterPlayerSubsystem
    FILE:           StarshatterPlayerSubsystem.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UStarshatterPlayerSubsystem

    Unreal-native player profile persistence manager.

    Responsibilities:
    - Owns authoritative FS_PlayerGameInfo instance (PlayerInfo)
    - Loads player save during Boot (LoadFromBoot)
    - Writes player save on demand (SavePlayer)
    - Tracks dirty state to avoid unnecessary writes
    - Stores SaveVersion in UPlayerSaveGame and supports forward migration

    FIRST-RUN FLOW
    ==============
    Boot calls LoadFromBoot(). This subsystem records whether a save existed:
      - HadExistingSaveOnLoad() == true  -> normal menu flow
      - HadExistingSaveOnLoad() == false -> show FirstRun dialog

    IMPORTANT:
    - When no save exists, subsystem does NOT auto-create a save by default.
      FirstRun dialog is expected to create the save explicitly.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerSaveGame.h"
#include "GameStructs.h"
#include "StarshatterPlayerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerSaveLoaded, bool, bSuccess);

UCLASS()
class STARSHATTERWARS_API UStarshatterPlayerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ------------------------------------------------------------------
    // Subsystem lifecycle
    // ------------------------------------------------------------------
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
   
    static UStarshatterPlayerSubsystem* Get(const UObject* WorldContextObject);
    static UStarshatterPlayerSubsystem* Get(const UWorld* World);

    // ------------------------------------------------------------------
    // Boot entry point
    // ------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Starshatter|PlayerSave")
    bool LoadFromBoot();

    // ------------------------------------------------------------------
    // Slot configuration
    // ------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Starshatter|PlayerSave")
    void SetSaveSlot(const FString& InSlotName, int32 InUserIndex);

    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    FString GetSlotName() const { return SlotName; }

    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    int32 GetUserIndex() const { return UserIndex; }

    // ------------------------------------------------------------------
    // State / flags
    // ------------------------------------------------------------------
    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    bool HasLoaded() const { return bLoaded; }

    // Did a save exist at the moment LoadPlayer() ran?
    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    bool HadExistingSaveOnLoad() const { return bHadExistingSave; }

    // ------------------------------------------------------------------
    // Player data access
    // ------------------------------------------------------------------
    const FS_PlayerGameInfo& GetPlayerInfo() const;
    FS_PlayerGameInfo& GetMutablePlayerInfo();

    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    FS_PlayerGameInfo GetPlayerInfoCopy() const { return PlayerInfo; }

    bool DoesSaveExistNow() const;

    // ------------------------------------------------------------------
    // Save / Load
    // ------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Starshatter|PlayerSave")
    bool LoadPlayer();

    UFUNCTION(BlueprintCallable, Category = "Starshatter|PlayerSave")
    bool SavePlayer(bool bForce = false);

    // ------------------------------------------------------------------
    // Events
    // ------------------------------------------------------------------
    UPROPERTY(BlueprintAssignable, Category = "Starshatter|PlayerSave")
    FOnPlayerSaveLoaded OnPlayerSaveLoaded;

    // Read-only convenience:
    static int32 GetPlayerIdSafe(const UObject* WorldContextObject, int32 DefaultId = 0);
    static int32 GetRankIdSafe(const UObject* WorldContextObject, int32 DefaultRankId = 0);
    static FString GetPlayerNameSafe(const UObject* WorldContextObject);

    // Rank display (routes to AwardInfoRegistry):
    static FString GetRankNameSafe(const UObject* WorldContextObject, int32 RankId);
    static FString GetRankDescSafe(const UObject* WorldContextObject, int32 RankId);

    // Training:
    bool HasTrained(int32 TrainingMissionId) const;
    void SetTrained(int32 TrainingMissionId, bool bTrained);
    static bool HasTrainedSafe(const UObject* WorldContextObject, int32 TrainingMissionId);

    // Campaign completion:
    bool HasCompletedCampaign(int32 CampaignBitIndex) const;
    void SetCampaignComplete(int32 CampaignBitIndex, bool bComplete);
    static bool HasCompletedCampaignSafe(const UObject* WorldContextObject, int32 CampaignBitIndex);
    static void SetCampaignCompleteSafe(const UObject* WorldContextObject, int32 CampaignBitIndex, bool bComplete, bool bSave = true);

    // Command eligibility (rank -> permissions):
    bool CanCommand(int32 CmdClass) const;
    static bool CanCommandSafe(const UObject* WorldContextObject, int32 CmdClass);

    static int32 GetPlayerId(const UObject* WorldContextObject);
    static FString GetPlayerName(const UObject* WorldContextObject);
    static int32 GetPlayerRankId(const UObject* WorldContextObject);

    // Training helpers
    static bool HasTrainedMission(const UObject* WorldContextObject, int32 MissionId);
    static void MarkTrainedMission(const UObject* WorldContextObject, int32 MissionId);

    // Campaign completion helpers
    static bool IsCampaignComplete(const UObject* WorldContextObject, int32 CampaignIndex0Based);
    static void SetCampaignComplete(const UObject* WorldContextObject, int32 CampaignIndex0Based, bool bComplete);


    // Utility:
    void MarkDirty();

private:
    // ------------------------------------------------------------------
    // Internal helpers
    // ------------------------------------------------------------------
    bool SaveGameInternal(const FString& InSlotName, int32 InUserIndex, const FS_PlayerGameInfo& InPlayerData, int32 InSaveVersion);
    bool LoadGameInternal(const FString& InSlotName, int32 InUserIndex, FS_PlayerGameInfo& OutPlayerData, int32& OutSaveVersion);

    bool MigratePlayerSave(int32 FromVersion, int32 ToVersion, FS_PlayerGameInfo& InOutPlayerInfo);

private:
    // ------------------------------------------------------------------
    // Persistent settings
    // ------------------------------------------------------------------
    FString SlotName = TEXT("PlayerSave");
    int32   UserIndex = 0;

    // ------------------------------------------------------------------
    // Authoritative in-memory player state
    // ------------------------------------------------------------------
    FS_PlayerGameInfo PlayerInfo;

    // ------------------------------------------------------------------
    // Flags
    // ------------------------------------------------------------------
    bool bLoaded = false;
    bool bDirty = false;

    // First-run detection:
    bool bHadExistingSave = false;

    void SyncLegacySnapshot() const;

private:
    // Increment when the save schema changes
    static constexpr int32 CURRENT_SAVE_VERSION = 1;
};
