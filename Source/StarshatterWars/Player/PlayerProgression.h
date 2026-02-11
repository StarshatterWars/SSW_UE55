/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024-2026. All Rights Reserved.

    SUBSYSTEM:      Player Progression
    FILE:           PlayerProgression.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    PlayerProgression

    Static helper layer for mutating persistent player progression
    through UStarshatterPlayerSubsystem.

    Replaces legacy PlayerCharacter progression mutation logic.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"

// Forward declares only (do NOT include heavy headers here)
class UWorld;
class UStarshatterPlayerSubsystem;

class PlayerProgression
{
public:
    // Rank
    static int32 RankFromName(UWorld* World, const FString& RankName);
    static void  SetRank(UWorld* World, int32 RankId);

    // Campaign completion
    static void  SetCampaignComplete(UWorld* World, int32 CampaignId);

    // Medal (bitmask id)
    static void  GrantMedal(UWorld* World, int32 MedalId);

private:
    static UStarshatterPlayerSubsystem* GetPlayerSubsystem(UWorld* World);
};
