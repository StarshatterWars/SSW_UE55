/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         View.h
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO
	==========================
	John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Abstract View class
*/

#pragma once

#include "Types.h"

// Minimal Unreal include required for UE_LOG:
#include "Logging/LogMacros.h"
#include "Math/Vector.h"   // FVector
#include "Math/Color.h"    // FColor

static FColor hud_color = FColor::Black;
static FColor txt_color = FColor::Black;

// +--------------------------------------------------------------------+

class Window;

// +--------------------------------------------------------------------+

class View
{
	friend class Window;

public:
	static const char* TYPENAME() { return "View"; }

	View(Window* c) : window(c) {}
	virtual ~View() {}

	int operator==(const View& that) const { return this == &that; }

	// Operations:
	virtual void		Refresh() {}
	virtual void		OnWindowMove() {}
	virtual void		OnShow() {}
	virtual void		OnHide() {}

	virtual void		SetWindow(Window* w) { window = w; OnWindowMove(); }
	virtual Window*		GetWindow() { return window; }

protected:
	Window* window;
};
