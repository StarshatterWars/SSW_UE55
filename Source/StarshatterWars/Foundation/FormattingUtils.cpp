/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         FormattingUtils.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Implementation for FormattingUtils helpers.
*/

#include "FormattingUtils.h"

#include "UObject/UnrealType.h" // UEnum
#include "Logging/LogMacros.h"

// -----------------------------------------------------------------------------
// Enum display wrappers
// -----------------------------------------------------------------------------
FString UFormattingUtils::GetGroupTypeDisplayName(ECOMBATGROUP_TYPE Type)
{
    return EnumToDisplayString(Type);
}

FString UFormattingUtils::GetUnitTypeDisplayName(ECOMBATUNIT_TYPE Type)
{
    return EnumToDisplayString(Type);
}

// -----------------------------------------------------------------------------
// Empire helpers
// -----------------------------------------------------------------------------
EEMPIRE_NAME UFormattingUtils::GetEmpireTypeFromIndex(int32 Index)
{
    UEnum* EnumPtr = StaticEnum<EEMPIRE_NAME>();
    if (EnumPtr && Index >= 0 && Index < EnumPtr->NumEnums())
    {
        const int64 RawValue = EnumPtr->GetValueByIndex(Index);
        return static_cast<EEMPIRE_NAME>(RawValue);
    }

    UE_LOG(LogTemp, Warning, TEXT("GetEmpireTypeFromIndex: Invalid Index=%d"), Index);

    // Fallback: pick a sane default for your game
    return EEMPIRE_NAME::Terellian;
}

int32 UFormattingUtils::GetIndexFromEmpireType(EEMPIRE_NAME Type)
{
    UEnum* EnumPtr = StaticEnum<EEMPIRE_NAME>();

    // Your old default was 8. Keep it to preserve UI behavior.
    int32 EmpireIndex = 8;

    if (EnumPtr)
    {
        const int64 Value = static_cast<int64>(Type);
        const int32 FoundIndex = EnumPtr->GetIndexByValue(Value);
        if (FoundIndex != INDEX_NONE)
        {
            EmpireIndex = FoundIndex;
        }
    }

    return EmpireIndex;
}

FString UFormattingUtils::GetEmpireTypeNameByIndex(int32 Index)
{
    UEnum* EnumPtr = StaticEnum<EEMPIRE_NAME>();
    if (!EnumPtr)
    {
        return TEXT("Invalid");
    }

    const int64 EnumValue = EnumPtr->GetValueByIndex(Index);
    return EnumPtr->GetDisplayNameTextByValue(EnumValue).ToString();
}

FString UFormattingUtils::GetEmpireDisplayName(EEMPIRE_NAME Empire)
{
    const UEnum* EnumPtr = StaticEnum<EEMPIRE_NAME>();
    if (!EnumPtr)
    {
        return TEXT("Invalid");
    }

    return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Empire)).ToString();
}

FString UFormattingUtils::GetEmpireNameFromType(EEMPIRE_NAME Empire)
{
    switch (Empire)
    {
    case EEMPIRE_NAME::Terellian:    return TEXT("Terellian Alliance");
    case EEMPIRE_NAME::Marakan:      return TEXT("Marakan Hegemony");
    case EEMPIRE_NAME::Independent:  return TEXT("Independent System");
    case EEMPIRE_NAME::Dantari:      return TEXT("Dantari Separatists");
    case EEMPIRE_NAME::Zolon:        return TEXT("Zolon Empire");
    case EEMPIRE_NAME::Other:        return TEXT("Other");
    case EEMPIRE_NAME::Pirate:       return TEXT("Brotherhood of Iron");
    case EEMPIRE_NAME::Neutral:      return TEXT("Neutral");
    case EEMPIRE_NAME::Unknown:      return TEXT("Unknown");
    case EEMPIRE_NAME::Silessian:    return TEXT("Silessian Confederacy");
    case EEMPIRE_NAME::Solus:        return TEXT("Independent System of Solus");
    case EEMPIRE_NAME::Haiche:       return TEXT("Haiche Protectorate");
    default:                         return TEXT("Unknown");
    }
}

// -----------------------------------------------------------------------------
// Unit naming helpers
// -----------------------------------------------------------------------------
FString UFormattingUtils::GetUnitPrefixFromType(ECOMBATUNIT_TYPE Type)
{
    switch (Type)
    {
    case ECOMBATUNIT_TYPE::CRUISER:   return TEXT("CA-");
    case ECOMBATUNIT_TYPE::CARRIER:   return TEXT("CV-");
    case ECOMBATUNIT_TYPE::FRIGATE:   return TEXT("FF-");
    case ECOMBATUNIT_TYPE::DESTROYER: return TEXT("DD-");
    default:                          return TEXT("UNK-");
    }
}
