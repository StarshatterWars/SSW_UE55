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
#include "SystemFont.h"
#include "SimSystem.h"

// Minimal Unreal include required for UE_LOG:
#include "Logging/LogMacros.h"
#include "Math/Vector.h"   // FVector
#include "Math/Color.h"    // FColor

static FColor hud_color = FColor::Black;
static FColor txt_color = FColor::Black;
static bool show_menu = false;

SystemFont* hud_font = nullptr;
SystemFont* big_font = nullptr;

static bool   mouse_in = false;
static int    mouse_latch = 0;
static int    mouse_index = -1;

static int ship_status = SimSystem::NOMINAL;
static int tgt_status = SimSystem::NOMINAL;


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
