/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026. All Rights Reserved.

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
#include "SystemFont.h"

// Render asset replacement:
#include "Engine/Texture2D.h"

// +====================================================================+

class DisplayElement
{
public:
	static const char* TYPENAME() { return "DisplayElement"; }

	DisplayElement()
		: image(0), font(0), blend(0), hold(0), fade_in(0), fade_out(0)
	{
	}

	Text        text;
	Bitmap*		image;
	SystemFont* font;
	FColor      color;
	Rect        rect;
	int         blend;
	double      hold;
	double      fade_in;
	double      fade_out;
};

// +====================================================================+

static DisplayView* display_view = 0;

static FORCEINLINE uint8 ScaleByte(uint8 Value, float Scale)
{
	const int32 Scaled = FMath::RoundToInt((float)Value * Scale);
	return (uint8)FMath::Clamp(Scaled, 0, 255);
}

static FORCEINLINE FColor ScaleColor(const FColor& In, float Scale)
{
	return FColor(
		ScaleByte(In.R, Scale),
		ScaleByte(In.G, Scale),
		ScaleByte(In.B, Scale),
		ScaleByte(In.A, Scale)   // scale alpha too for fade
	);
}

DisplayView::DisplayView(Window* c)
	: View(c), width(0), height(0), xcenter(0), ycenter(0)
{
	display_view = this;
	DisplayView::OnWindowMove();
}

DisplayView::~DisplayView()
{
	if (display_view == this)
		display_view = 0;

	elements.destroy();
}

DisplayView*
DisplayView::GetInstance()
{
	if (display_view == 0)
		display_view = new DisplayView(0);

	return display_view;
}

void
DisplayView::OnWindowMove()
{
	if (window) {
		width = window->Width();
		height = window->Height();
		xcenter = (width / 2.0) - 0.5;
		ycenter = (height / 2.0) + 0.5;
	}
}

// +--------------------------------------------------------------------+

void
DisplayView::Refresh()
{
	ListIter<DisplayElement> iter = elements;
	while (++iter) {
		DisplayElement* elem = iter.value();

		// convert relative rect to window rect:
		Rect elem_rect = elem->rect;
		if (elem_rect.x == 0 && elem_rect.y == 0 && elem_rect.w == 0 && elem_rect.h == 0) {
			// stretch to fit
			elem_rect.w = width;
			elem_rect.h = height;
		}
		else if (elem_rect.w < 0 && elem_rect.h < 0) {
			// center image in window
			elem_rect.w *= -1;
			elem_rect.h *= -1;

			elem_rect.x = (width - elem_rect.w) / 2;
			elem_rect.y = (height - elem_rect.h) / 2;
		}
		else {
			// offset from right or bottom
			if (elem_rect.x < 0) elem_rect.x += width;
			if (elem_rect.y < 0) elem_rect.y += height;
		}

		// compute current fade,
		// assumes fades are 1 second or less:
		double fade = 0;
		if (elem->fade_in > 0)        fade = 1 - elem->fade_in;
		else if (elem->hold > 0)      fade = 1;
		else if (elem->fade_out > 0)  fade = elem->fade_out;

		// draw text:
		if (elem->text.length() && elem->font) {
			elem->font->SetColor(elem->color);
			elem->font->SetAlpha(fade);
			window->SetFont(elem->font);
			window->DrawText(elem->text, elem->text.length(), elem_rect, DT_WORDBREAK);
		}

		// draw image:
		else if (elem->image) {

			const float FadeF = (float)fade;

			window->FadeTexture(
				elem_rect.x,
				elem_rect.y,
				elem_rect.x + elem_rect.w,
				elem_rect.y + elem_rect.h,
				elem->image,
				ScaleColor(elem->color, FadeF),
				elem->blend
			);
		}
	}
}

// +--------------------------------------------------------------------+

void
DisplayView::ExecFrame()
{
	double seconds = Game::GUITime();

	ListIter<DisplayElement> iter = elements;
	while (++iter) {
		DisplayElement* elem = iter.value();

		if (elem->fade_in > 0)
			elem->fade_in -= seconds;

		else if (elem->hold > 0)
			elem->hold -= seconds;

		else if (elem->fade_out > 0)
			elem->fade_out -= seconds;

		else
			delete iter.removeItem();
	}
}

// +--------------------------------------------------------------------+

void
DisplayView::ClearDisplay()
{
	elements.destroy();
}

// +--------------------------------------------------------------------+

void
DisplayView::AddText(const char* text,
	SystemFont* font,
	FColor        color,
	const Rect& rect,
	double       hold,
	double       fade_in,
	double       fade_out)
{
	DisplayElement* elem = new DisplayElement;

	if (fade_in == 0 && fade_out == 0 && hold == 0)
		hold = 300;

	elem->text = text;
	elem->font = font;
	elem->color = color;
	elem->rect = rect;
	elem->hold = hold;
	elem->fade_in = fade_in;
	elem->fade_out = fade_out;

	elements.append(elem);
}

void
DisplayView::AddImage(Bitmap* texture,
	FColor        color,
	int          blend,
	const Rect& rect,
	double       hold,
	double       fade_in,
	double       fade_out)
{
	DisplayElement* elem = new DisplayElement;

	if (fade_in == 0 && fade_out == 0 && hold == 0)
		hold = 300;

	elem->image = texture;
	elem->rect = rect;
	elem->color = color;
	elem->blend = blend;
	elem->hold = hold;
	elem->fade_in = fade_in;
	elem->fade_out = fade_out;

	elements.append(elem);
}
