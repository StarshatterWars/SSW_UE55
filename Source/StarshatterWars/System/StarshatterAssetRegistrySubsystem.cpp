/*=============================================================================
    Project:        Starshatter Wars (Unreal Engine Port)
    Studio:         Fractal Dev Studios
    Copyright:      (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:      Asset Registry
    FILE:           StarshatterAssetRegistrySubsystem.cpp
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UStarshatterAssetRegistrySubsystem

    Unreal-native central asset binding and resolution layer.

    Responsibilities:
      - Read config-backed bindings from UStarshatterAssetRegistrySettings
      - Cache AssetId -> Soft reference mappings in memory
      - Validate required AssetIds during BootAssets()
      - Resolve assets on demand (optional synchronous load)

    NOTES
    =====
    - No hard paths outside of this registry/settings layer.
    - Widget references should be stored as generated class references:
        /Game/.../WB_MainMenu.WB_MainMenu_C
      so they resolve to a UClass at runtime (no editor-only UWidgetBlueprint).

=============================================================================*/

#include "StarshatterAssetRegistrySubsystem.h"

#include "StarshatterAssetRegistrySettings.h"

#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Blueprint/UserWidget.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterAssetRegistry, Log, All);

void UStarshatterAssetRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    Cache.Reset();
    bReady = false;

    UE_LOG(LogStarshatterAssetRegistry, Log, TEXT("[ASSETS] Initialize()"));
}

void UStarshatterAssetRegistrySubsystem::Deinitialize()
{
    UE_LOG(LogStarshatterAssetRegistry, Log, TEXT("[ASSETS] Deinitialize()"));

    Cache.Reset();
    bReady = false;

    Super::Deinitialize();
}

bool UStarshatterAssetRegistrySubsystem::InitRegistry()
{
    Cache.Reset();
    bReady = false;

    const UStarshatterAssetRegistrySettings* Settings =
        GetDefault<UStarshatterAssetRegistrySettings>();

    if (!Settings)
    {
        UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] InitRegistry: Settings missing"));
        return false;
    }

    // 1) Copy generic AssetId -> SoftObject map
    Cache = Settings->Assets;

    // 2) Example: add Weapon Design DataTable convenience binding
    //    (This keeps your typed field usable even if the user does not populate the Assets map.)
    if (!Settings->WeaponDesignTable.IsNull())
    {
        Cache.Add(
            TEXT("Data.WeaponDesignTable"),
            TSoftObjectPtr<UObject>(Settings->WeaponDesignTable.ToSoftObjectPath())
        );

        UE_LOG(LogStarshatterAssetRegistry, Log, TEXT("[ASSETS] Bound Data.WeaponDesignTable -> %s"),
            *Settings->WeaponDesignTable.ToSoftObjectPath().ToString());
    }
    else
    {
        UE_LOG(LogStarshatterAssetRegistry, Warning,
            TEXT("[ASSETS] WeaponDesignTable not set in Project Settings"));
    }

    UE_LOG(LogStarshatterAssetRegistry, Log, TEXT("[ASSETS] InitRegistry: Loaded %d entries"), Cache.Num());

    bReady = true;
    return true;
}

bool UStarshatterAssetRegistrySubsystem::ValidateRequired(const TArray<FName>& Required, bool bLoadNow)
{
    if (!bReady)
    {
        UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] ValidateRequired: Registry not ready"));
        return false;
    }

    const UStarshatterAssetRegistrySettings* Settings =
        GetDefault<UStarshatterAssetRegistrySettings>();

    const bool bFailHard = Settings ? Settings->bFailOnMissingRequired : true;

    bool bAllOk = true;

    for (const FName& Id : Required)
    {
        if (!Cache.Contains(Id))
        {
            UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] Missing required AssetId: %s"), *Id.ToString());
            bAllOk = false;
            if (bFailHard)
                return false;
            continue;
        }

        if (bLoadNow)
        {
            UObject* Obj = GetAsset(Id, true);
            if (!Obj)
            {
                UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] Failed to load required AssetId: %s"), *Id.ToString());
                bAllOk = false;
                if (bFailHard)
                    return false;
            }
        }
    }

    return bAllOk;
}

UObject* UStarshatterAssetRegistrySubsystem::GetAsset(FName AssetId, bool bLoadNow)
{
    if (!bReady)
    {
        UE_LOG(LogStarshatterAssetRegistry, Warning, TEXT("[ASSETS] GetAsset: Registry not ready (%s)"), *AssetId.ToString());
        return nullptr;
    }

    const TSoftObjectPtr<UObject>* Found = Cache.Find(AssetId);
    if (!Found)
    {
        UE_LOG(LogStarshatterAssetRegistry, Warning, TEXT("[ASSETS] GetAsset: Unknown AssetId: %s"), *AssetId.ToString());
        return nullptr;
    }

    if (bLoadNow)
    {
        UObject* Loaded = Found->LoadSynchronous();
        if (!Loaded)
        {
            UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] GetAsset: Load failed: %s (%s)"),
                *AssetId.ToString(), *Found->ToSoftObjectPath().ToString());
        }
        return Loaded;
    }

    return Found->Get();
}

UDataTable* UStarshatterAssetRegistrySubsystem::GetDataTable(FName AssetId, bool bLoadNow)
{
    return Cast<UDataTable>(GetAsset(AssetId, bLoadNow));
}

TSubclassOf<UUserWidget> UStarshatterAssetRegistrySubsystem::GetWidgetClass(FName AssetId, bool bLoadNow)
{
    UObject* Obj = GetAsset(AssetId, bLoadNow);
    if (!Obj)
        return nullptr;

    // Runtime-safe widget reference must be a UClass (generated blueprint class),
    // e.g. "/Game/Screens/WB_MainMenu.WB_MainMenu_C"
    UClass* AsClass = Cast<UClass>(Obj);
    if (!AsClass)
    {
        UE_LOG(LogStarshatterAssetRegistry, Error,
            TEXT("[ASSETS] GetWidgetClass: AssetId %s is not a UClass. Use generated class path (..._C)."),
            *AssetId.ToString());
        return nullptr;
    }

    if (!AsClass->IsChildOf(UUserWidget::StaticClass()))
    {
        UE_LOG(LogStarshatterAssetRegistry, Error,
            TEXT("[ASSETS] GetWidgetClass: AssetId %s class is not a UUserWidget subclass (%s)"),
            *AssetId.ToString(), *AsClass->GetName());
        return nullptr;
    }

    return AsClass;
}
