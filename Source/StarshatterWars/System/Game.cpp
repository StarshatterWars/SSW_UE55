/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    System
	FILE:         Game.cpp
	AUTHOR:       Carlos Bott
*/



#include "Game.h"
#include "Screen.h"
#include "Video.h"
#include "GameDataLoader.h"	

Game* game = 0;

bool     Game::active = false;
bool     Game::paused = false;
bool     Game::server = false;
bool     Game::show_mouse = false;
DWORD    Game::base_game_time = 0;
DWORD    Game::real_time = 0;
DWORD    Game::game_time = 0;
DWORD    Game::time_comp = 1;
DWORD    Game::frame_number = 0;

const int    VIDEO_FPS = 30;
const double MAX_FRAME_TIME_VIDEO = 1.0 / (double)VIDEO_FPS;
const double MAX_FRAME_TIME_NORMAL = 1.0 / 5.0;
const double MIN_FRAME_TIME_NORMAL = 1.0 / 60.0;

double   Game::max_frame_length = MAX_FRAME_TIME_NORMAL;
double   Game::min_frame_length = MIN_FRAME_TIME_NORMAL;

char     Game::panicbuf[256];

static LARGE_INTEGER  perf_freq;
static LARGE_INTEGER  perf_cnt1;
static LARGE_INTEGER  perf_cnt2;

Game::Game()
{
}

Game::~Game()
{
}

bool Game::Init()
{
	return false;
}

int Game::Run()
{
	return 0;
}

void Game::Exit()
{
}

void Game::Activate(bool f)
{
}

void Game::Pause(bool f)
{
}

DWORD GetRealTime()
{
	return Game::RealTime();
}

DWORD Game::RealTime()
{
	return real_time;
}

DWORD Game::GameTime()
{
	return game_time;
}

DWORD Game::TimeCompression()
{
	return time_comp;
}

void Game::SetTimeCompression(DWORD comp)
{
	if (comp > 0 && comp <= 100)
		time_comp = comp;
}

DWORD Game::Frame()
{
	return frame_number;
}

FString Game::GetMonth(int month)
{
	switch (month) {
		case 1: 
			return "Jan";
			break;
		case 2:
			return "Feb";
			break;
		case 3:
			return "Mar";
			break;
		case 4:
			return "Apr";
			break;
		case 5:
			return "May";
			break;
		case 6:
			return "Jun";
			break;
		case 7:
			return "Jul";
			break;
		case 8:
			return "Aug";
			break;
		case 9:
			return "Sep";
			break;
		case 10:
			return "Oct";
			break;
		case 11:
			return "Nov";
			break;
		case 12:
			return "Dec";
			break;
		default:
			return "";
			break;
	}
}

void Game::ResetGameTime()
{
	game_time = 0;
}

void Game::SkipGameTime(double seconds)
{
	if (seconds > 0)
		game_time += (DWORD)(seconds * 1000);
}

double Game::FrameRate()
{
	return 0.0;
}

double Game::FrameTime()
{
	return 0.0;
}

double Game::GUITime()
{
	return 0.0;
}

void
Game::SetMaxTexSize(int n)
{
	if (game && n >= 64 && n <= 4096)
		game->max_tex_size = n;
}

int
Game::MaxTexSize()
{
	if (game && game->video) {
		int max_vid_size = game->video->MaxTexSize();
		return max_vid_size < game->max_tex_size ?
			max_vid_size : game->max_tex_size;
	}
	else if (Video::GetInstance()) {
		return Video::GetInstance()->MaxTexSize();
	}

	return 256;
}

int
Game::MaxTexAspect()
{
	if (game && game->video) {
		return game->video->MaxTexAspect();
	}
	else if (Video::GetInstance()) {
		return Video::GetInstance()->MaxTexAspect();
	}

	return 1;
}
Game* Game::GetInstance()
{
	return nullptr;
}

Color Game::GetScreenColor()
{
	return Color();
}

void Game::SetScreenColor(FColor c)
{
}

int Game::GetScreenWidth()
{
	return 0;
}

int Game::GetScreenHeight()
{
	return 0;
}

bool Game::IsWindowed()
{
	return false;
}

bool Game::GameLoop()
{
	return false;
}

void Game::UpdateWorld()
{
}

void Game::GameState()
{
}

void Game::UpdateScreen()
{
}

void Game::CollectStats()
{
}

bool Game::InitContent()
{
	return false;
}

bool Game::InitGame()
{
	return false;
}

bool Game::ToggleFullscreen()
{
	return false;
}

bool Game::AdjustWindowForChange()
{
	return false;
}

void Game::ShowStats()
{
}

void Game::Panic(const char* msg)
{
}

Text
Game::GetText(const char* key)
{
	if (game && game->content && game->content->IsContentBundleLoaded())
		return game->content->GetContentBundleText(key);

	return key;
}

FString
Game::GetGameVersion()
{
	return "5.1.87 EX";
}

Video*
Game::GetVideo()
{
	if (game)
		return game->video;

	return 0;
}
