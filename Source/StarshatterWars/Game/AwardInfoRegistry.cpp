/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024–2026. All Rights Reserved.

    SUBSYSTEM:      AwardInfoRegistry
    FILE:           AwardInfoRegistry.cpp
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    UAwardInfoRegistry

    Unreal-native runtime registry for award, rank, and medal definitions.

    This class replaces legacy:
        - AwardInfo
        - rank_table
        - medal_table
        - LoadAwardTables()
        - static rank/medal lookup functions

    The registry loads and indexes FS_AwardInfo rows from a DataTable
    (typically DT_Awards) and provides fast, centralized lookup services
    for rank titles, medal names, descriptions, and progression logic.

    RESPONSIBILITIES
    ================
    - Load award definitions from UDataTable
    - Separate rank entries from medal entries
    - Provide rank lookup (RankName, RankDescription, RankAbrv)
    - Provide medal lookup (MedalName, MedalDescription)
    - Compute command eligibility by ship class
    - Determine rank progression by TotalPoints
    - Provide clean runtime API for UI and gameplay systems

    ARCHITECTURAL ROLE
    ==================
    FS_AwardInfo:
        Data-only struct (row definition)

    UAwardInfoRegistry:
        Runtime logic + indexing + lookup layer

    UStarshatterPlayerSubsystem:
        Owns player state (FS_PlayerGameInfo)
        Uses this registry for rank/award display and evaluation

    DESIGN NOTES
    ============
    - No persistence logic lives here.
    - No player state is owned here.
    - No gameplay mutation occurs here.
    - Pure lookup + evaluation layer.
    - Safe for UI use.
    - Safe for subsystem injection.

    FUTURE EXTENSIONS
    =================
    - Cached TMap<int32, const FS_AwardInfo*> for O(1) lookups
    - Rank progression evaluation by PlayerPoints
    - Medal eligibility evaluation helpers
    - Async asset loading for insignia images
=============================================================================*/



#include "AwardInfoRegistry.h"

#include "Logging/LogMacros.h"

TWeakObjectPtr<UDataTable> UAwardInfoRegistry::AwardsTable;
TMap<int32, const FS_AwardInfo*> UAwardInfoRegistry::RankById;
TMap<int32, const FS_AwardInfo*> UAwardInfoRegistry::MedalById;

void UAwardInfoRegistry::Initialize(UDataTable* InAwardsTable)
{
    AwardsTable = InAwardsTable;
    BuildCaches();
}

bool UAwardInfoRegistry::IsInitialized()
{
    return AwardsTable.IsValid() && (RankById.Num() > 0 || MedalById.Num() > 0);
}

void UAwardInfoRegistry::Reset()
{
    AwardsTable.Reset();
    RankById.Reset();
    MedalById.Reset();
}

const TCHAR* UAwardInfoRegistry::SafeStr(const FString& S, const TCHAR* Fallback)
{
    return S.IsEmpty() ? Fallback : *S;
}

const FS_AwardInfo* UAwardInfoRegistry::FindById(const TMap<int32, const FS_AwardInfo*>& Map, int32 Id)
{
    if (const FS_AwardInfo* const* Found = Map.Find(Id))
    {
        return *Found;
    }
    return nullptr;
}

void UAwardInfoRegistry::BuildCaches()
{
    RankById.Reset();
    MedalById.Reset();

    UDataTable* DT = AwardsTable.Get();
    if (!DT)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AwardInfoRegistry] Initialize called with null DataTable."));
        return;
    }

    // NOTE: Row pointers are stable while DT is loaded.
    const TMap<FName, uint8*>& RowMap = DT->GetRowMap();
    for (const TPair<FName, uint8*>& Pair : RowMap)
    {
        const FS_AwardInfo* Row = reinterpret_cast<const FS_AwardInfo*>(Pair.Value);
        if (!Row)
            continue;

        // Normalize type text (be forgiving)
        const FString TypeLower = Row->AwardType.ToLower();

        if (TypeLower == TEXT("rank"))
        {
            RankById.Add(Row->AwardId, Row);
        }
        else if (TypeLower == TEXT("medal"))
        {
            MedalById.Add(Row->AwardId, Row);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[AwardInfoRegistry] Cached %d ranks, %d medals from DataTable '%s'."),
        RankById.Num(), MedalById.Num(), *DT->GetName());
}

// ------------------------------------------------------------
// Legacy lookups: Ranks
// ------------------------------------------------------------

const FS_AwardInfo* UAwardInfoRegistry::FindRank(int32 RankId)
{
    return FindById(RankById, RankId);
}

const TCHAR* UAwardInfoRegistry::RankName(int32 RankId)
{
    if (const FS_AwardInfo* Row = FindRank(RankId))
        return SafeStr(Row->AwardName, TEXT("Conscript"));

    return TEXT("Conscript");
}

const TCHAR* UAwardInfoRegistry::RankAbrv(int32 RankId)
{
    if (const FS_AwardInfo* Row = FindRank(RankId))
        return SafeStr(Row->AwardAbrv, TEXT(""));

    return TEXT("");
}

const TCHAR* UAwardInfoRegistry::RankDescription(int32 RankId)
{
    if (const FS_AwardInfo* Row = FindRank(RankId))
        return SafeStr(Row->AwardDesc, TEXT(""));

    return TEXT("");
}

// ------------------------------------------------------------
// Legacy lookups: Medals
// ------------------------------------------------------------

const FS_AwardInfo* UAwardInfoRegistry::FindMedal(int32 MedalId)
{
    return FindById(MedalById, MedalId);
}

const TCHAR* UAwardInfoRegistry::MedalName(int32 MedalId)
{
    if (const FS_AwardInfo* Row = FindMedal(MedalId))
        return SafeStr(Row->AwardName, TEXT(""));

    return TEXT("");
}

const TCHAR* UAwardInfoRegistry::MedalAbrv(int32 MedalId)
{
    if (const FS_AwardInfo* Row = FindMedal(MedalId))
        return SafeStr(Row->AwardAbrv, TEXT(""));

    return TEXT("");
}

const TCHAR* UAwardInfoRegistry::MedalDescription(int32 MedalId)
{
    if (const FS_AwardInfo* Row = FindMedal(MedalId))
        return SafeStr(Row->AwardDesc, TEXT(""));

    return TEXT("");
}
