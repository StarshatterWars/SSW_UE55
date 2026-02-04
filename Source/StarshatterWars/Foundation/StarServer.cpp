/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025–2026. All Rights Reserved.

	SUBSYSTEM:    Stars
	FILE:         StarServer.cpp
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Dedicated server application controller
*/

// --------------------------------------------------------------------
// Core includes
// --------------------------------------------------------------------

#include "StarServer.h"

#include "Campaign.h"
#include "CombatRoster.h"
#include "Galaxy.h"
#include "Mission.h"
#include "Sim.h"
#include "SimEvent.h"
#include "Ship.h"
#include "SimContact.h"
#include "QuantumDrive.h"
#include "Power.h"
#include "SystemDesign.h"
#include "WeaponDesign.h"
#include "SimShot.h"
#include "Drive.h"
#include "Explosion.h"
#include "FlightDeck.h"
#include "RadioTraffic.h"
#include "Random.h"

#include "Token.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "EventDispatch.h"
#include "MultiController.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "Resource.h"
#include "GameSTructs.h"

// --------------------------------------------------------------------
// Unreal logging bridge
// --------------------------------------------------------------------

#include "Logging/LogMacros.h"

// --------------------------------------------------------------------

StarServer* StarServer::instance = nullptr;

static Mission* current_mission = nullptr;
static bool     exit_latch = true;

extern const char* versionInfo;

// --------------------------------------------------------------------

StarServer::StarServer()
	: loader(nullptr)
	, time_mark(0)
	, minutes(0)
	, game_mode(EMODE::MENU_MODE)
{
	if (!instance)
		instance = this;

	app_name = "Starshatter Wars Server";
	title_text = "Starshatter Wars";

	Game::server = true;
	Game::show_mouse = true;

	DataLoader::Initialize();
	loader = DataLoader::GetLoader();

	// server has no media
	loader->EnableMedia(false);
}

StarServer::~StarServer()
{
	delete world;
	world = nullptr;

	Drive::Close();
	Explosion::Close();
	FlightDeck::Close();
	Campaign::Close();
	CombatRoster::Close();
	Galaxy::Close();
	RadioTraffic::Close();
	Ship::Close();
	WeaponDesign::Close();
	SystemDesign::Close();
	DataLoader::Close();
	//NetServerConfig::Close();
	//ModConfig::Close();

	instance = nullptr;
	Game::server = false;
}

// --------------------------------------------------------------------

bool
StarServer::Init(HINSTANCE hi, HINSTANCE hpi, LPSTR cmdline, int nCmdShow)
{
	if (loader)
		loader->UseFileSystem(false);

	return Game::Init();
}

// --------------------------------------------------------------------

bool
StarServer::InitGame()
{
	if (!Game::InitGame())
		return false;

	RandomInit();

	SystemDesign::Initialize("sys.def");
	WeaponDesign::Initialize("wep.def");
	Ship::Initialize();
	Galaxy::Initialize();
	CombatRoster::Initialize();
	Campaign::Initialize();

	Drive::Initialize();
	Explosion::Initialize();
	FlightDeck::Initialize();
	SimShot::Initialize();
	RadioTraffic::Initialize();

	time_mark = Game::GameTime();
	minutes = 0;

	return true;
}

// --------------------------------------------------------------------

void
StarServer::SetGameMode(EMODE m)
{
	if (game_mode == m)
		return;

	if (m == EMODE::LOAD_MODE) {
		Print("GameMode = LOAD\n");
		paused = true;
	}
	else if (m == EMODE::PLAY_MODE) {
		Print("GameMode = PLAY\n");

		if (!world) {
			CreateWorld();
			InstantiateMission();
		}

		SetTimeCompression(1);
		Pause(true);
	}
	else {
		Print("GameMode = MENU\n");
		paused = true;

		Sim* sim = static_cast<Sim*>(world);
		if (sim)
			sim->UnloadMission();
	}

	game_mode = m;
}

// --------------------------------------------------------------------

void
StarServer::SetNextMission(const char* script)
{
	//if (lobby_server)
	//	lobby_server->SetServerMission(script);
}

// --------------------------------------------------------------------

void
StarServer::CreateWorld()
{
	RadioTraffic::Initialize();

	if (!world) {
		world = new Sim(nullptr);
		Print("World created\n");
	}
}

void
StarServer::InstantiateMission()
{
	current_mission = nullptr;

	if (Campaign::GetCampaign())
		current_mission = Campaign::GetCampaign()->GetMission();

	Sim* sim = static_cast<Sim*>(world);
	if (!sim)
		return;

	sim->UnloadMission();

	if (current_mission) {
		sim->LoadMission(current_mission);
		sim->ExecMission();
		sim->SetTestMode(false);
		Print("Mission instantiated\n");
	}
	else {
		Print("WARNING: No mission selected\n");
	}
}

// --------------------------------------------------------------------

bool
StarServer::GameLoop()
{
	if (active && paused) {
		UpdateWorld();
		GameState();
	}
	else if (!active) {
		UpdateWorld();
		GameState();
		FPlatformProcess::Sleep(0.01f);
	}

	Game::GameLoop();
	return false;
}

// --------------------------------------------------------------------

void
StarServer::UpdateWorld()
{
	long   new_time = real_time;
	double delta = new_time - frame_time;

	seconds = max_frame_length;
	gui_seconds = delta * 0.001;

	if (frame_time == 0)
		gui_seconds = 0;

	time_comp = 1;

	if (delta < max_frame_length * 1000)
		seconds = delta * 0.001;

	frame_time = new_time;

	if (Galaxy* galaxy = Galaxy::GetInstance())
		galaxy->ExecFrame();

	if (Campaign* campaign = Campaign::GetCampaign())
		campaign->ExecFrame();

	if (paused) {
		if (world)
			world->ExecFrame(0);
	}
	else {
		game_time += static_cast<DWORD>(seconds * 1000);
		Drive::StartFrame();
		if (world)
			world->ExecFrame(seconds);
	}
}

// --------------------------------------------------------------------

void
StarServer::GameState()
{

	if (game_mode == EMODE::LOAD_MODE) {
		CreateWorld();
		InstantiateMission();
		SetGameMode(EMODE::PLAY_MODE);
	}
}

// --------------------------------------------------------------------

bool
StarServer::OnPaint()
{
	return true;
}

// --------------------------------------------------------------------

void
StarServer::Shutdown(bool restart)
{
	Print("Server shutdown requested (restart=%d)\n", restart ? 1 : 0);
	Exit();
}
