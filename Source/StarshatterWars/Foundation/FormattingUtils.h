/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.
    All Rights Reserved.

    MODULE:        StarshatterWars (Unreal Engine)
    FILE:          FormattingUtils.h
    AUTHOR:        Carlos Bott

    OVERVIEW
    ========
    FormattingUtils

    Stateless helper utilities for:
      - Enum display name formatting
      - Enum index/value conversions
      - Combat unit/group string helpers
      - Empire display name resolution
      - Shared string formatting logic

    This class replaces duplicated formatting helpers previously
    located in USSWGameInstance and related systems.

    DESIGN NOTES
    ============
    - Static-only utility class (no instances)
    - Header-safe templates for enum display helpers
    - No engine lifecycle dependencies
    - May be used by UI, subsystems, loaders, and gameplay code
*/

#pragma once

#include "CoreMinimal.h"
#include "GameStructs.h"   // ECOMBATUNIT_TYPE, ECOMBATGROUP_TYPE, EEMPIRE_NAME

class STARSHATTERWARS_API FormattingUtils
{
public:

    // ------------------------------------------------------------
    // Generic enum helpers
    // ------------------------------------------------------------

    template <typename TEnum>
    static FString EnumToDisplayString(TEnum EnumValue)
    {
        static_assert(TIsEnum<TEnum>::Value,
            "EnumToDisplayString only works with UENUM types.");

        const UEnum* EnumPtr = StaticEnum<TEnum>();
        if (!EnumPtr)
        {
            return TEXT("Invalid");
        }

        return EnumPtr
            ->GetDisplayNameTextByValue(static_cast<int64>(EnumValue))
            .ToString();
    }

    // ------------------------------------------------------------
    // Combat group / unit helpers
    // ------------------------------------------------------------

    static FString GetGroupDisplayName(ECOMBATGROUP_TYPE Type);
    static FString GetUnitDisplayName(ECOMBATUNIT_TYPE Type);
    static FString GetUnitPrefix(ECOMBATUNIT_TYPE Type);

    // ------------------------------------------------------------
    // Empire helpers
    // ------------------------------------------------------------

    static FString GetEmpireDisplayName(EEMPIRE_NAME Empire);
    static FString GetEmpireNameLong(EEMPIRE_NAME Empire);

    static EEMPIRE_NAME GetEmpireFromIndex(int32 Index);
    static int32 GetEmpireIndex(EEMPIRE_NAME Empire);
};
