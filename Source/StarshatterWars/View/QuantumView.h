/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         QuantumView.h
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

class Font;
class HUDView;
class Menu;
class RadioMessage;
class Ship;

class Sim;
class Window;

// +--------------------------------------------------------------------+

class QuantumView : public View, public SimObserver
{
public:
	QuantumView(Window* c);
	virtual ~QuantumView();

	// Operations:
	virtual void      Refresh();
	virtual void      OnWindowMove();
	virtual void      ExecFrame();

	virtual Menu* GetQuantumMenu(Ship* ship);
	virtual bool      IsMenuShown();
	virtual void      ShowMenu();
	virtual void      CloseMenu();

	virtual bool         Update(SimObject* obj);
	virtual const char* GetObserverName() const;

	static void       SetColor(Color c);

	static void       Initialize();
	static void       Close();

	static QuantumView* GetInstance() { return quantum_view; }

protected:
	int         width, height;
	double      xcenter, ycenter;

	Font* font;
	Sim* sim;
	Ship* ship;

	static QuantumView* quantum_view;
};
