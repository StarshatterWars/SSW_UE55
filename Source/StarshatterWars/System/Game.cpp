/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    System
	FILE:         Game.cpp
	AUTHOR:       Carlos Bott
*/


#include "Game.h"

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
	if (game)
		return Game::RealTime();

	return timeGetTime();
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

Game* Game::GetInstance()
{
	return nullptr;
}

Color Game::GetScreenColor()
{
	return Color();
}

void Game::SetScreenColor(Color c)
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

Text Game::GetText(const char* key)
{
	return Text();
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
