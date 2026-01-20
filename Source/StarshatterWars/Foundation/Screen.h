/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Screen.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    General Screen class - maintains and displays a list of windows
*/

#pragma once

#include "Types.h"
#include "Color.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Bitmap;
class Window;
class Video;
struct Rect;

// +--------------------------------------------------------------------+

class Screen
{
public:
    static const char* TYPENAME() { return "Screen"; }

    Screen(Video* v);
    virtual ~Screen();

    virtual bool      SetBackgroundColor(Color c);

    virtual bool      Resize(int w, int h);
    virtual bool      Refresh();
    virtual bool      AddWindow(Window* c);
    virtual bool      DelWindow(Window* c);

    int               Width()     const { return width; }
    int               Height()    const { return height; }

    virtual void      ClearAllFrames(bool clear_all);
    virtual void      ClearNextFrames(int num_frames);

    virtual Video* GetVideo()  const { return video; }

protected:
    int               width;
    int               height;
    int               clear;
    int               closed;

    Video* video;

    List<Window>      window_list;
};
