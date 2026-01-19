/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         SystemFont.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo, Destroyer Studios LLC


    OVERVIEW
    ========
    Font Resource class
*/

#pragma once

#include "Types.h"
#include "Color.h"
#include "Geometry.h"

// Minimal Unreal include required for UTexture2D declaration usage:
#include "UObject/WeakObjectPtr.h"

// +--------------------------------------------------------------------+

class UTexture2D;

struct Poly;
struct Material;
struct VertexSet;
class  Video;

// +--------------------------------------------------------------------+

struct FontChar
{
    short offset;
    short width;
};

// +--------------------------------------------------------------------+

class SystemFont
{
public:
    static const char* TYPENAME() { return "SystemFont"; }

    enum FLAGS {
        FONT_FIXED_PITCH = 1,
        FONT_ALL_CAPS = 2,
        FONT_NO_KERN = 4
    };

    enum CHARS {
        PIPE_NBSP = 16,
        PIPE_VERT = 17,
        PIPE_LT = 18,
        PIPE_TEE = 19,
        PIPE_UL = 20,
        PIPE_LL = 21,
        PIPE_HORZ = 22,
        PIPE_PLUS = 23,
        PIPE_MINUS = 24,
        ARROW_UP = 25,
        ARROW_DOWN = 26,
        ARROW_LEFT = 27,
        ARROW_RIGHT = 28
    };

    // default constructor:
    SystemFont();
    SystemFont(const char* name);
    ~SystemFont();

    bool     Load(const char* name);

    int      CharWidth(char c) const;
    int      SpaceWidth() const;
    int      KernWidth(char left, char right) const;
    int      StringWidth(const char* str, int len = 0) const;

    void     DrawText(const char* txt, int count, Rect& txt_rect, DWORD flags, UTexture2D* tgt_texture = 0);
   
    

    int      DrawString(const char* txt, int len, int x1, int y1, const Rect& clip, UTexture2D* tgt_texture = 0);

    int      Height()                   const { return height; }
    int      Baseline()                 const { return baseline; }
    WORD     GetFlags()                 const { return flags; }
    void     SetFlags(WORD s) { flags = s; }
    Color    GetColor()                 const { return color; }
    void     SetColor(const Color& c) { color = c; }
    double   GetExpansion()             const { return expansion; }
    void     SetExpansion(double e) { expansion = (float)e; }
    double   GetAlpha()                 const { return alpha; }
    void     SetAlpha(double a) { alpha = (float)a; }
    int      GetBlend()                 const { return blend; }
    void     SetBlend(int b) { blend = b; }

    void     SetKern(char left, char right, int k = 0);

    int      GetCaretIndex()            const { return caret_index; }
    void     SetCaretIndex(int n) { caret_index = n; }

private:
    void        AutoKern();
    void        FindEdges(BYTE c, double* l, double* r);
    int         CalcWidth(BYTE c) const;
    int         GlyphOffset(BYTE c) const;
    int         GlyphLocationX(BYTE c) const;
    int         GlyphLocationY(BYTE c) const;

    void        DrawTextSingle(const char* text, int count, const Rect& text_rect, Rect& clip_rect);
    void        DrawTextWrap(const char* text, int count, const Rect& text_rect, Rect& clip_rect);
    void        DrawTextMulti(const char* text, int count, const Rect& text_rect, Rect& clip_rect);

    void        LoadDef(char* defname, char* imgname);

    char        name[64];
    WORD        flags;
    BYTE        height;
    BYTE        baseline;
    BYTE        interspace;
    BYTE        spacewidth;
    float       expansion;
    float       alpha;
    int         blend;
    int         scale;

    int         caret_index;
    int         caret_x;
    int         caret_y;

    int         imagewidth;
    BYTE* image;

    // Starshatter Wars (UE): replace Bitmap render assets with UE texture handle:
    // Forward declared above; keep header light.
    TWeakObjectPtr<UTexture2D> texture;
    UTexture2D* tgt_texture;

    Material* material;
    VertexSet* vset;
    Poly* polys;
    int         npolys;

    FontChar    glyph[256];
    Color       color;

    char        kern[256][256];
};
