/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         RadioView.h
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
#include "Text.h"

#include "Math/Color.h"

// +--------------------------------------------------------------------+

class SystemFont;
class SimElement;
class Ship;
class RadioMessage;
class CameraView;
class HUDView;
class Menu;
class MenuItem;
class SimElement;

class Sim;
class ThreadSync;
class Window;

// +--------------------------------------------------------------------+

class RadioView : public View, public SimObserver
{
public:
	RadioView(Window* c);
	virtual ~RadioView();

	// Operations:
	virtual void      Refresh();
	virtual void      OnWindowMove();
	virtual void      ExecFrame();

	virtual Menu*	  GetRadioMenu(Ship* ship);
	virtual bool      IsMenuShown();
	virtual void      ShowMenu();
	virtual void      CloseMenu();

	static void       Message(const char* msg);
	static void       ClearMessages();

	virtual bool         Update(SimObject* obj);
	virtual const char* GetObserverName() const;

	static void       SetColor(FColor c);

	static void       Initialize();
	static void       Close();

	static RadioView* GetInstance() { return radio_view; }

protected:
	void              SendRadioMessage(Ship* ship, MenuItem* item);

	int         width, height;
	double      xcenter, ycenter;

	SystemFont* font;
	Sim* sim;
	Ship* ship;
	SimElement* dst_elem;

	enum { MAX_MSG = 6 };
	Text        msg_text[MAX_MSG];
	double      msg_time[MAX_MSG];

	static RadioView* radio_view;
	static ThreadSync sync;
};
