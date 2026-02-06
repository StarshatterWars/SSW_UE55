/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         FormattingUtils.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    FormattingUtils
    - Central place for display/formatting helpers (enum display names, prefixes, empire names).
    - Keeps USSWGameInstance lean (no generic helper clutter).
    - Safe to call from C++ anywhere.
    - Also exposed to Blueprints via UBlueprintFunctionLibrary where possible.

    NOTES
    =====
    - EnumToDisplayString<TEnum>() is a header-only template (not a UFUNCTION).
    - Blueprint-callable wrappers exist for your common UENUM types.
*/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameStructs.h" // ECOMBATGROUP_TYPE, ECOMBATUNIT_TYPE, EEMPIRE_NAME
#include "FormattingUtils.generated.h"

// -----------------------------------------------------------------------------
// Header-only generic enum display helper (C++ only)
// -----------------------------------------------------------------------------
template <typename TEnum>
FORCEINLINE FString EnumToDisplayString(TEnum EnumValue)
{
    static_assert(TIsEnum<TEnum>::Value, "EnumToDisplayString only works with enums.");

    UEnum* EnumPtr = StaticEnum<TEnum>();
    if (!EnumPtr)
    {
        return TEXT("Invalid");
    }

    return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(EnumValue)).ToString();
}

// -----------------------------------------------------------------------------
// FormattingUtils (Blueprint + C++ static utility)
// -----------------------------------------------------------------------------
UCLASS()
class STARSHATTERWARS_API UFormattingUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // -------------------------------------------------------------------------
    // Enum display name wrappers (Blueprint-friendly)
    // -------------------------------------------------------------------------
    UFUNCTION(BlueprintPure, Category = "Formatting")
    static FString GetGroupTypeDisplayName(ECOMBATGROUP_TYPE Type);

    UFUNCTION(BlueprintPure, Category = "Formatting")
    static FString GetUnitTypeDisplayName(ECOMBATUNIT_TYPE Type);

    // -------------------------------------------------------------------------
    // Empire helpers
    // -------------------------------------------------------------------------
    UFUNCTION(BlueprintPure, Category = "Formatting|Empire")
    static EEMPIRE_NAME GetEmpireTypeFromIndex(int32 Index);

    UFUNCTION(BlueprintPure, Category = "Formatting|Empire")
    static int32 GetIndexFromEmpireType(EEMPIRE_NAME Type);

    UFUNCTION(BlueprintPure, Category = "Formatting|Empire")
    static FString GetEmpireTypeNameByIndex(int32 Index);

    UFUNCTION(BlueprintPure, Category = "Formatting|Empire")
    static FString GetEmpireDisplayName(EEMPIRE_NAME Empire);

    UFUNCTION(BlueprintPure, Category = "Formatting|Empire")
    static FString GetEmpireNameFromType(EEMPIRE_NAME Empire);

    // -------------------------------------------------------------------------
    // Unit naming helpers
    // -------------------------------------------------------------------------
    UFUNCTION(BlueprintPure, Category = "Formatting|Units")
    static FString GetUnitPrefixFromType(ECOMBATUNIT_TYPE Type);
};