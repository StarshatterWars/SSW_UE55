/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright © 2025-2026. All Rights Reserved.

	ORIGINAL AUTHOR AND STUDIO: John DiCamillo / Destroyer Studios LLC

	SUBSYSTEM:    Stars.exe
	FILE:         FlightComputer.cpp
	AUTHOR:       Carlos Bott


	OVERVIEW
	========
	Flight Computer System class
*/

#include "FlightComputer.h"

#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"
#include "Logging/LogMacros.h"

#include "Ship.h"
#include "ShipDesign.h"
#include "Thruster.h"

#ifndef STARSHATTERWARS_LOG_DEFINED
#define STARSHATTERWARS_LOG_DEFINED
DECLARE_LOG_CATEGORY_EXTERN(LogStarshatterWars, Log, All);
#endif

// +----------------------------------------------------------------------+

FlightComputer::FlightComputer(int comp_type, const char* comp_name)
	: Computer(comp_type, comp_name)
	, mode(0)
	, halt(0)
	, throttle(0.0f)
	, vlimit(0.0f)
	, trans_x_limit(0.0f)
	, trans_y_limit(0.0f)
	, trans_z_limit(0.0f)
{
}

// +----------------------------------------------------------------------+

FlightComputer::FlightComputer(const Computer& c)
	: Computer(c)
	, mode(0)
	, halt(0)
	, throttle(0.0f)
	, vlimit(0.0f)
	, trans_x_limit(0.0f)
	, trans_y_limit(0.0f)
	, trans_z_limit(0.0f)
{
}

// +--------------------------------------------------------------------+

FlightComputer::~FlightComputer()
{
}

// +--------------------------------------------------------------------+

void
FlightComputer::SetTransLimit(double x, double y, double z)
{
	trans_x_limit = 0.0f;
	trans_y_limit = 0.0f;
	trans_z_limit = 0.0f;

	if (x >= 0) trans_x_limit = (float)x;
	if (y >= 0) trans_y_limit = (float)y;
	if (z >= 0) trans_z_limit = (float)z;
}

// +--------------------------------------------------------------------+

void
FlightComputer::ExecSubFrame()
{
	if (ship) {
		ExecThrottle();
		ExecTrans();
	}
}

// +--------------------------------------------------------------------+

void
FlightComputer::ExecThrottle()
{
	throttle = (float)ship->Throttle();

	if (throttle > 5)
		halt = false;
}

// +--------------------------------------------------------------------+

void
FlightComputer::ExecTrans()
{
	double tx = ship->TransX();
	double ty = ship->TransY();
	double tz = ship->TransZ();

	double trans_x = tx;
	double trans_y = ty;
	double trans_z = tz;

	bool flcs_operative = false;

	if (IsPowerOn())
		flcs_operative = Status() == SimSystem::NOMINAL ||
		Status() == SimSystem::DEGRADED;

	// ----------------------------------------------------------
	// FIGHTER FLCS AUTO MODE
	// ----------------------------------------------------------

	if (mode == Ship::FLCS_AUTO) {
		// auto thrust to align flight path with orientation:
		if (tx == 0) {
			if (flcs_operative)
				trans_x = (ship->Velocity() * ship->BeamLine() * -200);

			else
				trans_x = 0;
		}

		// manual thrust up to vlimit/2:
		else {
			double vfwd = ship->BeamLine() * ship->Velocity();

			if (FMath::Abs(vfwd) >= vlimit) {
				if (trans_x > 0 && vfwd > 0)
					trans_x = 0;

				else if (trans_x < 0 && vfwd < 0)
					trans_x = 0;
			}
		}

		if (halt && flcs_operative) {
			if (ty == 0) {
				double vfwd = ship->Heading() * ship->Velocity();
				double vmag = FMath::Abs(vfwd);

				if (vmag > 0) {
					if (vfwd > 0)
						trans_y = -trans_y_limit;
					else
						trans_y = trans_y_limit;

					if (vfwd < vlimit / 2)
						trans_y *= (vmag / (vlimit / 2));
				}
			}
		}

		// auto thrust to align flight path with orientation:
		if (tz == 0) {
			if (flcs_operative)
				trans_z = (ship->Velocity() * ship->LiftLine() * -200);

			else
				trans_z = 0;
		}

		// manual thrust up to vlimit/2:
		else {
			double vfwd = ship->LiftLine() * ship->Velocity();

			if (FMath::Abs(vfwd) >= vlimit) {
				if (trans_z > 0 && vfwd > 0)
					trans_z = 0;

				else if (trans_z < 0 && vfwd < 0)
					trans_z = 0;
			}
		}
	}

	// ----------------------------------------------------------
	// STARSHIP HELM MODE
	// ----------------------------------------------------------

	else if (mode == Ship::FLCS_HELM) {
		if (flcs_operative) {
			double compass_heading = ship->CompassHeading();
			double compass_pitch = ship->CompassPitch();

			// rotate helm into compass orientation:
			double helm = ship->GetHelmHeading() - compass_heading;

			if (helm > PI)
				helm -= 2 * PI;
			else if (helm < -PI)
				helm += 2 * PI;

			// turn to align with helm heading:
			if (helm != 0)
				ship->ApplyYaw(helm);

			// pitch to align with helm pitch:
			if (compass_pitch != ship->GetHelmPitch())
				ship->ApplyPitch(compass_pitch - ship->GetHelmPitch());

			// roll to align with world coordinates:
			if (ship->Design()->auto_roll > 0) {
				const FVector Vrt = ship->Cam().vrt();
				double deflection = Vrt.Y;

				if (FMath::Abs(helm) < PI / 16 || ship->Design()->turn_bank < 0.01) {
					if (ship->Design()->auto_roll > 1) {
						ship->ApplyRoll(0.5);
					}
					else if (deflection != 0) {
						double theta = FMath::Asin(deflection);
						ship->ApplyRoll(-theta);
					}
				}

				// else roll through turn maneuvers:
				else {
					double desired_bank = ship->Design()->turn_bank;

					if (helm >= 0)
						desired_bank = -desired_bank;

					double current_bank = FMath::Asin(deflection);
					double theta = desired_bank - current_bank;
					ship->ApplyRoll(theta);

					// coordinate the turn:
					if ((current_bank < 0 && desired_bank < 0) ||
						(current_bank > 0 && desired_bank > 0)) {

						double coord_pitch =
							compass_pitch
							- ship->GetHelmPitch()
							- FMath::Abs(helm) * FMath::Abs(current_bank);

						ship->ApplyPitch(coord_pitch);
					}
				}
			}
		}

		// flcs inoperative, set helm heading based on actual compass heading:
		else {
			ship->SetHelmHeading(ship->CompassHeading());
			ship->SetHelmPitch(ship->CompassPitch());
		}

		// auto thrust to align flight path with helm order:
		if (tx == 0) {
			if (flcs_operative)
				trans_x = (ship->Velocity() * ship->BeamLine() * ship->Mass() * -1);

			else
				trans_x = 0;
		}

		// manual thrust up to vlimit/2:
		else {
			double vfwd = ship->BeamLine() * ship->Velocity();

			if (FMath::Abs(vfwd) >= vlimit / 2) {
				if (trans_x > 0 && vfwd > 0)
					trans_x = 0;

				else if (trans_x < 0 && vfwd < 0)
					trans_x = 0;
			}
		}

		if (trans_y == 0 && halt) {
			double vfwd = ship->Heading() * ship->Velocity();
			double vdesired = 0;

			if (vfwd > vdesired) {
				trans_y = -trans_y_limit;

				if (!flcs_operative)
					trans_y = 0;

				double vdelta = vfwd - vdesired;
				if (vdelta < vlimit / 2)
					trans_y *= (vdelta / (vlimit / 2));
			}
		}

		// auto thrust to align flight path with helm order:
		if (tz == 0) {
			if (flcs_operative)
				trans_z = (ship->Velocity() * ship->LiftLine() * ship->Mass() * -1);

			else
				trans_z = 0;
		}

		// manual thrust up to vlimit/2:
		else {
			double vfwd = ship->LiftLine() * ship->Velocity();

			if (FMath::Abs(vfwd) > vlimit / 2) {
				if (trans_z > 0 && vfwd > 0)
					trans_z = 0;

				else if (trans_z < 0 && vfwd < 0)
					trans_z = 0;
			}
		}
	}

	if (ship->GetThruster()) {
		ship->GetThruster()->ExecTrans(trans_x, trans_y, trans_z);
	}
	else {
		ship->SetTransX(trans_x);
		ship->SetTransY(trans_y);
		ship->SetTransZ(trans_z);
	}
}
