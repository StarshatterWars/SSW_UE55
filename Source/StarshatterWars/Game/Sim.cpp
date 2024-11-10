// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "Sim.h"
#include "Sim.h"
//#include "SimEvent.h"
#include "SimObject.h"
//#include "Starshatter.h"
#include "../Space/StarSystem.h"

//#include "Contact.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Element.h"
#include "Instruction.h"
//#include "RadioTraffic.h"
//#include "Shot.h"
//#include "Drone.h"
//#include "Explosion.h"
//#include "Debris.h"
//#include "Asteroid.h"
//#include "Drive.h"
//#include "QuantumDrive.h"
//#include "Sensor.h"
//#include "NavLight.h"
//#include "Shield.h"
//#include "Weapon.h"
//#include "WeaponGroup.h"
//#include "Hangar.h"
//#include "FlightDeck.h"
//#include "Sky.h"
//#include "Grid.h"
//#include "MFD.h"
//#include "AudioConfig.h"
#include "Mission.h"
#include "MissionEvent.h"
//#include "CameraDirector.h"
//#include "MusicDirector.h"
#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
//#include "HUDView.h"
//#include "SeekerAI.h"
//#include "ShipAI.h"
//#include "Power.h"
#include "Callsign.h"
//#include "GameScreen.h"
//#include "Terrain.h"
//#include "TerrainPatch.h"

//#include "NetGame.h"
//#include "NetClientConfig.h"
//#include "NetServerConfig.h"
//#include "NetPlayer.h"
//#include "NetUtil.h"
//#include "NetData.h"

#include "../System/Game.h"
//#include "Sound.h"
//#include "Bolt.h"
//#include "Solid.h"
//#include "Sprite.h"
//#include "Light.h"
//#include "Bitmap.h"
#include "../Foundation/DataLoader.h"
#include "../Foundation/ParseUtil.h"
//#include "MouseController.h"
#include "PlayerData.h"
#include "../Foundation/Random.h"
//#include "Video.h"

const char* FormatGameTime();

// +--------------------------------------------------------------------+

// +--------------------------------------------------------------------+
static bool first_frame = true;
USim* USim::sim = 0;

SimHyper::SimHyper(UShip* o, SimRegion* r, const Point& l, int t, bool h, UShip* fc1, UShip* fc2)
{
	ship = o;
	rgn = r;
	loc = l;
	type = t;
	hyperdrive = h;
	fc_src = fc1;
	fc_dst = fc2;
}

SimSplash::SimSplash(SimRegion* r, const Point& l, double d, double n)
{
	rgn = r;
	loc = l;
	damage = d;
	range = n;
	owner_name = "Collateral Damage";
	missile = false;
}

USim::USim()
{
	ctrl = 0;
	test_mode = false;
	grid_shown = false;
	dust = 0;
	star_system = 0;
	active_region = 0;
	mission = 0;
	netgame = 0;
	start_time = 0;

	//Drive::Initialize();
	//Explosion::Initialize();
	//FlightDeck::Initialize();
	//NavLight::Initialize();
	//Shot::Initialize();
	//MFD::Initialize();
	//Asteroid::Initialize();

	if (!sim)
		sim = this;

	UE_LOG(LogTemp, Log, TEXT("Simulation Created"));

	//cam_dir = CameraDirector::GetInstance();
}

USim::USim(MotionController* c)
{
	ctrl = c;
	test_mode = false;
	grid_shown = false;
	dust = 0;
	star_system = 0;
	active_region = 0;
	mission = 0;
	netgame = 0;
	start_time = 0;

	//Drive::Initialize();
	//Explosion::Initialize();
	//FlightDeck::Initialize();
	//NavLight::Initialize();
	//Shot::Initialize();
	//MFD::Initialize();
	//Asteroid::Initialize();

	if (!sim)
		sim = this;

	//cam_dir = CameraDirector::GetInstance();
}

double
USim::MissionClock() const
{
	return (Game::GameTime() - start_time) / 1000.0;
}
