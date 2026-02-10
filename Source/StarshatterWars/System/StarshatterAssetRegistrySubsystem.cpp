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

    Centralized runtime asset binding and resolution layer.

    Responsibilities:
      - Read config-backed bindings from UStarshatterAssetRegistrySettings
      - Cache AssetId -> Soft reference mappings in memory
      - Validate required assets during BootAssets()
      - Resolve assets on demand (optional synchronous load)

    NOTES
    =====
    - No hard-coded paths outside of the settings layer.
    - Widget references should use generated class paths (..._C) so they load as UClass.
    - This subsystem owns no gameplay state. It is a pure lookup/resolution service.

=============================================================================*/

#include "StarshatterAssetRegistrySubsystem.h"

#include "StarshatterAssetRegistrySettings.h"

#include "Engine/DataTable.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY(LogStarshatterAssetRegistry);

void UStarshatterAssetRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    Cache.Reset();
    bReady = false;

    InitRegistry();
    UE_LOG(LogStarshatterAssetRegistry, Log, TEXT("[ASSETS] Initialize()"));
}

void UStarshatterAssetRegistrySubsystem::Deinitialize()
{
    UE_LOG(LogStarshatterAssetRegistry, Log, TEXT("[ASSETS] Deinitialize()"));

    Cache.Reset();
    bReady = false;

    Super::Deinitialize();
}

UObject* UStarshatterAssetRegistrySubsystem::GetAsset(FName AssetId, bool bLoadNow)
{
    if (!bReady)
    {
        UE_LOG(LogStarshatterAssetRegistry, Warning,
            TEXT("[ASSETS] GetAsset: Registry not ready (%s) - attempting InitRegistry()"),
            *AssetId.ToString());

        if (!InitRegistry())
        {
            UE_LOG(LogStarshatterAssetRegistry, Error,
                TEXT("[ASSETS] GetAsset: InitRegistry failed; cannot resolve %s"),
                *AssetId.ToString());
            return nullptr;
        }
    }

    const TSoftObjectPtr<UObject>* Found = Cache.Find(AssetId);
    if (!Found)
    {
        UE_LOG(LogStarshatterAssetRegistry, Error,
            TEXT("[ASSETS] GetAsset: Missing AssetId=%s"),
            *AssetId.ToString());
        return nullptr;
    }

    return ResolveSoftObject(*Found, AssetId, bLoadNow);
}

bool UStarshatterAssetRegistrySubsystem::InitRegistry()
{
    Cache.Reset();
    bReady = false;

    const UStarshatterAssetRegistrySettings* Settings = GetDefault<UStarshatterAssetRegistrySettings>();
    if (!Settings)
    {
        UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] InitRegistry: Settings missing"));
        return false;
    }

    // 1) Copy generic map (AssetId -> SoftObject)
    Cache = Settings->Assets;

    // 2) Inject typed design tables (do not overwrite explicit map entries)

    // Data.WeaponDesignTable
    if (!Cache.Contains(TEXT("Data.WeaponDesignTable")))
    {
        if (!Settings->WeaponDesignTable.IsNull())
        {
            const FSoftObjectPath Path = Settings->WeaponDesignTable.ToSoftObjectPath();
            Cache.Add(TEXT("Data.WeaponDesignTable"), TSoftObjectPtr<UObject>(Path));

            UE_LOG(LogStarshatterAssetRegistry, Log, TEXT("[ASSETS] Bind Data.WeaponDesignTable -> %s"),
                *Path.ToString());
        }
        else
        {
            UE_LOG(LogStarshatterAssetRegistry, Warning,
                TEXT("[ASSETS] WeaponDesignTable is not set in Project Settings"));
        }
    }

    // Data.ShipDesignTable
    if (!Cache.Contains(TEXT("Data.ShipDesignTable")))
    {
        if (!Settings->ShipDesignTable.IsNull())
        {
            const FSoftObjectPath Path = Settings->ShipDesignTable.ToSoftObjectPath();
            Cache.Add(TEXT("Data.ShipDesignTable"), TSoftObjectPtr<UObject>(Path));

            UE_LOG(LogStarshatterAssetRegistry, Log, TEXT("[ASSETS] Bind Data.ShipDesignTable -> %s"),
                *Path.ToString());
        }
        else
        {
            UE_LOG(LogStarshatterAssetRegistry, Warning,
                TEXT("[ASSETS] ShipDesignTable is not set in Project Settings"));
        }
    }

    // Data.SystemDesignTable
    if (!Cache.Contains(TEXT("Data.SystemDesignTable")))
    {
        if (!Settings->SystemDesignTable.IsNull())
        {
            const FSoftObjectPath Path = Settings->SystemDesignTable.ToSoftObjectPath();
            Cache.Add(TEXT("Data.SystemDesignTable"), TSoftObjectPtr<UObject>(Path));

            UE_LOG(LogStarshatterAssetRegistry, Log, TEXT("[ASSETS] Bind Data.SystemDesignTable -> %s"),
                *Path.ToString());
        }
        else
        {
            UE_LOG(LogStarshatterAssetRegistry, Warning,
                TEXT("[ASSETS] SystemDesignTable is not set in Project Settings"));
        }
    }

    UE_LOG(LogStarshatterAssetRegistry, Log, TEXT("[ASSETS] InitRegistry: Loaded %d entries"), Cache.Num());

    bReady = true;
    return true;
}

bool UStarshatterAssetRegistrySubsystem::ValidateRequired(const TArray<FName>& RequiredIds, bool bLoadNow)
{
    if (!bReady)
    {
        UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] ValidateRequired: Registry not ready"));
        return false;
    }

    const UStarshatterAssetRegistrySettings* Settings = GetDefault<UStarshatterAssetRegistrySettings>();
    const bool bFailHard = Settings ? Settings->bFailOnMissingRequired : true;

    bool bAllOk = true;

    for (const FName& Id : RequiredIds)
    {
        const TSoftObjectPtr<UObject>* Found = Cache.Find(Id);
        if (!Found)
        {
            UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] Missing required AssetId: %s"), *Id.ToString());
            bAllOk = false;
            if (bFailHard)
                return false;
            continue;
        }

        if (bLoadNow)
        {
            UObject* Obj = ResolveSoftObject(*Found, Id, true);
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

UObject* UStarshatterAssetRegistrySubsystem::ResolveSoftObject(const TSoftObjectPtr<UObject>& SoftPtr, FName AssetId, bool bLoadNow)
{
    if (SoftPtr.IsNull())
    {
        UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] ResolveSoftObject: Null soft reference for %s"),
            *AssetId.ToString());
        return nullptr;
    }

    if (bLoadNow)
    {
        UObject* Loaded = SoftPtr.LoadSynchronous();
        if (!Loaded)
        {
            UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] ResolveSoftObject: Load failed for %s (%s)"),
                *AssetId.ToString(), *SoftPtr.ToSoftObjectPath().ToString());
        }
        return Loaded;
    }

    return SoftPtr.Get();
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

    UClass* AsClass = Cast<UClass>(Obj);
    if (!AsClass)
    {
        UE_LOG(LogStarshatterAssetRegistry, Error,
            TEXT("[ASSETS] GetWidgetClass: %s is not a UClass. Use generated class path (..._C)."),
            *AssetId.ToString());
        return nullptr;
    }

    if (!AsClass->IsChildOf(UUserWidget::StaticClass()))
    {
        UE_LOG(LogStarshatterAssetRegistry, Error,
            TEXT("[ASSETS] GetWidgetClass: %s is not a UUserWidget subclass (%s)"),
            *AssetId.ToString(), *AsClass->GetName());
        return nullptr;
    }

    return AsClass;
}
