/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         FadeView.h
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Non-rendering view class that controls the fade level (fade-in/fade-out)
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

// +--------------------------------------------------------------------+

class FadeView : public View
{
public:
    static const char* TYPENAME() { return "FadeView"; }

    enum FadeState { StateStart, State2, StateIn, StateHold, StateOut, StateDone };

    FadeView(Window* c, double fade_in = 1, double fade_out = 1, double hold_time = 4);
    virtual ~FadeView();

    // Operations:
    virtual void      Refresh();
    virtual bool      Done()    const { return state == StateDone; }
    virtual bool      Holding() const { return state == StateHold; }

    // Control:
    virtual void      FastFade(int fade_fast);
    virtual void      FadeIn(double fade_in);
    virtual void      FadeOut(double fade_out);
    virtual void      StopHold();

    // Fade state control (replaces legacy Color::SetFade):
    virtual void      SetFade(double fade, const FColor& color, int build_shade = 0);

protected:
    double      fade_in;
    double      fade_out;
    double      hold_time;
    double      time;
    double      step_time;

    int         fast;
    FadeState   state;
};
