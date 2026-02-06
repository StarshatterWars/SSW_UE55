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

    Centralized player save/load manager.
    Owns FS_PlayerGameInfo and reads/writes UPlayerSaveGame.

    VERSIONING
    ==========
    SaveVersion is stored inside UPlayerSaveGame.
    - On load:
        * Older versions are migrated forward in memory
        * Newer versions are accepted but logged (future build case)
    - After migration, subsystem can optionally re-save to repair slot.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerSaveGame.h"
#include "StarshatterPlayerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerSaveLoaded, bool, bSuccess);

UCLASS()
class STARSHATTERWARS_API UStarshatterPlayerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Starshatter|PlayerSave")
    bool LoadFromBoot();

    UFUNCTION(BlueprintCallable, Category = "Starshatter|PlayerSave")
    void SetSaveSlot(const FString& InSlotName, int32 InUserIndex);

    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    FString GetSlotName() const { return SlotName; }

    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    int32 GetUserIndex() const { return UserIndex; }

    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    bool HasLoaded() const { return bLoaded; }

    const FS_PlayerGameInfo& GetPlayerInfo() const;
    FS_PlayerGameInfo& GetMutablePlayerInfo();

    UFUNCTION(BlueprintPure, Category = "Starshatter|PlayerSave")
    FS_PlayerGameInfo GetPlayerInfoCopy() const { return PlayerInfo; }

    UFUNCTION(BlueprintCallable, Category = "Starshatter|PlayerSave")
    bool LoadPlayer();

    UFUNCTION(BlueprintCallable, Category = "Starshatter|PlayerSave")
    bool SavePlayer(bool bForce = false);

    UPROPERTY(BlueprintAssignable, Category = "Starshatter|PlayerSave")
    FOnPlayerSaveLoaded OnPlayerSaveLoaded;

private:
    // Internal helpers
    bool SaveGameInternal(const FString& InSlotName, int32 InUserIndex, const FS_PlayerGameInfo& InPlayerData, int32 InSaveVersion);
    bool LoadGameInternal(const FString& InSlotName, int32 InUserIndex, FS_PlayerGameInfo& OutPlayerData, int32& OutSaveVersion);

    // Versioning / migration
    bool MigratePlayerSave(int32 FromVersion, int32 ToVersion, FS_PlayerGameInfo& InOutPlayerInfo);

private:
    FString SlotName = TEXT("PlayerSave");
    int32   UserIndex = 0;

    FS_PlayerGameInfo PlayerInfo;

    bool bLoaded = false;
    bool bDirty = false;

private:
    // Increment this when the save schema changes
    static constexpr int32 CURRENT_SAVE_VERSION = 1;
};
