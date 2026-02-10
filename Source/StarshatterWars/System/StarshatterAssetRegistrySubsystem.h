/*=============================================================================
    Project:        Starshatter Wars (Unreal Engine Port)
    Studio:         Fractal Dev Studios
    Copyright:      (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:      Asset Registry
    FILE:           StarshatterAssetRegistrySubsystem.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UStarshatterAssetRegistrySubsystem

    Runtime soft-reference registry for game assets:
      - DataTables
      - Widget classes
      - Audio, textures, etc.

    Goals:
      - Remove scattered ConstructorHelpers usage
      - Centralize asset IDs and references
      - Boot-time validation via BootAssets()

    Notes:
      - This subsystem is runtime-safe (no editor-only headers)
      - Uses UDeveloperSettings (config-backed) as the source of truth

=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/SoftObjectPtr.h"
#include "StarshatterAssetRegistrySubsystem.generated.h"

class UDataTable;
class UUserWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogStarshatterAssetRegistry, Log, All);

UCLASS()
class STARSHATTERWARS_API UStarshatterAssetRegistrySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build internal cache from settings
    UFUNCTION(BlueprintCallable, Category = "Starshatter|Assets")
    bool InitRegistry();

    // Resolve an asset by ID (optionally load sync)
    UFUNCTION(BlueprintCallable, Category = "Starshatter|Assets")
    UObject* GetAsset(FName AssetId, bool bLoadSync = false);

    // Typed helpers
    UFUNCTION(BlueprintCallable, Category = "Starshatter|Assets")
    UDataTable* GetDataTable(FName AssetId, bool bLoadSync = false);

    UFUNCTION(BlueprintCallable, Category = "Starshatter|Assets")
    TSubclassOf<UUserWidget> GetWidgetClass(FName AssetId, bool bLoadSync = false);

    // Validate required asset IDs exist and (optionally) load
    UFUNCTION(BlueprintCallable, Category = "Starshatter|Assets")
    bool ValidateRequired(const TArray<FName>& RequiredIds, bool bLoadNow);

    // Quick state
    UFUNCTION(BlueprintPure, Category = "Starshatter|Assets")
    bool IsReady() const { return bReady; }

private:
    // Internal cache of AssetId -> soft pointer
    TMap<FName, TSoftObjectPtr<UObject>> Cache;

    bool bReady = false;
};
