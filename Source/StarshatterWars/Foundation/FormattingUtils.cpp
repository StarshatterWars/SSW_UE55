/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.
    All Rights Reserved.

    MODULE:        StarshatterWars (Unreal Engine)
    FILE:          FormattingUtils.cpp
    AUTHOR:        Carlos Bott
*/

#include "FormattingUtils.h"
#include "UObject/UnrealType.h"
#include "Logging/LogMacros.h"

// ------------------------------------------------------------
// Combat group / unit helpers
// ------------------------------------------------------------

FString FormattingUtils::GetGroupDisplayName(ECOMBATGROUP_TYPE Type)
{
    return EnumToDisplayString(Type);
}

FString FormattingUtils::GetUnitDisplayName(ECOMBATUNIT_TYPE Type)
{
    return EnumToDisplayString(Type);
}

FString FormattingUtils::GetUnitPrefix(ECOMBATUNIT_TYPE Type)
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

// ------------------------------------------------------------
// Empire helpers
// ------------------------------------------------------------

FString FormattingUtils::GetEmpireDisplayName(EEMPIRE_NAME Empire)
{
    return EnumToDisplayString(Empire);
}

FString FormattingUtils::GetEmpireNameLong(EEMPIRE_NAME Empire)
{
    switch (Empire)
    {
    case EEMPIRE_NAME::Terellian:   return TEXT("Terellian Alliance");
    case EEMPIRE_NAME::Marakan:     return TEXT("Marakan Hegemony");
    case EEMPIRE_NAME::Independent: return TEXT("Independent System");
    case EEMPIRE_NAME::Dantari:     return TEXT("Dantari Separatists");
    case EEMPIRE_NAME::Zolon:       return TEXT("Zolon Empire");
    case EEMPIRE_NAME::Pirate:      return TEXT("Brotherhood of Iron");
    case EEMPIRE_NAME::Neutral:     return TEXT("Neutral");
    case EEMPIRE_NAME::Silessian:   return TEXT("Silessian Confederacy");
    case EEMPIRE_NAME::Solus:       return TEXT("Independent System of Solus");
    case EEMPIRE_NAME::Haiche:      return TEXT("Haiche Protectorate");
    case EEMPIRE_NAME::Other:       return TEXT("Other");
    case EEMPIRE_NAME::Unknown:
    default:
        return TEXT("Unknown");
    }
}

EEMPIRE_NAME FormattingUtils::GetEmpireFromIndex(int32 Index)
{
    const UEnum* EnumPtr = StaticEnum<EEMPIRE_NAME>();
    if (!EnumPtr || Index < 0 || Index >= EnumPtr->NumEnums())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("FormattingUtils::GetEmpireFromIndex invalid index: %d"),
            Index);
        return EEMPIRE_NAME::Unknown;
    }

    return static_cast<EEMPIRE_NAME>(
        EnumPtr->GetValueByIndex(Index)
        );
}

int32 FormattingUtils::GetEmpireIndex(EEMPIRE_NAME Empire)
{
    const UEnum* EnumPtr = StaticEnum<EEMPIRE_NAME>();
    if (!EnumPtr)
    {
        return INDEX_NONE;
    }

    return EnumPtr->GetIndexByValue(
        static_cast<int64>(Empire)
    );
}
