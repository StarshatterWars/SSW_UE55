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

#include "StarshatterPlayerCharacter.h"
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

    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    bool HadExistingSaveOnLoad() const { return bHadExistingSave; }

    UFUNCTION(BlueprintCallable, Category = "Starshatter|PlayerSave")
    void MarkDirty() { bDirty = true; }

    // Optional: nuke runtime state to defaults (does NOT save unless you call SavePlayer)
    UFUNCTION(BlueprintCallable, Category = "Starshatter|PlayerSave")
    void ResetToDefaults();

    // ------------------------------------------------------------------
    // Player data access
    // ------------------------------------------------------------------
    const FS_PlayerGameInfo& GetPlayerInfo() const;
    FS_PlayerGameInfo& GetMutablePlayerInfo();

    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    FS_PlayerGameInfo GetPlayerInfoCopy() const { return PlayerInfo; }

    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    bool DoesSaveExistNow() const;

    // Convenience view-model object (optional, but useful for UI)
    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    UStarshatterPlayerCharacter* GetPlayerObject() const { return PlayerObject; }

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

private:
    // ------------------------------------------------------------------
    // Internal helpers
    // ------------------------------------------------------------------
    bool SaveGameInternal(const FString& InSlotName, int32 InUserIndex,
        const FS_PlayerGameInfo& InPlayerData, int32 InSaveVersion);

    bool LoadGameInternal(const FString& InSlotName, int32 InUserIndex,
        FS_PlayerGameInfo& OutPlayerData, int32& OutSaveVersion);

    bool MigratePlayerSave(int32 FromVersion, int32 ToVersion, FS_PlayerGameInfo& InOutPlayerInfo);

    void RebuildPlayerObject();
    void SyncPlayerObjectFromInfo();
    void SyncInfoFromPlayerObject();

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

    // Optional convenience UObject mirror for UI/Blueprints
    UPROPERTY(Transient)
    TObjectPtr<UStarshatterPlayerCharacter> PlayerObject = nullptr;

    // ------------------------------------------------------------------
    // Flags
    // ------------------------------------------------------------------
    bool bLoaded = false;
    bool bDirty = false;

    bool bHadExistingSave = false;

private:
    // Increment when the save schema changes
    static constexpr int32 CURRENT_SAVE_VERSION = 1;
};
