/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025–2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterBootSubsystem.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Starshatter Boot Subsystem

    Central bootstrapper for global subsystems that must initialize early:
      - Settings save subsystem (load-or-create)
      - Font subsystem
      - Audio subsystem
      - Video subsystem
      - Controls subsystem

    This subsystem should be lightweight and deterministic: it orchestrates
    initialization order and calls “load/apply” routines on subsystems.
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StarshatterBootSubsystem.generated.h"

UCLASS()
class STARSHATTERWARS_API UStarshatterBootSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    void BootSettings();   
    void BootFonts();
    void BootAudio();
    void BootVideo();
    void BootControls();   
};
