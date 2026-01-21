/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    System
	FILE:         Game.h
	AUTHOR:       Carlos Bott
*/

#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "Color.h"
#include "Text.h"
#include "SSWGameInstance.h"

#include "Math/Color.h"

//#include "Screen.h"
//#include "Video.h"

/**
 * 
 */

 // +--------------------------------------------------------------------+

void Print(const char* fmt, ...);

class AGameDataLoader;
class Locale;
class Universe;
class Sound;
class SoundCard;
class Video;
class VideoFactory;
class VideoSettings;
class AviFile;
class Text;

class STARSHATTERWARS_API Game
{
public:
	Game();
	virtual ~Game();

	static const char* TYPENAME() { return "Game"; }

	//
	// MAIN GAME FUNCTIONALITY:
	//

	virtual bool      Init();
	virtual int       Run();
	virtual void      Exit();
	virtual bool      OnPaint() { return false; }
	virtual bool      OnHelp() { return false; }

	virtual void      Activate(bool f);
	virtual void      Pause(bool f);
	int               Status() const { return status; }


	//
	// GENERAL GAME CLASS UTILITY METHODS:
	//

	static void       Panic(const char* msg = 0);
	
	static DWORD      RealTime();
	static DWORD      GameTime();
	static DWORD      TimeCompression();
	static void       SetTimeCompression(DWORD comp);
	static DWORD      Frame();
	static FString    GetMonth(int month);
	static void       ResetGameTime();
	static void       SkipGameTime(double seconds);

	static double     FrameRate();
	static double     FrameTime();
	static double     GUITime();
	static void       SetMaxFrameLength(double seconds) { max_frame_length = seconds; }
	static void       SetMinFrameLength(double seconds) { min_frame_length = seconds; }
	static double     GetMaxFrameLength() { return max_frame_length; }
	static double     GetMinFrameLength() { return min_frame_length; }

	static void       SetMaxTexSize(int n);

	static int        MaxTexSize();
	static int        MaxTexAspect();

	static Game* GetInstance();
	
	static Color      GetScreenColor();
	static void       SetScreenColor(FColor c);
	static int        GetScreenWidth();
	static int        GetScreenHeight();

	static bool       Active() { return active; }
	static bool       Paused() { return paused; }
	static bool       Server() { return server; }
	static bool       ShowMouse() { return show_mouse; }
	static bool       IsWindowed();


	static Text       GetText(const char* key);

	static FString	  GetGameVersion();

	static const char* GetPanicMessage() { return panicbuf; }

	virtual bool      GameLoop();
	virtual void      UpdateWorld();
	virtual void      GameState();
	virtual void      UpdateScreen();
	virtual void      CollectStats();

	virtual bool      InitContent();
	virtual bool      InitGame();
	
	virtual bool      ToggleFullscreen();
	virtual bool      AdjustWindowForChange();

	virtual void      ShowStats();

	static double     max_frame_length;
	static double     min_frame_length;

	static const int TIME_NEVER = (int)1e9;
	static const int ONE_DAY = (int)24 * 3600;

protected:
	friend  bool      ProfileGameLoop(void);
	
	AGameDataLoader* content;
	Universe* world;
	VideoFactory* video_factory;
	Video* video;
	VideoSettings* video_settings;
	SoundCard* soundcard;
	
	int               gamma;
	int               max_tex_size;

	//RenderStats       stats;
	DWORD             totaltime;

	PALETTEENTRY      standard_palette[256];
	BYTE              inverse_palette[32768];

	DWORD             winstyle;

	char* app_name;
	char* title_text;
	char* palette_name;


	

	// Internal variables for the state of the app
	bool              is_windowed;
	bool              is_active;
	bool              is_device_lost;
	bool              is_minimized;
	bool              is_maximized;
	bool              ignore_size_change;
	bool              is_device_initialized;
	bool              is_device_restored;
	DWORD             window_style;        // Saved window style for mode switches
	RECT              bounds_rect;         // Saved window bounds for mode switches
	RECT              client_rect;         // Saved client area size for mode switches


	double            gui_seconds;
	double            seconds;
	double            frame_rate;
	int               frame_count;
	int               frame_count0;
	int               frame_time;
	int               frame_time0;

	int               status;
	int               exit_code;
	Color             screen_color;

	AviFile* avi_file;

	static bool       active;
	static bool       paused;
	static bool       server;
	static bool       show_mouse;
	static DWORD      base_game_time;
	static DWORD      real_time;
	static DWORD      game_time;
	static DWORD      time_comp;
	static DWORD      frame_number;

	static char       panicbuf[256];

	static FString    VersionInfo;
};

// +--------------------------------------------------------------------+
