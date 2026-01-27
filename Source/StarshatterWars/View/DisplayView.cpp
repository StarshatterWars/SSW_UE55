/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         DisplayView.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR: John DiCamillo
    ORIGINAL STUDIO: Destroyer Studios LLC

    OVERVIEW
    ========
    View class for Quantum Destination HUD Overlay
*/

#include "DisplayView.h"

#include "Window.h"
#include "Game.h"
#include "Bitmap.h"

// SystemFont replaced by UE font:
#include "Engine/Font.h"

// Render asset replacement:
#include "Engine/Texture2D.h"

// +====================================================================+

TWeakObjectPtr<UDisplayView> UDisplayView::DisplayViewInstance;

// +====================================================================+

UDisplayView::UDisplayView(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UDisplayView* UDisplayView::GetInstance(UWorld* World, TSubclassOf<UDisplayView> WidgetClass)
{
    if (DisplayViewInstance.IsValid())
        return DisplayViewInstance.Get();

    if (!World || !WidgetClass)
        return nullptr;

    // Use the first local player controller if available:
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
        return nullptr;

    UDisplayView* W = CreateWidget<UDisplayView>(PC, WidgetClass);
    DisplayViewInstance = W;
    return W;
}

void UDisplayView::InitializeView(Window* InWindow)
{
    SetWindow(InWindow);     // inherited from UView
    UpdateCachedWindowMetrics();
}

void UDisplayView::UpdateCachedWindowMetrics()
{
    Window* W = GetWindow();
    if (!W)
        return;

    width = W->Width();
    height = W->Height();
    xcenter = (width / 2.0) - 0.5;
    ycenter = (height / 2.0) + 0.5;
}

void UDisplayView::OnWindowMove()
{
    UpdateCachedWindowMetrics();
}

// +--------------------------------------------------------------------+

void UDisplayView::Refresh()
{
    Window* W = GetWindow();
    if (!W)
        return;

    // Legacy iteration:
    for (const FDisplayElement& Elem : Elements)
    {
        // Convert relative rect to window rect:
        Rect elem_rect = Elem.RectValue;

        if (elem_rect.x == 0 && elem_rect.y == 0 && elem_rect.w == 0 && elem_rect.h == 0)
        {
            // Stretch to fit:
            elem_rect.w = width;
            elem_rect.h = height;
        }
        else if (elem_rect.w < 0 && elem_rect.h < 0)
        {
            // Center image in window:
            elem_rect.w *= -1;
            elem_rect.h *= -1;

            elem_rect.x = (width - elem_rect.w) / 2;
            elem_rect.y = (height - elem_rect.h) / 2;
        }
        else
        {
            // Offset from right or bottom:
            if (elem_rect.x < 0) elem_rect.x += width;
            if (elem_rect.y < 0) elem_rect.y += height;
        }

        // Compute current fade (assumes fades are ~1 second or less in legacy logic):
        double fade = 0.0;
        if (Elem.FadeIn > 0) fade = 1.0 - Elem.FadeIn;
        else if (Elem.Hold > 0) fade = 1.0;
        else if (Elem.FadeOut > 0) fade = Elem.FadeOut;

        // Clamp fade defensively:
        fade = FMath::Clamp(fade, 0.0, 1.0);

        // Draw text:
        if (Elem.HasText() && Elem.Font)
        {
            // IMPORTANT:
            // This assumes your Window wrapper supports UE fonts now (SystemFont*),
            // or that Window::SetFont performs any conversion needed.
            W->SetFont(Elem.Font);

            // If your wrapper supports per-draw color/alpha:
            W->SetTextColor(Elem.Color);
            W->SetTextAlpha((float)fade);

            W->DrawText(Elem.TextValue, Elem.TextValue.length(), elem_rect, DT_WORDBREAK);
        }

        // Draw image:
        else if (Elem.HasImage())
        {
            const FColor FadedColor(
                (uint8)FMath::Clamp((int)(Elem.Color.R * fade), 0, 255),
                (uint8)FMath::Clamp((int)(Elem.Color.G * fade), 0, 255),
                (uint8)FMath::Clamp((int)(Elem.Color.B * fade), 0, 255),
                Elem.Color.A
            );

            W->FadeBitmap(
                elem_rect.x,
                elem_rect.y,
                elem_rect.x + elem_rect.w,
                elem_rect.y + elem_rect.h,
                Elem.Image,
                FadedColor,
                Elem.Blend
            );
        }
    }
}

// +--------------------------------------------------------------------+

void UDisplayView::ExecFrame(float DeltaTime)
{
    // Legacy used Game::GUITime() for step seconds.
    // If you still want the exact legacy behavior, you can ignore DeltaTime and use Game::GUITime().
    // But keeping DeltaTime makes this usable from UE tick.
    const double seconds = (DeltaTime > 0.0f) ? (double)DeltaTime : (double)Game::GUITime();

    for (int32 i = Elements.Num() - 1; i >= 0; --i)
    {
        FDisplayElement& Elem = Elements[i];

        if (Elem.FadeIn > 0.0)
            Elem.FadeIn -= seconds;
        else if (Elem.Hold > 0.0)
            Elem.Hold -= seconds;
        else if (Elem.FadeOut > 0.0)
            Elem.FadeOut -= seconds;
        else
            Elements.RemoveAtSwap(i);
    }
}

// +--------------------------------------------------------------------+

void UDisplayView::ClearDisplay()
{
    Elements.Reset();
}

// +--------------------------------------------------------------------+

void UDisplayView::AddText(
    const char* text,
    SystemFont* font,
    FColor color,
    const Rect& rect,
    double hold,
    double fade_in,
    double fade_out)
{
    if (fade_in == 0 && fade_out == 0 && hold == 0)
        hold = 300;

    FDisplayElement Elem;
    Elem.TextValue = text;
    Elem.Font = font;
    Elem.Color = color;
    Elem.RectValue = rect;
    Elem.Hold = hold;
    Elem.FadeIn = fade_in;
    Elem.FadeOut = fade_out;

    Elements.Add(Elem);
}

void UDisplayView::AddImage(
    Bitmap* texture,
    FColor color,
    int blend,
    const Rect& rect,
    double hold,
    double fade_in,
    double fade_out)
{
    if (fade_in == 0 && fade_out == 0 && hold == 0)
        hold = 300;

    FDisplayElement Elem;
    Elem.Image = texture;
    Elem.Color = color;
    Elem.Blend = blend;
    Elem.RectValue = rect;
    Elem.Hold = hold;
    Elem.FadeIn = fade_in;
    Elem.FadeOut = fade_out;

    Elements.Add(Elem);
}
