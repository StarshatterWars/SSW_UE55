// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Types.h"

/**
 * 
 */

 // +--------------------------------------------------------------------+

class Window;

// +--------------------------------------------------------------------+

class STARSHATTERWARS_API View
{
	friend class Window;

public:
	static const char* TYPENAME() { return "View"; }

	View(Window* c) : window(c) {}
	virtual ~View() {}

	int operator == (const View& that) const { return this == &that; }

	// Operations:
	virtual void      Refresh() {}
	virtual void      OnWindowMove() {}
	virtual void      OnShow() {}
	virtual void      OnHide() {}

	virtual void      SetWindow(Window* w) { window = w; OnWindowMove(); }
	virtual Window*   GetWindow() { return window; }

protected:
	Window* window;
};
