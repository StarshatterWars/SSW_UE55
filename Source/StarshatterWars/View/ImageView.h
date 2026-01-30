/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    nGenEx.lib
    FILE:         ImageView.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    ImageView
    - Bitmap "billboard" view.
    - Centers a bitmap within its parent Window.
    - Direct port of legacy ImgView (Starshatter 4.5).
*/

#pragma once

#include "Types.h"
#include "View.h"

// --------------------------------------------------------------------

class Bitmap;

// --------------------------------------------------------------------

class ImageView : public View
{
public:
    static const char* TYPENAME() { return "ImageView"; }

    ImageView(Window* InWindow, Bitmap* InBitmap);
    virtual ~ImageView();

    // ------------------------------------------------------------
    // View interface
    // ------------------------------------------------------------
    virtual void Refresh() override;

    // ------------------------------------------------------------
    // Accessors
    // ------------------------------------------------------------
    Bitmap* GetPicture() const { return Image; }
    void    SetPicture(Bitmap* InBmp);

    int     GetBlend() const { return Blend; }
    void    SetBlend(int InBlend) { Blend = InBlend; }

protected:
    // ------------------------------------------------------------
    // Internal state
    // ------------------------------------------------------------
    Bitmap* Image = nullptr;

    int XOffset = 0;
    int YOffset = 0;
    int Width = 0;
    int Height = 0;

    int Blend = 0;
};
