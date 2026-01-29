/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MapView.cpp
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR: John DiCamillo
	ORIGINAL STUDIO: Destroyer Studios LLC

	OVERVIEW
	========
	Star Map class
*/

#include "MapView.h"

// Minimal Unreal includes (only what this TU requires):
#include "Math/Vector.h"        // FVector
#include "Logging/LogMacros.h"  // UE_LOG

#include "Galaxy.h"
#include "StarSystem.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Instruction.h"
#include "SimElement.h"
#include "NavAI.h"
#include "Weapon.h"
#include "Sim.h"
#include "Mission.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "SimContact.h"
#include "MenuView.h"

#include "Game.h"
#include "DataLoader.h"
#include "EventDispatch.h"
#include "Video.h"
#include "UIButton.h"
#include "SystemFont.h"
#include "FontManager.h"
#include "Mouse.h"
#include "FormatUtil.h"
#include "Menu.h"
#include "GameStructs.h"

#include "Math/UnrealMathUtility.h"

// +--------------------------------------------------------------------+

// Supported Selection Modes:

const int SELECT_NONE = -1;
const int SELECT_SYSTEM = 0;
const int SELECT_PLANET = 1;
const int SELECT_REGION = 2;
const int SELECT_STATION = 3;
const int SELECT_STARSHIP = 4;
const int SELECT_FIGHTER = 5;
const int SELECT_NAVPT = 6;

const int VIEW_GALAXY = 0;
const int VIEW_SYSTEM = 1;
const int VIEW_REGION = 2;

// +--------------------------------------------------------------------+

static FORCEINLINE FVector ToFVector(double X, double Y, double Z = 0.0)
{
	return FVector((float)X, (float)Y, (float)Z);
}

static FORCEINLINE FVector ToFVector(const Point& P)
{
	return FVector((float)P.X, (float)P.Y, (float)P.Z);
}

MapView::MapView(Window* win)
	: View(win, 0, 0, win ? win->Width() : 0, win ? win->Height() : 0)
	, system(nullptr), zoom(1.1f), offset_x(0), offset_y(0), ship(nullptr), campaign(nullptr)
	, captured(false), dragging(false), adding_navpt(false)
	, moving_navpt(false), moving_elem(false)
	, view_mode(VIEW_SYSTEM), seln_mode(SELECT_REGION), ship_filter(0xffffffff)
	, current_star(0), current_planet(0), current_region(0)
	, current_ship(nullptr), current_elem(nullptr), current_navpt(nullptr), mission(nullptr)
	, scrolling(0), scroll_x(0), scroll_y(0), click_x(0), click_y(0)
	, active_menu(nullptr), map_menu(nullptr), map_system_menu(nullptr), map_sector_menu(nullptr)
	, ship_menu(nullptr), editor(false)
	, nav_menu(nullptr), action_menu(nullptr), objective_menu(nullptr), formation_menu(nullptr), speed_menu(nullptr)
	, hold_menu(nullptr), farcast_menu(nullptr), menu_view(nullptr)
{
	for (int i = 0; i < 3; i++) {
		view_zoom[i] = zoom;
		view_offset_x[i] = offset_x;
		view_offset_y[i] = offset_y;
	}

	menu_view = new MenuView(window);

	title_font = FontManager::Find("Limerick12");
	font = FontManager::Find("Verdana");

	active_window = (ActiveWindow*)window;

	if (active_window)
		active_window->AddView(this);
}

// +--------------------------------------------------------------------+

MapView::~MapView()
{
	ClearMenu();

	// galaxy_image is now an Unreal render asset pointer (UTexture2D*), not a Starshatter Bitmap.
	galaxy_image = nullptr;

	delete menu_view;
}

// +--------------------------------------------------------------------+

void
MapView::OnWindowMove()
{
	if (menu_view)
		menu_view->OnWindowMove();
}

// +--------------------------------------------------------------------+

void
MapView::SetGalaxy(List<StarSystem>& g)
{
	system_list.clear();
	system_list.append(g);

	if (system_list.size() > 0) {
		SetSystem(system_list[0]);
	}
}

void
MapView::SetSystem(StarSystem* s)
{
	if (system != s) {
		system = s;

		// forget invalid selection:
		current_star = 0;
		current_planet = 0;
		current_region = 0;
		current_ship = 0;
		current_elem = 0;
		current_navpt = 0;

		// flush old object pointers:
		stars.clear();
		planets.clear();
		regions.clear();

		// insert objects from star system:
		if (system) {
			ListIter<OrbitalBody> star = system->Bodies();
			while (++star) {
				switch (star->Type()) {
				case Orbital::STAR:       stars.append(star.value());
					break;
				case Orbital::PLANET:
				case Orbital::MOON:       planets.append(star.value());
					break;
				}
			}

			ListIter<OrbitalBody> planet = star->Satellites();
			while (++planet) {
				planets.append(planet.value());

				ListIter<OrbitalBody> moon = planet->Satellites();
				while (++moon) {
					planets.append(moon.value());
				}
			}

			ListIter<OrbitalRegion> rgn = system->AllRegions();
			while (++rgn)
				regions.append(rgn.value());

			// sort region list by distance from the star:
			regions.sort();
		}

		BuildMenu();
	}
}

// +--------------------------------------------------------------------+

void
MapView::SetShip(Ship* s)
{
	if (ship != s) {
		ship = s;

		// forget invalid selection:
		current_star = 0;
		current_planet = 0;
		current_region = 0;
		current_ship = 0;
		current_elem = 0;
		current_navpt = 0;

		if (ship && system_list.size() > 0) {
			SimRegion* rgn = ship->GetRegion();

			if (rgn && rgn->GetSystem()) {
				system = 0;
				SetSystem(rgn->GetSystem());
			}
		}

		BuildMenu();
	}
}

// +--------------------------------------------------------------------+

void
MapView::SetMission(Mission* m)
{
	if (mission != m) {
		mission = m;

		// forget invalid selection:
		current_star = 0;
		current_planet = 0;
		current_region = 0;
		current_ship = 0;
		current_elem = 0;
		current_navpt = 0;

		if (mission && system_list.size() > 0) {
			system = 0;
			SetSystem(mission->GetStarSystem());
		}

		BuildMenu();
	}
}

// +--------------------------------------------------------------------+

enum MapView_MENU {
	MAP_SYSTEM = 1000,
	MAP_SECTOR = 2000,
	MAP_SHIP = 3000,
	MAP_NAV = 4000,
	MAP_ADDNAV = 4001,
	MAP_DELETE = 4002,
	MAP_CLEAR = 4003,
	MAP_ACTION = 5000,
	MAP_FORMATION = 6000,
	MAP_SPEED = 7000,
	MAP_HOLD = 8000,
	MAP_FARCAST = 8500,
	MAP_OBJECTIVE = 9000
};

void
MapView::SetCampaign(Campaign* in_campaign)
{
	if (campaign != in_campaign) {
		campaign = in_campaign;

		// forget invalid selection:
		current_star = 0;
		current_planet = 0;
		current_region = 0;
		current_ship = 0;
		current_elem = 0;
		current_navpt = 0;

		if (campaign)
			SetGalaxy(campaign->GetSystemList());
	}
}

void
MapView::BuildMenu()
{
	ClearMenu();

	map_system_menu = new Menu(Game::GetText("MapView.menu.STARSYSTEM"));

	if (system_list.size() > 0) {
		int i = 0;
		ListIter<StarSystem> iter = system_list;
		while (++iter) {
			StarSystem* s = iter.value();
			map_system_menu->AddItem(s->Name(), MAP_SYSTEM + i);
			i++;
		}
	}

	else if (system) {
		map_system_menu->AddItem(system->Name(), MAP_SYSTEM);
	}

	map_sector_menu = new Menu(Game::GetText("MapView.menu.SECTOR"));
	for (int i = 0; i < regions.size(); i++) {
		Orbital* rgn = regions[i];
		map_sector_menu->AddItem(rgn->Name(), MAP_SECTOR + i);
	}

	map_menu = new Menu(Game::GetText("MapView.menu.MAP"));
	map_menu->AddMenu("System", map_system_menu);
	map_menu->AddMenu("Sector", map_sector_menu);

	if (ship || mission) {
		ship_menu = new Menu(Game::GetText("MapView.menu.SHIP"));
		ship_menu->AddMenu(Game::GetText("MapView.item.Starsystem"), map_system_menu);
		ship_menu->AddMenu(Game::GetText("MapView.item.Sector"), map_sector_menu);

		ship_menu->AddItem("", 0);
		ship_menu->AddItem(Game::GetText("MapView.item.Add-Nav"), MAP_ADDNAV);
		ship_menu->AddItem(Game::GetText("MapView.item.Clear-All"), MAP_CLEAR);

		action_menu = new Menu(Game::GetText("MapView.menu.ACTION"));
		for (int i = 0; i < Instruction::NUM_ACTIONS; i++) {
			action_menu->AddItem(Game::GetText(Text("MapView.item.") + Instruction::ActionName(i)), MAP_ACTION + i);
		}

		formation_menu = new Menu(Game::GetText("MapView.menu.FORMATION"));
		for (int i = 0; i < Instruction::NUM_FORMATIONS; i++) {
			formation_menu->AddItem(Game::GetText(Text("MapView.item.") + Instruction::FormationName(i)), MAP_FORMATION + i);
		}

		speed_menu = new Menu(Game::GetText("MapView.menu.SPEED"));
		speed_menu->AddItem("250", MAP_SPEED + 0);
		speed_menu->AddItem("500", MAP_SPEED + 1);
		speed_menu->AddItem("750", MAP_SPEED + 2);
		speed_menu->AddItem("1000", MAP_SPEED + 3);

		hold_menu = new Menu(Game::GetText("MapView.menu.HOLD"));
		hold_menu->AddItem(Game::GetText("MapView.item.None"), MAP_HOLD + 0);
		hold_menu->AddItem(Game::GetText("MapView.item.1-Minute"), MAP_HOLD + 1);
		hold_menu->AddItem(Game::GetText("MapView.item.5-Minutes"), MAP_HOLD + 2);
		hold_menu->AddItem(Game::GetText("MapView.item.10-Minutes"), MAP_HOLD + 3);
		hold_menu->AddItem(Game::GetText("MapView.item.15-Minutes"), MAP_HOLD + 4);

		farcast_menu = new Menu(Game::GetText("MapView.menu.FARCAST"));
		farcast_menu->AddItem(Game::GetText("MapView.item.Use-Quantum"), MAP_FARCAST + 0);
		farcast_menu->AddItem(Game::GetText("MapView.item.Use-Farcast"), MAP_FARCAST + 1);

		objective_menu = new Menu(Game::GetText("MapView.menu.OBJECTIVE"));

		nav_menu = new Menu(Game::GetText("MapView.menu.NAVPT"));
		nav_menu->AddMenu(Game::GetText("MapView.item.Action"), action_menu);
		nav_menu->AddMenu(Game::GetText("MapView.item.Objective"), objective_menu);
		nav_menu->AddMenu(Game::GetText("MapView.item.Formation"), formation_menu);
		nav_menu->AddMenu(Game::GetText("MapView.item.Speed"), speed_menu);
		nav_menu->AddMenu(Game::GetText("MapView.item.Hold"), hold_menu);
		nav_menu->AddMenu(Game::GetText("MapView.item.Farcast"), farcast_menu);
		nav_menu->AddItem("", 0);
		nav_menu->AddItem(Game::GetText("MapView.item.Add-Nav"), MAP_ADDNAV);
		nav_menu->AddItem(Game::GetText("MapView.item.Del-Nav"), MAP_DELETE);
	}

	else if (campaign) {
		ship_menu = 0;
		speed_menu = 0;
		hold_menu = 0;
		farcast_menu = 0;
		objective_menu = 0;
		formation_menu = 0;
		nav_menu = 0;
	}

	active_menu = map_menu;
}

// +--------------------------------------------------------------------+

void
MapView::ClearMenu()
{
	delete map_menu;
	delete map_system_menu;
	delete map_sector_menu;
	delete ship_menu;
	delete nav_menu;
	delete action_menu;
	delete objective_menu;
	delete formation_menu;
	delete speed_menu;
	delete hold_menu;
	delete farcast_menu;

	map_menu = 0;
	map_system_menu = 0;
	map_sector_menu = 0;
	ship_menu = 0;
	nav_menu = 0;
	action_menu = 0;
	objective_menu = 0;
	formation_menu = 0;
	speed_menu = 0;
	hold_menu = 0;
	farcast_menu = 0;
}

// +--------------------------------------------------------------------+

void
MapView::ProcessMenuItem(int action)
{
	bool send_nav_data = false;
	bool can_command = true;

	if (ship && current_ship && ship != current_ship) {
		if (ship->GetElement() && current_ship->GetElement()) {
			if (!ship->GetElement()->CanCommand(current_ship->GetElement())) {
				can_command = false;
			}
		}
	}


	if (action >= MAP_OBJECTIVE) {
		int index = action - MAP_OBJECTIVE;

		if (current_navpt && can_command) {
			current_navpt->SetTarget(objective_menu->GetItem(index)->GetText());
			send_nav_data = true;
		}
	}

	else if (action >= MAP_FARCAST) {
		if (current_navpt && can_command) {
			current_navpt->SetFarcast(action - MAP_FARCAST);
			send_nav_data = true;
		}
	}

	else if (action >= MAP_HOLD) {
		int hold_time = 0;
		switch (action) {
		default:
		case MAP_HOLD + 0:  hold_time = 0;    break;
		case MAP_HOLD + 1:  hold_time = 60;   break;
		case MAP_HOLD + 2:  hold_time = 300;  break;
		case MAP_HOLD + 3:  hold_time = 600;  break;
		case MAP_HOLD + 4:  hold_time = 900;  break;
		}

		if (current_navpt && can_command) {
			current_navpt->SetHoldTime(hold_time);
			send_nav_data = true;
		}
	}

	else if (action >= MAP_SPEED) {
		if (current_navpt && can_command) {
			current_navpt->SetSpeed((action - MAP_SPEED + 1) * 250);
			send_nav_data = true;
		}
	}

	else if (action >= MAP_FORMATION) {
		if (current_navpt && can_command) {
			current_navpt->SetFormation(action - MAP_FORMATION);
			send_nav_data = true;
		}
	}

	else if (action >= MAP_ACTION) {
		if (current_navpt && can_command) {
			current_navpt->SetAction(action - MAP_ACTION);
			SelectNavpt(current_navpt);
			send_nav_data = true;
		}
	}

	else if (action == MAP_ADDNAV) {
		Text           rgn_name = regions[current_region]->Name();
		Instruction* prior = current_navpt;
		Instruction* n = 0;

		if (current_ship && can_command) {
			Sim* sim = Sim::GetSim();
			SimRegion* rgn = sim->FindRegion(rgn_name);
			FVector     init_pt = FVector::ZeroVector;

			if (rgn) {
				if (rgn->IsAirSpace())
					init_pt.Z = 10000.0f;

				n = new Instruction(rgn, Point(init_pt.X, init_pt.Y, init_pt.Z));
			}
			else {
				n = new Instruction(rgn_name, Point(init_pt.X, init_pt.Y, init_pt.Z));
			}

			n->SetSpeed(500);

			if (prior) {
				n->SetAction(prior->Action());
				n->SetFormation(prior->Formation());
				n->SetSpeed(prior->Speed());
				n->SetTarget(prior->GetTarget());
			}

			current_ship->AddNavPoint(n, prior);
		}

		else if (current_elem && can_command) {
			FVector     init_pt = FVector::ZeroVector;

			if (regions[current_region]->Type() == Orbital::TERRAIN)
				init_pt.Z = 10000.0f;

			n = new Instruction(rgn_name, Point(init_pt.X, init_pt.Y, init_pt.Z));
			n->SetSpeed(500);

			if (prior) {
				n->SetAction(prior->Action());
				n->SetFormation(prior->Formation());
				n->SetSpeed(prior->Speed());
				n->SetTarget(prior->GetTarget());
			}

			current_elem->AddNavPoint(n, prior);
		}

		if (can_command) {
			current_navpt = n;
			current_status = Instruction::PENDING;
			adding_navpt = true;
			captured = SetCapture();
		}
	}

	else if (action == MAP_DELETE) {
		if (current_navpt && can_command) {
			if (current_ship)
				current_ship->DelNavPoint(current_navpt);
			else if (current_elem && can_command)
				current_elem->DelNavPoint(current_navpt);

			SelectNavpt(0);
		}
	}

	else if (action == MAP_CLEAR) {
		if (current_ship && can_command)
			current_ship->ClearFlightPlan();
		else if (current_elem && can_command)
			current_elem->ClearFlightPlan();

		SelectNavpt(0);
	}

	else if (action >= MAP_NAV) {
	}

	else if (action >= MAP_SHIP) {
	}

	else if (action >= MAP_SECTOR) {
		int index = action - MAP_SECTOR;

		if (index < regions.size())
			current_region = index;

		if (view_mode == VIEW_SYSTEM) {
			Orbital* s = regions[current_region];
			SetupScroll(s);
		}
	}

	else if (system_list.size() > 0 && action >= MAP_SYSTEM) {
		int index = action - MAP_SYSTEM;

		if (index < system_list.size())
			SetSystem(system_list[index]);
	}

	else {
	}

	Sim* sim = Sim::GetSim();
	if (send_nav_data && sim) {
		Ship* s = current_ship;

		if (s && s->GetElement()) {
			SimElement* elem = s->GetElement();
			int      index = elem->GetNavIndex(current_navpt);
		}
	}
}

// +--------------------------------------------------------------------+

bool
MapView::SetCapture()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
		return dispatch->CaptureMouse(this) ? true : false;

	return 0;
}

// +--------------------------------------------------------------------+

bool
MapView::ReleaseCapture()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
		return dispatch->ReleaseMouse(this) ? true : false;

	return 0;
}

// +--------------------------------------------------------------------+

void
MapView::SetViewMode(int mode)
{
	if (mode >= 0 && mode < 3) {
		// save state:
		view_zoom[view_mode] = zoom;
		view_offset_x[view_mode] = offset_x;
		view_offset_y[view_mode] = offset_y;

		// switch mode:
		view_mode = mode;

		// restore state:

		if (view_mode == VIEW_GALAXY) {
			zoom = 1;
			offset_x = 0;
			offset_y = 0;
		}
		else {
			zoom = view_zoom[view_mode];
			offset_x = view_offset_x[view_mode];
			offset_y = view_offset_y[view_mode];
		}

		scrolling = 0;
		scroll_x = 0;
		scroll_y = 0;
	}
}

// +--------------------------------------------------------------------+

bool
MapView::Update(SimObject* obj)
{
	if (obj == current_ship) {
		current_ship = 0;
		active_menu = map_menu;
	}

	return SimObserver::Update(obj);
}

// +--------------------------------------------------------------------+

void
MapView::SelectShip(Ship* selship)
{
	if (selship != current_ship) {
		current_ship = selship;

		if (current_ship) {
			if (current_ship->Life() == 0 || current_ship->IsDying() || current_ship->IsDead()) {
				current_ship = 0;
			}
			else {
				Observe(current_ship);
			}
		}
	}

	SelectNavpt(0);
}

// +--------------------------------------------------------------------+

void
MapView::SelectElem(MissionElement* elem)
{
	if (elem != current_elem) {
		current_elem = elem;

		if (current_elem) {
			if (current_elem->IsStarship()) {
				ship_menu->GetItem(3)->SetEnabled(true);
			}

			else if (current_elem->IsDropship()) {
				ship_menu->GetItem(3)->SetEnabled(true);
			}

			else {
				ship_menu->GetItem(3)->SetEnabled(false);
			}
		}
	}

	SelectNavpt(0);
}

// +--------------------------------------------------------------------+

void
MapView::SelectNavpt(Instruction* navpt)
{
	current_navpt = navpt;

	if (current_navpt) {
		current_status = current_navpt->Status();

		List<Text> ships;
		objective_menu->ClearItems();

		switch (current_navpt->Action()) {
		case Instruction::VECTOR:
		case Instruction::LAUNCH:
		case Instruction::PATROL:
		case Instruction::SWEEP:
		case Instruction::RECON:
			objective_menu->AddItem(Game::GetText("MapView.item.not-available"), 0);
			objective_menu->GetItem(0)->SetEnabled(false);
			break;

		case Instruction::DOCK:
			FindShips(true, true, true, false, ships);
			break;

		case Instruction::DEFEND:
			FindShips(true, true, true, false, ships);
			break;

		case Instruction::ESCORT:
			FindShips(true, false, true, true, ships);
			break;

		case Instruction::INTERCEPT:
			FindShips(false, false, false, true, ships);
			break;

		case Instruction::ASSAULT:
			FindShips(false, false, true, false, ships);
			break;

		case Instruction::STRIKE:
			FindShips(false, true, false, false, ships);
			break;
		}

		for (int i = 0; i < ships.size(); i++)
			objective_menu->AddItem(ships[i]->data(), MAP_OBJECTIVE + i);

		ships.destroy();
	}
	else {
		objective_menu->ClearItems();
		objective_menu->AddItem(Game::GetText("MapView.item.not-available"), 0);
		objective_menu->GetItem(0)->SetEnabled(false);
	}
}

// +--------------------------------------------------------------------+

void
MapView::FindShips(bool Friendly, bool Station, bool Starship, bool Dropship,
	List<Text>& Result)
{
	if (mission) {
		const List<MissionElement>& Elements = mission->GetElements();

		for (int ElementIndex = 0; ElementIndex < Elements.size(); ElementIndex++) {
			MissionElement* Elem = Elements.at(ElementIndex);

			if (Elem->IsSquadron())                 continue;
			if (!Station && Elem->IsStatic())     continue;
			if (!Starship && Elem->IsStarship())   continue;
			if (!Dropship && Elem->IsDropship())   continue;

			if (!editor && Friendly &&
				Elem->GetIFF() > 0 &&
				Elem->GetIFF() != mission->Team())
				continue;

			if (!editor && !Friendly &&
				(Elem->GetIFF() == 0 || Elem->GetIFF() == mission->Team()))
				continue;

			Result.append(new Text(Elem->Name()));
		}
	}
	else if (ship) {
		Sim* SimPtr = Sim::GetSim();
		if (!SimPtr)
			return;

		const List<SimRegion>& Regions = SimPtr->GetRegions();

		for (int RegionIndex = 0; RegionIndex < Regions.size(); RegionIndex++) {
			SimRegion* Region = Regions.at(RegionIndex);
			const List<Ship>& Ships = Region->GetShips();

			for (int ShipIndex = 0; ShipIndex < Ships.size(); ShipIndex++) {
				Ship* ShipPtr = Ships.at(ShipIndex);

				if (!Station && ShipPtr->IsStatic())     continue;
				if (!Starship && ShipPtr->IsStarship())   continue;
				if (!Dropship && ShipPtr->IsDropship())   continue;

				if (Friendly &&
					ShipPtr->GetIFF() > 0 &&
					ShipPtr->GetIFF() != ship->GetIFF())
					continue;

				if (!Friendly &&
					(ShipPtr->GetIFF() == 0 || ShipPtr->GetIFF() == ship->GetIFF()))
					continue;

				Result.append(new Text(ShipPtr->Name()));
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
MapView::SetSelectionMode(int mode)
{
	if (mode <= SELECT_NONE) {
		seln_mode = SELECT_NONE;
		return;
	}

	if (mode != seln_mode && mode <= SELECT_FIGHTER) {
		seln_mode = mode;

		// when changing mode,
		// select the item closest to the current center:
		if (system && view_mode == VIEW_SYSTEM)
			SelectAt(rect.x + rect.w / 2,
				rect.y + rect.h / 2);
	}
}

void
MapView::SetSelection(int Index)
{
	if (scrolling)
		return;

	Orbital* SelectedOrbital = 0;

	switch (seln_mode) {
	case SELECT_SYSTEM:
		if (Index < system_list.size())
			SetSystem(system_list[Index]);

		SelectedOrbital = stars[current_star];
		break;

	default:
	case SELECT_PLANET:
		if (Index < planets.size())
			current_planet = Index;

		SelectedOrbital = planets[current_planet];
		break;

	case SELECT_REGION:
		if (Index < regions.size())
			current_region = Index;

		SelectedOrbital = regions[current_region];
		break;

	case SELECT_STATION:
	{
		if (mission) {
			MissionElement* SelectedElem = 0;

			ListIter<MissionElement> ElemIter = mission->GetElements();
			while (++ElemIter) {
				if (ElemIter->IsStatic()) {
					if (ElemIter->Identity() == Index) {
						SelectedElem = ElemIter.value();
						break;
					}
				}
			}

			SelectElem(SelectedElem);

			if (SelectedElem && regions.size()) {
				ListIter<Orbital> RegionIter = regions;
				while (++RegionIter) {
					if (!_stricmp(SelectedElem->Region(), RegionIter->Name())) {
						Orbital* ElemRegion = RegionIter.value();
						current_region = regions.index(ElemRegion);
					}
				}
			}
		}
		else {
			Ship* SelectedShip = 0;

			if (ship) {
				SimRegion* ShipRegion = ship->GetRegion();
				if (ShipRegion) {
					ListIter<Ship> ShipIter = ShipRegion->GetShips();
					while (++ShipIter) {
						if (ShipIter->IsStatic()) {
							if (ShipIter->Identity() == Index) {
								SelectedShip = ShipIter.value();
								break;
							}
						}
					}
				}
			}

			SelectShip(SelectedShip);

			if (SelectedShip) {
				SelectedOrbital = SelectedShip->GetRegion()->GetOrbitalRegion();
				current_region = regions.index(SelectedOrbital);
			}
		}
	}
	break;

	case SELECT_STARSHIP:
	{
		if (mission) {
			MissionElement* SelectedElem = 0;

			ListIter<MissionElement> ElemIter = mission->GetElements();
			while (++ElemIter) {
				if (ElemIter->IsStarship()) {
					if (ElemIter->Identity() == Index) {
						SelectedElem = ElemIter.value();
						break;
					}
				}
			}

			SelectElem(SelectedElem);

			if (SelectedElem && regions.size()) {
				ListIter<Orbital> RegionIter = regions;
				while (++RegionIter) {
					if (!_stricmp(SelectedElem->Region(), RegionIter->Name())) {
						Orbital* ElemRegion = RegionIter.value();
						current_region = regions.index(ElemRegion);
					}
				}
			}
		}
		else {
			Ship* SelectedShip = 0;

			if (ship) {
				SimRegion* ShipRegion = ship->GetRegion();
				if (ShipRegion) {
					ListIter<Ship> ShipIter = ShipRegion->GetShips();
					while (++ShipIter) {
						if (ShipIter->IsStarship()) {
							if (ShipIter->Identity() == Index) {
								SelectedShip = ShipIter.value();
								break;
							}
						}
					}
				}
			}

			SelectShip(SelectedShip);

			if (SelectedShip) {
				SelectedOrbital = SelectedShip->GetRegion()->GetOrbitalRegion();
				current_region = regions.index(SelectedOrbital);
			}
		}
	}
	break;

	case SELECT_FIGHTER:
	{
		if (mission) {
			MissionElement* SelectedElem = 0;

			ListIter<MissionElement> ElemIter = mission->GetElements();
			while (++ElemIter) {
				if (ElemIter->IsDropship() && !ElemIter->IsSquadron()) {
					if (ElemIter->Identity() == Index) {
						SelectedElem = ElemIter.value();
						break;
					}
				}
			}

			SelectElem(SelectedElem);

			if (SelectedElem && regions.size()) {
				ListIter<Orbital> RegionIter = regions;
				while (++RegionIter) {
					if (!_stricmp(SelectedElem->Region(), RegionIter->Name())) {
						Orbital* ElemRegion = RegionIter.value();
						current_region = regions.index(ElemRegion);
					}
				}
			}
		}
		else {
			Ship* SelectedShip = 0;

			if (ship) {
				SimRegion* ShipRegion = ship->GetRegion();
				if (ShipRegion) {
					ListIter<Ship> ShipIter = ShipRegion->GetShips();
					while (++ShipIter) {
						if (ShipIter->IsDropship()) {
							if (ShipIter->Identity() == Index) {
								SelectedShip = ShipIter.value();
								break;
							}
						}
					}
				}
			}

			SelectShip(SelectedShip);

			if (SelectedShip) {
				SelectedOrbital = SelectedShip->GetRegion()->GetOrbitalRegion();
				current_region = regions.index(SelectedOrbital);
			}
		}
	}
	break;
	}

	SetupScroll(SelectedOrbital);
}


void
MapView::SetSelectedShip(Ship* InShip)
{
	if (scrolling)
		return;

	Orbital* SelectedOrbital = 0;
	Ship* SelectedShip = 0;

	switch (seln_mode) {
	case SELECT_SYSTEM:
	case SELECT_PLANET:
	case SELECT_REGION:
	default:
		break;

	case SELECT_STATION:
	case SELECT_STARSHIP:
	case SELECT_FIGHTER:
	{
		if (InShip) {
			SimRegion* ShipRegion = InShip->GetRegion();

			if (ShipRegion && ShipRegion->GetNumShips()) {
				SelectedShip = ShipRegion->GetShips().find(InShip);
			}
		}

		SelectShip(SelectedShip);

		if (SelectedShip) {
			SelectedOrbital = SelectedShip->GetRegion()->GetOrbitalRegion();
			current_region = regions.index(SelectedOrbital);
		}
	}
	break;
	}

	if (SelectedShip)
		SetupScroll(SelectedOrbital);
}

void
MapView::SetSelectedElem(MissionElement* elem)
{
	if (scrolling) return;
	Orbital* s = 0;

	switch (seln_mode) {
	case SELECT_SYSTEM:
	case SELECT_PLANET:
	case SELECT_REGION:
	default:
		break;

	case SELECT_STATION:
	case SELECT_STARSHIP:
	case SELECT_FIGHTER:
	{
		SelectElem(elem);
	}
	break;
	}

	if (current_elem)
		SetupScroll(s);
}

void
MapView::SetupScroll(Orbital* s)
{
	switch (view_mode) {
	case VIEW_GALAXY:
		zoom = 1;
		offset_x = 0;
		offset_y = 0;
		scrolling = 0;
		break;

	case VIEW_SYSTEM:
		if (!s) {
			offset_x = 0;
			offset_y = 0;
			scrolling = 0;
		}
		else {
			const FVector& Loc = s->Location();

			scroll_x = (offset_x + Loc.X) / 5.0;
			scroll_y = (offset_y + Loc.Y) / 5.0;
			scrolling = 5;
		}
		break;

	case VIEW_REGION:
		if (current_navpt) {
			// don't move the map
			scrolling = 0;
		}
		else if (current_ship) {
			const FVector Sloc = current_ship->Location();

			if (!IsVisible(Sloc)) {
				scroll_x = (offset_x + Sloc.X) / 5.0;
				scroll_y = (offset_y + Sloc.Y) / 5.0;
				scrolling = 5;
			}
			else {
				scroll_x = 0;
				scroll_y = 0;
				scrolling = 0;
			}
		}
		else if (current_elem) {
			const FVector Sloc = current_elem->Location();

			if (!IsVisible(Sloc)) {
				scroll_x = (offset_x + Sloc.X) / 5.0;
				scroll_y = (offset_y + Sloc.Y) / 5.0;
				scrolling = 5;
			}
			else {
				scroll_x = 0;
				scroll_y = 0;
				scrolling = 0;
			}
		}
		else {
			offset_x = 0;
			offset_y = 0;
			scrolling = 0;
		}
		break;
	}
}

bool
MapView::IsVisible(const FVector& Loc)
{
	if (view_mode == VIEW_REGION) {
		double scale = c / r;
		double ox = offset_x * scale;
		double oy = offset_y * scale;
		double sx = Loc.X * scale;
		double sy = Loc.Y * scale;
		double cx = rect.w / 2;
		double cy = rect.h / 2;

		int    test_x = (int)(cx + sx + ox);
		int    test_y = (int)(cy + sy + oy);

		bool   visible = test_x >= 0 && test_x < rect.w &&
			test_y >= 0 && test_y < rect.h;

		return visible;
	}

	return false;
}

// +--------------------------------------------------------------------+

void
MapView::SetRegion(OrbitalRegion* rgn)
{
	if (scrolling || rgn == 0) return;

	int index = regions.index(rgn);

	if (index < 0 || index == current_region || index >= regions.size())
		return;

	current_region = index;
	Orbital* s = regions[current_region];

	if (!s)
		return;

	switch (view_mode) {
	case VIEW_GALAXY:
	case VIEW_SYSTEM:
		scroll_x = (offset_x + s->Location().X) / 5.0;
		scroll_y = (offset_y + s->Location().Y) / 5.0;
		scrolling = 5;
		break;

	case VIEW_REGION:
		offset_x = 0;
		offset_y = 0;
		scrolling = 0;
		break;
	}
}

// +--------------------------------------------------------------------+

void
MapView::SetRegionByName(const char* rgn_name)
{
	OrbitalRegion* rgn = nullptr;

	for (int i = 0; i < regions.size(); i++) {
		Orbital* orb = regions[i];
		if (orb && !strcmp(rgn_name, orb->Name())) {
			rgn = static_cast<OrbitalRegion*>(orb);
			break;
		}
	}

	SetRegion(rgn);
}

// +--------------------------------------------------------------------+

void
MapView::SelectAt(int x, int y)
{
	if (scrolling) return;
	if (c == 0)    return;
	if (seln_mode < 0) return;

	Orbital* selection_orbital = nullptr;

	const double scale = r / c;
	const double cx = rect.w / 2.0;
	const double cy = rect.h / 2.0;
	const double test_x = (x - rect.x - cx) * scale - offset_x;
	const double test_y = (y - rect.y - cy) * scale - offset_y;

	double best_dist = 1.0e20;
	int    closest = 0;

	if (view_mode == VIEW_GALAXY) {
		c = (cx > cy) ? cx : cy;
		r = 10;

		Galaxy* g = Galaxy::GetInstance();
		if (g)
			r = g->Radius();

		StarSystem* closest_system = nullptr;

		ListIter<StarSystem> iter = system_list;
		while (++iter) {
			StarSystem* sys = iter.value();

			const double dx = (sys->Location().X - test_x);
			const double dy = (sys->Location().Y - test_y);
			const double d = FMath::Sqrt(dx * dx + dy * dy);

			if (d < best_dist) {
				best_dist = d;
				closest_system = sys;
			}
		}

		if (closest_system)
			SetSystem(closest_system);
	}

	else if (view_mode == VIEW_SYSTEM) {
		switch (seln_mode) {
		case SELECT_SYSTEM: {
			if (stars.isEmpty()) return;

			int index = 0;
			ListIter<Orbital> star = stars;
			while (++star) {
				const double dx = (star->Location().X - test_x);
				const double dy = (star->Location().Y - test_y);
				const double d = FMath::Sqrt(dx * dx + dy * dy);

				if (d < best_dist) {
					best_dist = d;
					closest = index;
				}

				index++;
			}

			current_star = closest;
		}
						  selection_orbital = stars[current_star];
						  break;

		case SELECT_PLANET: {
			if (planets.isEmpty()) return;

			int index = 0;
			ListIter<Orbital> planet = planets;
			while (++planet) {
				const double dx = (planet->Location().X - test_x);
				const double dy = (planet->Location().Y - test_y);
				const double d = FMath::Sqrt(dx * dx + dy * dy);

				if (d < best_dist) {
					best_dist = d;
					closest = index;
				}

				index++;
			}

			current_planet = closest;
		}
						  selection_orbital = planets[current_planet];
						  break;

		default:
		case SELECT_REGION: {
			if (regions.isEmpty()) return;

			int index = 0;
			ListIter<Orbital> region = regions;
			while (++region) {
				const double dx = (region->Location().X - test_x);
				const double dy = (region->Location().Y - test_y);
				const double d = FMath::Sqrt(dx * dx + dy * dy);

				if (d < best_dist) {
					best_dist = d;
					closest = index;
				}

				index++;
			}

			current_region = closest;
		}
						  selection_orbital = regions[current_region];
						  break;
		}
	}

	else if (view_mode == VIEW_REGION) {
		best_dist = 5.0e3;

		if (mission) {
			Orbital* rgn = regions[current_region];
			MissionElement* sel_elem = nullptr;
			Instruction* sel_nav = nullptr;

			if (!rgn) return;

			// check nav points:
			ListIter<MissionElement> elem = mission->GetElements();
			while (++elem) {
				MissionElement* e = elem.value();

				if (!e->IsSquadron() && (editor || e->GetIFF() == mission->Team())) {
					ListIter<Instruction> navpt = e->NavList();
					while (++navpt) {
						Instruction* n = navpt.value();

						if (!_stricmp(n->RegionName(), rgn->Name())) {
							const FVector nloc = n->Location();

							const double dx = nloc.X - test_x;
							const double dy = nloc.Y - test_y;
							const double d = FMath::Sqrt(dx * dx + dy * dy);

							if (d < best_dist) {
								best_dist = d;
								sel_nav = n;
								sel_elem = e;
							}
						}
					}
				}
			}

			if (sel_nav) {
				SelectElem(sel_elem);
				SelectNavpt(sel_nav);
			}

			// check elements:
			else {
				elem.reset();
				while (++elem) {
					MissionElement* e = elem.value();

					if (e->Region() == rgn->Name() && !e->IsSquadron()) {
						const FVector sloc = e->Location();

						const double dx = sloc.X - test_x;
						const double dy = sloc.Y - test_y;
						const double d = FMath::Sqrt(dx * dx + dy * dy);

						if (d < best_dist) {
							best_dist = d;
							sel_elem = e;
						}
					}
				}

				SelectElem(sel_elem);

				if (sel_elem)
					selection_orbital = rgn;
			}
		}

		else if (ship) {
			Sim* sim = Sim::GetSim();
			Orbital* rgn = regions[current_region];
			SimRegion* simrgn = nullptr;
			Ship* sel_ship = nullptr;
			Instruction* sel_nav = nullptr;

			if (sim && rgn)
				simrgn = sim->FindRegion(rgn->Name());

			// check nav points:
			if (simrgn) {
				for (int rr = 0; rr < sim->GetRegions().size(); rr++) {
					SimRegion* sr = sim->GetRegions().at(rr);

					for (int i = 0; i < sr->GetShips().size(); i++) {
						Ship* sref = sr->GetShips().at(i);

						if (sref->GetIFF() == ship->GetIFF() && sref->GetElementIndex() == 1) {
							ListIter<Instruction> navpt = sref->GetFlightPlan();
							while (++navpt) {
								Instruction* n = navpt.value();

								if (!_stricmp(n->RegionName(), rgn->Name())) {
									const FVector nloc = n->Location();

									const double dx = nloc.X - test_x;
									const double dy = nloc.Y - test_y;
									const double d = FMath::Sqrt(dx * dx + dy * dy);

									if (d < best_dist) {
										best_dist = d;
										sel_nav = n;
										sel_ship = sref;
									}
								}
							}
						}
					}
				}
			}

			if (sel_nav) {
				SelectShip(sel_ship);
				SelectNavpt(sel_nav);
			}

			// check ships:
			else if (simrgn && simrgn->GetNumShips()) {
				ListIter<Ship> ship_iter = simrgn->GetShips();
				while (++ship_iter) {
					Ship* sref = ship_iter.value();

					if (sref && !IsClutter(*sref)) {
						const FVector sloc = sref->Location();

						const double dx = sloc.X - test_x;
						const double dy = sloc.Y - test_y;
						const double d = FMath::Sqrt(dx * dx + dy * dy);

						if (d < best_dist) {
							best_dist = d;
							sel_ship = sref;
						}
					}
				}

				SelectShip(sel_ship);
			}
			else {
				SelectShip(nullptr);
				SelectNavpt(nullptr);
			}
		}
	}

	if (selection_orbital)
		SetupScroll(selection_orbital);
}

// +--------------------------------------------------------------------+

Orbital*
MapView::GetSelection()
{
	Orbital* s = nullptr;

	switch (seln_mode) {
	case SELECT_SYSTEM:
		if (current_star < stars.size())
			s = stars[current_star];
		break;

	default:
	case SELECT_PLANET:
		if (current_planet < planets.size())
			s = planets[current_planet];
		break;

	case SELECT_REGION:
		if (current_region < regions.size())
			s = regions[current_region];
		break;

	case SELECT_STATION:
	case SELECT_STARSHIP:
	case SELECT_FIGHTER:
		break;
	}

	return s;
}

Ship*
MapView::GetSelectedShip()
{
	return current_ship;
}

MissionElement*
MapView::GetSelectedElem()
{
	return current_elem;
}

// +--------------------------------------------------------------------+

int
MapView::GetSelectionIndex()
{
	int s = 0;

	switch (seln_mode) {
	case SELECT_SYSTEM:  s = current_star;   break;
	default:
	case SELECT_PLANET:  s = current_planet; break;
	case SELECT_REGION:  s = current_region; break;

	case SELECT_STATION:
	case SELECT_STARSHIP:
	case SELECT_FIGHTER:
	{
		s = -1;

		Sim* sim = Sim::GetSim();
		Orbital* rgn = regions[current_region];
		SimRegion* simrgn = nullptr;

		if (sim && rgn)
			simrgn = sim->FindRegion(rgn->Name());

		if (simrgn) {
			if (current_ship && simrgn->GetNumShips()) {
				s = simrgn->GetShips().index(current_ship);
			}
		}
	}
	break;
	}

	return s;
}

void
MapView::DrawTabbedText(SystemFont* MapFont, const char* text)
{
	if (MapFont && text && *text) {
		Rect label_rect;

		label_rect.w = rect.w;
		label_rect.h = rect.h;

		label_rect.Inset(8, 8, 8, 8);

		const DWORD text_flags = DT_WORDBREAK | DT_LEFT;

		active_window->SetFont(MapFont);
		active_window->DrawText(text, 0, label_rect, text_flags);
	}
}

// +--------------------------------------------------------------------+

void
MapView::Refresh()
{
	rect = window->GetRect();

	if (!system) {
		DrawGrid();
		DrawTabbedText(title_font, Game::GetText("MapView.item.no-system"));
		return;
	}

	if (font)
		font->SetColor(FColor::White);

	if (scrolling) {
		offset_x -= scroll_x;
		offset_y -= scroll_y;
		scrolling--;
	}

	rect.w -= 2;
	rect.h -= 2;

	switch (view_mode) {
	case VIEW_GALAXY: DrawGalaxy(); break;
	case VIEW_SYSTEM: DrawSystem(); break;
	case VIEW_REGION: DrawRegion(); break;
	default:          DrawGrid();   break;
	}

	rect.w += 2;
	rect.h += 2;

	if (menu_view) {
		if (current_navpt) {
			active_menu = nav_menu;
		}
		else if (current_ship) {
			if (current_ship->GetIFF() == ship->GetIFF())
				active_menu = ship_menu;
			else
				active_menu = map_menu;
		}
		else if (current_elem) {
			if (editor || current_elem->GetIFF() == mission->Team())
				active_menu = ship_menu;
			else
				active_menu = map_menu;
		}
		else {
			active_menu = map_menu;
		}

		menu_view->SetBackColor(FColor(128, 128, 128, 255));
		menu_view->SetTextColor(FColor::White);
		menu_view->SetMenu(active_menu);
		menu_view->DoMouseFrame();

		if (menu_view->GetAction()) {
			ProcessMenuItem(menu_view->GetAction());
		}

		menu_view->Refresh();
	}

	DrawTitle();
}

void
MapView::DrawTitle()
{
	title_font->SetColor(active_window->GetForeColor());
	DrawTabbedText(title_font, title);
}

// +--------------------------------------------------------------------+

void
MapView::DrawGalaxy()
{
	title = Game::GetText("MapView.title.Galaxy");
	DrawGrid();

	const double cx = rect.w / 2.0;
	const double cy = rect.h / 2.0;

	c = (cx > cy) ? cx : cy;
	r = 10;

	Galaxy* g = Galaxy::GetInstance();
	if (g)
		r = g->Radius();

	const double scale = c / r;
	double ox = 0;
	double oy = 0;

	// compute offset:
	ListIter<StarSystem> iter = system_list;
	while (++iter) {
		StarSystem* s = iter.value();

		if (system == s) {
			if(FMath::Abs(s->Location().X) > 10.0 || FMath::Abs(s->Location().Y) > 10.0) {
				int sx = (int)s->Location().X;
				int sy = (int)s->Location().Y;

				sx -= sx % 10;
				sy -= sy % 10;

				ox = sx * -scale;
				oy = sy * -scale;
			}
		}
	}

	// draw the list of systems, and their connections:
	iter.reset();
	while (++iter) {
		StarSystem* s = iter.value();

		const int sx = (int)(cx + ox + s->Location().X * scale);
		const int sy = (int)(cy + oy + s->Location().Y * scale);

		if (sx < 4 || sx > rect.w - 4 || sy < 4 || sy > rect.h - 4)
			continue;

		window->DrawEllipse(sx - 7, sy - 7, sx + 7, sy + 7, Ship::IFFColor(s->Affiliation()));

		if (s == system) {
			window->DrawLine(0, sy, rect.w, sy, FColor(128, 128, 128), Video::BLEND_ADDITIVE);
			window->DrawLine(sx, 0, sx, rect.h, FColor(128, 128, 128), Video::BLEND_ADDITIVE);
		}

		ListIter<StarSystem> iter2 = system_list;
		while (++iter2) {
			StarSystem* s2 = iter2.value();

			if (s != s2 && s->HasLinkTo(s2)) {
				int ax = sx;
				int ay = sy;

				int bx = (int)(cx + ox + s2->Location().X * scale);
				int by = (int)(cy + oy + s2->Location().Y * scale);

				if (ax == bx) {
					if (ay < by) { ay += 8; by -= 8; }
					else { ay -= 8; by += 8; }
				}
				else if (ay == by) {
					if (ax < bx) { ax += 8; bx -= 8; }
					else { ax -= 8; bx += 8; }
				}
				else {
					FVector d((float)(bx - ax), (float)(by - ay), 0.0f);
					d = d.GetSafeNormal();

					ax += (int)(8.0f * d.X);
					ay += (int)(8.0f * d.Y);

					bx -= (int)(8.0f * d.X);
					by -= (int)(8.0f * d.Y);
				}

				window->DrawLine(ax, ay, bx, by, FColor(120, 120, 120), Video::BLEND_ADDITIVE);
			}
		}
	}

	// finally draw all the stars in the galaxy:
	if (g) {
		ListIter<Star> s_iter = g->Stars();
		while (++s_iter) {
			Star* s = s_iter.value();

			const int sx = (int)(cx + ox + s->Location().X * scale);
			const int sy = (int)(cy + oy + s->Location().Y * scale);
			const int sr = s->GetSize();

			if (sx < 4 || sx > rect.w - 4 || sy < 4 || sy > rect.h - 4)
				continue;

			window->FillEllipse(sx - sr, sy - sr, sx + sr, sy + sr, s->GetColor());

			if (!std::strncmp(s->Name(), "GSC", 3))
				font->SetColor(FColor(100, 100, 100));
			else
				font->SetColor(FColor::White);

			Rect name_rect(sx - 60, sy + 8, 120, 20);
			active_window->SetFont(font);
			active_window->DrawText(s->Name(), 0, name_rect, DT_SINGLELINE | DT_CENTER);
		}
	}

	font->SetColor(FColor::White);
}

// +--------------------------------------------------------------------+

void
MapView::DrawSystem()
{
	Text caption = Game::GetText("MapView.title.Starsystem");
	caption += " ";
	caption += system->Name();

	if (current_ship) {
		caption += "\n";
		caption += Game::GetText("MapView.title.Ship");
		caption += " ";
		caption += current_ship->Name();
	}
	else if (current_elem) {
		caption += "\n";
		caption += Game::GetText("MapView.title.Ship");
		caption += " ";
		caption += current_elem->Name();
	}

	title = caption;

	ListIter<OrbitalBody> star = system->Bodies();
	while (++star) {
		int p_orb = 1;

		ListIter<OrbitalBody> planet = star->Satellites();
		while (++planet) {
			DrawOrbital(*planet, p_orb++);

			int m_orb = 1;

			ListIter<OrbitalBody> moon = planet->Satellites();
			while (++moon) {
				DrawOrbital(*moon, m_orb++);

				ListIter<OrbitalRegion> region = moon->Regions();
				while (++region) {
					DrawOrbital(*region, 1);
				}
			}

			ListIter<OrbitalRegion> region = planet->Regions();
			while (++region) {
				DrawOrbital(*region, 1);
			}
		}

		ListIter<OrbitalRegion> region = star->Regions();
		while (++region) {
			DrawOrbital(*region, 1);
		}

		DrawOrbital(*star, 0);
	}

	char r_txt[32];
	FormatNumber(r_txt, system->Radius() * zoom);

	char resolution[64];
	sprintf_s(resolution, "%s: %s", Game::GetText("MapView.info.Resolution").data(), r_txt);

	active_window->SetFont(font);

	Rect TextRect(4, 4, rect.w - 8, 24);
	active_window->DrawText(resolution, -1, TextRect, DT_SINGLELINE | DT_RIGHT);
}

// +--------------------------------------------------------------------+

void
MapView::DrawRegion()
{
	OrbitalRegion* rgn = (OrbitalRegion*)regions[current_region];

	Text caption = Game::GetText("MapView.title.Sector");
	caption += " ";
	caption += rgn->Name();

	if (current_ship) {
		caption += "\n";
		caption += Game::GetText("MapView.title.Ship");
		caption += " ";
		caption += current_ship->Name();
	}
	else if (current_elem) {
		caption += "\n";
		caption += Game::GetText("MapView.title.Ship");
		caption += " ";
		caption += current_elem->Name();
	}

	title = caption;

	const double cx = rect.w / 2.0;
	const double cy = rect.h / 2.0;

	const int size = (int)rgn->Radius();
	const int step = (int)rgn->GridSpace();

	c = (cx < cy) ? cx : cy;
	r = rgn->Radius() * zoom;

	const double scale = c / r;

	const double ox = offset_x * scale;
	const double oy = offset_y * scale;

	const int left = (int)(-size * scale + ox + cx);
	const int right = (int)(size * scale + ox + cx);
	const int top = (int)(-size * scale + oy + cy);
	const int bottom = (int)(size * scale + oy + cy);

	FColor major(48, 48, 48);
	FColor minor(24, 24, 24);

	int x, y;
	int tick = 0;

	for (x = 0; x <= size; x += step) {
		int lx = (int)(x * scale + ox + cx);
		if (!tick) window->DrawLine(lx, top, lx, bottom, major, Video::BLEND_ADDITIVE);
		else       window->DrawLine(lx, top, lx, bottom, minor, Video::BLEND_ADDITIVE);

		lx = (int)(-x * scale + ox + cx);
		if (!tick) window->DrawLine(lx, top, lx, bottom, major, Video::BLEND_ADDITIVE);
		else       window->DrawLine(lx, top, lx, bottom, minor, Video::BLEND_ADDITIVE);

		if (++tick > 3) tick = 0;
	}

	tick = 0;

	for (y = 0; y <= size; y += step) {
		int ly = (int)(y * scale + oy + cy);
		if (!tick) window->DrawLine(left, ly, right, ly, major, Video::BLEND_ADDITIVE);
		else       window->DrawLine(left, ly, right, ly, minor, Video::BLEND_ADDITIVE);

		ly = (int)(-y * scale + oy + cy);
		if (!tick) window->DrawLine(left, ly, right, ly, major, Video::BLEND_ADDITIVE);
		else       window->DrawLine(left, ly, right, ly, minor, Video::BLEND_ADDITIVE);

		if (++tick > 3) tick = 0;
	}

	int rep = 3;
	if (r > 70e3) rep = 2;
	if (r > 250e3) rep = 1;

	if (campaign && rgn) {
		// draw the combatants in this region:
		ListIter<Combatant> iter = campaign->GetCombatants();
		while (++iter) {
			Combatant* combatant = iter.value();
			DrawCombatGroup(combatant->GetForce(), rep);
		}
	}

	else if (mission && rgn) {
		// draw the elements in this region:
		ListIter<MissionElement> elem = mission->GetElements();
		while (++elem)
			if (!elem->IsSquadron())
				DrawElem(*elem, (elem.value() == current_elem), rep);
	}

	else if (ship) {
		// draw the ships in this region:
		Sim* sim = Sim::GetSim();
		SimRegion* simrgn = nullptr;

		if (sim && rgn)
			simrgn = sim->FindRegion(rgn->Name());

		if (simrgn) {
			ListIter<SimContact> c_iter = simrgn->TrackList(ship->GetIFF());
			while (++c_iter) {
				SimContact* contact = c_iter.value();
				Ship* s = contact->GetShip();

				if (s && ((int)s->Class() & ship_filter) && !IsClutter(*s) && s != ship)
					DrawShip(*s, (s == current_ship), rep);
			}

			ListIter<Ship> s_iter = simrgn->GetShips();
			while (++s_iter) {
				Ship* s = s_iter.value();

				if (s && (s->IsStatic()) && !IsClutter(*s) &&
					(s->GetIFF() == ship->GetIFF() || s->GetIFF() == 0))
					DrawShip(*s, (s == current_ship), rep);
			}

			// draw nav routes for allied ships not in the region:
			ListIter<SimRegion> r_iter = sim->GetRegions();
			while (++r_iter) {
				SimRegion* rrgn = r_iter.value();

				if (rrgn != simrgn) {
					ListIter<Ship> rs_iter = rrgn->GetShips();
					while (++rs_iter) {
						Ship* s = rs_iter.value();

						if (s && !s->IsStatic() && !IsClutter(*s) &&
							(s->GetIFF() == ship->GetIFF() || s->GetIFF() == 0)) {
							DrawNavRoute(simrgn->GetOrbitalRegion(),
								s->GetFlightPlan(),
								s->MarkerColor(),
								s, 0);
						}
					}
				}
			}
		}

		// draw our own ship:
		DrawShip(*ship, (ship == current_ship), rep);
	}

	char r_txt[32];
	FormatNumber(r_txt, r * 2);

	char resolution[64];
	sprintf_s(resolution, "%s: %s", Game::GetText("MapView.info.Resolution").data(), r_txt);

	active_window->SetFont(font);

	Rect TextRect(4, 4, rect.w - 8, 24);
	active_window->DrawText(resolution, -1, TextRect, DT_SINGLELINE | DT_RIGHT);
}

// +--------------------------------------------------------------------+

// +--------------------------------------------------------------------+

void
MapView::DrawOrbital(Orbital& body, int index)
{
	int type = body.Type();

	if (type == Orbital::NOTHING)
		return;

	int  x1, y1, x2, y2;
	Rect label_rect;
	int  label_w = 64;
	int  label_h = 18;

	const double cx = rect.w / 2.0;
	const double cy = rect.h / 2.0;

	c = (cx < cy) ? cx : cy;
	r = system->Radius() * zoom;

	if ((r > 300e9) && (type > Orbital::PLANET))
		return;

	const double xscale = cx / r;
	const double yscale = cy / r * 0.75;

	const double ox = offset_x * xscale;
	const double oy = offset_y * yscale;

	double bo_x = body.Orbit() * xscale;
	double bo_y = body.Orbit() * yscale;
	double br = body.Radius() * yscale;
	double bx = body.Location().X * xscale;
	double by = body.Location().Y * yscale;

	double px = 0;
	double py = 0;

	if (body.Primary()) {
		double min_pr = GetMinRadius(body.Primary()->Type());

		if (index) {
			if (min_pr < 4)
				min_pr = 4;

			min_pr *= (index + 1);
		}

		const double min_x = min_pr * xscale / yscale;
		const double min_y = min_pr;

		if (bo_x < min_x) bo_x = min_x;
		if (bo_y < min_y) bo_y = min_y;

		px = body.Primary()->Location().X * xscale;
		py = body.Primary()->Location().Y * yscale;
	}

	if (type == Orbital::TERRAIN)
		bo_x = bo_y;

	const int ipx = (int)(cx + px + ox);
	const int ipy = (int)(cy + py + oy);
	const int ibo_x = (int)bo_x;
	const int ibo_y = (int)bo_y;

	x1 = ipx - ibo_x;
	y1 = ipy - ibo_y;
	x2 = ipx + ibo_x;
	y2 = ipy + ibo_y;

	if (type != Orbital::TERRAIN) {
		const double a = x2 - x1;
		const double b = rect.w * 32.0;

		if (a < b)
			window->DrawEllipse(x1, y1, x2, y2, FColor(64, 64, 64), Video::BLEND_ADDITIVE);
	}

	// show body's location on possibly magnified orbit:
	bx = px + bo_x * FMath::Cos(body.Phase());
	by = py + bo_y * FMath::Sin(body.Phase());

	const double min_br = GetMinRadius(type);
	if (br < min_br) br = min_br;

	FColor color;

	switch (type) {
	case Orbital::STAR:  
		color = FColor(248, 248, 128); 
		break;
	case Orbital::PLANET:  
		color = FColor(64, 64, 192);
		break;
	case Orbital::MOON:    
		color = FColor(32, 192, 96); 
		break;
	case Orbital::REGION:  
		color = FColor(255, 255, 255); 
		break;
	case Orbital::TERRAIN: 
		color = FColor(16, 128, 48); 
		break;
	default:            
		color = FColor::White;   
		break;
	}

	const int icx = (int)(cx + bx + ox);
	const int icy = (int)(cy + by + oy);
	const int ibr = (int)br;

	x1 = icx - ibr;
	y1 = icy - ibr;
	x2 = icx + ibr;
	y2 = icy + ibr;

	// NOTE: Bitmap->UTexture2D conversion is handled at the type level.
	// Here we preserve the original "has icon" logic, but do not assume Bitmap APIs exist.
	if (type < Orbital::REGION) {
		// If you keep a texture/icon pointer on Orbital, branch here to draw it.
		// Otherwise, fall back to a simple filled ellipse:
		window->FillEllipse(x1, y1, x2, y2, color);
	}
	else {
		window->DrawRect(x1, y1, x2, y2, color);

		if (campaign) {
			ListIter<Combatant> iter = campaign->GetCombatants();
			while (++iter) {
				Combatant* combatant = iter.value();

				if (ibr >= 4 || combatant->GetIFF() == 1)
					DrawCombatantSystem(combatant, &body, icx, icy, ibr);
			}
		}
	}

	if (type == Orbital::STAR || bo_y > label_h) {
		label_rect.x = x1 - label_w + (int)br;
		label_rect.y = y1 - label_h;
		label_rect.w = label_w * 2;
		label_rect.h = label_h;

		active_window->SetFont(font);
		active_window->DrawText(body.Name(), -1, label_rect, DT_SINGLELINE | DT_CENTER);
	}
}

// +--------------------------------------------------------------------+

double
MapView::GetMinRadius(int type)
{
	switch (type) {
	case Orbital::STAR:   return 8;
	case Orbital::PLANET: return 4;
	case Orbital::MOON:   return 2;
	case Orbital::REGION: return 2;
	default:              break;
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
MapView::DrawShip(Ship& s, bool current, int rep)
{
	OrbitalRegion* rgn = (OrbitalRegion*)regions[current_region];
	if (!rgn) return;

	int   x1, y1, x2, y2;
	POINT shiploc;

	// NOTE: Original used OtherHand(). Ensure your Ship::Location() already matches the map handedness.
	const FVector sloc = s.Location();

	const double cx = rect.w / 2.0;
	const double cy = rect.h / 2.0;

	c = (cx < cy) ? cx : cy;
	r = rgn->Radius() * zoom;

	const double scale = c / r;

	const double ox = offset_x * scale;
	const double oy = offset_y * scale;

	const double rlx = 0;
	const double rly = 0;

	int sprite_width = 10;

	window->SetFont(font);

	// draw ship icon:
	if (rep && view_mode == VIEW_REGION && rgn == s.GetRegion()->GetOrbitalRegion()) {
		const double sx = (sloc.X + rlx) * scale;
		const double sy = (sloc.Y + rly) * scale;

		shiploc.x = (int)(cx + sx + ox);
		shiploc.y = (int)(cy + sy + oy);

		const bool ship_visible =
			shiploc.x >= 0 && shiploc.x < rect.w &&
			shiploc.y >= 0 && shiploc.y < rect.h;

		if (ship_visible) {
			if (rep < 3) {
				window->FillRect(shiploc.x - 2, shiploc.y - 2, shiploc.x + 2, shiploc.y + 2, s.MarkerColor());
				sprite_width = 2;

				if (&s == ship || !IsCrowded(s))
					window->Print(shiploc.x - sprite_width, shiploc.y + sprite_width + 2, s.Name());
			}
			else {
				FVector heading = s.Heading();
				heading.Z = 0.0f;
				heading = heading.GetSafeNormal();

				double theta = 0;

				if (heading.Y > 0)
					theta = FMath::Acos(FMath::Clamp(heading.X, -1.0, 1.0));
				else
					theta = -FMath::Acos(FMath::Clamp(heading.X, -1.0, 1.0));

				const double THETA_SLICE = 4.0 / PI;
				const double THETA_OFFSET = PI + THETA_SLICE / 2.0;

				int sprite_index = (int)((theta + THETA_OFFSET) * THETA_SLICE);
				const int nsprites = s.Design()->map_sprites.size();

				if (nsprites) {
					if (sprite_index < 0 || sprite_index >= nsprites)
						sprite_index = sprite_index % nsprites;

					// NOTE: map_sprites should be migrated from Bitmap* to UTexture2D* at the design layer.
					// When that happens, replace this branch with your engine/UI draw call for textures.
					UE_LOG(LogTemp, VeryVerbose, TEXT("MapView::DrawShip: sprite draw path not yet migrated (Ship=%s)"), *FString(s.Name()));
				}
				else {
					// fallback primitives:
					if (s.IsStatic()) {
						window->FillRect(shiploc.x - 6, shiploc.y - 6, shiploc.x + 6, shiploc.y + 6, s.MarkerColor());
						window->DrawRect(shiploc.x - 6, shiploc.y - 6, shiploc.x + 6, shiploc.y + 6, FColor::White);
					}
					else if (s.IsStarship()) {
						window->FillRect(shiploc.x - 4, shiploc.y - 4, shiploc.x + 4, shiploc.y + 4, s.MarkerColor());
						window->DrawRect(shiploc.x - 4, shiploc.y - 4, shiploc.x + 4, shiploc.y + 4, FColor::White);
					}
					else {
						window->FillRect(shiploc.x - 3, shiploc.y - 3, shiploc.x + 3, shiploc.y + 3, s.MarkerColor());
						window->DrawRect(shiploc.x - 3, shiploc.y - 3, shiploc.x + 3, shiploc.y + 3, FColor::White);
					}
				}

				window->Print(shiploc.x - sprite_width, shiploc.y + sprite_width + 2, s.Name());
			}
		}
	}

	// draw current ship marker:
	if (current && Text(s.GetRegion()->GetName()) == regions[current_region]->Name()) {
		x1 = (int)(shiploc.x - sprite_width - 1);
		x2 = (int)(shiploc.x + sprite_width + 1);
		y1 = (int)(shiploc.y - sprite_width - 1);
		y2 = (int)(shiploc.y + sprite_width + 1);

		window->DrawRect(x1, y1, x2, y2, FColor::White);
	}

	// only see routes for your own team:
	if (s.GetIFF() == 0 || (ship && s.GetIFF() == ship->GetIFF())) {
		DrawNavRoute(rgn, s.GetFlightPlan(), s.MarkerColor(), &s, 0);
	}
}

// +--------------------------------------------------------------------+

void
MapView::DrawElem(MissionElement& s, bool current, int rep)
{
	if (!mission) return;

	const bool visible =
		editor ||
		s.GetIFF() == 0 ||
		s.GetIFF() == mission->Team() ||
		s.IntelLevel() > Intel::KNOWN;

	if (!visible) return;

	OrbitalRegion* rgn = (OrbitalRegion*)regions[current_region];
	if (!rgn) return;

	int   x1, y1, x2, y2;
	POINT shiploc;

	const double cx = rect.w / 2.0;
	const double cy = rect.h / 2.0;

	c = (cx < cy) ? cx : cy;
	r = rgn->Radius() * zoom;

	const double scale = c / r;

	const double ox = offset_x * scale;
	const double oy = offset_y * scale;

	const double rlx = 0;
	const double rly = 0;

	int sprite_width = 10;

	window->SetFont(font);

	// draw element icon:
	if (!_stricmp(s.Region(), rgn->Name())) {
		const FVector loc = s.Location();

		const double sx = (loc.X + rlx) * scale;
		const double sy = (loc.Y + rly) * scale;

		shiploc.x = (int)(cx + sx + ox);
		shiploc.y = (int)(cy + sy + oy);

		const bool ship_visible =
			shiploc.x >= 0 && shiploc.x < rect.w &&
			shiploc.y >= 0 && shiploc.y < rect.h;

		if (ship_visible) {
			if (rep < 3) {
				window->FillRect(shiploc.x - 2, shiploc.y - 2, shiploc.x + 2, shiploc.y + 2, s.MarkerColor());
				sprite_width = 2;

				if (!IsCrowded(s))
					window->Print(shiploc.x - sprite_width, shiploc.y + sprite_width + 2, s.Name());
			}
			else {
				double theta = s.Heading();

				const double THETA_SLICE = 4.0 / PI;
				const double THETA_OFFSET = PI / 2.0;

				int sprite_index = (int)((theta + THETA_OFFSET) * THETA_SLICE);
				int nsprites = 0;

				if (s.GetDesign())
					nsprites = s.GetDesign()->map_sprites.size();

				if (nsprites > 0) {
					if (sprite_index < 0 || sprite_index >= nsprites)
						sprite_index = sprite_index % nsprites;

					// NOTE: map_sprites should be migrated from Bitmap* to UTexture2D* at the design layer.
					UE_LOG(LogTemp, VeryVerbose, TEXT("MapView::DrawElem: sprite draw path not yet migrated (Elem=%s)"), *FString(s.Name()));
				}
				else {
					theta -= PI / 2.0;

					if (s.IsStatic()) {
						window->FillRect(shiploc.x - 6, shiploc.y - 6, shiploc.x + 6, shiploc.y + 6, s.MarkerColor());
						window->DrawRect(shiploc.x - 6, shiploc.y - 6, shiploc.x + 6, shiploc.y + 6, FColor::White);
					}
					else if (s.IsStarship()) {
						window->FillRect(shiploc.x - 4, shiploc.y - 4, shiploc.x + 4, shiploc.y + 4, s.MarkerColor());
						window->DrawRect(shiploc.x - 4, shiploc.y - 4, shiploc.x + 4, shiploc.y + 4, FColor::White);
					}
					else {
						window->FillRect(shiploc.x - 3, shiploc.y - 3, shiploc.x + 3, shiploc.y + 3, s.MarkerColor());
						window->DrawRect(shiploc.x - 3, shiploc.y - 3, shiploc.x + 3, shiploc.y + 3, FColor::White);
					}
				}

				char label[64];

				if (s.Count() > 1)
					sprintf_s(label, "%s x %d", (const char*)s.Name(), s.Count());
				else
					strcpy_s(label, (const char*)s.Name());

				window->Print(shiploc.x - sprite_width, shiploc.y + sprite_width + 2, label);
			}
		}
	}

	// draw current marker:
	if (current && s.Region() == regions[current_region]->Name()) {
		x1 = (int)(shiploc.x - sprite_width - 1);
		x2 = (int)(shiploc.x + sprite_width + 1);
		y1 = (int)(shiploc.y - sprite_width - 1);
		y2 = (int)(shiploc.y + sprite_width + 1);

		window->DrawRect(x1, y1, x2, y2, FColor::White);
	}

	// only see routes for your own team:
	if (editor || s.GetIFF() == 0 || (mission && s.GetIFF() == mission->Team())) {
		DrawNavRoute(rgn, s.NavList(), s.MarkerColor(), 0, &s);
	}
}

// +--------------------------------------------------------------------+

void
MapView::DrawNavRoute(OrbitalRegion* Region,
	List<Instruction>& Route,
	FColor RouteMarker,
	Ship* InShip,
	MissionElement* InElem)
{
	int X1 = 0, Y1 = 0, X2 = 0, Y2 = 0;

	const double CX = rect.w / 2.0;
	const double CY = rect.h / 2.0;

	c = (CX < CY) ? CX : CY;
	r = system->Radius() * zoom;

	if (view_mode == VIEW_REGION) {
		if (!Region)
			return;

		r = Region->Radius() * zoom;
	}

	const double Scale = c / r;
	const double OX = offset_x * Scale;
	const double OY = offset_y * Scale;

	FVector OldLoc(0, 0, 0);
	double  OldX = 0;
	double  OldY = 0;
	bool    OldIn = false;

	FVector FirstLoc(0, 0, 0);
	int     FirstX = 0;
	int     FirstY = 0;
	bool    FirstIn = false;

	bool DrawRoute = true;
	bool DrawBold = false;

	if (InShip && InShip->GetElementIndex() > 1)
		DrawRoute = false;
	
	if (InShip && InShip == current_ship) {
		RouteMarker = (FLinearColor(RouteMarker) * 1.5f).ToFColor(true);
		DrawBold = true;
	}
	else if (InElem && InElem == current_elem) {
		RouteMarker = (FLinearColor(RouteMarker) * 1.5f).ToFColor(true);
		DrawBold = true;
	}

	for (int RouteIndex = 0; RouteIndex < Route.size(); RouteIndex++) {
		Instruction* NavPt = Route[RouteIndex];

		if (!_stricmp(NavPt->RegionName(), Region->Name())) {
			const FVector NavLoc = NavPt->Location();
			const double  NavX = NavLoc.X * Scale;
			const double  NavY = NavLoc.Y * Scale;

			const int ISX = (int)(CX + NavX + OX);
			const int ISY = (int)(CY + NavY + OY);

			if (OldIn && DrawRoute) {
				const int IOX = (int)(CX + OldX + OX);
				const int IOY = (int)(CY + OldY + OY);
				window->DrawLine(IOX, IOY, ISX, ISY, RouteMarker);

				const int DXP = (IOX - ISX);
				const int DYP = (IOY - ISY);

				if (DrawBold) {
					if (DXP > DYP)
						window->DrawLine(IOX, IOY + 1, ISX, ISY + 1, RouteMarker);
					else
						window->DrawLine(IOX + 1, IOY, ISX + 1, ISY, RouteMarker);
				}

				if ((DXP * DXP + DYP * DYP) > 2000) {
					const double Dist = (NavLoc - OldLoc).Size();

					const int IMX = (int)(CX + (OldX + NavX) / 2.0 + OX);
					const int IMY = (int)(CY + (OldY + NavY) / 2.0 + OY);

					char DistTxt[32];
					FormatNumber(DistTxt, Dist);

					font->SetColor(FColor(128, 128, 128));
					window->SetFont(font);
					window->Print(IMX - 20, IMY - 6, DistTxt);
					font->SetColor(FColor::White);
				}
			}

			X1 = ISX - 3;
			Y1 = ISY - 3;
			X2 = ISX + 3;
			Y2 = ISY + 3;

			FColor Mark = FColor::White;
			if (NavPt->Status() > Instruction::ACTIVE) {
				Mark = FColor(128, 128, 128);
			}
			else if (!FirstIn) {
				FirstIn = true;
				FirstLoc = NavLoc;
				FirstX = ISX;
				FirstY = ISY;
			}

			if (DrawRoute) {
				window->DrawLine(X1, Y1, X2, Y2, Mark);
				window->DrawLine(X1, Y2, X2, Y1, Mark);

				if (NavPt == current_navpt)
					window->DrawRect(X1 - 2, Y1 - 2, X2 + 2, Y2 + 2, Mark);

				char Buf[256];
				sprintf_s(Buf, "%d", RouteIndex + 1);

				window->SetFont(font);
				window->Print(X2 + 3, Y1, Buf);

				if (NavPt == current_navpt) {
					if (NavPt->TargetName() && strlen(NavPt->TargetName())) {
						sprintf_s(Buf, "%s %s",
							Game::GetText(Text("MapView.item.") + Instruction::ActionName(NavPt->Action())).data(),
							NavPt->TargetName());
						window->Print(X2 + 3, Y1 + 10, Buf);
					}
					else {
						sprintf_s(Buf, "%s",
							Game::GetText(Text("MapView.item.") + Instruction::ActionName(NavPt->Action())).data());
						window->Print(X2 + 3, Y1 + 10, Buf);
					}

					sprintf_s(Buf, "%s",
						Game::GetText(Text("MapView.item.") + Instruction::FormationName(NavPt->Formation())).data());
					window->Print(X2 + 3, Y1 + 20, Buf);

					sprintf_s(Buf, "%d", NavPt->Speed());
					window->Print(X2 + 3, Y1 + 30, Buf);

					if (NavPt->HoldTime()) {
						char HoldTime[32];
						FormatTime(HoldTime, NavPt->HoldTime());

						sprintf_s(Buf, "%s %s", Game::GetText("MapView.item.Hold").data(), HoldTime);
						window->Print(X2 + 3, Y1 + 40, Buf);
					}
				}
			}

			OldLoc = NavLoc;
			OldX = NavX;
			OldY = NavY;
			OldIn = true;
		}
		else {
			OldLoc = NavPt->Location();
			OldX = 0;
			OldY = 0;
			OldIn = false;
		}
	}

	// If the ship/element and the first active navpoint are both in-region,
	// draw a line from the ship/element to the first active navpoint:
	if (FirstIn) {
		OldIn = false;

		if (InShip && InShip->GetRegion()) {
			OldIn = (InShip->GetRegion()->GetOrbitalRegion() == Region);

			if (OldIn) {
				OldLoc = InShip->Location();
				OldX = OldLoc.X * Scale;
				OldY = OldLoc.Y * Scale;
			}
		}
		else if (InElem) {
			OldIn = (_stricmp(InElem->Region(), Region->Name()) == 0);

			if (OldIn) {
				OldLoc = InElem->Location();
				OldX = OldLoc.X * Scale;
				OldY = OldLoc.Y * Scale;
			}
		}

		if (OldIn) {
			const int IOX = (int)(CX + OldX + OX);
			const int IOY = (int)(CY + OldY + OY);

			window->DrawLine(IOX, IOY, FirstX, FirstY, RouteMarker);

			const int DXP = (IOX - FirstX);
			const int DYP = (IOY - FirstY);

			if (DrawBold) {
				if (DXP > DYP)
					window->DrawLine(IOX, IOY + 1, FirstX, FirstY + 1, RouteMarker);
				else
					window->DrawLine(IOX + 1, IOY, FirstX + 1, FirstY, RouteMarker);
			}

			if ((DXP * DXP + DYP * DYP) > 2000) {
				const double Dist = (FirstLoc - OldLoc).Size();

				const double NavX = FirstLoc.X * Scale;
				const double NavY = FirstLoc.Y * Scale;

				const int IMX = (int)(CX + (OldX + NavX) / 2.0 + OX);
				const int IMY = (int)(CY + (OldY + NavY) / 2.0 + OY);

				char DistTxt[32];
				FormatNumber(DistTxt, Dist);

				font->SetColor(FColor(128, 128, 128));
				window->SetFont(font);
				window->Print(IMX - 20, IMY - 6, DistTxt);
				font->SetColor(FColor::White);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
MapView::DrawCombatantSystem(Combatant* InCombatant, Orbital* InRegion, int X, int Y, int Radius)
{
	const int Team = InCombatant ? InCombatant->GetIFF() : 0;

	int X1 = 0;
	int X2 = 0;
	int Y1 = Y - Radius;
	int Align = 0;

	switch (Team) {
	case 0:
		X1 = X - 64;
		X2 = X + 64;
		Y1 = Y + Radius + 4;
		Align = DT_CENTER;
		break;

	case 1:
		X1 = X - 200;
		X2 = X - Radius - 4;
		Align = DT_RIGHT;
		break;

	default:
		X1 = X + Radius + 4;
		X2 = X + 200;
		Align = DT_LEFT;
		break;
	}

	DrawCombatGroupSystem(InCombatant ? InCombatant->GetForce() : 0, InRegion, X1, X2, Y1, Align);
}

// +--------------------------------------------------------------------+

void
MapView::DrawCombatGroupSystem(CombatGroup* group, Orbital* rgn, int x1, int x2, int& y, int a)
{
	if (!group || group->IsReserve() || group->CalcValue() < 1)
		return;

	char txt[80];
	Rect TextRect(x1, y, x2 - x1, 12);

	if (group->GetRegion() == rgn->Name()) {
		switch (group->GetType()) {
		case ECOMBATGROUP_TYPE::CARRIER_GROUP:
		case ECOMBATGROUP_TYPE::BATTLE_GROUP:
		case ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:
			sprintf_s(txt, "%s '%s'", group->GetShortDescription(), group->Name().data());
			active_window->SetFont(font);
			active_window->DrawText(txt, 0, TextRect, a);
			y += 10;
			break;

		case ECOMBATGROUP_TYPE::BATTALION:
		case ECOMBATGROUP_TYPE::STATION:
		case ECOMBATGROUP_TYPE::STARBASE:
		case ECOMBATGROUP_TYPE::MINEFIELD:
		case ECOMBATGROUP_TYPE::BATTERY:
		case ECOMBATGROUP_TYPE::MISSILE:
			active_window->SetFont(font);
			active_window->DrawText(group->GetShortDescription(), 0, TextRect, a);
			y += 10;

			break;

		default:
			break;
		}
	}

	ListIter<CombatGroup> iter = group->GetComponents();
	while (++iter) {
		CombatGroup* g = iter.value();
		DrawCombatGroupSystem(g, rgn, x1, x2, y, a);
	}
}

// +--------------------------------------------------------------------+

void
MapView::DrawCombatGroup(CombatGroup* group, int rep)
{
	// does group even exist yet?
	if (!group || group->IsReserve() || group->CalcValue() < 1)
		return;

	// is group a squadron? don't draw squadrons on map:
	if (group->GetType() >= ECOMBATGROUP_TYPE::WING && group->GetType() < ECOMBATGROUP_TYPE::FLEET)
		return;

	// has group been discovered yet?
	CombatGroup* player_group = campaign->GetPlayerGroup();
	if (group->GetIFF() && player_group && player_group->GetIFF() != group->GetIFF())
		if (group->IntelLevel() <= Intel::KNOWN)
			return;

	// has group been destroyed already?
	if (group->CalcValue() < 1)
		return;

	OrbitalRegion* rgn = (OrbitalRegion*)regions[current_region];
	if (!rgn) return;

	POINT shiploc;

	const double cx = rect.w / 2.0;
	const double cy = rect.h / 2.0;

	c = (cx < cy) ? cx : cy;
	r = rgn->Radius() * zoom;

	const double scale = c / r;

	const double ox = offset_x * scale;
	const double oy = offset_y * scale;

	const double rlx = 0;
	const double rly = 0;

	int sprite_width = 10;

	if (group->GetUnits().size() > 0) {
		CombatUnit* unit = nullptr;

		for (int i = 0; i < group->GetUnits().size(); i++) {
			unit = group->GetUnits().at(i);
			if (unit && (unit->Count() - unit->DeadCount() > 0))
				break;
		}

		if (unit && unit->GetRegion() == rgn->Name() && unit->Type() > (int) CLASSIFICATION::LCA && unit->Count() > 0) {
			const FVector uloc = unit->Location();

			const double sx = (uloc.X + rlx) * scale;
			const double sy = (uloc.Y + rly) * scale;

			shiploc.x = (int)(cx + sx + ox);
			shiploc.y = (int)(cy + sy + oy);

			const bool ship_visible =
				shiploc.x >= 0 && shiploc.x < rect.w &&
				shiploc.y >= 0 && shiploc.y < rect.h;

			if (ship_visible) {
				if (rep < 3) {
					window->FillRect(shiploc.x - 2, shiploc.y - 2, shiploc.x + 2, shiploc.y + 2, unit->MarkerColor());
					sprite_width = 2;

					char buf[256];
					sprintf_s(buf, "%s", unit->Name().data());

					window->SetFont(font);
					window->Print(shiploc.x - sprite_width, shiploc.y + sprite_width + 2, buf);
				}
				else {
					const int sprite_index = 2;

					int nsprites = 0;
					if (unit->GetDesign())
						nsprites = unit->GetDesign()->map_sprites.size();

					if (nsprites) {
						// NOTE: map_sprites should be migrated from Bitmap* to UTexture2D* at the design layer.
						UE_LOG(LogTemp, VeryVerbose, TEXT("MapView::DrawCombatGroup: sprite draw path not yet migrated (Unit=%s)"), *FString(unit->Name().data()));
					}
					else {
						if (unit->IsStatic()) {
							window->FillRect(shiploc.x - 6, shiploc.y - 6, shiploc.x + 6, shiploc.y + 6, unit->MarkerColor());
							window->DrawRect(shiploc.x - 6, shiploc.y - 6, shiploc.x + 6, shiploc.y + 6, FColor::White);
						}
						else if (unit->IsStarship()) {
							window->FillRect(shiploc.x - 4, shiploc.y - 4, shiploc.x + 4, shiploc.y + 4, unit->MarkerColor());
							window->DrawRect(shiploc.x - 4, shiploc.y - 4, shiploc.x + 4, shiploc.y + 4, FColor::White);
						}
						else {
							window->FillRect(shiploc.x - 3, shiploc.y - 3, shiploc.x + 3, shiploc.y + 3, unit->MarkerColor());
							window->DrawRect(shiploc.x - 3, shiploc.y - 3, shiploc.x + 3, shiploc.y + 3, FColor::White);
						}
					}

					char label[128];
					strcpy_s(label, unit->GetDescription());

					window->SetFont(font);
					window->Print(shiploc.x - sprite_width, shiploc.y + sprite_width + 2, label);
				}
			}
		}
	}

	// recurse:
	ListIter<CombatGroup> iter = group->GetComponents();
	while (++iter) {
		CombatGroup* g = iter.value();
		DrawCombatGroup(g, rep);
	}
}

// +--------------------------------------------------------------------+

bool MapView::IsClutter(Ship& Test)
{
	// Get leader
	Ship* Lead = Test.GetLeader();

	// This ship is the leader
	if (Lead == &Test)
	{
		return false;
	}

	// Too close to leader?
	if (Lead)
	{
		FVector TestLoc;
		FVector LeadLoc;

		GetShipLoc(Test, TestLoc);
		GetShipLoc(*Lead, LeadLoc);

		const float Dx = TestLoc.X - LeadLoc.X;
		const float Dy = TestLoc.Y - LeadLoc.Y;
		const float DistSq = Dx * Dx + Dy * Dy;

		// 8 units squared = 64
		if (DistSq <= 64.0f)
		{
			return true;
		}
	}

	return false;
}


// +--------------------------------------------------------------------+

bool
MapView::IsCrowded(Ship& test)
{
	FVector testloc, refloc;

	Sim* sim = Sim::GetSim();
	if (!sim) return false;

	Orbital* rgn = regions[current_region];
	if (!rgn) return false;

	SimRegion* simrgn = sim->FindRegion(rgn->Name());

	if (simrgn) {
		GetShipLoc(test, testloc);

		ListIter<Ship> s = simrgn->GetShips();
		while (++s) {
			Ship* ref = s.value();

			// too close?
			if (ref && ref != &test) {
				GetShipLoc(*ref, refloc);

				const double dx = (double)testloc.X - (double)refloc.X;
				const double dy = (double)testloc.Y - (double)refloc.Y;
				const double d = dx * dx + dy * dy;

				if (d <= 64)
					return true;
			}
		}
	}

	return false;
}

void
MapView::GetShipLoc(Ship& s, FVector& shiploc)
{
	const double cx = rect.w / 2.0;
	const double cy = rect.h / 2.0;

	c = (cx < cy) ? cx : cy;
	r = system->Radius() * zoom;

	OrbitalRegion* rgn = (OrbitalRegion*)regions[current_region];

	if (view_mode == VIEW_REGION) {
		if (!rgn) return;
		r = rgn->Radius() * zoom;
	}

	const double scale = c / r;

	const double ox = offset_x * scale;
	const double oy = offset_y * scale;

	double rlx = 0;
	double rly = 0;

	if (view_mode == VIEW_SYSTEM) {
		rgn = system->ActiveRegion();

		if (rgn) {
			rlx = rgn->Location().X;
			rly = rgn->Location().Y;
		}
	}

	if (view_mode == VIEW_SYSTEM ||
		(view_mode == VIEW_REGION && rgn == s.GetRegion()->GetOrbitalRegion())) {

		// NOTE: Original used handedness conversion elsewhere. Ensure Ship::Location() matches your map convention.
		const FVector loc = s.Location();

		const double sx = (loc.X + rlx) * scale;
		const double sy = (loc.Y + rly) * scale;

		shiploc.X = (int)(cx + sx + ox);
		shiploc.Y = (int)(cy + sy + oy);
	}
	else {
		shiploc.X = -1;
		shiploc.Y = -1;
	}
}

// +--------------------------------------------------------------------+

bool
MapView::IsCrowded(MissionElement& test)
{
	FVector testloc, refloc;

	Orbital* rgn = regions[current_region];
	if (!rgn) return false;

	if (mission) {
		GetElemLoc(test, testloc);

		ListIter<MissionElement> s = mission->GetElements();
		while (++s) {
			MissionElement* ref = s.value();

			if (ref && ref != &test && !_stricmp(ref->Region(), rgn->Name())) {
				GetElemLoc(*ref, refloc);

				const double dx = (double)testloc.X - (double)refloc.X;
				const double dy = (double)testloc.Y - (double)refloc.Y;
				const double d = dx * dx + dy * dy;

				if (d <= 64)
					return true;
			}
		}
	}

	return false;
}

void
MapView::GetElemLoc(MissionElement& s, FVector& shiploc)
{
	const double cx = rect.w / 2.0;
	const double cy = rect.h / 2.0;

	c = (cx < cy) ? cx : cy;
	r = system->Radius() * zoom;

	OrbitalRegion* rgn = (OrbitalRegion*)regions[current_region];

	if (view_mode == VIEW_REGION) {
		if (!rgn) return;
		r = rgn->Radius() * zoom;
	}

	const double scale = c / r;

	const double ox = offset_x * scale;
	const double oy = offset_y * scale;

	double rlx = 0;
	double rly = 0;

	if (view_mode == VIEW_SYSTEM) {
		rgn = system->ActiveRegion();

		if (rgn) {
			rlx = rgn->Location().X;
			rly = rgn->Location().Y;
		}
	}

	if (view_mode == VIEW_SYSTEM ||
		(view_mode == VIEW_REGION && !_stricmp(s.Region(), rgn->Name()))) {

		const FVector loc = s.Location();

		const double sx = (loc.X + rlx) * scale;
		const double sy = (loc.Y + rly) * scale;

		shiploc.X = (int)(cx + sx + ox);
		shiploc.Y = (int)(cy + sy + oy);
	}
	else {
		shiploc.X = -1;
		shiploc.Y = -1;
	}
}

// +--------------------------------------------------------------------+

OrbitalRegion*
MapView::GetRegion() const
{
	OrbitalRegion* result = nullptr;

	if (current_region < regions.size())
		result = (OrbitalRegion*)regions[current_region];

	return result;
}

// +--------------------------------------------------------------------+

void
MapView::ZoomIn()
{
	zoom *= 0.9;

	if (view_mode == VIEW_SYSTEM) {
		if (system && zoom * system->Radius() < 2e6) {
			zoom = 2e6 / system->Radius();
		}
	}
	else if (view_mode == VIEW_REGION) {
		OrbitalRegion* rgn = GetRegion();
		if (rgn && zoom * rgn->Radius() < 1e3) {
			zoom = 1e3 / rgn->Radius();
		}
	}
}

void
MapView::ZoomOut()
{
	zoom *= 1.1;

	if (view_mode == VIEW_SYSTEM) {
		if (system && zoom * system->Radius() > 500e9) {
			zoom = 500e9 / system->Radius();
		}
	}
	else if (view_mode == VIEW_REGION) {
		OrbitalRegion* rgn = GetRegion();
		if (rgn && zoom * rgn->Radius() > 1e6) {
			zoom = 1e6 / rgn->Radius();
		}
	}
}

// +--------------------------------------------------------------------+

void
MapView::OnShow()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
		dispatch->Register(this);
}

void
MapView::OnHide()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
		dispatch->Unregister(this);

	if (captured) {
		ReleaseCapture();
		captured = false;
		Mouse::Show(true);
	}

	dragging = false;
}

// +--------------------------------------------------------------------+

bool
MapView::IsEnabled() const
{
	if (active_window)
		return active_window->IsEnabled();

	return false;
}

bool
MapView::IsVisible() const
{
	if (active_window)
		return active_window->IsVisible();

	return false;
}

bool
MapView::IsFormActive() const
{
	if (active_window)
		return active_window->IsFormActive();

	return false;
}

Rect
MapView::TargetRect() const
{
	if (active_window)
		return active_window->TargetRect();

	return Rect();
}

// +--------------------------------------------------------------------+

int
MapView::OnMouseMove(int x, int y)
{
	if (captured) {
		EventTarget* test = nullptr;
		EventDispatch* dispatch = EventDispatch::GetInstance();
		if (dispatch)
			test = dispatch->GetCapture();

		if (test != this) {
			captured = false;
			Mouse::Show(true);
		}
		else {
			if (dragging) {
				const int delta_x = x - mouse_x;
				const int delta_y = y - mouse_y;

				offset_x += delta_x * r / c;
				offset_y += delta_y * r / c;

				Mouse::SetCursorPos(mouse_x, mouse_y);
			}
			else if (view_mode == VIEW_REGION) {
				const double scale = r / c;

				click_x = (x - rect.x - rect.w / 2) * scale - offset_x;
				click_y = (y - rect.y - rect.h / 2) * scale - offset_y;

				if ((adding_navpt || moving_navpt) && current_navpt) {
					FVector loc = current_navpt->Location();
					loc.X = (float)click_x;
					loc.Y = (float)click_y;

					current_navpt->SetLocation(loc);
					current_navpt->SetStatus(current_status);
				}
				else if (editor && moving_elem && current_elem) {
					FVector loc = current_elem->Location();
					loc.X = (float)click_x;
					loc.Y = (float)click_y;

					current_elem->SetLocation(loc);
				}
			}
		}
	}

	return active_window->OnMouseMove(x, y);
}

// +--------------------------------------------------------------------+

int
MapView::OnRButtonDown(int x, int y)
{
	if (!captured)
		captured = SetCapture();

	if (captured) {
		dragging = true;
		mouse_x = x;
		mouse_y = y;
		Mouse::Show(false);
	}

	return active_window->OnRButtonDown(x, y);
}

// +--------------------------------------------------------------------+

int
MapView::OnRButtonUp(int x, int y)
{
	if (captured) {
		ReleaseCapture();
		captured = false;
		Mouse::Show(true);
	}

	dragging = false;

	return active_window->OnRButtonUp(x, y);
}

// +--------------------------------------------------------------------+

int
MapView::OnClick()
{
	return active_window->OnClick();
}

int
MapView::OnLButtonDown(int x, int y)
{
	if (menu_view && menu_view->IsShown()) {
		// ignore this event...
	}
	else {
		if (!captured)
			captured = SetCapture();

		if (view_mode == VIEW_REGION) {
			const double scale = r / c;

			click_x = (x - rect.x - rect.w / 2) * scale - offset_x;
			click_y = (y - rect.y - rect.h / 2) * scale - offset_y;

			if (current_navpt) {
				const FVector nloc = current_navpt->Location();

				const double dx = nloc.X - click_x;
				const double dy = nloc.Y - click_y;
				const double d = FMath::Sqrt(dx * dx + dy * dy);

				if (d < 5e3) {
					moving_navpt = true;

					if (ship && current_ship && ship != current_ship) {
						if (ship->GetElement() && current_ship->GetElement()) {
							if (!ship->GetElement()->CanCommand(current_ship->GetElement())) {
								moving_navpt = false;
							}
						}
					}
				}
			}

			else if (editor && current_elem) {
				const FVector nloc = current_elem->Location();

				const double dx = nloc.X - click_x;
				const double dy = nloc.Y - click_y;
				const double d = FMath::Sqrt(dx * dx + dy * dy);

				if (d < 5e3) {
					moving_elem = true;
				}
			}
		}
	}

	return active_window->OnLButtonDown(x, y);
}

int
MapView::OnLButtonUp(int x, int y)
{
	bool process_event = false;

	if (captured) {
		process_event = true;
		ReleaseCapture();
		captured = false;
		Mouse::Show(true);
	}

	if (process_event && !adding_navpt) {
		if (!moving_navpt && !moving_elem) {
			UIButton::PlaySound();
		}

		if (view_mode == VIEW_REGION) {
			const double scale = r / c;

			click_x = (x - rect.x - rect.w / 2) * scale - offset_x;
			click_y = (y - rect.y - rect.h / 2) * scale - offset_y;

			if (!scrolling)
				SelectAt(x, y);

			active_window->ClientEvent(EID_MAP_CLICK, x, y);
		}
		else if (!scrolling) {
			SelectAt(x, y);
			active_window->ClientEvent(EID_MAP_CLICK, x, y);
		}
	}

	if ((adding_navpt || moving_navpt) && current_navpt) {
		current_navpt->SetStatus(current_status);

		Sim* sim = Sim::GetSim();
		if (sim) {
			Ship* s = current_ship;

			if (s && s->GetElement()) {
				SimElement* elem = s->GetElement();
				const int index = elem->GetNavIndex(current_navpt);
			}
		}
	}

	adding_navpt = false;
	moving_navpt = false;
	moving_elem = false;

	return active_window->OnLButtonUp(x, y);
}




