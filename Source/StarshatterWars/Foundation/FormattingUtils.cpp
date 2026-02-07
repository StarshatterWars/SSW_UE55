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
// Legacy design class table (matches original Starshatter ordering)
// -----------------------------------------------------------------------------
static const char* GShipDesignClassName[32] =
{
    "Drone",          "Fighter",
    "Attack",         "LCA",
    "Courier",        "Cargo",

    "Corvette",       "Freighter",
    "Frigate",        "Destroyer",
    "Cruiser",        "Battleship",

    "Carrier",        "Dreadnaught",
    "Station",        "Farcaster",

    "Mine",           "DEFSAT",
    "COMSAT",         "SWACS",

    "Building",       "Factory",
    "SAM",            "EWR",
    "C3I",            "Starbase",

    "0x04000000",     "0x08000000",
    "0x10000000",     "0x20000000",
    "0x40000000",     "0x80000000"
};

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
// Ordinals
// -----------------------------------------------------------------------------
FString UFormattingUtils::GetOrdinal(int32 Id)
{
    // Keep behavior deterministic for <= 0 too
    const int32 AbsId = FMath::Abs(Id);
    const int32 LastTwo = AbsId % 100;

    const FString Num = FString::FromInt(Id);

    if (LastTwo >= 11 && LastTwo <= 13)
    {
        return Num + TEXT("th");
    }

    switch (AbsId % 10)
    {
    case 1:  return Num + TEXT("st");
    case 2:  return Num + TEXT("nd");
    case 3:  return Num + TEXT("rd");
    default: return Num + TEXT("th");
    }
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

// -----------------------------------------------------------------------------
// Design class helpers (GetDesignClass / GetDesignClassName / Display mask)
// -----------------------------------------------------------------------------
int32 UFormattingUtils::GetDesignClassFromName(const char* ClassName)
{
    if (!ClassName || !ClassName[0])
        return 0;

    for (int32 i = 0; i < 32; ++i)
    {
        if (GShipDesignClassName[i] && !_stricmp(ClassName, GShipDesignClassName[i]))
        {
            return (1 << i);
        }
    }

    return 0;
}

const char* UFormattingUtils::GetDesignClassName(int32 Mask)
{
    if (Mask != 0)
    {
        int32 Index = 0;
        int32 Temp = Mask;

        // Find first set bit
        while ((Temp & 1) == 0)
        {
            Temp >>= 1;
            ++Index;
            if (Index >= 32)
            {
                return "Unknown";
            }
        }

        if (Index >= 0 && Index < 32)
        {
            return GShipDesignClassName[Index] ? GShipDesignClassName[Index] : "Unknown";
        }
    }

    return "Unknown";
}

FString UFormattingUtils::GetDesignClassMaskDisplayString(int32 Mask)
{
    if (Mask == 0)
    {
        return TEXT("None");
    }

    TArray<FString> Parts;
    Parts.Reserve(8);

    for (int32 i = 0; i < 32; ++i)
    {
        const int32 Bit = (1 << i);
        if ((Mask & Bit) != 0)
        {
            const char* Name = GShipDesignClassName[i];
            Parts.Add(Name ? FString(Name) : TEXT("Unknown"));
        }
    }

    return FString::Join(Parts, TEXT(" | "));
}

// -----------------------------------------------------------------------------
// CLASSIFICATION helpers
// -----------------------------------------------------------------------------
static int32 ClassificationToMask(CLASSIFICATION V)
{
    return static_cast<int32>(static_cast<uint32>(V));
}

FString UFormattingUtils::GetClassificationName(CLASSIFICATION Value)
{
    // Prefer UENUM DisplayName if you add UMETA(DisplayName="...") in GameStructs.h
    const UEnum* EnumPtr = StaticEnum<CLASSIFICATION>();
    if (EnumPtr)
    {
        // If you pass combined flags, UE will not have a single display name.
        // This function is intended for a single enumerator.
        const int64 Raw = static_cast<int64>(static_cast<uint32>(Value));
        return EnumPtr->GetDisplayNameTextByValue(Raw).ToString();
    }

    // Fallback (should not hit if CLASSIFICATION is a UENUM)
    return TEXT("Unknown");
}

FString UFormattingUtils::GetClassificationMaskDisplayString(int32 Mask)
{
    if (Mask == 0)
    {
        return TEXT("None");
    }

    // Build from enum values, not hardcoded strings.
    // Note: this assumes each leaf enumerator is a single bit (which yours are).
    TArray<FString> Parts;
    Parts.Reserve(8);

    const UEnum* EnumPtr = StaticEnum<CLASSIFICATION>();
    if (!EnumPtr)
    {
        return TEXT("Invalid");
    }

    // Walk all enum entries, include only single-bit leaf flags (skip EMPTY and category masks)
    const int32 Num = EnumPtr->NumEnums();
    for (int32 i = 0; i < Num; ++i)
    {
        const int64 Raw = EnumPtr->GetValueByIndex(i);
        const uint32 V = static_cast<uint32>(Raw);

        if (V == 0)
        {
            continue; // EMPTY
        }

        // leaf = exactly one bit set
        const bool bSingleBit = (V & (V - 1)) == 0;

        // skip your category masks (multi-bit)
        if (!bSingleBit)
        {
            continue;
        }

        if ((static_cast<uint32>(Mask) & V) != 0)
        {
            Parts.Add(EnumPtr->GetDisplayNameTextByValue(Raw).ToString());
        }
    }

    if (Parts.Num() == 0)
    {
        return TEXT("None");
    }

    return FString::Join(Parts, TEXT(" | "));
}

FString UFormattingUtils::SanitizeEnumToken(const FString& In)
{
    FString S = In;
    S.TrimStartAndEndInline();

    // Unify separators:
    S.ReplaceInline(TEXT("\t"), TEXT(" "));
    S.ReplaceInline(TEXT("-"), TEXT("_"));
    S.ReplaceInline(TEXT(" "), TEXT("_"));

    // Collapse repeated underscores:
    while (S.Contains(TEXT("__")))
    {
        S.ReplaceInline(TEXT("__"), TEXT("_"));
    }

    // Optional: remove quotes
    S.ReplaceInline(TEXT("\""), TEXT(""));

    // Some legacy files might use lowercase:
    S = S.ToUpper();

    return S;
}

bool UFormattingUtils::GetRegionTypeFromString(const FString& InString, EOrbitalType& OutValue)
{
    const UEnum* Enum = StaticEnum<EOrbitalType>();
    if (!Enum)
        return false;

    const FString Key = SanitizeEnumToken(InString);

    // 0) Numeric (legacy sometimes stores ints)
    {
        int32 AsInt = 0;
        if (LexTryParseString(AsInt, *Key))
        {
            // Validate the int is a real enumerator value:
            const int32 Idx = Enum->GetIndexByValue((int64)AsInt);
            if (Idx != INDEX_NONE)
            {
                OutValue = static_cast<EOrbitalType>(Enum->GetValueByIndex(Idx));
                return true;
            }
        }
    }

    // 1) Name match (try raw, then try EnumName::Value)
    {
        // Try "PLANET"
        int64 Val = Enum->GetValueByNameString(Key);
        if (Val != INDEX_NONE)
        {
            OutValue = static_cast<EOrbitalType>(Val);
            return true;
        }

        // Try "EOrbitalType::PLANET"
        const FString Prefixed = FString::Printf(TEXT("%s::%s"), *Enum->GetName(), *Key);
        Val = Enum->GetValueByNameString(Prefixed);
        if (Val != INDEX_NONE)
        {
            OutValue = static_cast<EOrbitalType>(Val);
            return true;
        }
    }

    // 2) DisplayName match fallback
    for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
    {
        const FString Display = SanitizeEnumToken(Enum->GetDisplayNameTextByIndex(i).ToString());
        if (Display.Equals(Key, ESearchCase::IgnoreCase))
        {
            OutValue = static_cast<EOrbitalType>(Enum->GetValueByIndex(i));
            return true;
        }
    }

    return false;
}