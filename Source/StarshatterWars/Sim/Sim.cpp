/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Sim.cpp
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Simulation Universe and Region classes
*/

#include "Sim.h"
#include "SimEvent.h"
#include "SimObject.h"
#include "Starshatter.h"
#include "StarSystem.h"
#include "SimContact.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "SimElement.h"
#include "Instruction.h"
#include "RadioTraffic.h"
#include "SimShot.h"
#include "Drone.h"
#include "Explosion.h"
#include "Debris.h"
#include "Asteroid.h"
#include "Drive.h"
#include "QuantumDrive.h"
#include "Sensor.h"
#include "NavLight.h"
#include "Shield.h"
#include "Weapon.h"
#include "WeaponGroup.h"
#include "Hangar.h"
#include "FlightDeck.h"
#include "Sky.h"
#include "Grid.h"
#include "MFD.h"
#include "AudioConfig.h"
#include "Mission.h"
#include "MissionEvent.h"
#include "CameraManager.h"
#include "MusicManager.h"
#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "HUDView.h"
#include "SeekerAI.h"
#include "ShipAI.h"
#include "Power.h"
#include "Callsign.h"
#include "GameScreen.h"
#include "Terrain.h"
#include "TerrainPatch.h"
#include "SimScene.h"

#include "Game.h"
#include "Sound.h"
#include "Bolt.h"
#include "Solid.h"
#include "Sprite.h"
#include "SimLight.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "MouseController.h"
#include "PlayerCharacter.h"
#include "Random.h"
#include "Video.h"
#include "Graphic.h"

// Minimal Unreal includes (logging + FVector):
#include "Logging/LogMacros.h"
#include "Math/Vector.h"
#include "Math/RandomStream.h"

// NOTE: Render assets like Bitmap are handled as Unreal assets elsewhere (e.g., UTexture2D*).
// This translation unit does notxinclude Bitmap.h.

// --------------------------------------------------------------------
// UE logging bridge (replaces legacy Print debugging)
// --------------------------------------------------------------------

DEFINE_LOG_CATEGORY_STATIC(LogStarshatterSim, Log, All);

static void SSLogf(const char* Fmt, ...)
{
	if (!Fmt || !*Fmt)
		return;

	char Buffer[4096];

	va_list Args;
	va_start(Args, Fmt);
#if defined(_MSC_VER)
	vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Fmt, Args);
#else
	vsnprintf(Buffer, sizeof(Buffer), Fmt, Args);
#endif
	va_end(Args);

	UE_LOG(LogStarshatterSim, Log, TEXT("%s"), ANSI_TO_TCHAR(Buffer));
}

#ifndef Print
#define Print SSLogf
#endif

// --------------------------------------------------------------------
// FVector helpers
// --------------------------------------------------------------------

static FORCEINLINE double VecLen(const FVector& V)
{
	return V.Size();
}

// Normalizes V in-place; returns original length (legacy Point::Normalize semantics)
static FORCEINLINE double NormalizeRetLen(FVector& V)
{
	const double Len = V.Size();
	if (Len > SMALL_NUMBER)
	{
		V /= Len;
	}
	else
	{
		V = FVector::ZeroVector;
	}
	return Len;
}

// Legacy “OtherHand” conversion point.
// If you still need a handedness flip, implement it here consistently.
// For now, identity mapping is used (expected to be reconciled in core math migration).
static FORCEINLINE FVector OtherHand(const FVector& V)
{
	return V;
}

const char* FormatGameTime();

// +--------------------------------------------------------------------+

class SimHyper
{
public:
	SimHyper(Ship* o, SimRegion* r, const FVector& l, int t, bool h, Ship* fc1, Ship* fc2)
		: ship(o), rgn(r), loc(l), type(t), hyperdrive(h), fc_src(fc1), fc_dst(fc2) {
	}

	Ship* ship;
	SimRegion* rgn;
	FVector     loc;
	int         type;
	bool        hyperdrive;
	Ship* fc_src;
	Ship* fc_dst;
};

// +--------------------------------------------------------------------+

class SimSplash
{
public:
	SimSplash(SimRegion* r, const FVector& l, double d, double n)
		: rgn(r), loc(l), damage(d), range(n),
		owner_name("Collateral Damage"), missile(false) {
	}

	Text        owner_name;
	FVector     loc;
	double      damage;
	double      range;
	SimRegion* rgn;
	bool        missile;
};

// +--------------------------------------------------------------------+

static bool first_frame = true;
Sim* Sim::sim = 0;

Sim::Sim(MotionController* c)
	: ctrl(c), test_mode(false), grid_shown(false), dust(0),
	star_system(0), active_region(0), mission(0),
	start_time(0)
{
	Drive::Initialize();
	Explosion::Initialize();
	FlightDeck::Initialize();
	NavLight::Initialize();
	SimShot::Initialize();
	MFD::Initialize();
	Asteroid::Initialize();

	if (!sim)
		sim = this;

	cam_dir = CameraManager::GetInstance();
}

Sim::~Sim()
{
	UnloadMission();

	SimShot::Close();
	FlightDeck::Close();
	NavLight::Close();
	Token::close();
	Asteroid::Close();

	if (sim == this)
		sim = 0;
}

// +--------------------------------------------------------------------+

void
Sim::CommitMission()
{
	for (int i = 0; i < regions.size(); i++)
		regions[i]->CommitMission();

	if (ShipStats::NumStats() > 0) {
		Print("\n\nFINAL SCORE '%s'\n", (const char*)mission->Name());
		Print("Name              Kill1  Kill2  Died   Colls  Points  Cmd Pts\n");
		Print("----------------  -----  -----  -----  -----  ------  ------\n");

		int tk1 = 0;
		int tk2 = 0;
		int td = 0;
		int tc = 0;

		for (int i = 0; i < ShipStats::NumStats(); i++) {
			ShipStats* s = ShipStats::GetStats(i);
			s->Summarize();

			Print("%-16s  %5d  %5d  %5d  %5d  %6d  %6d\n",
				s->GetName(),
				s->GetGunKills(),
				s->GetMissileKills(),
				s->GetDeaths(),
				s->GetColls(),
				s->GetPoints(),
				s->GetCommandPoints());

			tk1 += s->GetGunKills();
			tk2 += s->GetMissileKills();
			td += s->GetDeaths();
			tc += s->GetColls();

			CombatGroup* group = s->GetCombatGroup();

			if (group) {
				Combatant* c = group->GetCombatant();

				if (c)
					c->AddScore(s->GetPoints());

				if (s->GetElementIndex() == 1)
					group->SetSorties(group->Sorties() + 1);

				group->SetKills(group->Kills() + s->GetGunKills() + s->GetMissileKills());
				group->SetPoints(group->Points() + s->GetPoints());
			}

			if (s->IsPlayer()) {
				PlayerCharacter* p = PlayerCharacter::GetCurrentPlayer();
				p->ProcessStats(s, start_time);

				if (mission && mission->Type() == Mission::TRAINING &&
					s->GetDeaths() == 0 && s->GetColls() == 0)
					p->SetTrained(mission->Identity());

				PlayerCharacter::Save(); // save training state right now before we forget!
			}
		}

		Print("--------------------------------------------\n");
		Print("TOTAL             %5d  %5d  %5d  %5d\n\n", tk1, tk2, td, tc);

		ShipStats::Initialize();
	}
}

// +--------------------------------------------------------------------+

void
Sim::UnloadMission()
{
	HUDView* hud = HUDView::GetInstance();
	if (hud)
		hud->HideAll();

	ShipStats::Initialize();

	events.destroy();
	mission_elements.destroy();
	elements.destroy();
	finished.destroy();

	if (active_region)
		active_region->Deactivate();

	if (star_system)
		star_system->Deactivate();

	if (mission) {
		mission->SetActive(false);
		mission->SetComplete(true);
	}

	regions.destroy();
	scene.Collect();

	GRAPHIC_DESTROY(dust);

	star_system = 0;
	active_region = 0;
	mission = 0;

	// reclaim memory used by radio traffic:
	RadioTraffic::DiscardMessages();

	// release texture memory for 2D screens:
	Starshatter* stars = Starshatter::GetInstance();
	if (stars)
		stars->InvalidateTextureCache();

	cam_dir = CameraManager::GetInstance();
	if (cam_dir)
		cam_dir->SetShip(0);

	AudioConfig::SetTraining(false);
}

bool
Sim::IsActive() const
{
	return mission && mission->IsActive();
}

bool
Sim::IsComplete() const
{
	return mission && mission->IsComplete();
}

// +--------------------------------------------------------------------+

void
Sim::LoadMission(Mission* m, bool preload_textures)
{
	cam_dir = CameraManager::GetInstance();

	if (!mission) {
		mission = m;
		mission->SetActive(true);

		if (preload_textures) {
			Video* video = Game::GetVideo();
			List<Model>    all_models;
			// Textures are Unreal assets now (e.g., UTexture2D*), so legacy Bitmap preload is disabled.

			ListIter<MissionElement> elem_iter = mission->GetElements();
			while (++elem_iter) {
				MissionElement* elem = elem_iter.value();
				const ShipDesign* design = elem->GetDesign();

				if (design) {
					for (int i = 0; i < 4; i++) {
						List<Model>& models = (List<Model>&) design->models[i]; // cast-away const

						ListIter<Model> model_iter = models;
						while (++model_iter) {
							Model* model = model_iter.value();
							if (!all_models.contains(model)) {
								all_models.append(model);

								ListIter<Surface> surf_iter = model->GetSurfaces();
								while (++surf_iter) {
									Surface* surface = surf_iter.value();
									video->PreloadSurface(surface);
								}
							}
						}
					}
				}
			}
		}
	}
}

void
Sim::ExecMission()
{
	cam_dir = CameraManager::GetInstance();

	if (!mission) {
		Print("Sim::ExecMission() - No mission to execute.\n");
		return;
	}

	if (elements.size() || finished.size()) {
		Print("Sim::ExecMission(%s) mission is already executing.\n", mission->Name());
		return;
	}

	Print("\nExec Mission: '%s'\n", (const char*)mission->Name());

	if (cam_dir)
		cam_dir->Reset();

	if (mission->Stardate() > 0)
		StarSystem::SetBaseTime(mission->Stardate(), true);

	star_system = mission->GetStarSystem();
	star_system->Activate(scene);

	int dust_factor = 0;

	if (Starshatter::GetInstance())
		dust_factor = Starshatter::GetInstance()->Dust();

	if (star_system->NumDust() * dust_factor) {
		dust = new Dust(star_system->NumDust() * 2 * (dust_factor + 1), dust_factor > 1);
		scene.AddGraphic(dust);
	}

	CreateRegions();
	BuildLinks();
	CreateElements();
	CopyEvents();

	first_frame = true;
	start_time = Game::GameTime();

	AudioConfig::SetTraining(mission->Type() == Mission::TRAINING);
}

// +--------------------------------------------------------------------+

void
Sim::CreateRegions()
{
	const char* active_region_name = 0;

	if (mission)
		active_region_name = mission->GetRegion();
	else return;

	ListIter<StarSystem> iter = mission->GetSystemList();
	while (++iter) {
		StarSystem* sys = iter.value();

		// insert objects from star system:
		ListIter<OrbitalBody> star = sys->Bodies();
		while (++star) {
			ListIter<OrbitalBody> planet = star->Satellites();
			while (++planet) {
				ListIter<OrbitalBody> moon = planet->Satellites();
				while (++moon) {
					ListIter<OrbitalRegion> rgn = moon->Regions();
					while (++rgn) {
						SimRegion* sim_region = new SimRegion(this, rgn.value());
						regions.append(sim_region);
						if (!strcmp(active_region_name, sim_region->GetName())) {
							ActivateRegion(sim_region);
						}
					}
				}

				ListIter<OrbitalRegion> rgn = planet->Regions();
				while (++rgn) {
					SimRegion* sim_region = new SimRegion(this, rgn.value());
					regions.append(sim_region);
					if (!strcmp(active_region_name, sim_region->GetName())) {
						ActivateRegion(sim_region);
					}
				}
			}

			ListIter<OrbitalRegion> rgn = star->Regions();
			while (++rgn) {
				SimRegion* sim_region = new SimRegion(this, rgn.value());
				regions.append(sim_region);
				if (!strcmp(active_region_name, sim_region->GetName())) {
					ActivateRegion(sim_region);
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
Sim::BuildLinks()
{
	ListIter<SimRegion> iter = regions;
	while (++iter) {
		SimRegion* rgn = iter.value();
		OrbitalRegion* orb = rgn->GetOrbitalRegion();

		if (orb) {
			ListIter<Text> lnk_iter = orb->Links();
			while (++lnk_iter) {
				Text* t = lnk_iter.value();

				SimRegion* tgt = FindRegion(*t);

				if (tgt && !rgn->GetLinks().contains(tgt))
					rgn->GetLinks().append(tgt);
			}
		}
	}
}

void
Sim::CreateElements()
{
	ListIter<MissionElement> e_iter = mission->GetElements();
	while (++e_iter) {
		MissionElement* msn_elem = e_iter.value();

		// add element to a carrier?
		if (msn_elem->IsSquadron()) {
			Ship* carrier = FindShip(msn_elem->Carrier());
			if (carrier) {
				Hangar* hangar = carrier->GetHangar();

				if (hangar) {
					int* def_load = 0;

					if (msn_elem->Loadouts().size()) {
						MissionLoad* m = msn_elem->Loadouts().at(0);

						if (m->GetName().length()) {
							ShipDesign* dsn = (ShipDesign*)msn_elem->GetDesign();
							ListIter<ShipLoad> sl_iter = dsn->loadouts;
							while (++sl_iter) {
								ShipLoad* sl = sl_iter.value();

								if (m->GetName() == sl->name)
									def_load = sl->load;
							}
						}

						if (!def_load) {
							def_load = m->GetStations();
						}
					}

					hangar->CreateSquadron(msn_elem->Name(), msn_elem->GetCombatGroup(),
						msn_elem->GetDesign(), msn_elem->Count(),
						msn_elem->GetIFF(),
						def_load, msn_elem->MaintCount(), msn_elem->DeadCount());

					SimElement* element = CreateElement(msn_elem->Name(),
						msn_elem->GetIFF(),
						msn_elem->MissionRole());

					element->SetCarrier(carrier);
					element->SetCombatGroup(msn_elem->GetCombatGroup());
					element->SetCombatUnit(msn_elem->GetCombatUnit());
					element->SetCount(msn_elem->Count());
					element->SetRogue(false);
					element->SetPlayable(false);
					element->SetLoadout(def_load);
				}
			}
		}

		// create the element in space:
		else {
			Ship* carrier = 0;
			Hangar* hangar = 0;
			int         squadron = -1;
			int         slot = 0;

			// first create the package element:
			SimElement* element = CreateElement(msn_elem->Name(),
				msn_elem->GetIFF(),
				msn_elem->MissionRole());

			element->SetPlayer(msn_elem->Player());
			element->SetCombatGroup(msn_elem->GetCombatGroup());
			element->SetCombatUnit(msn_elem->GetCombatUnit());
			element->SetCommandAILevel(msn_elem->CommandAI());
			element->SetHoldTime(msn_elem->HoldTime());
			element->SetZoneLock(msn_elem->ZoneLock() ? true : false);
			element->SetRogue(msn_elem->IsRogue());
			element->SetPlayable(msn_elem->IsPlayable());
			element->SetIntelLevel(msn_elem->IntelLevel());

			// if this is the player's element, make sure to activate the region:
			if (msn_elem->Player()) {
				SimRegion* rgn = FindRegion(msn_elem->Region());

				if (rgn && rgn != active_region)
					ActivateRegion(rgn);
			}

			// if element belongs to a squadron,
			// find the carrier, squadron, flight deck, etc.:
			if (msn_elem->Squadron().length() > 0) {
				MissionElement* squadron_elem = mission->FindElement(msn_elem->Squadron());

				if (squadron_elem) {
					element->SetSquadron(msn_elem->Squadron());

					SimElement* cmdr = FindElement(squadron_elem->Carrier());

					if (cmdr) {
						element->SetCommander(cmdr);
						carrier = cmdr->GetShip(1);

						if (carrier) {
							element->SetCarrier(carrier);
							hangar = carrier->GetHangar();

							for (int s = 0; s < hangar->NumSquadrons(); s++) {
								if (hangar->SquadronName(s) == msn_elem->Squadron()) {
									squadron = s;
									break;
								}
							}
						}
					}
				}
			}

			else if (msn_elem->Commander().length() > 0) {
				SimElement* cmdr = FindElement(msn_elem->Commander());

				if (cmdr) {
					element->SetCommander(cmdr);
				}
			}

			ListIter<Instruction> obj = msn_elem->Objectives();
			while (++obj) {
				Instruction* o = obj.value();
				Instruction* instr = 0;

				instr = new Instruction(*o);

				element->AddObjective(instr);
			}

			if (msn_elem->Instructions().size() > 0) {
				ListIter<Text> instr = msn_elem->Instructions();
				while (++instr) {
					element->AddInstruction(*instr);
				}
			}

			ListIter<Instruction> nav = msn_elem->NavList();
			while (++nav) {
				SimRegion* rgn = FindRegion(nav->RegionName());

				if (!rgn)
					rgn = FindRegion(msn_elem->Region());

				if (rgn) {
					Instruction* npt = new
						Instruction(rgn, OtherHand(nav->Location()), nav->Action());

					npt->SetStatus(nav->Status());
					npt->SetEMCON(nav->EMCON());
					npt->SetFormation(nav->Formation());
					npt->SetSpeed(nav->Speed());
					npt->SetTarget(nav->TargetName());
					npt->SetHoldTime(nav->HoldTime());
					npt->SetFarcast(nav->Farcast());

					element->AddNavPoint(npt);
				}
			}

			bool  alertPrep = false;
			int* loadout = 0;
			int   respawns = msn_elem->RespawnCount();

			// if ships are to start on alert,
			// spot them onto the appropriate launch deck:
			if (hangar && element && msn_elem->Count() > 0 && msn_elem->IsAlert()) {
				FlightDeck* deck = 0;
				int               queue = 1000;
				const ShipDesign* dsn = msn_elem->GetDesign();

				if (dsn) {
					for (int i = 0; i < carrier->NumFlightDecks(); i++) {
						FlightDeck* d = carrier->GetFlightDeck(i);
						int         dq = hangar->PreflightQueue(d);

						if (d && d->IsLaunchDeck() && d->SpaceLeft(dsn->type) && dq < queue) {
							queue = dq;
							deck = d;
						}
					}
				}

				if (deck) {
					alertPrep = true;

					// choose best loadout:
					if (msn_elem->Loadouts().size()) {
						MissionLoad* l = msn_elem->Loadouts().at(0);
						if (l->GetName().length()) {
							ListIter<ShipLoad> sl = ((ShipDesign*)dsn)->loadouts;
							while (++sl) {
								if (!_stricmp(sl->name, l->GetName()))
									loadout = sl->load;
							}
						}

						else {
							loadout = l->GetStations();
						}
					}

					element->SetLoadout(loadout);

					for (int i = 0; i < msn_elem->Count(); i++) {
						int   squadron = -1;
						int   slot = -1;

						if (hangar->FindAvailSlot(msn_elem->GetDesign(), squadron, slot)) {
							alertPrep = alertPrep &&
								hangar->GotoAlert(squadron,
									slot,
									deck,
									element,
									loadout,
									true,    // package for launch
									true);   // expedite

							HangarSlot* s = (HangarSlot*)hangar->GetSlot(squadron, slot);
							Ship* alertShip = hangar->GetShip(s);

							if (alertShip) {
								alertShip->SetRespawnCount(respawns);

								if (msn_elem->Player() == i + 1) {
									if (alertShip->GetRegion()) {
										alertShip->GetRegion()->SetPlayerShip(alertShip);
									}
									else {
										Print("WARNING: alert ship '%s' region is null\n", alertShip->Name());
									}
								}
							}
						}
					}
				}
			}

			if (!alertPrep) {
				// then, create the ships:
				for (int i = 0; i < msn_elem->Count(); i++) {
					MissionShip* msn_ship = 0;
					Text           sname = msn_elem->GetShipName(i);
					Text           rnum = msn_elem->GetRegistry(i);
					Text           rgn_name = msn_elem->Region();

					if (msn_elem->Ships().size() > i) {
						msn_ship = msn_elem->Ships()[i];
						sname = msn_ship->Name();
						rnum = msn_ship->RegNum();
						rgn_name = msn_ship->Region();
					}

					FVector l2 = OtherHand(msn_elem->Location());

					if (msn_ship && fabs(msn_ship->Location().X) < 1e9) {
						l2 = OtherHand(msn_ship->Location());
					}
					else if (i) {
						FVector offset = OtherHand(FVector(
							FMath::FRandRange(-1.f, 1.f),
							FMath::FRandRange(-1.f, 1.f),
							0.f
						));

						offset.Z = FMath::FRandRange(-1000.f, 1000.f);

						if (msn_elem->Count() < 5)
							offset *= 0.3;

						l2 += offset;
					}

					// choose best loadout:
					ListIter<MissionLoad> l = msn_elem->Loadouts();
					while (++l) {
						if ((l->GetShip() == i) || (l->GetShip() < 0 && loadout == 0)) {
							if (l->GetName().length()) {
								ListIter<ShipLoad> sl = ((ShipDesign*)msn_elem->GetDesign())->loadouts;
								while (++sl) {
									if (!_stricmp(sl->name, l->GetName()))
										loadout = sl->load;
								}
							}

							else {
								loadout = l->GetStations();
							}
						}
					}

					element->SetLoadout(loadout);

					Ship* ship = CreateShip(sname, rnum,
						(ShipDesign*)msn_elem->GetDesign(),
						rgn_name, l2,
						msn_elem->GetIFF(),
						msn_elem->CommandAI(),
						loadout);

					if (ship) {
						double      heading = msn_elem->Heading();
						const Skin* skin = msn_elem->GetSkin();

						if (msn_ship) {
							heading = msn_ship->Heading();

							if (msn_ship->GetSkin())
								skin = msn_ship->GetSkin();
						}

						ship->SetRogue(msn_elem->IsRogue());
						ship->SetInvulnerable(msn_elem->IsInvulnerable());
						ship->SetHeading(0, 0, heading + PI);
						ship->SetRespawnCount(respawns);
						ship->UseSkin(skin);

						FRandomStream Stream(SeedValue); // stable per mission/campaign
						ship->SetRespawnLoc(OtherHand(Stream.VRand()) * 2.f);

						if (ship->IsStarship())
							ship->SetHelmHeading(heading);

						else if (ship->IsAirborne() && ship->AltitudeAGL() > 25)
							ship->SetVelocity(OtherHand(ship->Heading()) * 250);

						if (element)
							element->AddShip(ship);

						if (hangar)
							hangar->FindSlot(ship, squadron, slot, Hangar::ACTIVE);

						if (ship->GetRegion() && msn_elem->Player() == i + 1)
							ship->GetRegion()->SetPlayerShip(ship);

						if (ship->NumFlightDecks()) {
							for (int ifd = 0; ifd < ship->NumFlightDecks(); ifd++) {
								FlightDeck* deck = ship->GetFlightDeck(ifd);
								if (deck)
									deck->Orient(ship);
							}
						}

						if (msn_ship) {
							ship->SetVelocity(OtherHand(msn_ship->Velocity()));
							ship->SetIntegrity((float)msn_ship->Integrity());
							ship->SetRespawnCount(msn_ship->Respawns());

							if (msn_ship->Ammo()[0] > -10) {
								for (int i = 0; i < 64; i++) {
									Weapon* w = ship->GetWeaponByIndex(i + 1);
									if (w)
										w->SetAmmo(msn_ship->Ammo()[i]);
									else
										break;
								}
							}

							if (msn_ship->Fuel()[0] > -10) {
								for (int i = 0; i < 4; i++) {
									if (ship->Reactors().size() > i) {
										PowerSource* p = ship->Reactors()[i];
										p->SetCapacity(msn_ship->Fuel()[i]);
									}
								}
							}

							if (msn_ship->Decoys() > -10) {
								Weapon* w = ship->GetDecoy();
								if (w)
									w->SetAmmo(msn_ship->Decoys());
							}

							if (msn_ship->Probes() > -10) {
								Weapon* w = ship->GetProbeLauncher();
								if (w)
									w->SetAmmo(msn_ship->Probes());
							}
						}

						Shield* shield = ship->GetShield();

						if (shield) {
							shield->SetPowerLevel(50);
						}

						if (ship->Class() > Ship::FRIGATE) {
							ListIter<WeaponGroup> iter = ship->Weapons();
							while (++iter) {
								WeaponGroup* weapon = iter.value();

								// anti-air weapon?
								if (weapon->GetDesign()->target_type & Ship::DRONE) {
									weapon->SetFiringOrders(Weapon::POINT_DEFENSE);
								}
								else {
									weapon->SetFiringOrders(Weapon::MANUAL);
								}
							}
						}

						if (ship->Class() > Ship::DRONE && ship->Class() < Ship::STATION) {
							ShipStats* stats = ShipStats::Find(sname);
							if (stats) {
								char design[64];
								sprintf_s(design, "%s %s", ship->Abbreviation(), ship->Design()->display_name);
								stats->SetType(design);
								stats->SetShipClass(ship->Class());
								stats->SetRole(Mission::RoleName(msn_elem->MissionRole()));
								stats->SetIFF(ship->GetIFF());
								stats->SetRegion(msn_elem->Region());
								stats->SetCombatGroup(msn_elem->GetCombatGroup());
								stats->SetCombatUnit(msn_elem->GetCombatUnit());
								stats->SetPlayer(msn_elem->Player() == i + 1);
								stats->SetElementIndex(ship->GetElementIndex());
							}
						}
					}  // ship
				}     // count
			}
		}
	}
}

void
Sim::CopyEvents()
{
	events.destroy();

	if (mission) {
		ListIter<MissionEvent> iter = mission->GetEvents();
		while (++iter) {
			MissionEvent* orig = iter.value();
			MissionEvent* event = new MissionEvent(*orig);
			events.append(event);
		}
	}
}

// +--------------------------------------------------------------------+

const char*
Sim::FindAvailCallsign(int IFF)
{
	const char* call = "Unidentified";

	for (int i = 0; i < 32; i++) {
		call = Callsign::GetCallsign(IFF);

		if (!FindElement(call))
			break;
	}

	return call;
}

SimElement*
Sim::CreateElement(const char* callsign, int IFF, int type)
{
	SimElement* elem = new SimElement(callsign, IFF, type);
	elements.append(elem);
	return elem;
}

void
Sim::DestroyElement(SimElement* elem)
{
	if (elements.contains(elem))
		elements.remove(elem);

	delete elem;
}

SimElement*
Sim::FindElement(const char* name)
{
	ListIter<SimElement> iter = elements;

	while (++iter) {
		SimElement* elem = iter.value();
		Text     ename = elem->Name();

		if (ename == name)
			return elem;
	}

	return 0;
}

// +--------------------------------------------------------------------+

int
Sim::GetAssignedElements(SimElement* elem, List<SimElement>& assigned)
{
	assigned.clear();

	if (elem) {
		for (int i = 0; i < elements.size(); i++) {
			SimElement* e = elements.at(i);
			if (!e->IsSquadron() && e->GetAssignment() == elem)
				assigned.append(e);
		}
	}

	return assigned.size();
}

// +--------------------------------------------------------------------+

Ship*
Sim::CreateShip(const char* name, const char* reg_num, ShipDesign* design, const char* rgn_name, const FVector& loc, int IFF, int cmd_ai, const int* loadout)
{
	if (!design) {
		Print("WARNING: CreateShip(%s): invalid design\n", name);
		return 0;
	}

	SimRegion* rgn = FindRegion(rgn_name);

	if (!rgn) {
		return 0;
	}

	Ship* ship = new Ship(name, reg_num, design, IFF, cmd_ai, loadout);
	ship->MoveTo(OtherHand(loc));

	if (rgn) {
		Print("Inserting Ship(%s) into Region(%s) (%s)\n", ship->Name(), rgn->GetName(), FormatGameTime());
		rgn->InsertObject(ship);

		if (ship->IsAirborne() && ship->AltitudeAGL() > 25)
			ship->SetVelocity(OtherHand(ship->Heading()) * 250);
	}

	return ship;
}

Ship*
Sim::FindShip(const char* name, const char* rgn_name)
{
	Ship* ship = 0;

	if (rgn_name) {
		SimRegion* rgn = FindRegion(rgn_name);
		if (rgn)
			ship = rgn->FindShip(name);
	}

	if (!ship) {
		ListIter<SimRegion> rgn = regions;
		while (++rgn && !ship)
			ship = rgn->FindShip(name);
	}

	return ship;
}

void
Sim::DestroyShip(Ship* ship)
{
	SimRegion* rgn = ship->GetRegion();
	if (rgn)
		rgn->DestroyShip(ship);
}

void
Sim::NetDockShip(Ship* ship, Ship* carrier, FlightDeck* deck)
{
	SimRegion* rgn = ship->GetRegion();
	if (rgn)
		rgn->NetDockShip(ship, carrier, deck);
}

Ship*
Sim::FindShipByObjID(DWORD objid)
{
	Ship* ship = 0;

	ListIter<SimRegion> rgn = regions;
	while (++rgn && !ship)
		ship = rgn->FindShipByObjID(objid);

	return ship;
}

SimShot*
Sim::FindShotByObjID(DWORD objid)
{
	SimShot* shot = 0;

	ListIter<SimRegion> rgn = regions;
	while (++rgn && !shot)
		shot = rgn->FindShotByObjID(objid);

	return shot;
}

// +--------------------------------------------------------------------+

Orbital*
Sim::FindOrbitalBody(const char* name)
{
	Orbital* body = 0;

	if (mission) {
		ListIter<StarSystem> iter = mission->GetSystemList();
		while (++iter && !body) {
			StarSystem* sys = iter.value();
			body = sys->FindOrbital(name);
		}
	}

	return body;
}


// +--------------------------------------------------------------------+

SimShot*
Sim::CreateShot(const FVector& pos, const Camera& shot_cam, WeaponDesign* design, const Ship* ship, SimRegion* rgn)
{
	SimShot* shot = 0;

	if (design->drone)
		shot = new Drone(pos, shot_cam, design, ship);
	else
		shot = new SimShot(pos, shot_cam, design, ship);

	if (rgn)
		rgn->InsertObject(shot);

	else if (active_region)
		active_region->InsertObject(shot);

	return shot;
}

// +--------------------------------------------------------------------+

Explosion*
Sim::CreateExplosion(const FVector& pos, const FVector& vel, int type, float exp_scale, float part_scale, SimRegion* rgn, SimObject* source, SimSystem* sys)
{
	// don't bother creating explosions that can't be seen:
	if (!rgn || !active_region || rgn != active_region)
		return 0;

	Explosion* exp = new Explosion(type, pos, vel, exp_scale, part_scale, rgn, source);

	if (rgn)
		rgn->InsertObject(exp);

	else if (active_region)
		active_region->InsertObject(exp);

	return exp;
}

// +--------------------------------------------------------------------+

Debris*
Sim::CreateDebris(const FVector& pos, const FVector& vel, Model* model, double mass, SimRegion* rgn)
{
	Debris* debris = new Debris(model, pos, vel, mass);

	if (rgn)
		rgn->InsertObject(debris);

	else if (active_region)
		active_region->InsertObject(debris);

	return debris;
}

// +--------------------------------------------------------------------+

Asteroid*
Sim::CreateAsteroid(const FVector& pos, int t, double mass, SimRegion* rgn)
{
	Asteroid* asteroid = new Asteroid(t, pos, mass);

	if (rgn)
		rgn->InsertObject(asteroid);

	else if (active_region)
		active_region->InsertObject(asteroid);

	return asteroid;
}

// +--------------------------------------------------------------------+

void
Sim::CreateSplashDamage(Ship* ship)
{
	if (ship && ship->GetRegion() && ship->Design()->splash_radius > 1) {
		SimSplash* splash = new
			SimSplash(ship->GetRegion(),
				OtherHand(ship->Location()),
				ship->Design()->integrity / 4,
				ship->Design()->splash_radius);

		splash->owner_name = ship->Name();
		splashlist.append(splash);
	}
}

// +--------------------------------------------------------------------+

void
Sim::CreateSplashDamage(SimShot* shot)
{
	if (shot && shot->GetRegion()) {
		double damage = shot->Damage();
		if (damage < shot->Design()->damage)
			damage = shot->Design()->damage;

		SimSplash* splash = new
			SimSplash(shot->GetRegion(),
				OtherHand(shot->Location()),
				damage,
				shot->Design()->lethal_radius);

		if (shot->Owner())
			splash->owner_name = shot->Owner()->Name();

		splash->missile = shot->IsMissile();

		splashlist.append(splash);
		CreateExplosion(OtherHand(shot->Location()), FVector::ZeroVector, Explosion::SHOT_BLAST, 20.0f, 1.0f, shot->GetRegion());
	}
}

// +--------------------------------------------------------------------+

void
Sim::ShowGrid(int show)
{
	PlayerCharacter* player = PlayerCharacter::GetCurrentPlayer();

	if (player && player->GridMode() == 0) {
		show = 0;
		grid_shown = false;
	}

	ListIter<SimRegion> rgn = regions;
	while (++rgn) {
		rgn->ShowGrid(show);
	}

	grid_shown = show ? true : false;
}

bool
Sim::GridShown() const
{
	return grid_shown;
}

// +--------------------------------------------------------------------+

List<StarSystem>&
Sim::GetSystemList()
{
	if (mission)
		return mission->GetSystemList();

	static List<StarSystem> dummy_system_list;
	return dummy_system_list;
}

// +--------------------------------------------------------------------+

void
Sim::NextView()
{
	if (active_region)
		active_region->NextView();
}

Ship*
Sim::GetPlayerShip()
{
	if (active_region)
		return active_region->GetPlayerShip();

	Starshatter* stars = Starshatter::GetInstance();
	if (stars && stars->InCutscene()) {
		Ship* player = 0;

		ListIter<SimRegion> rgn = regions;
		while (++rgn && !player) {
			player = rgn->GetPlayerShip();
		}

		return player;
	}

	return 0;
}

SimElement*
Sim::GetPlayerElement()
{
	SimElement* elem = 0;

	for (int i = 0; i < elements.size(); i++) {
		SimElement* e = elements[i];

		if (e->Player() > 0)
			elem = e;
	}

	return elem;
}

bool
Sim::IsSelected(Ship* s)
{
	if (active_region)
		return active_region->IsSelected(s);

	return false;
}

ListIter<Ship>
Sim::GetSelection()
{
	if (active_region)
		return active_region->GetSelection();

	static List<Ship> empty;
	return empty;
}

void
Sim::ClearSelection()
{
	if (active_region)
		active_region->ClearSelection();
}

void
Sim::AddSelection(Ship* s)
{
	if (active_region)
		active_region->AddSelection(s);
}

void
Sim::SetSelection(Ship* newsel)
{
	if (active_region)
		active_region->SetSelection(newsel);
}

// +--------------------------------------------------------------------+

void
Sim::SetTestMode(bool t)
{
	test_mode = t;
	Ship* pship = GetPlayerShip();

	if (pship)
		if (IsTestMode())
			pship->SetControls(0);
		else
			pship->SetControls(ctrl);
}

// +--------------------------------------------------------------------+

SimRegion*
Sim::FindRegion(const char* name)
{
	ListIter<SimRegion> rgn = regions;
	while (++rgn)
		if (rgn->name == name)
			return rgn.value();

	return 0;
}

SimRegion*
Sim::FindRegion(OrbitalRegion* orgn)
{
	ListIter<SimRegion> rgn = regions;
	while (++rgn)
		if (rgn->orbital_region == orgn)
			return rgn.value();

	return 0;
}

// +--------------------------------------------------------------------+

SimRegion*
Sim::FindNearestSpaceRegion(SimObject* object)
{
	return FindNearestRegion(object, REAL_SPACE);
}

SimRegion*
Sim::FindNearestTerrainRegion(SimObject* object)
{
	return FindNearestRegion(object, AIR_SPACE);
}

SimRegion*
Sim::FindNearestRegion(SimObject* object, int type)
{
	if (!object) return 0;

	SimRegion* result = 0;
	double      distance = 1.0e40;
	FVector     objloc = OtherHand(object->Location());

	if (object->GetRegion())
		objloc += OtherHand(object->GetRegion()->Location());

	ListIter<SimRegion> rgn = regions;
	while (++rgn) {
		if (rgn->GetType() == type) {
			OrbitalRegion* orgn = rgn->GetOrbitalRegion();
			if (orgn) {
				const double test = FMath::Abs(VecLen(OtherHand(orgn->Location()) - objloc));
				if (test < distance) {
					result = rgn.value();
					distance = test;
				}
			}
		}
	}

	return result;
}



// +--------------------------------------------------------------------+

bool
Sim::ActivateRegion(SimRegion* rgn)
{
	if (rgn && active_region != rgn && regions.contains(rgn)) {
		if (active_region)
			active_region->Deactivate();

		if (!active_region || active_region->GetSystem() != rgn->GetSystem()) {
			if (active_region)
				active_region->GetSystem()->Deactivate();
			rgn->GetSystem()->Activate(scene);
		}

		active_region = rgn;
		star_system = active_region->GetSystem();

		if (star_system) {
			star_system->SetActiveRegion(active_region->orbital_region);
		}
		else {
			Print("WARNING: Sim::ActivateRegion() No star system found for rgn '%s'", rgn->GetName());
		}

		active_region->Activate();
		return true;
	}

	return false;
}

// +--------------------------------------------------------------------+

void
Sim::RequestHyperJump(Ship* obj, SimRegion* rgn, const FVector& loc,
	int type, Ship* fc1, Ship* fc2)
{
	bool hyperdrive = false;

	if (obj->GetQuantumDrive() && obj->GetQuantumDrive()->Subtype() == QuantumDrive::HYPER)
		hyperdrive = true;

	jumplist.append(new SimHyper(obj, rgn, loc, type, hyperdrive, fc1, fc2));
}

// +--------------------------------------------------------------------+

void
Sim::ExecFrame(double seconds)
{
	if (first_frame) {
		first_frame = false;
		//netgame = NetGame::Create();
	}

	//if (netgame)
	//	netgame->ExecFrame();

	if (regions.isEmpty()) {
		active_region = 0;
		rgn_queue.clear();
		jumplist.destroy();
		scene.Collect();
		return;
	}

	ListIter<SimElement> elem = elements;
	while (++elem)
		if (!elem->IsSquadron())
			elem->ExecFrame(seconds);

	ListIter<SimRegion> rgn = regions;
	while (++rgn)
		if (rgn.value() != active_region && rgn->GetNumShips() && !rgn_queue.contains(rgn.value()))
			rgn_queue.append(rgn.value());

	// execframe for one inactive sim region:
	if (rgn_queue.size()) {
		SimRegion* exec_rgn = rgn_queue.removeIndex(0);

		while (exec_rgn && (exec_rgn->GetNumShips() == 0 || exec_rgn == active_region))
			if (rgn_queue.size())
				exec_rgn = rgn_queue.removeIndex(0);
			else
				exec_rgn = 0;

		if (exec_rgn)
			exec_rgn->ExecFrame(seconds);
	}

	if (active_region)
		active_region->ExecFrame(seconds);

	ExecEvents(seconds);
	ResolveHyperList();
	ResolveSplashList();

	// GC all the dead objects:
	scene.Collect();

	if (!IsTestMode()) {
		ListIter<SimElement> e_iter = elements;
		while (++e_iter) {
			SimElement* elem = e_iter.value();
			if (!elem->IsSquadron() && elem->IsFinished()) {
				finished.append(e_iter.removeItem());
			}
		}
	}

	// setup music
	if (!MusicManager::IsNoMusic()) {
		Starshatter* stars = Starshatter::GetInstance();
		if (stars && stars->GetGameMode() == Starshatter::PLAY_MODE) {
			Ship* player_ship = GetPlayerShip();
			if (player_ship) {
				int phase = player_ship->GetFlightPhase();

				if (phase < Ship::ACTIVE) {
					MusicManager::SetMode(MusicManager::LAUNCH);
				}

				else if (phase > Ship::ACTIVE) {
					MusicManager::SetMode(MusicManager::RECOVERY);
				}

				else {
					if (player_ship->IsInCombat()) {
						MusicManager::SetMode(MusicManager::COMBAT);
					}

					else {
						MusicManager::SetMode(MusicManager::FLIGHT);
					}
				}
			}
		}
	}
}

void
Sim::ExecEvents(double seconds)
{
	ListIter<MissionEvent> iter = events;
	while (++iter) {
		MissionEvent* event = iter.value();
		event->ExecFrame(seconds);
	}
}

void
Sim::ResolveHyperList()
{
	// resolve the hyper space transitions:
	if (jumplist.size()) {
		Ship* pship = GetPlayerShip();

		ListIter<SimHyper> j_iter = jumplist;
		while (++j_iter) {
			SimHyper* jump = j_iter.value();
			Ship* jumpship = jump->ship;

			if (jumpship) {
				SimRegion* dest = jump->rgn;

				if (!dest)
					dest = FindNearestSpaceRegion(jumpship);

				if (dest) {
					// bring along fighters on deck:
					ListIter<FlightDeck> deck = jumpship->FlightDecks();
					while (++deck) {
						for (int i = 0; i < deck->NumSlots(); i++) {
							Ship* s = deck->GetShip(i);

							if (s) {
								dest->InsertObject(s);
								s->ClearTrack();
							}
						}
					}

					if (jump->type == 0 && !jump->hyperdrive) {
						// bring along nearby ships:
						// have to do it in two parts, because inserting the ships
						// into the destination corrupts the iter over the current
						// region's list of ships...

						// part one: gather the ships that will be jumping:
						List<Ship> riders;
						ListIter<Ship> neighbor = jumpship->GetRegion()->GetShips();
						while (++neighbor) {
							if (neighbor->IsDropship()) {
								Ship* s = neighbor.value();
								if (s == jumpship) continue;

								const FVector Delta = s->Location() - jumpship->Location();

								if (Delta.Size() < 5e3) {
									riders.append(s);
								}
							}
						}

						// part two: now transfer the list to the destination:
						for (int i = 0; i < riders.size(); i++) {
							Ship* s = riders[i];
							const FVector Delta = s->Location() - jumpship->Location();

							dest->InsertObject(s);
							s->MoveTo(OtherHand(jump->loc) + Delta);
							s->ClearTrack();

							if (jump->fc_dst) {
								const double r = jump->fc_dst->Roll();
								const double p = jump->fc_dst->Pitch();
								const double w = jump->fc_dst->Yaw();

								s->SetAbsoluteOrientation(r, p, w);
								s->SetVelocity(jump->fc_dst->Heading() * 500.0);
							}

							ProcessEventTrigger(MissionEvent::TRIGGER_JUMP, 0, s->Name());
						}
					}

					// now it is safe to move the main jump ship:
					dest->InsertObject(jumpship);
					jumpship->MoveTo(OtherHand(jump->loc));
					jumpship->ClearTrack();

					ProcessEventTrigger(MissionEvent::TRIGGER_JUMP, 0, jumpship->Name());
					//NetUtil::SendObjHyper(jumpship, dest->Name(), jump->loc, jump->fc_src, jump->fc_dst, jump->type);

					// if using farcaster:
					if (jump->fc_src) {
						Print("Ship '%s' farcast to '%s'\n", jumpship->Name(), dest->GetName());
						CreateExplosion(jumpship->Location(), FVector::ZeroVector, Explosion::QUANTUM_FLASH, 1.0f, 0.0f, dest);

						if (jump->fc_dst) {
							const double r = jump->fc_dst->Roll();
							const double p = jump->fc_dst->Pitch();
							const double w = jump->fc_dst->Yaw();

							jumpship->SetAbsoluteOrientation(r, p, w);
							jumpship->SetVelocity(jump->fc_dst->Heading() * 500.0);
						}

						jumpship->SetHelmHeading(jumpship->CompassHeading());
						jumpship->SetHelmPitch(0);
					}

					// break orbit:
					else if (jump->type == Ship::TRANSITION_DROP_ORBIT) {
						Print("Ship '%s' broke orbit to '%s'\n", jumpship->Name(), dest->GetName());
						jumpship->SetAbsoluteOrientation(0, PI / 4, 0);
						jumpship->SetVelocity(jumpship->Heading() * 1.0e3);
					}

					// make orbit:
					else if (jump->type == Ship::TRANSITION_MAKE_ORBIT) {
						Print("Ship '%s' achieved orbit '%s'\n", jumpship->Name(), dest->GetName());
						jumpship->LookAt(FVector::ZeroVector);
						jumpship->SetVelocity(jumpship->Heading() * 500.0);
					}

					// hyper jump:
					else {
						Print("Ship '%s' quantum to '%s'\n", jumpship->Name(), dest->GetName());

						if (jump->hyperdrive)
							CreateExplosion(jumpship->Location(), FVector::ZeroVector, Explosion::HYPER_FLASH, 1.0f, 1.0f, dest);
						else
							CreateExplosion(jumpship->Location(), FVector::ZeroVector, Explosion::QUANTUM_FLASH, 1.0f, 0.0f, dest);

						jumpship->LookAt(FVector::ZeroVector);
						jumpship->SetVelocity(jumpship->Heading() * 500.0);
						jumpship->SetHelmHeading(jumpship->CompassHeading());
						jumpship->SetHelmPitch(0);
					}
				}

				else if (regions.size() > 1) {
					Print("Warning: Unusual jump request for ship '%s'\n", jumpship->Name());
					regions[1]->InsertObject(jumpship);
				}

				Sensor* sensor = jumpship->GetSensor();
				if (sensor)
					sensor->ClearAllContacts();
			}
		}

		jumplist.destroy();

		if (pship && pship->GetRegion()) {
			if (active_region != pship->GetRegion()) {
				pship->GetRegion()->SetPlayerShip(pship);
			}
		}
	}
}

void
Sim::ResolveSplashList()
{
	if (splashlist.size()) {
		ListIter<SimSplash> iter = splashlist;
		while (++iter) {
			SimSplash* splash = iter.value();

			if (!splash->rgn)
				continue;

			// damage ships:
			ListIter<Ship> s_iter = splash->rgn->GetShips();
			while (++s_iter) {
				Ship* ship = s_iter.value();

				const double distance = (ship->Location() - splash->loc).Size();

				if (distance > 1 && distance < splash->range) {
					const double damage = splash->damage * (1 - distance / splash->range);
					//if (!NetGame::IsNetGameClient()) {
						ship->InflictDamage(damage);
					//}

					const int ship_destroyed = (!ship->InTransition() && ship->Integrity() < 1.0f);

					// then delete the ship:
					if (ship_destroyed) {
						//NetUtil::SendObjKill(ship, 0, NetObjKill::KILL_MISC);
						Print("    %s Killed %s (%s)\n", (const char*)splash->owner_name, ship->Name(), FormatGameTime());

						// record the kill
						ShipStats* killer = ShipStats::Find(splash->owner_name);
						if (killer) {
							if (splash->missile)
								killer->AddEvent(SimEvent::MISSILE_KILL, ship->Name());
							else
								killer->AddEvent(SimEvent::GUNS_KILL, ship->Name());
						}

						Ship* owner = FindShip(splash->owner_name, splash->rgn->GetName());
						if (owner && owner->GetIFF() != ship->GetIFF()) {
							if (ship->GetIFF() > 0 || owner->GetIFF() > 1) {
								killer->AddPoints(ship->Value());

								SimElement* elem = owner->GetElement();
								if (elem) {
									if (owner->GetElementIndex() > 1) {
										Ship* s = elem->GetShip(1);

										if (s) {
											ShipStats* cmdr_stats = ShipStats::Find(s->Name());
											if (cmdr_stats) {
												cmdr_stats->AddCommandPoints(ship->Value() / 2);
											}
										}
									}

									SimElement* cmdr = elem->GetCommander();
									if (cmdr) {
										Ship* s = cmdr->GetShip(1);

										if (s) {
											ShipStats* cmdr_stats = ShipStats::Find(s->Name());
											if (cmdr_stats) {
												cmdr_stats->AddCommandPoints(ship->Value() / 2);
											}
										}
									}
								}
							}
						}

						ShipStats* killee = ShipStats::Find(ship->Name());
						if (killee)
							killee->AddEvent(SimEvent::DESTROYED, splash->owner_name);

						ship->DeathSpiral();
					}
				}
			}

			// damage drones:
			ListIter<Drone> drone_iter = splash->rgn->GetDrones();
			while (++drone_iter) {
				Drone* drone = drone_iter.value();

				const double distance = (drone->Location() - splash->loc).Size();

				if (distance > 1 && distance < splash->range) {
					const double damage = splash->damage * (1 - distance / splash->range);
					drone->InflictDamage(damage);

					const int destroyed = (drone->Integrity() < 1.0f);

					// then mark the drone for deletion:
					if (destroyed) {
						//NetUtil::SendWepDestroy(drone);
						sim->CreateExplosion(drone->Location(), drone->Velocity(), 21 /* was LARGE_EXP */, 1.0f, 1.0f, splash->rgn);
						drone->SetLife(0);
					}
				}
			}
		}

		splashlist.destroy();
	}
}

// +--------------------------------------------------------------------+

void
Sim::ProcessEventTrigger(int type, int event_id, const char* ship, int param)
{
	Text ship_name = ship;

	ListIter<MissionEvent> iter = events;
	while (++iter) {
		MissionEvent* event = iter.value();

		if (event->IsPending() && event->Trigger() == type) {
			switch (type) {
			case MissionEvent::TRIGGER_DAMAGE:
			case MissionEvent::TRIGGER_DESTROYED:
			case MissionEvent::TRIGGER_JUMP:
			case MissionEvent::TRIGGER_LAUNCH:
			case MissionEvent::TRIGGER_DOCK:
			case MissionEvent::TRIGGER_TARGET:
				if (event->TriggerParam() <= param) {
					if (ship_name.indexOf(event->TriggerShip()) == 0)
						event->Activate();
				}
				break;

			case MissionEvent::TRIGGER_NAVPT:
				if (event->TriggerParam() == param) {
					if (ship_name.indexOf(event->TriggerShip()) == 0)
						event->Activate();
				}
				break;

			case MissionEvent::TRIGGER_EVENT:
			case MissionEvent::TRIGGER_SKIPPED:
				if (event->TriggerParam() == event_id)
					event->Activate();
				break;
			}
		}
	}
}

double
Sim::MissionClock() const
{
	return (Game::GameTime() - start_time) / 1000.0;
}

// +--------------------------------------------------------------------+

void
Sim::SkipCutscene()
{
	Starshatter* stars = Starshatter::GetInstance();
	if (stars && stars->InCutscene()) {
		ListIter<MissionEvent>  iter = events;
		bool                    end = false;
		double                  end_time = 0;

		while (++iter && !end) {
			MissionEvent* event = iter.value();

			if (event->IsPending() || event->IsActive()) {
				if (event->Event() == MissionEvent::END_SCENE ||
					event->Event() == MissionEvent::END_MISSION) {
					end = true;
					end_time = event->Time();
				}

				if (event->Event() == MissionEvent::FIRE_WEAPON) {
					event->Skip();
				}
				else {
					event->Activate();
					event->Execute(true);
				}
			}
		}

		const double skip_time = end_time - MissionClock();
		if (skip_time > 0) {
			Game::SkipGameTime(skip_time);
		}
	}
}

// +--------------------------------------------------------------------+

void
Sim::ResolveTimeSkip(double seconds)
{
	double skipped = 0;

	// allow elements to process hold time, and release as needed:
	ListIter<Element> elem = elements;
	while (++elem)
		elem->ExecFrame(seconds);

	// step through the skip, ten seconds at a time:
	if (active_region) {
		double total_skip = seconds;
		double frame_skip = 10;

		while (total_skip > frame_skip) {
			if (active_region->CanTimeSkip()) {
				active_region->ResolveTimeSkip(frame_skip);
				total_skip -= frame_skip;
				skipped += frame_skip;
			}
			// break out early if player runs into bad guys...
			else {
				total_skip = 0;
			}
		}

		if (total_skip > 0)
			active_region->ResolveTimeSkip(total_skip);

		skipped += total_skip;
	}

	// give player control after time skip:
	Ship* player_ship = GetPlayerShip();
	if (player_ship) {
		player_ship->SetAutoNav(false);
		player_ship->SetThrottle(75);

		HUDView* hud = HUDView::GetInstance();
		if (hud)
			hud->SetHUDMode(HUDView::HUD_MODE_TAC);

		if (IsTestMode())
			player_ship->SetControls(0);
	}

	Game::SkipGameTime(skipped);
	CameraManager::SetCameraMode(CameraManager::MODE_COCKPIT);
}

// +--------------------------------------------------------------------+

ListIter<MissionElement>
Sim::GetMissionElements()
{
	mission_elements.destroy();

	ListIter<SimElement> iter = elements;
	while (++iter) {
		SimElement* elem = iter.value();

		int num_live_ships = 0;

		for (int i = 0; i < elem->NumShips(); i++) {
			Ship* s = elem->GetShip(i + 1);

			if (s && !s->IsDying() && !s->IsDead())
				num_live_ships++;
		}

		if (elem->IsSquadron() || num_live_ships > 0) {
			MissionElement* msn_elem = CreateMissionElement(elem);

			if (msn_elem)
				mission_elements.append(msn_elem);
		}
	}

	return mission_elements;
}

MissionElement*
Sim::CreateMissionElement(SimElement* elem)
{
	MissionElement* msn_elem = 0;

	if (elem->IsSquadron()) {
		if (!elem->GetCarrier() || elem->GetCarrier()->Integrity() < 1)
			return msn_elem;
	}

	if (elem && !elem->IsNetObserver()) {
		msn_elem = new MissionElement;

		msn_elem->SetName(elem->Name());
		msn_elem->SetIFF(elem->GetIFF());
		msn_elem->SetMissionRole(elem->Type());

		if (elem->IsSquadron() && elem->GetCarrier()) {
			Ship* carrier = elem->GetCarrier();

			msn_elem->SetCarrier(carrier->Name());
			msn_elem->SetCount(elem->GetCount());
			msn_elem->SetLocation(OtherHand(carrier->Location()));

			if (carrier->GetRegion())
				msn_elem->SetRegion(carrier->GetRegion()->GetName());

			int     squadron_index = 0;
			Hangar* hangar = FindSquadron(elem->Name(), squadron_index);

			if (hangar) {
				msn_elem->SetDeadCount(hangar->NumShipsDead(squadron_index));
				msn_elem->SetMaintCount(hangar->NumShipsMaint(squadron_index));

				const ShipDesign* design = hangar->SquadronDesign(squadron_index);
				msn_elem->SetDesign(design);

				Text design_path = design->path_name;
				design_path.setSensitive(false);

				if (design_path.indexOf("/Mods/Ships") == 0) {
					design_path = design_path.substring(11, 1000);
					msn_elem->SetPath(design_path);
				}
			}
		}
		else {
			msn_elem->SetSquadron(elem->GetSquadron());
			msn_elem->SetCount(elem->NumShips());
		}

		if (elem->GetCommander())
			msn_elem->SetCommander(elem->GetCommander()->Name());

		msn_elem->SetCombatGroup(elem->GetCombatGroup());
		msn_elem->SetCombatUnit(elem->GetCombatUnit());

		Ship* ship = elem->GetShip(1);
		if (ship) {
			if (ship->GetRegion())
				msn_elem->SetRegion(ship->GetRegion()->GetName());

			msn_elem->SetLocation(OtherHand(ship->Location()));
			msn_elem->SetDesign(ship->Design());

			msn_elem->SetPlayer(elem->Player());
			msn_elem->SetCommandAI(elem->GetCommandAILevel());
			msn_elem->SetHoldTime((int)elem->GetHoldTime());
			msn_elem->SetZoneLock(elem->GetZoneLock());
			msn_elem->SetHeading(ship->CompassHeading());

			msn_elem->SetPlayable(elem->IsPlayable());
			msn_elem->SetRogue(elem->IsRogue());
			msn_elem->SetIntelLevel(elem->IntelLevel());

			Text design_path = ship->Design()->path_name;
			design_path.setSensitive(false);

			if (design_path.indexOf("/Mods/Ships") == 0) {
				design_path = design_path.substring(11, 1000);
				msn_elem->SetPath(design_path);
			}

			msn_elem->SetRespawnCount(ship->RespawnCount());
		}

		MissionLoad* loadout = new MissionLoad;
		FMemory::Memcpy(loadout->GetStations(), elem->Loadout(), 16 * sizeof(int));
		msn_elem->Loadouts().append(loadout);

		const int num_obj = elem->NumObjectives();
		for (int i = 0; i < num_obj; i++) {
			Instruction* o = elem->GetObjective(i);
			Instruction* instr = new Instruction(*o);
			msn_elem->AddObjective(instr);
		}

		const int num_inst = elem->NumInstructions();
		for (int i = 0; i < num_inst; i++) {
			Text instr = elem->GetInstruction(i);
			msn_elem->AddInstruction(instr);
		}

		ListIter<Instruction> nav_iter = elem->GetFlightPlan();
		while (++nav_iter) {
			Instruction* nav = nav_iter.value();
			Instruction* npt = new Instruction(nav->RegionName(), nav->Location(), nav->Action());

			npt->SetFormation(nav->Formation());
			npt->SetSpeed(nav->Speed());
			npt->SetTarget(nav->TargetName());
			npt->SetHoldTime(nav->HoldTime());
			npt->SetFarcast(nav->Farcast());
			npt->SetStatus(nav->Status());

			msn_elem->AddNavPoint(npt);
		}

		for (int i = 0; i < elem->NumShips(); i++) {
			ship = elem->GetShip(i + 1);

			if (ship) {
				MissionShip* s = new MissionShip;

				s->SetName(ship->Name());
				s->SetRegNum(ship->Registry());
				s->SetRegion(ship->GetRegion()->GetName());
				s->SetLocation(OtherHand(ship->Location()));
				s->SetVelocity(OtherHand(ship->Velocity()));

				s->SetRespawns(ship->RespawnCount());
				s->SetHeading(ship->CompassHeading());
				s->SetIntegrity(ship->Integrity());

				if (ship->GetDecoy())
					s->SetDecoys(ship->GetDecoy()->Ammo());

				if (ship->GetProbeLauncher())
					s->SetProbes(ship->GetProbeLauncher()->Ammo());

				int n;
				int ammo[16];
				int fuel[4];

				for (n = 0; n < 16; n++) {
					Weapon* w = ship->GetWeaponByIndex(n + 1);

					if (w)
						ammo[n] = w->Ammo();
					else
						ammo[n] = -10;
				}

				for (n = 0; n < 4; n++) {
					if (ship->Reactors().size() > n)
						fuel[n] = ship->Reactors()[n]->Charge();
					else
						fuel[n] = -10;
				}

				s->SetAmmo(ammo);
				s->SetFuel(fuel);

				msn_elem->Ships().append(s);
			}
		}
	}

	return msn_elem;
}

Hangar*
Sim::FindSquadron(const char* name, int& index)
{
	Hangar* hangar = 0;

	ListIter<SimRegion> iter = regions;
	while (++iter && !hangar) {
		SimRegion* rgn = iter.value();

		ListIter<Ship> s_iter = rgn->GetCarriers();
		while (++s_iter && !hangar) {
			Ship* carrier = s_iter.value();
			Hangar* h = carrier->GetHangar();

			for (int i = 0; i < h->NumSquadrons() && !hangar; i++) {
				if (h->SquadronName(i) == name) {
					hangar = h;
					index = i;
				}
			}
		}
	}

	return hangar;
}

// +===================================================================-+

SimRegion::SimRegion(Sim* s, const char* n, int t)
	: sim(s), name(n), type(t), orbital_region(0), star_system(0)
	, player_ship(0), grid(0), active(false), current_view(0), sim_time(0)
	, ai_index(0), terrain(0)
{
	if (sim) {
		star_system = sim->GetStarSystem();
	}
}

SimRegion::SimRegion(Sim* s, OrbitalRegion* r)
	: sim(s), orbital_region(r), type(REAL_SPACE), star_system(0)
	, player_ship(0), grid(0), active(false), current_view(0), sim_time(0)
	, ai_index(0), terrain(0)
{
	if (r) {
		star_system = r->System();
	}

	if (orbital_region) {
		name = orbital_region->Name();
		grid = new Grid((int)orbital_region->Radius(),
			(int)orbital_region->GridSpace());

		if (orbital_region->Type() == Orbital::TERRAIN) {
			TerrainRegion* trgn = (TerrainRegion*)orbital_region;
			terrain = new Terrain(trgn);
			type = AIR_SPACE;
		}

		else if (orbital_region->Asteroids() > 0) {
			const int asteroids = orbital_region->Asteroids();

			for (int i = 0; i < asteroids; i++) {
				const FVector init_loc(
					(float)((rand() - 16384.0f) * 30.0f),
					(float)((rand() - 16384.0f) * 3.0f),
					(float)((rand() - 16384.0f) * 30.0f)
				);

				FRandomStream Stream(SeedValue); // store SeedValue somewhere stable
				const double Mass = 1.0e7 + (static_cast<double>(Stream.FRand()) * (1.0e8 - 1.0e7));
				sim->CreateAsteroid(init_loc, i, Mass, this);
			}
		}
	}
	else {
		name = Game::GetText("Unknown");
	}
}

const char* FormatGameTime()
{
	static char txt[64];

	int t = Game::GameTime();

	int h = (t / 3600000);
	int m = ((t - h * 3600000) / 60000);
	int s = ((t - h * 3600000 - m * 60000) / 1000);
	int e = (t - h * 3600000 - m * 60000 - s * 1000);

	if (h > 0)
		sprintf_s(txt, "%02d:%02d:%02d.%03d", h, m, s, e);
	else
		sprintf_s(txt, "%02d:%02d.%03d", m, s, e);

	return txt;
}
