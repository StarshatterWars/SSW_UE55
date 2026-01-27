/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Starshatter.h
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo / Destroyer Studios LLC
*/

#pragma once

#include "Types.h"
#include "Game.h"
#include "KeyMap.h"
#include "Text.h"
#include "MenuScreen.h"

// Forward declarations to keep header light:
class Bitmap;

class Campaign;
class UCmpnScreen;
class PlanScreen;
class ULoadScreen;
class UGameScreen;
class Ship;
class Sim;
class UFadeView;
class CameraManager;
class MultiController;
class MouseController;
class MusicManager;
class DataLoader;
class SystemFont;
class Mission;
class Window;
// +--------------------------------------------------------------------+

class Starshatter : public Game
{
public:
	Starshatter();
	virtual ~Starshatter();

	virtual bool Init();
	virtual bool InitGame();
	virtual bool ChangeVideo();
	virtual void GameState();
	virtual void Exit();
	virtual bool OnHelp();

	int       GetGameMode() { return game_mode; }
	void      SetGameMode(int mode);
	void      RequestChangeVideo();
	void      LoadVideoConfig(const char* filename);
	void      SaveVideoConfig(const char* filename);
	void      SetupSplash();
	void      SetupMenuScreen();
	void      SetupCmpnScreen();
	void      SetupPlanScreen();
	void      SetupLoadScreen();
	void      SetupGameScreen();
	void      OpenTacticalReference();
	void      CreateWorld();
	int       KeyDown(int action) const;

	void      PlayerCam(int mode);
	void      ViewSelection();

	void      MapKeys();
	static void MapKeys(KeyMap* mapping, int nkeys);
	static void MapKey(int act, int key, int alt = 0);

	void      SetTestMode(int t) { test_mode = t; }

	static Starshatter* GetInstance() { return instance; }

	int               GetScreenWidth();
	int               GetScreenHeight();

	// graphic options:
	int               LensFlare() { return lens_flare; }
	int               Corona() { return corona; }
	int               Nebula() { return nebula; }
	int               Dust() { return dust; }

	KeyMap& GetKeyMap() { return keycfg; }

	int               GetLoadProgress() { return load_progress; }
	const char* GetLoadActivity() { return load_activity; }

	void              InvalidateTextureCache();

	int               GetChatMode() const { return chat_mode; }
	void              SetChatMode(int c);
	const char* GetChatText() const { return chat_text.data(); }

	void              ExecCutscene(const char* msn_file, const char* path);
	void              BeginCutscene();
	void              EndCutscene();
	bool              InCutscene() const { return cutscene > 0; }
	Mission* GetCutsceneMission() const;
	const char* GetSubtitles() const;
	void              EndMission();

	void              StartOrResumeGame();

	static bool       UseFileSystem();

protected:
	virtual void DoMenuScreenFrame();
	virtual void DoCmpnScreenFrame();
	virtual void DoPlanScreenFrame();
	virtual void DoLoadScreenFrame();
	virtual void DoGameScreenFrame();
	virtual void DoMouseFrame();

	virtual void DoChatMode();
	virtual void DoGameKeys();

	virtual bool GameLoop();
	virtual void UpdateWorld();
	virtual void InstantiateMission();
	virtual bool ResizeVideo();
	virtual void InitMouse();

	static Starshatter* instance;
	Window*		gamewin;
	UMenuScreen* menuscreen;
	ULoadScreen* LoadScreen;
	PlanScreen* planscreen;
	UGameScreen* gamescreen;
	UCmpnScreen* cmpnscreen;

	UFadeView* splash;
	int                 splash_index;

	// Replaced Bitmap (render asset) with Unreal texture pointer:
	Bitmap* splash_image;

	MultiController* input;
	MouseController* mouse_input;;
	DataLoader* loader;

	Ship* player_ship;
	CameraManager* cam_dir;
	MusicManager* music_dir;

	SystemFont* HUDfont;
	SystemFont* GUIfont;
	SystemFont* GUI_small_font;
	SystemFont* terminal;
	SystemFont* verdana;
	SystemFont* title_font;
	SystemFont* limerick18;
	SystemFont* limerick12;
	SystemFont* ocrb;

	DWORD               time_mark;
	DWORD               minutes;

	double              field_of_view;
	double              orig_fov;

	static int          keymap[256];
	static int          keyalt[256];
	KeyMap              keycfg;

	bool                tactical;
	bool                spinning;
	int                 mouse_x;
	int                 mouse_y;
	int                 mouse_dx;
	int                 mouse_dy;

	int                 game_mode;
	int                 test_mode;
	int                 req_change_video;
	int                 video_changed;

	int                 lens_flare;
	int                 corona;
	int                 nebula;
	int                 dust;

	double              exit_time;

	int                 load_step;
	int                 load_progress;
	Text                load_activity;
	int                 catalog_index;

	int                 cutscene;
	int                 lobby_mode;
	int                 chat_mode;
	Text                chat_text;
};
