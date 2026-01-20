/*  Project STARSHATTER WARS
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR: John DiCamillo
	ORIGINAL STUDIO: Destroyer Studios

	SUBSYSTEM:    Stars.exe
	FILE:         QuitView.h
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	View class for End Mission menu
*/

#pragma once

#include "Types.h"
#include "View.h"
#include "SimObject.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class HUDView;
class Menu;
class Sim;

// +--------------------------------------------------------------------+

class QuitView : public View
{
public:
	QuitView(Window* c);
	virtual ~QuitView();

	// Operations:
	virtual void      Refresh();
	virtual void      OnWindowMove();
	virtual void      ExecFrame();

	virtual bool      CanAccept();
	virtual bool      IsMenuShown();
	virtual void      ShowMenu();
	virtual void      CloseMenu();

	static void       Initialize();
	static void       Close();

	static QuitView* GetInstance() { return quit_view; }

protected:
	int         width, height;
	int         xcenter, ycenter;
	bool        mouse_latch;

	Sim* sim;

	static QuitView* quit_view;
};

