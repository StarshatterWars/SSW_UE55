/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025–2026. All Rights Reserved.

	SUBSYSTEM:    Stars
	FILE:         StarServer.h
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Main game server / application controller
*/

#pragma once

// --------------------------------------------------------------------
// Core Starshatter includes
// --------------------------------------------------------------------

#include "Types.h"
#include "Game.h"
#include "KeyMap.h"

// --------------------------------------------------------------------
// Minimal Unreal includes
// --------------------------------------------------------------------

#include "Math/Vector.h"                 // FVector
#include "Math/Color.h"                  // FColor
#include "Math/UnrealMathUtility.h"      // FMath

// --------------------------------------------------------------------
// Forward declarations (keep header light)
// --------------------------------------------------------------------

class Campaign;
class Ship;
class Sim;
class FadeView;
class CameraManager;
class MultiController;
class MouseController;
class DataLoader;

class NetServer;
class NetLobbyServer;

// +--------------------------------------------------------------------+

class StarServer : public Game
{
public:
	StarServer();
	virtual ~StarServer();

	// Game lifecycle
	virtual bool      Init(HINSTANCE hi, HINSTANCE hpi, LPSTR cmdline, int nCmdShow);
	virtual bool      InitGame();
	virtual void      GameState();
	virtual bool      OnPaint();

	// Game modes
	enum MODE {
		MENU_MODE,   // main menu
		LOAD_MODE,   // loading mission into simulator
		PLAY_MODE    // active simulation
	};

	// Accessors / mutators
	int               GetGameMode() const { return game_mode; }
	void              SetGameMode(int mode);
	void              SetNextMission(const char* script);

	// World control
	void              CreateWorld();
	void              Shutdown(bool restart = false);

	// Singleton access
	static StarServer* GetInstance() { return instance; }

protected:
	// Internal game loop hooks
	virtual bool      GameLoop();
	virtual void      UpdateWorld();
	virtual void      InstantiateMission();

protected:
	static StarServer* instance;

	NetServer* admin_server;
	NetLobbyServer* lobby_server;
	DataLoader* loader;

	int                 game_mode;
	DWORD               time_mark;
	DWORD               minutes;
};
