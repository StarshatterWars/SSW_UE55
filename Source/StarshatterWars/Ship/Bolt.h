/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Bolt.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    3D Bolt Object
*/

#pragma once

#include "Graphic.h"
#include "Polygon.h"

// Minimal Unreal includes (Vec3/Point -> FVector)
#include "Math/Vector.h"

// Forward declarations (keep header light)
class UTexture2D;

// +--------------------------------------------------------------------+

class Bolt : public Graphic
{
public:
    static const char* TYPENAME() { return "Bolt"; }

    Bolt(double len = 16, double wid = 1, UTexture2D* tex = 0, int share = 0);
    virtual ~Bolt();

    // operations
    virtual void   Render(Video* video, DWORD flags);
    virtual void   Update();

    // accessors / mutators
    virtual void   SetOrientation(const Matrix& o);
    void           SetDirection(const FVector& v);
    void           SetEndPoints(const FVector& from, const FVector& to);
    void           SetTextureOffset(double from, double to);

    virtual void   TranslateBy(const FVector& ref);

    double         Shade() const { return shade; }
    void           SetShade(double s) { shade = s; }
    virtual bool   IsBolt() const { return true; }

protected:
    double         length;
    double         width;
    double         shade;

    Poly           poly;
    Material       mtl;
    VertexSet      vset;
    UTexture2D* texture;
    int            shared;

    FVector        vpn;
    FVector        origin;
};

// +--------------------------------------------------------------------+

