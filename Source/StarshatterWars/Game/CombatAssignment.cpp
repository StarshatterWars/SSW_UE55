/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatAssignment.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	High level assignment of one group to damage another
*/


#include "CombatAssignment.h"
#include "CombatGroup.h"
#include "Mission.h"


CombatAssignment::CombatAssignment()
{
}

// +--------------------------------------------------------------------+

CombatAssignment::CombatAssignment(int t, CombatGroup* obj, CombatGroup* rsc)
	: type(t), objective(obj), resource(rsc)
{
}

// +--------------------------------------------------------------------+

CombatAssignment::~CombatAssignment()
{
}

// +--------------------------------------------------------------------+
// This is used to sort assignments into a priority list.
// Higher priorities should come first in the list, so the
// sense of the operator is "backwards" from the usual.

int
CombatAssignment::operator < (const CombatAssignment& a) const
{
	if (!objective)
		return 0;

	if (!a.objective)
		return 1;

	return objective->GetPlanValue() > a.objective->GetPlanValue();
}

// +--------------------------------------------------------------------+

const char*
CombatAssignment::GetDescription() const
{
	static char desc[256];

	if (!resource)
		sprintf_s(desc, "%s %s",
			(const char*)Mission::RoleName(type),
			(const char*)objective->Name());
	else
		sprintf_s(desc, "%s %s %s",
			(const char*)resource->Name(),
			(const char*)Mission::RoleName(type),
			(const char*)objective->Name());

	return desc;
}


