/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (c) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CameraManager.cpp
	AUTHOR:       Carlos Bott

	ORIGINAL AUTHOR AND STUDIO:
	John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Camera Manager singleton manages the main view camera based on the current ship
	(Renamed from CameraDirector to avoid Unreal Engine filename collisions)
*/

#include "CameraManager.h"

#include "Ship.h"
#include "FlightDeck.h"
#include "SimContact.h"
#include "Sim.h"
#include "StarSystem.h"
#include "Terrain.h"
#include "HUDView.h"
#include "DetailSet.h"
#include "Starshatter.h"
#include "Game.h"

// Minimal Unreal types used by this implementation:
#include "Math/UnrealMathUtility.h"   // FMath::Sin/Cos/Abs, PI
#include "Math/Vector.h"             // FVector

// +----------------------------------------------------------------------+

CameraManager* CameraManager::instance = 0;

static double range_max_limit = 300e3;

// +----------------------------------------------------------------------+

CameraManager*
CameraManager::GetInstance()
{
	if (!instance)
		instance = new CameraManager;

	return instance;
}

// +----------------------------------------------------------------------+

CameraManager::CameraManager()
	: mode(MODE_COCKPIT),
	requested_mode(MODE_NONE),
	old_mode(MODE_NONE),
	camera(),
	ship(0),
	region(0),
	external_body(0),
	external_ship(0),
	external_point(FVector::ZeroVector),
	base_loc(FVector::ZeroVector),
	virt_az(0),
	virt_el(0),
	virt_x(0),
	virt_y(0),
	virt_z(0),
	azimuth(PI / 4),
	elevation(PI / 4),
	az_rate(0),
	el_rate(0),
	range_rate(0),
	range(0),
	range_min(100),
	range_max(range_max_limit),
	base_range(0),
	transition(0),
	sim(0),
	hud(0)
{
	instance = this;
}

// +--------------------------------------------------------------------+

CameraManager::~CameraManager()
{
	if (instance == this)
		instance = 0;
}

// +--------------------------------------------------------------------+

void
CameraManager::Reset()
{
	mode = MODE_COCKPIT;
	requested_mode = MODE_NONE;
	old_mode = MODE_NONE;

	sim = 0;
	ship = 0;
	region = 0;
	external_ship = 0;
	external_body = 0;
	transition = 0;
	hud = 0;
}

// +--------------------------------------------------------------------+

void
CameraManager::SetShip(Ship* s)
{
	sim = Sim::GetSim();
	hud = HUDView::GetInstance();

	// can't take control of a dead ship:
	if (s && (s->Life() == 0 || s->IsDying() || s->IsDead()))
		return;

	// leaving the old ship, so make sure it is visible:
	if (ship && ship != s) {
		ship->ShowRep();
		ship->HideCockpit();
	}

	// taking control of the new ship:
	ship = s;

	if (ship) {
		Observe(ship);
		region = ship->GetRegion();

		if (sim && ship->GetRegion() != sim->GetActiveRegion())
			sim->ActivateRegion(ship->GetRegion());

		range = ship->Radius() * 4;

		if (mode == MODE_COCKPIT)
			mode = MODE_CHASE;

		SetMode(MODE_COCKPIT);
		ExecFrame(0);
	}
}

// +--------------------------------------------------------------------+

void
CameraManager::SetMode(int m, double t)
{
	if (requested_mode == m)
		return;

	external_point = FVector::ZeroVector;

	// save current mode for after transition:
	if (m == MODE_DROP && mode != MODE_DROP)
		old_mode = mode;

	// if manually leaving drop mode, forget about
	// restoring the previous mode when the drop
	// expires...
	else if (m != MODE_DROP && mode == MODE_DROP)
		old_mode = MODE_NONE;

	if (m == MODE_VIRTUAL && ship && !ship->Cockpit())
		return;

	if (mode == m) {
		if (mode == MODE_TARGET || mode == MODE_ORBIT)
			CycleViewObject();

		return;
	}

	if (m > MODE_NONE && m < MODE_LAST) {
		if (m <= MODE_VIRTUAL) {
			requested_mode = m;
			transition = t;
			external_ship = 0;
			ClearGroup();

			// no easy way to do a smooth transition between
			// certain modes, so just go immediately:
			if ((mode == MODE_TARGET && m == MODE_CHASE) ||
				(mode == MODE_COCKPIT && m == MODE_VIRTUAL) ||
				(mode == MODE_VIRTUAL && m == MODE_COCKPIT))
			{
				mode = m;
				requested_mode = 0;
				transition = 0;
			}
		}

		else if (m == MODE_TRANSLATE || m == MODE_ZOOM) {
			requested_mode = m;
			transition = t;
			base_range = range;
		}

		else if (m >= MODE_DOCKING) {
			mode = m;
			transition = 0;
			external_ship = 0;
			ClearGroup();
		}

		virt_az = 0;
		virt_el = 0;
		virt_x = 0;
		virt_y = 0;
		virt_z = 0;
	}
}

// +--------------------------------------------------------------------+

static const char* get_camera_mode_name(int m)
{
	switch (m) {
	default:
	case CameraManager::MODE_NONE:      return "";           break;
	case CameraManager::MODE_COCKPIT:   return "";           break;
	case CameraManager::MODE_CHASE:     return "Chase Cam";  break;
	case CameraManager::MODE_TARGET:    return "Padlock";    break;
	case CameraManager::MODE_THREAT:    return "Threatlock"; break;
	case CameraManager::MODE_VIRTUAL:   return "Virtual";    break;
	case CameraManager::MODE_ORBIT:
	case CameraManager::MODE_TRANSLATE:
	case CameraManager::MODE_ZOOM:      return "Orbit Cam";  break;
	case CameraManager::MODE_DOCKING:   return "Dock Cam";   break;
	case CameraManager::MODE_DROP:      return "";           break;
	}
}

const char*
CameraManager::GetModeName()
{
	int m = GetCameraMode();

	if (m != CameraManager::MODE_VIRTUAL) {
		return get_camera_mode_name(m);
	}
	else if (instance) {
		if (instance->virt_az > 30 * DEGREES && instance->virt_az < 100 * DEGREES)
			return "RIGHT";

		else if (instance->virt_az >= 100 * DEGREES)
			return "RIGHT-AFT";

		else if (instance->virt_az < -30 * DEGREES && instance->virt_az > -100 * DEGREES)
			return "LEFT";

		else if (instance->virt_az <= -100 * DEGREES)
			return "LEFT-AFT";

		else if (instance->virt_el > 15 * DEGREES)
			return "UP";

		else if (instance->virt_el < -15 * DEGREES)
			return "DOWN";

		return get_camera_mode_name(m);
	}

	return "";
}

int
CameraManager::GetCameraMode()
{
	if (instance) {
		int op_mode = instance->mode;
		if (instance->requested_mode > instance->mode)
			op_mode = instance->requested_mode;
		return op_mode;
	}

	return 0;
}

void
CameraManager::SetCameraMode(int m, double t)
{
	if (instance)
		instance->SetMode(m, t);
}

// +--------------------------------------------------------------------+

double
CameraManager::GetRangeLimit()
{
	return range_max_limit;
}

void
CameraManager::SetRangeLimit(double r)
{
	if (r >= 1e3)
		range_max_limit = r;
}

void
CameraManager::SetRangeLimits(double min, double max)
{
	if (instance) {
		instance->range_min = min;
		instance->range_max = max;
	}
}

// +--------------------------------------------------------------------+

void
CameraManager::ClearGroup()
{
	external_group.clear();
}

// +--------------------------------------------------------------------+

void
CameraManager::CycleViewObject()
{
	if (!ship) return;

	Ship* current = external_ship;
	external_ship = 0;

	ListIter<SimContact> iter = ship->ContactList();
	while (++iter && !external_ship) {
		SimContact* c = iter.value();
		Ship* c_ship = c->GetShip();

		if (c_ship && !current) {
			external_ship = c_ship;
		}

		else if (current && c_ship == current) {
			while (++iter && !external_ship) {
				c = iter.value();
				if (c->ActLock())
					external_ship = c->GetShip();
			}
		}
	}

	if (external_ship != current) {
		if (external_ship) {
			if (external_ship->Life() == 0 || external_ship->IsDying() || external_ship->IsDead()) {
				const Point loc = external_ship->Location();
				external_point = FVector((float)loc.X, (float)loc.Y, (float)loc.Z);
				external_ship = 0;
			}
			else {
				Observe(external_ship);
			}
		}

		if (mode == MODE_ORBIT) {
			SetMode(MODE_TRANSLATE);
			ExternalRange(1);
		}
	}
}

// +--------------------------------------------------------------------+

void
CameraManager::SetViewOrbital(Orbital* orb)
{
	external_body = orb;

	if (external_body) {
		range_min = external_body->Radius() * 2.5;
		ClearGroup();
		external_ship = 0;

		if (sim) {
			region = sim->FindNearestSpaceRegion(orb);
			if (region)
				sim->ActivateRegion(region);
		}

		if (ship && !region)
			region = ship->GetRegion();
	}
}

// +--------------------------------------------------------------------+

void
CameraManager::SetViewObject(Ship* obj, bool quick)
{
	if (!ship) return;

	if (!obj) {
		obj = ship;
		region = ship->GetRegion();
	}

	external_body = 0;
	external_point = FVector::ZeroVector;

	Starshatter* stars = Starshatter::GetInstance();

	if (obj->GetIFF() != ship->GetIFF() && !stars->InCutscene()) {
		// only view solid contacts:
		SimContact* c = ship->FindContact(obj);
		if (!c || !c->ActLock())
			return;
	}

	if (mode == MODE_TARGET) {
		ClearGroup();
		if (external_ship) {
			external_ship = 0;
		}
	}
	else if (mode >= MODE_ORBIT) {
		if (quick) {
			mode = MODE_ORBIT;
			transition = 0;
		}
		else {
			SetMode(MODE_TRANSLATE);
		}

		if (external_group.size()) {
			ClearGroup();

			if (external_ship) {
				external_ship = 0;
			}
		}

		else {
			if ((obj == external_ship) || (obj == ship && external_ship == 0)) {
				if (!quick)
					SetMode(MODE_ZOOM);
			}

			else if (external_ship) {
				external_ship = 0;
			}
		}
	}

	if (external_ship != obj) {
		external_ship = obj;

		if (external_ship) {
			region = external_ship->GetRegion();

			if (external_ship->Life() == 0 || external_ship->IsDying() || external_ship->IsDead()) {
				external_ship = 0;
				range_min = 100;
			}
			else {
				Observe(external_ship);

				if (sim)
					sim->ActivateRegion(external_ship->GetRegion());

				range_min = external_ship->Radius() * 1.5;
			}
		}

		Observe(external_ship);
		ExternalRange(1);
	}
}

// +--------------------------------------------------------------------+

void
CameraManager::SetViewObjectGroup(ListIter<Ship> group, bool quick)
{
	if (!ship) return;

	Starshatter* stars = Starshatter::GetInstance();

	if (!stars->InCutscene()) {
		// only view solid contacts:
		while (++group) {
			Ship* s = group.value();

			if (s->GetIFF() != ship->GetIFF()) {
				SimContact* c = ship->FindContact(s);
				if (!c || !c->ActLock())
					return;
			}

			if (s->Life() == 0 || s->IsDying() || s->IsDead())
				return;
		}
	}

	group.reset();

	if (external_group.size() > 1 &&
		external_group.size() == group.size()) {

		bool same = true;

		for (int i = 0; same && i < external_group.size(); i++) {
			if (external_group[i] != group.container()[i])
				same = false;
		}

		if (same) {
			SetMode(MODE_ZOOM);
			return;
		}
	}

	ClearGroup();

	if (quick) {
		mode = MODE_ORBIT;
		transition = 0;
	}
	else {
		SetMode(MODE_TRANSLATE);
	}

	external_group.append(group.container());

	ListIter<Ship> iter = external_group;
	while (++iter) {
		Ship* s = iter.value();
		region = s->GetRegion();
		Observe(s);
	}
}

// +--------------------------------------------------------------------+

void
CameraManager::VirtualHead(double az, double el)
{
	if (mode == MODE_VIRTUAL || mode == MODE_TARGET || mode == MODE_COCKPIT) {
		const double alimit = 3 * PI / 4;
		const double e_lo = PI / 8;
		const double e_hi = PI / 3;
		const double escale = e_hi;

		virt_az = az * alimit;
		virt_el = el * escale;

		if (virt_az > alimit)
			virt_az = alimit;
		else if (virt_az < -alimit)
			virt_az = -alimit;

		if (virt_el > e_hi)
			virt_el = e_hi;
		else if (virt_el < -e_lo)
			virt_el = -e_lo;
	}
}

void
CameraManager::VirtualHeadOffset(double x, double y, double z)
{
	if (mode == MODE_VIRTUAL || mode == MODE_TARGET || mode == MODE_COCKPIT) {
		virt_x = x;
		virt_y = y;
		virt_z = z;
	}
}

void
CameraManager::VirtualAzimuth(double delta)
{
	if (mode == MODE_VIRTUAL || mode == MODE_TARGET || mode == MODE_COCKPIT) {
		virt_az += delta;

		const double alimit = 3 * PI / 4;

		if (virt_az > alimit)
			virt_az = alimit;
		else if (virt_az < -alimit)
			virt_az = -alimit;
	}
}

void
CameraManager::VirtualElevation(double delta)
{
	if (mode == MODE_VIRTUAL || mode == MODE_TARGET || mode == MODE_COCKPIT) {
		virt_el += delta;

		const double e_lo = PI / 8;
		const double e_hi = PI / 3;

		if (virt_el > e_hi)
			virt_el = e_hi;
		else if (virt_el < -e_lo)
			virt_el = -e_lo;
	}
}

// +--------------------------------------------------------------------+

void
CameraManager::ExternalAzimuth(double delta)
{
	azimuth += delta;

	if (azimuth > PI)
		azimuth = -2 * PI + azimuth;

	else if (azimuth < -PI)
		azimuth = 2 * PI + azimuth;
}

void
CameraManager::ExternalElevation(double delta)
{
	elevation += delta;

	const double limit = (0.43 * PI);

	if (elevation > limit)
		elevation = limit;
	else if (elevation < -limit)
		elevation = -limit;
}

void
CameraManager::ExternalRange(double delta)
{
	range *= delta;

	if (ship && ship->IsAirborne())
		range_max = 30e3;
	else
		range_max = range_max_limit;

	if (range < range_min)
		range = range_min;
	else if (range > range_max)
		range = range_max;
}

// +--------------------------------------------------------------------+

void
CameraManager::SetOrbitPoint(double a, double e, double r)
{
	azimuth = a;
	elevation = e;
	range = r;

	if (external_body) {
		if (range < external_body->Radius() * 2)
			range = external_body->Radius() * 2;
		else if (range > external_body->Radius() * 6)
			range = external_body->Radius() * 6;
	}
	else {
		if (range < range_min)
			range = range_min;
		else if (range > range_max)
			range = range_max;
	}
}

void
CameraManager::SetOrbitRates(double ar, double er, double rr)
{
	az_rate = ar;
	el_rate = er;
	range_rate = rr;
}

// +--------------------------------------------------------------------+

bool
CameraManager::Update(SimObject* obj)
{
	if (obj->Type() == SimObject::SIM_SHIP) {
		Ship* s = (Ship*)obj;
		if (ship == s)
			ship = 0;

		if (external_ship == s) {
			const Point loc = s->Location();
			external_point = FVector((float)loc.x, (float)loc.y, (float)loc.z);
			external_ship = 0;
		}

		if (external_group.contains(s))
			external_group.remove(s);
	}

	return SimObserver::Update(obj);
}

const char*
CameraManager::GetObserverName() const
{
	return "CameraManager";
}

// +--------------------------------------------------------------------+

void
CameraManager::ExecFrame(double seconds)
{
	if (!ship)
		return;

	hud = HUDView::GetInstance();

	int flight_phase = ship->GetFlightPhase();

	if (flight_phase < Ship::LOCKED)
		SetMode(MODE_DOCKING);

	if (ship->IsAirborne()) {
		if (flight_phase >= Ship::DOCKING)
			SetMode(MODE_DOCKING);
	}
	else {
		if (flight_phase >= Ship::RECOVERY)
			SetMode(MODE_DOCKING);
	}

	if (flight_phase >= Ship::LOCKED && flight_phase < Ship::ACTIVE) {
		int m = GetMode();
		if (m != MODE_COCKPIT && m != MODE_VIRTUAL)
			SetMode(MODE_COCKPIT);
	}

	if (ship->InTransition()) {
		SetMode(MODE_DROP);
	}
	// automatically restore mode after transition:
	else if (old_mode != MODE_NONE) {
		mode = old_mode;
		requested_mode = old_mode;
		old_mode = MODE_NONE;
	}

	int op_mode = mode;
	if (requested_mode > mode)
		op_mode = requested_mode;

	Starshatter* stars = Starshatter::GetInstance();

	// if we are in padlock, and have not locked a ship
	// try to padlock the current target:
	if (op_mode == MODE_TARGET && !external_ship) {
		SimObject* tgt = ship->GetTarget();
		if (tgt && tgt->Type() == SimObject::SIM_SHIP)
			SetViewObject((Ship*)tgt);
	}

	// if in an external mode, check the external ship:
	else if (op_mode >= MODE_TARGET && op_mode <= MODE_ZOOM) {
		if (external_ship && external_ship != ship && !stars->InCutscene()) {
			Contact* c = ship->FindContact(external_ship);
			if (!c || !c->ActLock()) {
				SetViewObject(ship);
			}
		}
	}

	if (ship->Rep()) {
		if (op_mode == MODE_COCKPIT) {
			ship->HideRep();
			ship->HideCockpit();
		}
		else if (op_mode == MODE_VIRTUAL || op_mode == MODE_TARGET) {
			if (ship->Cockpit()) {
				ship->HideRep();
				ship->ShowCockpit();
			}
			else {
				ship->ShowRep();
			}
		}
		else {
			ship->Rep()->SetForeground(op_mode == MODE_DOCKING);
			ship->ShowRep();
			ship->HideCockpit();
		}
	}

	if (hud && hud->Ambient() != Color::Black)
		sim->GetScene()->SetAmbient(hud->Ambient());
	else
		sim->GetScene()->SetAmbient(sim->GetStarSystem()->Ambient());

	switch (op_mode) {
	default:
	case MODE_COCKPIT:      Cockpit(seconds);    break;
	case MODE_CHASE:        Chase(seconds);      break;
	case MODE_TARGET:       Target(seconds);     break;
	case MODE_THREAT:       Threat(seconds);     break;
	case MODE_VIRTUAL:      Virtual(seconds);    break;
	case MODE_ORBIT:
	case MODE_TRANSLATE:
	case MODE_ZOOM:         Orbit(seconds);      break;
	case MODE_DOCKING:      Docking(seconds);    break;
	case MODE_DROP:         Drop(seconds);       break;
	}

	if (ship->Shake() > 0 &&
		(op_mode < MODE_ORBIT ||
			(op_mode == MODE_VIRTUAL && ship->Cockpit()))) {

		Point vib = ship->Vibration() * 0.2;
		camera.MoveBy(vib);
		camera.Aim(0, vib.y, vib.z);
	}

	Transition(seconds);

	{
		const Point cam = camera.Pos();
		DetailSet::SetReference(region, FVector((float)cam.x, (float)cam.y, (float)cam.z));
	}
}

// +--------------------------------------------------------------------+

void
CameraManager::Transition(double seconds)
{
	if (transition > 0)
		transition -= seconds * 1.5;

	if (transition <= 0) {
		transition = 0;

		if (requested_mode && requested_mode != mode)
			mode = requested_mode;

		requested_mode = 0;

		if (mode == MODE_TRANSLATE || mode == MODE_ZOOM) {
			if (mode == MODE_ZOOM)
				range = range_min;

			mode = MODE_ORBIT;
		}
	}
}

// +--------------------------------------------------------------------+

bool
CameraManager::IsViewCentered()
{
	if (instance) {
		double fvaz = FMath::Abs(instance->virt_az);
		double fvel = FMath::Abs(instance->virt_el);

		return fvaz < 15 * DEGREES && fvel < 15 * DEGREES;
	}

	return true;
}

// +--------------------------------------------------------------------+

void
CameraManager::Cockpit(double seconds)
{
	camera.Clone(ship->Cam());

	Point bridge = ship->BridgeLocation();
	Point cpos = camera.Pos() +
		camera.vrt() * bridge.x +
		camera.vpn() * bridge.y +
		camera.vup() * bridge.z;

	camera.MoveTo(cpos);
}

// +--------------------------------------------------------------------+

void
CameraManager::Virtual(double seconds)
{
	camera.Clone(ship->Cam());

	Point bridge = ship->BridgeLocation();
	Point cpos = camera.Pos() +
		camera.vrt() * (bridge.x + virt_x) +
		camera.vpn() * (bridge.y + virt_z) +
		camera.vup() * (bridge.z + virt_y);

	camera.MoveTo(cpos);

	camera.Yaw(virt_az);
	camera.Pitch(-virt_el);

	double fvaz = FMath::Abs(virt_az);
	double fvel = FMath::Abs(virt_el);

	if (fvaz > 0.01 * DEGREES && fvaz < 15 * DEGREES) {
		double bleed = fvaz * 2;

		if (virt_az > 0)
			virt_az -= bleed * seconds;
		else
			virt_az += bleed * seconds;
	}

	if (fvel > 0.01 * DEGREES && fvel < 15 * DEGREES) {
		double bleed = fvel;

		if (virt_el > 0)
			virt_el -= bleed * seconds;
		else
			virt_el += bleed * seconds;
	}
}

// +--------------------------------------------------------------------+

void
CameraManager::Chase(double seconds)
{
	double step = 1;

	if (requested_mode == MODE_COCKPIT)
		step = transition;

	else if (requested_mode == MODE_CHASE)
		step = 1 - transition;

	camera.Clone(ship->Cam());
	Point velocity = camera.vpn();

	if (ship->Velocity().Length() > 10) {
		velocity = ship->Velocity();
		velocity.Normalize();
		velocity *= 0.25;
		velocity += camera.vpn() * 2;
		velocity.Normalize();
	}

	Point chase = ship->ChaseLocation();
	Point bridge = ship->BridgeLocation();
	Point cpos = camera.Pos() +
		camera.vrt() * bridge.X * (1 - step) +
		camera.vpn() * bridge.Y * (1 - step) +
		camera.vup() * bridge.Z * (1 - step) +
		velocity * chase.y * step +
		camera.vup() * chase.z * step;

	camera.MoveTo(cpos);
}

// +--------------------------------------------------------------------+

void
CameraManager::Target(double seconds)
{
	FVector target_loc = Point(external_point.X, external_point.Y, external_point.Z);

	if (external_ship)
		target_loc = external_ship->Location();

	if (!external_ship || external_ship == ship) {
		if (external_point.IsZero()) {
			if (ship->Cockpit())
				Virtual(seconds);
			else
				Orbit(seconds);

			return;
		}
	}

	double step = 1;

	if (requested_mode == MODE_COCKPIT)
		step = transition;

	else if (requested_mode == MODE_TARGET)
		step = 1 - transition;

	if (ship->Cockpit()) {
		// internal padlock:
		Cockpit(seconds);
		camera.Padlock(target_loc, 3 * PI / 4, PI / 8, PI / 3);
	}
	else {
		// external padlock:
		FVector delta = target_loc - ship->Location();
		delta.Normalize();
		delta *= -5 * ship->Radius() * step;
		delta.y += ship->Radius() * step;

		camera.MoveTo(ship->Location() + delta);
		camera.LookAt(target_loc);
	}
}

// +--------------------------------------------------------------------+

void
CameraManager::Threat(double seconds)
{
	Chase(seconds);
}

// +--------------------------------------------------------------------+

void
CameraManager::Orbit(double seconds)
{
	Point cpos = ship->Location();
	int   op_mode = GetCameraMode();

	if (seconds < 0)        seconds = 0;
	else if (seconds > 0.2) seconds = 0.2;

	// auto rotate
	azimuth += az_rate * seconds;
	elevation += el_rate * seconds;
	range *= 1 + range_rate * seconds;

	if (external_body && external_body->Rep()) {
		range_min = external_body->Radius() * 2.5;
		cpos = external_body->Rep()->Location();
	}

	else if (external_group.size()) {
		Point neg(1e9, 1e9, 1e9);
		Point pos(-1e9, -1e9, -1e9);

		ListIter<Ship> iter(external_group);
		while (++iter) {
			Point loc = iter->Location();

			if (loc.x < neg.x) neg.x = loc.x;
			if (loc.x > pos.x) pos.x = loc.x;
			if (loc.y < neg.y) neg.y = loc.y;
			if (loc.y > pos.y) pos.y = loc.y;
			if (loc.z < neg.z) neg.z = loc.z;
			if (loc.z > pos.z) pos.z = loc.z;
		}

		double dx = pos.x - neg.x;
		double dy = pos.y - neg.y;
		double dz = pos.z - neg.z;

		if (dx > dy) {
			if (dx > dz)   range_min = dx * 1.2;
			else           range_min = dz * 1.2;
		}
		else {
			if (dy > dz)   range_min = dy * 1.2;
			else           range_min = dz * 1.2;
		}

		// focus on median location:
		cpos = neg + Point(dx / 2, dy / 2, dz / 2);
	}
	else if (external_ship) {
		range_min = external_ship->Radius() * 1.5;
		cpos = external_ship->Location();
	}
	else {
		range_min = ship->Radius() * 1.5;
		cpos = ship->Location();
	}

	if (range < range_min)
		range = range_min;

	double er = range;
	double az = azimuth;
	double el = elevation;

	if (requested_mode == MODE_TRANSLATE) {
		const FVector cpos_v((float)cpos.x, (float)cpos.y, (float)cpos.z);
		const FVector lerp = cpos_v * (float)(1 - transition) + base_loc * (float)(transition);
		cpos = Point(lerp.X, lerp.Y, lerp.Z);
	}

	else if (requested_mode == MODE_ZOOM) {
		er = base_range * transition;

		if (er < range_min) {
			er = range_min;
			range = range_min;
			transition = 0;
			requested_mode = 0;
		}
	}

	// transitions:
	else if (mode < MODE_ORBIT || requested_mode > 0 && requested_mode < MODE_ORBIT) {
		double az0 = ship->CompassHeading();
		if (FMath::Abs(az - az0) > PI) az0 -= 2 * PI;

		double r0 = 0;
		double z0 = 0;

		if (mode == MODE_CHASE || requested_mode == MODE_CHASE ||
			mode == MODE_TARGET || requested_mode == MODE_TARGET) {
			r0 = ship->ChaseLocation().Length();
			z0 = 20 * DEGREES;
		}

		// pull out:
		if (mode < MODE_ORBIT) {
			er *= (1 - transition);
			az *= (1 - transition);
			el *= (1 - transition);

			er += r0 * transition;
			az += az0 * transition;
			el += z0 * transition;
		}

		// push in:
		else {
			er *= transition;
			az *= transition;
			el *= transition;

			er += r0 * (1 - transition);
			az += az0 * (1 - transition);
			el += z0 * (1 - transition);
		}
	}

	else {
		// save base location for next time we re-focus
		const Point tmp = cpos;
		base_loc = FVector((float)tmp.x, (float)tmp.y, (float)tmp.z);
	}

	const double dx = er * FMath::Sin(az) * FMath::Cos(el);
	const double dy = er * FMath::Cos(az) * FMath::Cos(el);
	const double dz = er * FMath::Sin(el);

	Point cloc = cpos + Point(dx, dz, dy);

	Terrain* terrain = ship->GetRegion()->GetTerrain();

	if (terrain) {
		double cam_agl = cloc.y - terrain->Height(cloc.x, cloc.z);

		if (cam_agl < 100)
			cloc.y = terrain->Height(cloc.x, cloc.z) + 100;
	}

	if (external_ship == 0 && er < 0.5 * ship->Radius())
		ship->Rep()->Hide();

	camera.MoveTo(cloc.x, cloc.y, cloc.z);
	camera.LookAt(cpos);
}

// +--------------------------------------------------------------------+

void
CameraManager::Docking(double seconds)
{
	FlightDeck* dock = ship->GetDock();

	if (!dock) {
		Cockpit(seconds);
		return;
	}

	if (!ship->IsAirborne())
		sim->GetScene()->SetAmbient(Color(120, 130, 140));

	int flight_phase = ship->GetFlightPhase();

	Point bridge = ship->BridgeLocation();
	Point cloc = ship->Location() +
		ship->Cam().vrt() * bridge.x +
		ship->Cam().vpn() * bridge.y +
		ship->Cam().vup() * bridge.z;

	Point cpos = dock->CamLoc();

	// preflight:
	if (flight_phase < Ship::LOCKED) {
		base_loc = FVector((float)cpos.x, (float)cpos.y, (float)cpos.z);
	}

	else if (flight_phase == Ship::LOCKED) {
		if (hud)
			hud->SetHUDMode(HUDView::HUD_MODE_TAC);

		const FVector base = base_loc;
		const FVector cloc_v((float)cloc.x, (float)cloc.y, (float)cloc.z);
		const FVector lerp = base * (float)transition + cloc_v * (float)(1 - transition);
		cpos = Point(lerp.X, lerp.Y, lerp.Z);
	}

	// recovery:
	else if (flight_phase > Ship::APPROACH) {
		if (hud)
			hud->SetTacticalMode(1);
	}

	camera.MoveTo(cpos);
	camera.LookAt(cloc);
}

// +--------------------------------------------------------------------+

void
CameraManager::Drop(double seconds)
{
	// orbital transitions use "drop cam" at transition_loc
	camera.MoveTo(ship->TransitionLocation());
	camera.LookAt(ship->Location());
}
