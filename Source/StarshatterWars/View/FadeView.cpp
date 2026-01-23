/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         FadeView.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Fading Bitmap "billboard" Image View class
*/

#include "FadeView.h"

#include "Window.h"
#include "Video.h"
#include "Screen.h"
#include "Game.h"

// Unreal:
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogFadeView, Log, All);

// +--------------------------------------------------------------------+
// Fade state (owned by FadeView; Unreal renderer/camera can consume it)

static double g_fade_value = 1.0;
static FColor g_fade_color = FColor::Black;
static int    g_shade_built = 0;

// +--------------------------------------------------------------------+

FadeView::FadeView(Window* c, double in, double out, double hold)
    : View(c)
    , fade_in(in * 1000)
    , fade_out(out * 1000)
    , hold_time(hold * 1000)
    , time(0)
    , step_time(0)
    , fast(1)
    , state(StateStart)
{
}

FadeView::~FadeView()
{
}

// +--------------------------------------------------------------------+

void FadeView::FadeIn(double in) { fade_in = in * 1000; }
void FadeView::FadeOut(double out) { fade_out = out * 1000; }
void FadeView::FastFade(int fade_fast) { fast = fade_fast; }

void FadeView::StopHold()
{
    UE_LOG(LogFadeView, VeryVerbose, TEXT("FadeView::StopHold()"));
    hold_time = 0;
}

// +--------------------------------------------------------------------+
// Replaces legacy Color::SetFade. This is FadeView-owned fade state.
// Palette/shade-table behavior is not applicable in Unreal.

void
FadeView::SetFade(double f, const FColor& c, int build_shade)
{
    // Clamp:
    if (f < 0.0) f = 0.0;
    if (f > 1.0) f = 1.0;

    // Preserve original early-out semantics:
    if (g_fade_value == f && g_fade_color == c && (build_shade ? g_shade_built : 1))
        return;

    g_fade_value = f;
    g_fade_color = c;

    // Unreal: no paletted-video pipeline and no global shade tables here.
    // Preserve the "shade built" flag semantics for callers that expect it:
    g_shade_built = build_shade ? 1 : 0;

    UE_LOG(LogFadeView, VeryVerbose,
        TEXT("FadeView::SetFade fade=%.3f color=(%d,%d,%d,%d) build_shade=%d"),
        g_fade_value,
        g_fade_color.R, g_fade_color.G, g_fade_color.B, g_fade_color.A,
        build_shade ? 1 : 0
    );
}

// +--------------------------------------------------------------------+

void
FadeView::Refresh()
{
    double msec = 0;

    if (state == StateStart) {
        time = Game::RealTime();
    }
    else if (state != StateDone) {
        const double new_time = Game::RealTime();
        msec = new_time - time;
        time = new_time;
    }

    switch (state) {
    case StateStart:
        if (fade_in) {
            // Initial fade value:
            SetFade(0.0, FColor::Black, 0);
        }

        step_time = 0;
        state = State2;
        break;

    case State2:
        if (fade_in) {
            SetFade(0.0, FColor::Black, 0);
        }

        step_time = 0;
        state = StateIn;
        break;

    case StateIn:
        if (step_time < fade_in) {
            const double fade = (fade_in > 0.0) ? (step_time / fade_in) : 1.0;
            SetFade(fade, FColor::Black, 0);
            step_time += msec;
        }
        else {
            SetFade(1.0, FColor::Black, 0);
            step_time = 0;
            state = StateHold;
        }
        break;

    case StateHold:
        if (step_time < hold_time) {
            step_time += msec;
        }
        else {
            step_time = 0;
            state = StateOut;
        }
        break;

    case StateOut:
        if (fade_out > 0.0) {
            if (step_time < fade_out) {
                const double fade = 1.0 - (step_time / fade_out);
                SetFade(fade, FColor::Black, 0);
                step_time += msec;
            }
            else {
                SetFade(0.0, FColor::Black, 0);
                step_time = 0;
                state = StateDone;
            }
        }
        else {
            // Match original behavior: no fade-out time => remain visible, then done.
            SetFade(1.0, FColor::Black, 0);
            step_time = 0;
            state = StateDone;
        }
        break;

    default:
    case StateDone:
        break;
    }
}
