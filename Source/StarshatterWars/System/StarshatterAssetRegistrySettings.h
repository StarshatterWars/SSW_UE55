/*=============================================================================
    Project:        Starshatter Wars (Unreal Engine Port)
    Studio:         Fractal Dev Studios
    Copyright:      (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:      Asset Registry
    FILE:           StarshatterAssetRegistrySettings.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    Asset Registry configuration (config-backed).

    Stores a map of AssetId -> Soft Object Path as authoritative bindings.
    Used by UStarshatterAssetRegistrySubsystem during BootAssets() to:
      - Initialize registry
      - Validate required assets
      - Load assets synchronously if requested

    This class contains NO loading logic. Runtime resolution is performed
    by the subsystem only.

=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "StarshatterAssetRegistrySettings.generated.h"

UCLASS(config = Game, defaultconfig, BlueprintType)
class STARSHATTERWARS_API UStarshatterAssetRegistrySettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UStarshatterAssetRegistrySettings(const FObjectInitializer& ObjectInitializer);

    // Map of AssetId -> Soft reference
    // Examples:
    //   Data.CampaignTable -> /Game/Game/DT_Campaign.DT_Campaign
    //   UI.MainMenu        -> /Game/Screens/WB_MainMenu.WB_MainMenu_C
    UPROPERTY(EditAnywhere, config, Category = "Starshatter|AssetRegistry")
    TMap<FName, TSoftObjectPtr<UObject>> Assets;

    // Optional: hard fail when missing required assets (recommended true)
    UPROPERTY(EditAnywhere, config, Category = "Starshatter|AssetRegistry")
    bool bFailOnMissingRequired = true;
};
