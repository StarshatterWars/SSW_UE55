/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         DisplayView.h
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR: John DiCamillo
	ORIGINAL STUDIO: Destroyer Studios LLC

	OVERVIEW
	========
	View class for Radio Communications HUD Overlay
*/

#pragma once

#include "Types.h"
#include "View.h"
#include "SimObject.h"
#include "Color.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class UTexture2D;

class DisplayElement;
class SystemFont;

// +--------------------------------------------------------------------+

class DisplayView : public View
{
public:
	DisplayView(Window* c);
	virtual ~DisplayView();

	// Operations:
	virtual void      Refresh();
	virtual void      OnWindowMove();
	virtual void      ExecFrame();
	virtual void      ClearDisplay();

	virtual void      AddText(const char* txt,
		SystemFont* font,
		Color        color,
		const Rect& rect,
		double       hold = 1e9,
		double       fade_in = 0,
		double       fade_out = 0);

	virtual void      AddImage(UTexture2D* texture,
		Color        color,
		int          blend,
		const Rect& rect,
		double       hold = 1e9,
		double       fade_in = 0,
		double       fade_out = 0);

	static DisplayView* GetInstance();

protected:
	int     width, height;
	double  xcenter, ycenter;

	List<DisplayElement>  elements;
};
