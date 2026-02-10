/*=============================================================================
    Project:        Starshatter Wars (Unreal Engine Port)
    Studio:         Fractal Dev Studios
    Copyright:      (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:      Asset Registry
    FILE:           StarshatterAssetRegistrySettings.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UStarshatterAssetRegistrySettings

    Config-backed asset registry bindings for Starshatter Wars.

    Stores authoritative bindings of:
      - AssetId -> Soft object reference (UObject)
      - Optional convenience bindings (ex: WeaponDesignTable) for high-traffic assets

    Used by UStarshatterAssetRegistrySubsystem during BootAssets() to:
      - Initialize registry cache
      - Validate required IDs
      - Load assets synchronously if requested

    This class contains NO loading logic. Runtime resolution is performed
    by the subsystem only.

=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPtr.h"

#include "StarshatterAssetRegistrySettings.generated.h"

UCLASS(config = Game, defaultconfig, BlueprintType)
class STARSHATTERWARS_API UStarshatterAssetRegistrySettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    // ------------------------------------------------------------------
    // Project Settings placement
    // ------------------------------------------------------------------
    virtual FName GetCategoryName() const override
    {
        return TEXT("Starshatter Wars");
    }

    virtual FName GetSectionName() const override
    {
        return TEXT("Asset Registry");
    }

    virtual FText GetSectionText() const override
    {
        return FText::FromString("Asset Registry");
    }

    virtual FText GetSectionDescription() const override
    {
        return FText::FromString(
            "Centralized asset bindings used during boot to validate and load required assets."
        );
    }

    // ------------------------------------------------------------------
    // Registry bindings
    // ------------------------------------------------------------------

    // Map of AssetId -> Soft reference
    // Examples:
    //   Data.CampaignTable     -> /Game/Game/DT_Campaign.DT_Campaign
    //   Data.WeaponDesignTable -> /Game/Game/DT_WeaponDesign.DT_WeaponDesign
    //   UI.MainMenu            -> /Game/Screens/WB_MainMenu.WB_MainMenu_C
    UPROPERTY(EditAnywhere, config, Category = "Starshatter|AssetRegistry")
    TMap<FName, TSoftObjectPtr<UObject>> Assets;

    // Optional: hard fail when missing required assets (recommended true)
    UPROPERTY(EditAnywhere, config, Category = "Starshatter|AssetRegistry")
    bool bFailOnMissingRequired = true;

    // ------------------------------------------------------------------
    // Optional convenience bindings (config-backed)
    // ------------------------------------------------------------------
    UPROPERTY(EditAnywhere, config, Category = "Starshatter|DataTables")
    TSoftObjectPtr<UDataTable> WeaponDesignTable;
};
