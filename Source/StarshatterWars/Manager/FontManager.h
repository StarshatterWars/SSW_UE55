/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib (ported to Unreal)
    FILE:         FontManager.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Font Resource Manager class
    - Unreal-friendly registry using TArray
    - Keeps legacy method names: Close/Register/Find
*/

#pragma once

#include "CoreMinimal.h"

// Forward declarations:
class SystemFont;

// +--------------------------------------------------------------------+

struct FFontItem
{
    FString     Name;
    int32       Size = 0;
    SystemFont* Font = nullptr;
};

// +--------------------------------------------------------------------+

class FontManager
{
public:
    static const char* TYPENAME() { return "FontManager"; }

    static void        Close();
    static void        Register(const char* NameAnsi, SystemFont* Font);
    static SystemFont* Find(const char* NameAnsi);

private:
    static TArray<FFontItem> Fonts;
};
