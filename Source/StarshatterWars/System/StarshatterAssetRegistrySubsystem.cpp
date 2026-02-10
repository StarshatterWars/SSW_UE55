/*=============================================================================
    Project:        Starshatter Wars (Unreal Engine Port)
    Studio:         Fractal Dev Studios
    Copyright:      (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:      Asset Registry
    FILE:           StarshatterAssetRegistrySubsystem.cpp
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    Runtime asset registry implementation.

    Loads the AssetId -> SoftObject mapping from
    UStarshatterAssetRegistrySettings and provides:
      - GetAsset
      - GetDataTable
      - GetWidgetClass
      - ValidateRequired

    Editor-only types are intentionally not used here.

=============================================================================*/

#include "StarshatterAssetRegistrySubsystem.h"
#include "StarshatterAssetRegistrySettings.h"

#include "Engine/DataTable.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Blueprint.h"

DEFINE_LOG_CATEGORY(LogStarshatterAssetRegistry);

void UStarshatterAssetRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UStarshatterAssetRegistrySubsystem::Deinitialize()
{
    Cache.Reset();
    bReady = false;

    Super::Deinitialize();
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

    Cache = Settings->Assets;

    UE_LOG(LogStarshatterAssetRegistry, Log, TEXT("[ASSETS] InitRegistry: Loaded %d entries"), Cache.Num());

    bReady = true;
    return true;
}

UObject* UStarshatterAssetRegistrySubsystem::GetAsset(FName AssetId, bool bLoadSync)
{
    if (!bReady)
    {
        UE_LOG(LogStarshatterAssetRegistry, Warning, TEXT("[ASSETS] GetAsset: Registry not ready (id=%s)"),
            *AssetId.ToString());
        return nullptr;
    }

    const TSoftObjectPtr<UObject>* Found = Cache.Find(AssetId);
    if (!Found)
    {
        UE_LOG(LogStarshatterAssetRegistry, Warning, TEXT("[ASSETS] GetAsset: Unknown id '%s'"), *AssetId.ToString());
        return nullptr;
    }

    if (Found->IsNull())
    {
        UE_LOG(LogStarshatterAssetRegistry, Warning, TEXT("[ASSETS] GetAsset: Null soft reference for id '%s'"),
            *AssetId.ToString());
        return nullptr;
    }

    if (bLoadSync)
    {
        UObject* Loaded = Found->LoadSynchronous();
        if (!Loaded)
        {
            UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] GetAsset: Failed to load '%s' (%s)"),
                *AssetId.ToString(), *Found->ToSoftObjectPath().ToString());
        }
        return Loaded;
    }

    return Found->Get();
}

UDataTable* UStarshatterAssetRegistrySubsystem::GetDataTable(FName AssetId, bool bLoadSync)
{
    UObject* Obj = GetAsset(AssetId, bLoadSync);
    if (!Obj)
        return nullptr;

    UDataTable* DT = Cast<UDataTable>(Obj);
    if (!DT)
    {
        UE_LOG(LogStarshatterAssetRegistry, Warning, TEXT("[ASSETS] '%s' is not a DataTable (type=%s)"),
            *AssetId.ToString(), *Obj->GetClass()->GetName());
    }

    return DT;
}

TSubclassOf<UUserWidget> UStarshatterAssetRegistrySubsystem::GetWidgetClass(FName AssetId, bool bLoadSync)
{
    UObject* Obj = GetAsset(AssetId, bLoadSync);
    if (!Obj)
        return nullptr;

    // Case 1: direct class reference
    if (UClass* Cls = Cast<UClass>(Obj))
    {
        if (Cls->IsChildOf(UUserWidget::StaticClass()))
            return Cls;

        UE_LOG(LogStarshatterAssetRegistry, Warning, TEXT("[ASSETS] '%s' is a UClass but not a UUserWidget class (%s)"),
            *AssetId.ToString(), *Cls->GetName());
        return nullptr;
    }

    // Case 2: blueprint asset (runtime-safe)
    if (UBlueprint* BP = Cast<UBlueprint>(Obj))
    {
        UClass* Gen = BP->GeneratedClass;
        if (Gen && Gen->IsChildOf(UUserWidget::StaticClass()))
            return Gen;

        UE_LOG(LogStarshatterAssetRegistry, Warning, TEXT("[ASSETS] '%s' Blueprint has no valid GeneratedClass"),
            *AssetId.ToString());
        return nullptr;
    }

    UE_LOG(LogStarshatterAssetRegistry, Warning, TEXT("[ASSETS] '%s' is not a widget class/blueprint (type=%s)"),
        *AssetId.ToString(), *Obj->GetClass()->GetName());

    return nullptr;
}

bool UStarshatterAssetRegistrySubsystem::ValidateRequired(const TArray<FName>& RequiredIds, bool bLoadNow)
{
    const UStarshatterAssetRegistrySettings* Settings = GetDefault<UStarshatterAssetRegistrySettings>();
    const bool bFail = Settings ? Settings->bFailOnMissingRequired : true;

    for (const FName& Id : RequiredIds)
    {
        UObject* Obj = GetAsset(Id, bLoadNow);
        if (!Obj)
        {
            UE_LOG(LogStarshatterAssetRegistry, Error, TEXT("[ASSETS] Missing required asset id: %s"),
                *Id.ToString());

            if (bFail)
                return false;
        }
    }

    return true;
}
