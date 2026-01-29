/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib (ported to Unreal)
    FILE:         FontManager.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC
*/

#include "FontManager.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogFontManager, Log, All);

TArray<FFontItem> FontManager::Fonts;

// +--------------------------------------------------------------------+

void FontManager::Close()
{
    // We do not own SystemFont memory here; just clear the registry.
    Fonts.Reset();
}

// +--------------------------------------------------------------------+

void FontManager::Register(const char* NameAnsi, SystemFont* Font)
{
    if (!NameAnsi || !*NameAnsi || !Font)
    {
        UE_LOG(LogFontManager, Warning, TEXT("FontManager::Register called with invalid parameters."));
        return;
    }

    const FString Name = UTF8_TO_TCHAR(NameAnsi);

    // Prevent duplicates:
    for (FFontItem& Item : Fonts)
    {
        if (Item.Name.Equals(Name, ESearchCase::IgnoreCase))
        {
            Item.Font = Font;   // update existing
            Item.Size = 0;
            UE_LOG(LogFontManager, Verbose, TEXT("FontManager::Register updated existing font: %s"), *Name);
            return;
        }
    }

    FFontItem NewItem;
    NewItem.Name = Name;
    NewItem.Size = 0;
    NewItem.Font = Font;

    Fonts.Add(MoveTemp(NewItem));
}

// +--------------------------------------------------------------------+

SystemFont* FontManager::Find(const char* NameAnsi)
{
    if (!NameAnsi || !*NameAnsi)
        return nullptr;

    const FString Name = UTF8_TO_TCHAR(NameAnsi);

    for (const FFontItem& Item : Fonts)
    {
        if (Item.Font && Item.Name.Equals(Name, ESearchCase::IgnoreCase))
            return Item.Font;
    }

    return nullptr;
}
