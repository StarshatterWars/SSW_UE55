/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib (legacy compatibility layer)
    FILE:         FontManager.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC
*/

#include "FontManager.h"
#include "FontManagerSubsystem.h"
#include "SystemFont.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogFontManager, Log, All);

// +--------------------------------------------------------------------+
// Legacy backing store (only for bridge period)
// - If you already had a static registry, keep it here.
// - This avoids needing WorldContext in legacy call sites.
// +--------------------------------------------------------------------+

struct FLegacyFontEntry
{
    FString     Name;
    SystemFont* Font = nullptr;
};

static TArray<FLegacyFontEntry> GLegacyFonts;

// +--------------------------------------------------------------------+
// Unreal subsystem helper
// +--------------------------------------------------------------------+

static UFontManagerSubsystem* GetFontManager(UObject* WorldContext)
{
    if (!WorldContext)
        return nullptr;

    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext))
        return GI->GetSubsystem<UFontManagerSubsystem>();

    return nullptr;
}

// +--------------------------------------------------------------------+
// New UE API
// +--------------------------------------------------------------------+

void FontManager::Close(UObject* WorldContext)
{
    if (UFontManagerSubsystem* FM = GetFontManager(WorldContext))
        FM->Close();
}

void FontManager::Register(UObject* WorldContext, const char* NameAnsi, const FSlateFontInfo& FontInfo)
{
    if (!NameAnsi || !*NameAnsi)
        return;

    if (UFontManagerSubsystem* FM = GetFontManager(WorldContext))
        FM->RegisterFont(FName(UTF8_TO_TCHAR(NameAnsi)), FontInfo);
}

bool FontManager::Find(UObject* WorldContext, const char* NameAnsi, FSlateFontInfo& OutFontInfo)
{
    if (!NameAnsi || !*NameAnsi)
        return false;

    if (UFontManagerSubsystem* FM = GetFontManager(WorldContext))
        return FM->FindFont(FName(UTF8_TO_TCHAR(NameAnsi)), OutFontInfo);

    return false;
}

// +--------------------------------------------------------------------+
// Legacy API (no WorldContext)
// - Uses a small bridge registry storing SystemFont*.
// - Later you can remove this once HUDView/MFDView are converted.
// +--------------------------------------------------------------------+

void FontManager::Close()
{
    GLegacyFonts.Reset();
}

void FontManager::Register(const char* NameAnsi, SystemFont* Font)
{
    if (!NameAnsi || !*NameAnsi || !Font)
        return;

    const FString Name = UTF8_TO_TCHAR(NameAnsi);

    for (FLegacyFontEntry& E : GLegacyFonts)
    {
        if (E.Name.Equals(Name, ESearchCase::IgnoreCase))
        {
            E.Font = Font;
            return;
        }
    }

    FLegacyFontEntry NewEntry;
    NewEntry.Name = Name;
    NewEntry.Font = Font;

    GLegacyFonts.Add(MoveTemp(NewEntry));
}

SystemFont* FontManager::Find(const char* NameAnsi)
{
    if (!NameAnsi || !*NameAnsi)
        return nullptr;

    const FString Name = UTF8_TO_TCHAR(NameAnsi);

    for (const FLegacyFontEntry& E : GLegacyFonts)
    {
        if (E.Font && E.Name.Equals(Name, ESearchCase::IgnoreCase))
            return E.Font;
    }

    return nullptr;
}
