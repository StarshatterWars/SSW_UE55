/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025–2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterControlsSubsystem.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UStarshatterControlsSubsystem
    - GameInstance subsystem that applies control settings at runtime.
    - Works with UStarshatterControlsSettings (config-backed CDO).
    - Bridges to legacy Starshatter runtime (KeyMap / MapKeys / Ship control model).
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StarshatterControlsSubsystem.generated.h"

class UObject;
class UStarshatterControlsSettings;

// Legacy:
class Starshatter;
class KeyMap;

UCLASS()
class STARSHATTERWARS_API UStarshatterControlsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ------------------------------------------------------------
    // Convenience accessor (matches your Audio pattern)
    // ------------------------------------------------------------
    static UStarshatterControlsSubsystem* Get(UObject* WorldContextObject);

    // ------------------------------------------------------------
    // UGameInstanceSubsystem
    // ------------------------------------------------------------
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ------------------------------------------------------------
    // Runtime apply entry point
    // ------------------------------------------------------------
    void ApplySettingsToRuntime(UObject* WorldContextObject);

private:
    Starshatter* GetStars() const;
    KeyMap* GetLegacyKeyMap() const;

    void ApplySpecialsToLegacy(KeyMap& KM, const UStarshatterControlsSettings& S) const;
    void ApplyActionBindingsToLegacy(KeyMap& KM, const UStarshatterControlsSettings& S) const;
};
