/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterKeyboardSubsystem.cpp
    AUTHOR:       Carlos Bott
*/

#include "StarshatterKeyboardSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include "UObject/Class.h"
#include "UObject/UnrealType.h"

#include "StarshatterKeyboardSettings.h"
#include "StarshatterSettingsSaveGame.h"

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterKeyboardSubsystem, Log, All);

// ------------------------------------------------------------
// Static accessor
// ------------------------------------------------------------

UStarshatterKeyboardSubsystem* UStarshatterKeyboardSubsystem::Get(UObject* WorldContextObject)
{
    if (!WorldContextObject)
        return nullptr;

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
        return nullptr;

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
        return nullptr;

    return GI->GetSubsystem<UStarshatterKeyboardSubsystem>();
}

// ------------------------------------------------------------
// Lifecycle
// ------------------------------------------------------------

void UStarshatterKeyboardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UStarshatterKeyboardSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

// ------------------------------------------------------------
// Runtime apply
// ------------------------------------------------------------

void UStarshatterKeyboardSubsystem::ApplySettingsToRuntime(UObject* WorldContextObject)
{
    if (!WorldContextObject)
        return;

    UStarshatterKeyboardSettings* Settings = UStarshatterKeyboardSettings::Get();
    if (!Settings)
        return;

    Settings->ApplyToRuntimeKeyboard(WorldContextObject);
}

// ------------------------------------------------------------
// SaveGame import (reflection-based)
// ------------------------------------------------------------

void UStarshatterKeyboardSubsystem::LoadFromSaveGame(UStarshatterSettingsSaveGame* SaveGame)
{
    if (!SaveGame)
    {
        UE_LOG(LogStarshatterKeyboardSubsystem, Warning, TEXT("LoadFromSaveGame: SaveGame is null"));
        return;
    }

    UStarshatterKeyboardSettings* Settings = UStarshatterKeyboardSettings::Get();
    if (!Settings)
        return;

    const UStruct* TargetStruct = FStarshatterKeyboardConfig::StaticStruct();
    FStarshatterKeyboardConfig FoundConfig;
    bool bFound = false;

    UClass* SGClass = SaveGame->GetClass();
    if (!SGClass)
        return;

    for (TFieldIterator<FProperty> It(SGClass); It; ++It)
    {
        FProperty* Prop = *It;
        FStructProperty* StructProp = CastField<FStructProperty>(Prop);
        if (!StructProp)
            continue;

        if (StructProp->Struct == TargetStruct)
        {
            void* ValuePtr = StructProp->ContainerPtrToValuePtr<void>(SaveGame);
            if (ValuePtr)
            {
                const FStarshatterKeyboardConfig* Src =
                    static_cast<const FStarshatterKeyboardConfig*>(ValuePtr);

                FoundConfig = *Src;
                bFound = true;

                UE_LOG(
                    LogStarshatterKeyboardSubsystem,
                    Log,
                    TEXT("Imported keyboard config from SaveGame property '%s'"),
                    *StructProp->GetName()
                );
                break;
            }
        }
    }

    if (!bFound)
    {
        UE_LOG(
            LogStarshatterKeyboardSubsystem,
            Warning,
            TEXT("No FStarshatterKeyboardConfig property found on SaveGame class '%s'"),
            *SGClass->GetName()
        );
        return;
    }

    Settings->SetKeyboardConfig(FoundConfig);
    Settings->Sanitize();
    Settings->Save();
}
