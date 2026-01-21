/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         FontManager.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Font Resource Manager class
*/

#pragma once

#include "List.h"
#include "Text.h"

// Forward declarations (keep header light)
class SystemFont;

// +--------------------------------------------------------------------+

struct FontItem
{
    static const char* TYPENAME() { return "FontItem"; }

    Text  name;
    int   size = 0;
    SystemFont* font = nullptr;
};

// +--------------------------------------------------------------------+

class FontManager
{
public:
    static const char* TYPENAME() { return "FontManager"; }

    static void   Close();
    static void   Register(const char* name, SystemFont* font);
    static SystemFont* Find(const char* name);

private:
    static List<FontItem> fonts;
};

