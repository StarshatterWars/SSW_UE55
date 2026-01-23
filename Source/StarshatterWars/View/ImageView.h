/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         ImageView.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Bitmap "Billboard" View class
*/

#pragma once

#include "Types.h"
#include "View.h"

// Unreal (minimal, per project standard):
#include "Math/Vector.h"               // FVector
#include "Math/Color.h"                // FColor
#include "Math/UnrealMathUtility.h"    // FMath

// +--------------------------------------------------------------------+
// Forward Declarations:

class Window;
class Bitmap;

// +--------------------------------------------------------------------+

class ImageView : public View
{
public:
    static const char* TYPENAME() { return "ImgView"; }

    ImageView(Window* c, Bitmap* bmp);
    virtual ~ImageView();

    // Operations:
    virtual void      Refresh();

    virtual Bitmap*     GetPicture() const { return img; }
    virtual void        SetPicture(Bitmap* bmp);
    virtual int         GetBlend()   const { return blend; }
    virtual void        SetBlend(int b) { blend = b; }

protected:
    Bitmap* img;
    int         x_offset;
    int         y_offset;
    int         width;
    int         height;
    int         blend;
};
