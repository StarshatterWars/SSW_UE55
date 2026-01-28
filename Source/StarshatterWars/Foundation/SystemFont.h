/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.

    SUBSYSTEM:    nGenEx.lib (ported to Unreal)
    FILE:         SystemFont.h
    AUTHOR:       Carlos Bott
    ORIGINAL:     John DiCamillo / Destroyer Studios LLC (1997-2004)

    OVERVIEW
    ========
    SystemFont
    - Unreal-backed font state wrapper (UFont + size + tint + flags)
    - Maintains legacy-style method names used across the port
    - DOES NOT RENDER. Window/Video layer is responsible for drawing.
*/

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"   // <-- REQUIRED (FSlateFontInfo is used in this header)

#ifdef _WIN32
#ifdef DrawText
#undef DrawText
#endif
#ifdef DrawTextEx
#undef DrawTextEx
#endif
#endif

class UFont;

class SystemFont
{
public:
    static const char* TYPENAME() { return "SystemFont"; }

    enum FLAGS
    {
        FONT_FIXED_PITCH = 1,
        FONT_ALL_CAPS = 2,
        FONT_NO_KERN = 4
    };

    SystemFont();
    explicit SystemFont(const char* InName);
    ~SystemFont();

    // Legacy-style load. Recommended usage is a UFont asset path:
    // "/Game/UI/Fonts/MyFont.MyFont"
    bool Load(const char* InName);

    // Legacy compatibility (delegates to the active Video/Window renderer):
    int DrawString(const char* text, int len, int x, int y, const Rect& clip, Video* video) const;
    int DrawTextW(const wchar_t* text, int len, int x, int y, const Rect& clip, Video* video) const;

    int DrawString(
        const char* text,
        int len,
        int x,
        int y,
        const Rect& clip
    ) const;

    int DrawTextW(
        const wchar_t* text,
        int len,
        int x,
        int y,
        const Rect& clip
    ) const;

    int DrawText(
        const char* text,
        int len,
        const Rect& clip,
        uint32 flags
    ) const;

    int DrawText(
        const Text& text,
        int len,
        const Rect& clip,
        uint32 flags
    ) const;

    int DrawText(
        const char* text,
        int len,
        const Rect& clip,
        uint32 flags,
        Bitmap* target
    ) const;

    int DrawText(
        const Text& text,
        int len,
        const Rect& clip,
        uint32 flags,
        Bitmap* target
    ) const;

    // Basic state:
    void   SetUFont(UFont* InFont);
    UFont* GetUFont() const;

    void   SetPointSize(int32 InSize);
    int32  GetPointSize() const;

    // Legacy API compatibility:
    uint16 GetFlags() const;
    void   SetFlags(uint16 InFlags);

    FColor GetColor() const;
    void   SetColor(const FColor& InColor);

    double GetExpansion() const;
    void   SetExpansion(double InExpansion);

    double GetAlpha() const;
    void   SetAlpha(double InAlpha);

    int    GetBlend() const;
    void   SetBlend(int InBlend);

    // Useful for Slate drawing:
    FLinearColor   GetLinearColor() const;
    FSlateFontInfo MakeSlateFontInfo() const;

    // Optional: keep name for debugging/lookup
    const char* GetName() const;

private:
    void UpdateHeuristics();

private:
    char  Name[64];

    uint16 Flags;
    float  Expansion;
    float  Alpha;
    int    Blend;

    int32  PointSize;
    FColor Color;

    // Unreal backing:
    TWeakObjectPtr<UFont> UnrealFont;
};
