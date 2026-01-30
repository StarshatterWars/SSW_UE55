/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    nGenEx.lib
    FILE:         ImageView.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    ImageView
    - Bitmap billboard renderer.
    - Centers image inside its Window.
*/

#include "ImageView.h"

#include "Window.h"
#include "Video.h"
#include "Bitmap.h"

// --------------------------------------------------------------------

ImageView::ImageView(Window* InWindow, Bitmap* InBitmap)
    : View(InWindow),
    Image(InBitmap),
    Blend(Video::BLEND_SOLID)
{
    if (Image) {
        Width = Image->Width();
        Height = Image->Height();
    }

    if (window && Width < window->Width()) {
        XOffset = (window->Width() - Width) / 2;
    }

    if (window && Height < window->Height()) {
        YOffset = (window->Height() - Height) / 2;
    }
}

ImageView::~ImageView()
{
}

// --------------------------------------------------------------------

void ImageView::Refresh()
{
    if (!Image || Width <= 0 || Height <= 0 || !window)
        return;

    window->DrawBitmap(
        XOffset,
        YOffset,
        XOffset + Width,
        YOffset + Height,
        Image,
        Blend
    );
}

// --------------------------------------------------------------------

void ImageView::SetPicture(Bitmap* InBmp)
{
    Image = InBmp;
    Width = 0;
    Height = 0;
    XOffset = 0;
    YOffset = 0;

    if (Image) {
        Width = Image->Width();
        Height = Image->Height();
    }

    if (window) {
        XOffset = (window->Width() - Width) / 2;
        YOffset = (window->Height() - Height) / 2;
    }
}
