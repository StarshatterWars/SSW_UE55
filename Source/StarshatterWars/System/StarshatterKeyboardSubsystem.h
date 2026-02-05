/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterKeyboardSubsystem.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UStarshatterKeyboardSubsystem
    - GameInstance subsystem for keyboard settings.
    - Imports keyboard config from SaveGame via reflection.
    - Applies settings to runtime through KeyboardSettings.
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StarshatterKeyboardSubsystem.generated.h"

class UObject;
class UStarshatterSettingsSaveGame;

UCLASS()
class STARSHATTERWARS_API UStarshatterKeyboardSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Convenience accessor
    static UStarshatterKeyboardSubsystem* Get(UObject* WorldContextObject);

    // UGameInstanceSubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Runtime apply (called by BootSubsystem)
    void ApplySettingsToRuntime(UObject* WorldContextObject);

    // SaveGame import (reflection-based, no hardcoded member name)
    void LoadFromSaveGame(UStarshatterSettingsSaveGame* SaveGame);
};
