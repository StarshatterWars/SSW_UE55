/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib (legacy compatibility layer)
    FILE:         FontManager.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC
*/

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"

class SystemFont;

// +--------------------------------------------------------------------+
// Legacy-Compatible Font Manager
// +--------------------------------------------------------------------+

class FontManager
{
public:
    static const char* TYPENAME() { return "FontManager"; }

    // New UE-style API:
    static void Close(UObject* WorldContext);
    static void Register(UObject* WorldContext, const char* NameAnsi, const FSlateFontInfo& FontInfo);
    static bool Find(UObject* WorldContext, const char* NameAnsi, FSlateFontInfo& OutFontInfo);

    // Legacy API (minimal churn):
    static void        Register(const char* NameAnsi, SystemFont* Font);
    static SystemFont* Find(const char* NameAnsi);
    static void        Close();
};
