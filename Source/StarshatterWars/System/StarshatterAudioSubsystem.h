/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025–2026.

    SUBSYSTEM:    Stars.exe (Unreal Engine Port)
    FILE:         StarshatterAudioSubsystem.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UStarshatterAudioSubsystem
    - Thin coordinator for audio settings + runtime application
    - Holds a pointer to UStarshatterAudioSettings (GI subsystem)
    - Provides convenience entry points for UI (AudioDlg) to apply/save
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StarshatterAudioSubsystem.generated.h"

class UStarshatterAudioSettings;

UCLASS()
class STARSHATTERWARS_API UStarshatterAudioSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UStarshatterAudioSettings* GetSettings() const { return Settings; }

    // Called by UI when user hits Apply:
    void ApplyAndSave();

    // Called by UI when user hits Cancel (optional behavior):
    void ReloadAndApply();

private:
    UPROPERTY()
    TObjectPtr<UStarshatterAudioSettings> Settings = nullptr;
};
