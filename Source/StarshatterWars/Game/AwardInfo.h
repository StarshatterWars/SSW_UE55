/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024–2026. All Rights Reserved.

    SUBSYSTEM:      Awards
    FILE:           AwardInfo.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    FAwardInfo

    Lightweight runtime wrapper + static helpers for award lookups.

    Replaces legacy PlayerCharacter static award helpers:
      - RankName
      - RankDescription
      - MedalName
      - MedalDescription

    Source of truth is now FS_AwardInfo rows (DataTables).
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameStructs.h"     

/**
 * Runtime wrapper for one award row, plus static lookup helpers.
 * No loading and no persistence: tables are provided by caller.
 */
class STARSHATTERWARS_API FAwardInfo
{
public:
    FAwardInfo() = default;
    explicit FAwardInfo(const FS_AwardInfo* InRow) : Row(InRow) {}

    bool IsValid() const { return Row != nullptr; }

    // ------------------------------------------------------------
    // Per-row accessors
    // ------------------------------------------------------------
    bool IsRank() const
    {
        return Row && Row->AwardType.Equals(TEXT("rank"), ESearchCase::IgnoreCase);
    }

    bool IsMedal() const
    {
        return Row && Row->AwardType.Equals(TEXT("medal"), ESearchCase::IgnoreCase);
    }

    int32 GetId() const { return Row ? Row->AwardId : 0; }
    const FString& GetNameRef() const { return Row ? Row->AwardName : Empty; }
    const FString& GetDescRef() const { return Row ? Row->AwardDesc : Empty; }

    // ------------------------------------------------------------
    // Static lookups (replacement for PlayerCharacter::RankName etc.)
    //
    // IMPORTANT:
    // - You must pass the correct DataTable pointer(s).
    // - RankId / MedalId match FS_AwardInfo::AwardId.
    // ------------------------------------------------------------

    static FString RankName(const UDataTable* RankTable, int32 RankId)
    {
        const FS_AwardInfo* Row = FindById(RankTable, RankId);
        return Row ? Row->AwardName : FString(TEXT("Conscript"));
    }

    static FString RankDescription(const UDataTable* RankTable, int32 RankId)
    {
        const FS_AwardInfo* Row = FindById(RankTable, RankId);
        return Row ? Row->AwardDesc : FString();
    }

    static FString MedalName(const UDataTable* MedalTable, int32 MedalId)
    {
        const FS_AwardInfo* Row = FindById(MedalTable, MedalId);
        return Row ? Row->AwardName : FString();
    }

    static FString MedalDescription(const UDataTable* MedalTable, int32 MedalId)
    {
        const FS_AwardInfo* Row = FindById(MedalTable, MedalId);
        return Row ? Row->AwardDesc : FString();
    }

private:
    // Find a FS_AwardInfo row where AwardId == DesiredId
    static const FS_AwardInfo* FindById(const UDataTable* Table, int32 DesiredId)
    {
        if (!Table)
            return nullptr;

        static const FString Context(TEXT("AwardInfoLookup"));

        // Iterate all rows (simple + robust; tables are small)
        TArray<FS_AwardInfo*> Rows;
        Table->GetAllRows(Context, Rows);

        for (const FS_AwardInfo* Row : Rows)
        {
            if (Row && Row->AwardId == DesiredId)
                return Row;
        }

        return nullptr;
    }

private:
    const FS_AwardInfo* Row = nullptr;
    static inline const FString Empty = TEXT("");
};
