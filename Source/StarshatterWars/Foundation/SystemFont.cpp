/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.

    SUBSYSTEM:    nGenEx.lib (ported to Unreal)
    FILE:         SystemFont.cpp
    AUTHOR:       Carlos Bott
    ORIGINAL:     John DiCamillo / Destroyer Studios LLC (1997-2004)
*/

#include "SystemFont.h"

#include "Engine/Font.h"
#include "Styling/CoreStyle.h"

SystemFont::SystemFont()
    : Flags(0)
    , Expansion(0.0f)
    , Alpha(1.0f)
    , Blend(0)
    , PointSize(12)
    , Color(FColor::White)
    , UnrealFont(nullptr)
{
    FMemory::Memzero(Name, sizeof(Name));
}

SystemFont::SystemFont(const char* InName)
    : SystemFont()
{
    if (InName && InName[0])
    {
        FCStringAnsi::Strncpy(Name, InName, sizeof(Name));
        Load(Name);
    }
}

SystemFont::~SystemFont()
{
    // no heap allocations
}

bool SystemFont::Load(const char* InName)
{
    if (!InName || !InName[0])
        return false;

    FCStringAnsi::Strncpy(Name, InName, sizeof(Name));

    const FString Path = UTF8_TO_TCHAR(InName);

    UFont* Loaded = Cast<UFont>(StaticLoadObject(UFont::StaticClass(), nullptr, *Path));
    if (!Loaded)
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemFont::Load failed. Expected UFont asset path. Got: %s"), *Path);
        UnrealFont = nullptr;
        return false;
    }

    UnrealFont = Loaded;
    UpdateHeuristics();
    return true;
}

void SystemFont::SetUFont(UFont* InFont)
{
    UnrealFont = InFont;
}

UFont* SystemFont::GetUFont() const
{
    return UnrealFont.Get();
}

void SystemFont::SetPointSize(int32 InSize)
{
    PointSize = FMath::Max(1, InSize);
    UpdateHeuristics();
}

int32 SystemFont::GetPointSize() const
{
    return PointSize;
}

uint16 SystemFont::GetFlags() const
{
    return Flags;
}

void SystemFont::SetFlags(uint16 InFlags)
{
    Flags = InFlags;
}

FColor SystemFont::GetColor() const
{
    return Color;
}

void SystemFont::SetColor(const FColor& InColor)
{
    Color = InColor;
}

double SystemFont::GetExpansion() const
{
    return (double)Expansion;
}

void SystemFont::SetExpansion(double InExpansion)
{
    Expansion = (float)InExpansion;
}

double SystemFont::GetAlpha() const
{
    return (double)Alpha;
}

void SystemFont::SetAlpha(double InAlpha)
{
    Alpha = (float)FMath::Clamp(InAlpha, 0.0, 1.0);
}

int SystemFont::GetBlend() const
{
    return Blend;
}

void SystemFont::SetBlend(int InBlend)
{
    Blend = InBlend;
}

FLinearColor SystemFont::GetLinearColor() const
{
    FColor C = Color;
    C.A = (uint8)FMath::Clamp((int)FMath::RoundToInt(Alpha * 255.0f), 0, 255);
    return FLinearColor(C);
}

FSlateFontInfo SystemFont::MakeSlateFontInfo() const
{
    if (UFont* UF = UnrealFont.Get())
    {
        return FSlateFontInfo(UF, PointSize);
    }

    return FCoreStyle::GetDefaultFontStyle("Regular", PointSize);
}

const char* SystemFont::GetName() const
{
    return Name;
}

void SystemFont::UpdateHeuristics()
{
    // Hook for future baseline/height if you need it.
}
