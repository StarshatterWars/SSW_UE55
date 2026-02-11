/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024–2026. All Rights Reserved.

    SUBSYSTEM:      Player / Awards
    FILE:           AwardInfoRegistry.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    AwardInfoRegistry

    Unreal-native replacement for legacy AwardInfo tables.

    - Uses UDataTable rows of FS_AwardInfo
    - Builds cached lookup maps for:
        * Ranks  (AwardType == "rank")
        * Medals (AwardType == "medal")
    - Provides legacy-style static lookup functions used by UI:
        RankName / RankAbrv / RankDescription
        MedalName / MedalAbrv / MedalDescription

    NOTE
    ----
    This registry is non-owning with respect to the DataTable asset.
    It caches pointers to rows inside the DataTable; keep the DataTable loaded.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameStructs.h"          // where FS_AwardInfo lives
#include "AwardInfoRegistry.generated.h"

UCLASS()
class STARSHATTERWARS_API UAwardInfoRegistry : public UObject
{
    GENERATED_BODY()

public:
    // ------------------------------------------------------------
    // Initialization
    // ------------------------------------------------------------
    // Call this once after you have a valid DataTable (e.g., in Boot or loader subsystem).
    static void Initialize(UDataTable* InAwardsTable);

    static bool IsInitialized();
    static void Reset();

    // ------------------------------------------------------------
    // Legacy lookup API (UI expects these)
    // ------------------------------------------------------------
    static const TCHAR* RankName(int32 RankId);
    static const TCHAR* RankAbrv(int32 RankId);
    static const TCHAR* RankDescription(int32 RankId);

    static const TCHAR* MedalName(int32 MedalId);
    static const TCHAR* MedalAbrv(int32 MedalId);
    static const TCHAR* MedalDescription(int32 MedalId);

    // Optional: raw row access
    static const FS_AwardInfo* FindRank(int32 RankId);
    static const FS_AwardInfo* FindMedal(int32 MedalId);

private:
    static void BuildCaches();
    static const FS_AwardInfo* FindById(const TMap<int32, const FS_AwardInfo*>& Map, int32 Id);
    static const TCHAR* SafeStr(const FString& S, const TCHAR* Fallback);

private:
    static TWeakObjectPtr<UDataTable> AwardsTable;

    // Cached row pointers (point into the DataTable’s row storage)
    static TMap<int32, const FS_AwardInfo*> RankById;
    static TMap<int32, const FS_AwardInfo*> MedalById;
};
