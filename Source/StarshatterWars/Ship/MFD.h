/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright 2025-2026. All Rights Reserved.

	Original Author and Studio: John DiCamillo, Destroyer Studios LLC
	SUBSYSTEM:    Stars.exe
	FILE:         MFD.h
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Class for all Multi Function Displays
*/

#pragma once

#include "Types.h"
#include "Geometry.h"
#include "Color.h"
#include "HUDView.h"

// Minimal Unreal include (requested):
#include "Math/Vector.h"

// +--------------------------------------------------------------------+

class SimContact;
class Ship;
class Sprite;
class Window;
class CameraView;
class Bitmap;
class UTexture2D;

// +--------------------------------------------------------------------+

class MFD
{
public:
	enum Modes {
		MFD_MODE_OFF,
		MFD_MODE_GAME,
		MFD_MODE_SHIP,
		MFD_MODE_FOV,
		/*MFD_MODE_FWD,  MFD_MODE_MAP,*/
		MFD_MODE_HSD,
		MFD_MODE_3D
	};

	MFD(Window* c, int index);
	virtual ~MFD();

	int operator == (const MFD& that) const { return this == &that; }

	static void          Initialize();
	static void          Close();
	static void          SetColor(Color c);

	// Operations:
	virtual void         Draw();
	virtual void         DrawGameMFD();
	virtual void         DrawStatusMFD();
	virtual void         DrawSensorMFD();
	virtual void         DrawHSD();
	virtual void         Draw3D();
	virtual void         DrawSensorLabels(const char* mfd_mode);
	virtual void         DrawMap();
	virtual void         DrawGauge(int x, int y, int percent);
	virtual void         SetStatusColor(int status);

	virtual void         SetWindow(Window* w) { window = w; }
	virtual Window* GetWindow() { return window; }
	virtual void         SetRect(const Rect& r);
	virtual const Rect& GetRect() const { return rect; }
	virtual void         SetMode(int m);
	virtual int          GetMode() const { return mode; }
	virtual Sprite* GetSprite() { return sprite; }

	virtual void         SetShip(Ship* s) { ship = s; }
	virtual Ship* GetShip() { return ship; }

	virtual void         Show();
	virtual void         Hide();

	virtual void         UseCameraView(CameraView* v);
	void                 DrawMFDText(int idx, const char* txt, Rect& r, int align, int status = -1);
	void                 HideMFDText(int idx);
	void                 SetText3DColor(Color c);

	// Starshatter render assets -> Unreal texture pointer:
	// Keep API name for call-site compatibility, but type is now UTexture2D*.
	void                 SetCockpitHUDTexture(UTexture2D* tex) { cockpit_hud_texture = tex; }

	bool                 IsMouseLatched() const;

protected:
	enum { TXT_LAST = 20 };

	Window* window = nullptr;
	Rect        rect;
	int         index = 0;
	int         mode = MFD_MODE_OFF;
	int         lines = 0;
	Sprite* sprite = nullptr;
	bool        hidden = false;
	Ship* ship = nullptr;
	HUDText     mfd_text[TXT_LAST];
	CameraView* camview = nullptr;

	UTexture2D* cockpit_hud_texture = nullptr;

	int         mouse_latch = 0;
	bool        mouse_in = false;
};
