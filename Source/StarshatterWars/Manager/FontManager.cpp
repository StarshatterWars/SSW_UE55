/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         FontManager.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Font Resource Manager class implementation
*/

#include "FontManager.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogFontManager, Log, All);

// +--------------------------------------------------------------------+

List<FontItem> FontManager::fonts;

// +--------------------------------------------------------------------+

void
FontManager::Close()
{
    fonts.destroy();
}

// +--------------------------------------------------------------------+

void
FontManager::Register(const char* name, Font* font)
{
    if (!name || !*name || !font) {
        UE_LOG(LogFontManager, Warning, TEXT("FontManager::Register called with invalid parameters."));
        return;
    }

    FontItem* item = new FontItem;

    if (item) {
        item->name = name;
        item->size = 0;
        item->font = font;

        fonts.append(item);
    }
    else {
        UE_LOG(LogFontManager, Error, TEXT("FontManager::Register failed to allocate FontItem."));
    }
}

// +--------------------------------------------------------------------+

Font*
FontManager::Find(const char* name)
{
    if (!name || !*name)
        return nullptr;

    ListIter<FontItem> item = fonts;
    while (++item) {
        if (item->name == name)
            return item->font;
    }

    return nullptr;
}
