/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         ImageView.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Bitmap "billboard" Image View class
*/

#include "ImageView.h"

#include "Window.h"
#include "Video.h"
#include "Bitmap.h"
#include "Screen.h"

// Unreal (for UE_LOG only):
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogImageView, Log, All);

// +--------------------------------------------------------------------+

ImageView::ImageView(Window* c, Bitmap* bmp)
    : View(c)
    , img(bmp)
    , x_offset(0)
    , y_offset(0)
    , width(0)
    , height(0)
    , blend(Video::BLEND_SOLID)
{
    if (img) {
        width = img->Width();
        height = img->Height();
    }

    if (window) {
        if (width < window->Width())
            x_offset = (window->Width() - width) / 2;

        if (height < window->Height())
            y_offset = (window->Height() - height) / 2;
    }
}

ImageView::~ImageView()
{
}

// +--------------------------------------------------------------------+

void
ImageView::Refresh()
{
    if (img && width > 0 && height > 0 && window) {
        window->DrawBitmap(
            x_offset,
            y_offset,
            x_offset + width,
            y_offset + height,
            img,
            blend
        );
    }
}

// +--------------------------------------------------------------------+

void
ImageView::SetPicture(Bitmap* bmp)
{
    img = bmp;
    width = 0;
    height = 0;
    x_offset = 0;
    y_offset = 0;

    if (img) {
        width = img->Width();
        height = img->Height();
    }

    if (window) {
        x_offset = (window->Width() - width) / 2;
        y_offset = (window->Height() - height) / 2;
    }
}

