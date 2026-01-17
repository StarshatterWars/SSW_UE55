/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Ship.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Starship (or space/ground station) class
*/


#include "Ship.h"
//#include "ShipAI.h"
//#include "ShipCtrl.h"
#include "ShipDesign.h"
//#include "ShipKiller.h"
//#include "Shot.h"
//#include "Drone.h"
//#include "SeekerAI.h"
//#include "HardPoint.h"
//#include "Weapon.h"
//#include "WeaponGroup.h"
//#include "Shield.h"
//#include "ShieldRep.h"
//#include "Computer.h"
//#include "FlightComp.h"
//#include "Drive.h"
//#include "QuantumDrive.h"
//#include "Farcaster.h"
//#include "Thruster.h"
//#include "Power.h"
//#include "FlightDeck.h"
//#include "LandingGear.h"
//#include "Hangar.h."
//#include "Sensor.h"
//#include "Contact.h"
//#include "CombatUnit.h"
#include "Element.h"
#include "Instruction.h"
#include "RadioMessage.h"
#include "RadioHandler.h"
#include "RadioTraffic.h"
//#include "NavLight.h"
//#include "NavSystem.h"
//#include "NavAI.h"
//#include "DropShipAI.h"
//#include "Explosion.h"
//#include "MissionEvent.h"
//#include "ShipSolid.h"
//#include "Sim.h"
//#include "SimEvent.h"
#include "StarSystem.h"
//#include "TerrainRegion.h"
//#include "Terrain.h"
//#include "System.h"
//#include "Component.h"
//#include "KeyMap.h"
#include "W_RadioView.h"
//#include "AudioConfig.h"
//#include "CameraDirector.h"
//#include "HUDView.h"

//#include "RadioVox.h"

//#include "NetGame.h"
//#include "NetUtil.h"

//#include "MotionController.h"
//#include "Keyboard.h"
//#include "Joystick.h"
//#include "Bolt.h"
#include "Game.h"
//#include "Solid.h"
//#include "Shadow.h"
//#include "Skin.h"
//#include "Sprite.h"
//#include "Light.h"
//#include "Bitmap.h"
//#include "Button.h"
//#include "Sound.h"
#include "Random.h"
#include "DataLoader.h"
#include "Parser.h"
#include "Reader.h"
#include "List.h"
#include "Color.h"

// +----------------------------------------------------------------------+

// +----------------------------------------------------------------------+

static int     base_contact_id = 0;
static double  range_min = 0;
static double  range_max = 250e3;

int      UShip::control_model = 0; // standard
int      UShip::flight_model = 0; // standard
int      UShip::landing_model = 0; // standard
double   UShip::friendly_fire_level = 1; // 100%

const int HIT_NOTHING = 0;
const int HIT_HULL = 1;
const int HIT_SHIELD = 2;
const int HIT_BOTH = 3;
const int HIT_TURRET = 4;

// +----------------------------------------------------------------------+

UShip::UShip()
{
}

UShip::UShip(const char* ship_name, const char* reg_num, ShipDesign* ship_dsn, int IFF, int cmd_ai, const int* load)
{
	IFF_code = IFF;
	killer = 0;
	throttle = 0;
	augmenter = false;
	throttle_request = 0;

	shield = 0;
	shieldRep = 0;
	main_drive = 0;
	quantum_drive = 0;
	farcaster = 0;
	
	check_fire = false;
	probe = 0;
	sensor_drone = 0;
	
	primary = 0;
	secondary = 1;

	cmd_chain_index = 0;
	target = 0;
	subtarget = 0;
	
	radio_orders =  0;
	launch_point = 0;

	g_force = 0.0f;
	sensor = 0;
	navsys = 0;
	flcs = 0;
	hangar = 0;
	
	respawns = 0;
	invulnerable = false;

	thruster = 0;
	decoy = 0;
	ai_mode = 2;
	command_ai_level = cmd_ai;
	flcs_mode = FLCS_AUTO;
	
	loadout = 0;
	emcon = 3;
	old_emcon = 3;
	master_caution = false;
	cockpit = 0;
	gear = 0;
	//skin(0),
		
	auto_repair = true;
	last_repair_time = 0;
	last_eval_time = 0;
	last_beam_time = 0;
	last_bolt_time = 0;

	warp_fov = 1;
	flight_phase = LAUNCH;
	launch_time = 0;
	carrier = 0;
	dock = 0;
	ff_count = 0;

	inbound = 0;
	element = 0;
	director_info = "Init";
	combat_unit = 0;
	net_control = 0;

	track = 0;
	ntrack = 0;
	track_time = 0;

	helm_heading = 0.0f;
	helm_pitch = 0.0f;
	altitude_agl = -1.0e6f;
	transition_time = 0.0f;
	transition_type = TRANSITION_NONE;

	friendly_fire_time = 0;
	ward = 0;
	net_observer_mode = false;
	orig_elem_index = -1;

	//sim = Sim::GetSim();

	strcpy_s(name, ship_name);
	if (reg_num && *reg_num)
		strcpy_s(regnum, reg_num);
	else regnum[0] = 0;

	design = ship_dsn;

	if (!design) {
		char msg[256];
		sprintf_s(msg, "No ship design found for '%s'\n", ship_name);
		Game::Panic(msg);
	}

	obj_type = USimObject::SIM_SHIP;

	radius = design->radius;
	mass = design->mass;
	integrity = design->integrity;
	vlimit = design->vlimit;

	agility = design->agility;
	wep_mass = 0.0f;
	wep_resist = 0.0f;

	CL = design->CL;
	CD = design->CD;
	stall = design->stall;

	chase_vec = design->chase_vec;
	bridge_vec = design->bridge_vec;

	acs = design->acs;
	pcs = design->acs;

	auto_repair = design->repair_auto;

	while (!base_contact_id)
		base_contact_id = rand() % 1000;

	contact_id = base_contact_id++;
	int sys_id = 0;

	/*for (int i = 0; i < design->reactors.size(); i++) {
		PowerSource* reactor = new PowerSource(*design->reactors[i]);
		reactor->SetShip(this);
		reactor->SetID(sys_id++);
		reactors.append(reactor);
		systems.append(reactor);
	}

	for (int i = 0; i < design->drives.size(); i++) {
		Drive* drive = new Drive(*design->drives[i]);
		drive->SetShip(this);
		drive->SetID(sys_id++);

		int src_index = drive->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(drive);

		drives.append(drive);
		systems.append(drive);
	}

	if (design->quantum_drive) {
		quantum_drive = new QuantumDrive(*design->quantum_drive);
		quantum_drive->SetShip(this);
		quantum_drive->SetID(sys_id++);

		int src_index = quantum_drive->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(quantum_drive);

		quantum_drive->SetShip(this);
		systems.append(quantum_drive);
	}

	if (design->farcaster) {
		farcaster = new Farcaster(*design->farcaster);
		farcaster->SetShip(this);
		farcaster->SetID(sys_id++);

		int src_index = farcaster->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(farcaster);

		farcaster->SetShip(this);
		systems.append(farcaster);
	}

	if (design->thruster) {
		thruster = new Thruster(*design->thruster);
		thruster->SetShip(this);
		thruster->SetID(sys_id++);

		int src_index = thruster->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(thruster);

		thruster->SetShip(this);
		systems.append(thruster);
	}

	if (design->shield) {
		shield = new Shield(*design->shield);
		shield->SetShip(this);
		shield->SetID(sys_id++);

		int src_index = shield->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(shield);

		if (design->shield_model) {
			shieldRep = new ShieldRep;
			shieldRep->UseModel(design->shield_model);
		}

		systems.append(shield);
	}

	for (int i = 0; i < design->flight_decks.size(); i++) {
		FlightDeck* deck = new FlightDeck(*design->flight_decks[i]);
		deck->SetShip(this);
		deck->SetCarrier(this);
		deck->SetID(sys_id++);
		deck->SetIndex(i);

		int src_index = deck->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(deck);

		flight_decks.append(deck);
		systems.append(deck);
	}

	if (design->flight_decks.size() > 0) {
		if (!hangar) {
			hangar = new Hangar;
			hangar->SetShip(this);
		}
	}

	if (design->squadrons.size() > 0) {
		if (!hangar) {
			hangar = new Hangar;
			hangar->SetShip(this);
		}

		for (int i = 0; i < design->squadrons.size(); i++) {
			ShipSquadron* s = design->squadrons[i];
			hangar->CreateSquadron(s->name, 0, s->design, s->count, GetIFF(), 0, 0, s->avail);
		}
	}

	if (design->gear) {
		gear = new LandingGear(*design->gear);
		gear->SetShip(this);
		gear->SetID(sys_id++);

		int src_index = gear->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(gear);

		systems.append(gear);
	}

	if (design->sensor) {
		sensor = new Sensor(*design->sensor);
		sensor->SetShip(this);
		sensor->SetID(sys_id++);

		int src_index = sensor->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(sensor);

		if (IsStarship() || IsStatic() || !strncmp(design->name, "Camera", 6))
			sensor->SetMode(Sensor::CST);

		systems.append(sensor);
	}

	int wep_index = 1;

	for (int i = 0; i < design->weapons.size(); i++) {
		Weapon* gun = new Weapon(*design->weapons[i]);
		gun->SetID(sys_id++);
		gun->SetOwner(this);
		gun->SetIndex(wep_index++);

		int src_index = gun->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(gun);

		WeaponGroup* group = FindWeaponGroup(gun->Group());
		group->AddWeapon(gun);
		group->SetAbbreviation(gun->Abbreviation());

		systems.append(gun);

		if (IsDropship() && gun->GetTurret())
			gun->SetFiringOrders(Weapon::POINT_DEFENSE);
		else
			gun->SetFiringOrders(Weapon::MANUAL);
	}

	int loadout_size = design->hard_points.size();

	if (load && loadout_size > 0) {
		loadout = new int[loadout_size];

		for (int i = 0; i < loadout_size; i++) {
			int mounted_weapon = loadout[i] = load[i];

			if (mounted_weapon < 0)
				continue;

			Weapon* missile = design->hard_points[i]->CreateWeapon(mounted_weapon);

			if (missile) {
				missile->SetID(sys_id++);
				missile->SetOwner(this);
				missile->SetIndex(wep_index++);

				WeaponGroup* group = FindWeaponGroup(missile->Group());
				group->AddWeapon(missile);
				group->SetAbbreviation(missile->Abbreviation());

				systems.append(missile);
			}
		}
	}

	if (weapons.size() > 1) {
		primary = -1;
		secondary = -1;

		for (int i = 0; i < weapons.size(); i++) {
			WeaponGroup* group = weapons[i];
			if (group->IsPrimary() && primary < 0) {
				primary = i;

				// turrets on fighters are set to point defense by default,
				// this forces the primary turret back to manual control
				group->SetFiringOrders(Weapon::MANUAL);
			}

			else if (group->IsMissile() && secondary < 0) {
				secondary = i;
			}
		}

		if (primary < 0)   primary = 0;
		if (secondary < 0)   secondary = 1;

		if (weapons.size() > 4) {
			::Print("WARNING: Ship '%s' type '%s' has %d wep groups (max=4)\n",
				Name(), DesignName(), weapons.size());
		}
	}

	if (design->decoy) {
		decoy = new Weapon(*design->decoy);
		decoy->SetOwner(this);
		decoy->SetID(sys_id++);
		decoy->SetIndex(wep_index++);

		int src_index = decoy->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(decoy);

		systems.append(decoy);
	}

	for (int i = 0; i < design->navlights.size(); i++) {
		NavLight* navlight = new NavLight(*design->navlights[i]);
		navlight->SetShip(this);
		navlight->SetID(sys_id++);
		navlight->SetOffset(((DWORD)this) << 2);
		navlights.append(navlight);
		systems.append(navlight);
	}

	if (design->navsys) {
		navsys = new NavSystem(*design->navsys);
		navsys->SetShip(this);
		navsys->SetID(sys_id++);

		int src_index = navsys->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(navsys);

		systems.append(navsys);
	}

	if (design->probe) {
		probe = new Weapon(*design->probe);
		probe->SetOwner(this);
		probe->SetID(sys_id++);
		probe->SetIndex(wep_index++);

		int src_index = probe->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(probe);

		systems.append(probe);
	}

	for (int i = 0; i < design->computers.size(); i++) {
		Computer* comp = 0;

		if (design->computers[i]->Subtype() == Computer::FLIGHT) {
			flcs = new FlightComp(*design->computers[i]);

			flcs->SetShip(this);
			flcs->SetMode(flcs_mode);
			flcs->SetVelocityLimit(vlimit);

			if (thruster)
				flcs->SetTransLimit(thruster->TransXLimit(),
					thruster->TransYLimit(),
					thruster->TransZLimit());
			else
				flcs->SetTransLimit(design->trans_x,
					design->trans_y,
					design->trans_z);

			comp = flcs;
		}
		else {
			comp = new Computer(*design->computers[i]);
		}

		comp->SetShip(this);
		comp->SetID(sys_id++);
		int src_index = comp->GetSourceIndex();
		if (src_index >= 0 && src_index < reactors.size())
			reactors[src_index]->AddClient(comp);

		computers.append(comp);
		systems.append(comp);
	}

	radio_orders = new Instruction("", Point(0, 0, 0));

	// Load Detail Set:
	for (int i = 0; i < DetailSet::MAX_DETAIL; i++) {
		if (design->models[i].size() > 0) {
			Solid* solid = new ShipSolid(this);
			solid->UseModel(design->models[i].at(0));
			solid->CreateShadows(1);

			Point* offset = 0;
			Point* spin = 0;

			if (design->offsets[i].size() > 0)
				offset = new Point(*design->offsets[i].at(0));

			if (design->spin_rates.size() > 0)
				spin = new Point(*design->spin_rates.at(0));

			detail_level = detail.DefineLevel(design->feature_size[i], solid, offset, spin);
		}

		if (design->models[i].size() > 1) {
			for (int n = 1; n < design->models[i].size(); n++) {
				Solid* solid = new ShipSolid(this); //Solid;
				solid->UseModel(design->models[i].at(n));
				solid->CreateShadows(1);

				Point* offset = 0;
				Point* spin = 0;

				if (design->offsets[i].size() > n)
					offset = new Point(*design->offsets[i].at(n));

				if (design->spin_rates.size() > n)
					spin = new Point(*design->spin_rates.at(n));

				detail.AddToLevel(detail_level, solid, offset, spin);
			}
		}
	}

	// start with lowest available detail:
	detail_level = 0; // this is highest -> detail.NumLevels()-1);
	rep = detail.GetRep(detail_level);

	if (design->cockpit_model) {
		cockpit = new Solid;
		cockpit->UseModel(design->cockpit_model);
		cockpit->SetForeground(true);
	}

	if (design->main_drive >= 0 && design->main_drive < drives.size())
		main_drive = drives[design->main_drive];

	// only use light from drives:
	light = 0;

	// setup starship helm stuff:
	if (IsStarship()) {
		flcs_mode = FLCS_HELM;
	}

	// initialize the AI:
	dir = 0;
	SetControls(0);

	for (int i = 0; i < 4; i++) {
		missile_id[i] = 0;
		missile_eta[i] = 0;
		trigger[i] = false;
	}*/
}

void UShip::CheckFriendlyFire()
{
}

void UShip::SelectWeapon(int n, int w)
{
}

bool UShip::FireWeapon(int n)
{
	return false;
}

bool UShip::FireDecoy()
{
	return false;
}

void UShip::CyclePrimary()
{
}

void UShip::CycleSecondary()
{
}

Weapon* UShip::GetPrimary() const
{
	return nullptr;
}

Weapon* UShip::GetSecondary() const
{
	return nullptr;
}

Weapon* UShip::GetWeaponByIndex(int n)
{
	return nullptr;
}

WeaponGroup* UShip::GetPrimaryGroup() const
{
	return nullptr;
}

WeaponGroup* UShip::GetSecondaryGroup() const
{
	return nullptr;
}

void UShip::LockTarget(USimObject* candidate)
{
}

bool UShip::IsTracking(USimObject* tgt)
{
	return false;
}

bool UShip::GetTrigger(int i) const
{
	return false;
}

void UShip::SetEMCON(int e, bool from_net)
{
}

Contact* UShip::FindContact(USimObject* s) const
{
	return nullptr;
}

bool UShip::IsHostileTo(const USimObject* o) const
{
	return false;
}

bool UShip::Update(USimObject* obj)
{
	return false;
}

int UShip::GetMissileEta(int index) const
{
	return 0;
}

WeaponDesign* UShip::GetPrimaryDesign() const
{
	return nullptr;
}

WeaponDesign* UShip::GetSecondaryDesign() const
{
	return nullptr;
}

void UShip::CycleSubTarget(int ct)
{
}

void UShip::DropTarget()
{
}

void UShip::LockTarget(int type, bool closest, bool hostile)
{
}

void UShip::Destroy()
{
}

int UShip::ShieldStrength() const
{
	return 0;
}

int UShip::HullStrength() const
{
	return 0;
}

int UShip::HitBy(Shot* shot, Point& impact)
{
	return 0;
}

int UShip::CollidesWith(UPhysical& o)
{
	return 0;
}

void UShip::SetIFF(int iff)
{
}

Color
UShip::IFFColor(int iff)
{
	Color c;

	switch (iff) {
	case 0:  // NEUTRAL, NON-COMBAT
		c = Color(192, 192, 192);
		break;

	case 1:  // TERELLIAN ALLIANCE
		c = Color(70, 70, 220);
		break;

	case 2:  // MARAKAN HEGEMONY
		c = Color(220, 20, 20);
		break;

	case 3:  // BROTHERHOOD OF IRON
		c = Color(200, 180, 20);
		break;

	case 4:  // ZOLON EMPIRE
		c = Color(20, 200, 20);
		break;

	case 5:
		c = Color(128, 0, 128);
		break;

	case 6:
		c = Color(40, 192, 192);
		break;

	default:
		c = Color(128, 128, 128);
		break;
	}

	return c;
}

Color
UShip::MarkerColor() const
{
	return IFFColor(IFF_code);
}

Weapon* UShip::GetDecoy() const
{
	return nullptr;
}

//List<Shot>& UShip::GetActiveDecoys()
//{
	// TODO: insert return statement here
//}

void UShip::AddActiveDecoy(Drone* d)
{
}

void UShip::DoEMCON()
{
}

double UShip::PCS() const
{
	return 0.0;
}

double UShip::ACS() const
{
	return 0.0;
}

int UShip::GetSensorMode() const
{
	return 0;
}

void UShip::LaunchProbe()
{
}

double UShip::InflictDamage(double damage, Shot* shot, int hit_type, Point hull_impact)
{
	return 0.0;
}

double UShip::InflictSystemDamage(double damage, Shot* shot, Point impact)
{
	return 0.0;
}

void UShip::InflictNetDamage(double damage, Shot* shot)
{
}

void UShip::InflictNetSystemDamage(System* system, double damage, BYTE type)
{
}

void UShip::SetNetSystemStatus(System* system, int status, int power, int reactor, double avail)
{
}

int UShip::GetTextureList(List<Bitmap>& textures)
{
	return 0;
}

void UShip::SetControls(MotionController* m)
{
}

void UShip::SetNetworkControl(Director* net_ctrl)
{
}

void UShip::SetFlightPhase(OP_MODE phase)
{
}

void UShip::Activate(Scene& scene)
{
}

void UShip::Deactivate(Scene& scene)
{
}

void UShip::SelectDetail(double seconds)
{
}

void UShip::SetRegion(SimRegion* rgn)
{
}

void UShip::ExecFrame(double seconds)
{
}

void UShip::AeroFrame(double seconds)
{
}

void UShip::DockFrame(double seconds)
{
}

void UShip::LinearFrame(double seconds)
{
}

void UShip::ExecSensors(double seconds)
{
}

void UShip::ApplyHelmYaw(double y)
{
}

void UShip::ApplyHelmPitch(double p)
{
}

void UShip::ApplyPitch(double pitch_acc)
{
}

void UShip::SetTransX(double t)
{
}

void UShip::SetTransY(double t)
{
}

void UShip::SetTransZ(double t)
{
}

void UShip::SetupAgility()
{
}

int UShip::GetRespawnCount()
{
	return respawns;
}

DWORD UShip::MissionClock() const
{
	if (launch_time > 0)
		return Game::GameTime() + 1 - launch_time;

	return 0;
}

bool UShip::IsStatic() const
{
	return false;
}

bool UShip::IsRogue() const
{
	return false;
}

int UShip::GetElementIndex() const
{
	return 0;
}

void UShip::SetElement(Element* e)
{
}

Instruction* UShip::GetRadioOrders() const
{
	return nullptr;
}

void UShip::HandleRadioMessage(RadioMessage* msg)
{
}

UShip::CLASSIFICATION
UShip::GetShipClass() const
{
	return (CLASSIFICATION)design->type;
}

bool UShip::IsStarship() const
{
	return false;
}

bool UShip::IsDropship() const
{
	return false;
}

int
UShip::ClassForName(const char* name)
{
	return ShipDesign::ClassForName(name);
}

Instruction* UShip::GetNextNavPoint()
{
	return nullptr;
}

void UShip::SetNavptStatus(Instruction* n, int status)
{
}

UShip* UShip::GetController() const
{
	return nullptr;
}
