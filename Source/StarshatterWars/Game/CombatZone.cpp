/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatZone.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	CombatZone is used by the dynamic campaign strategy
	and logistics algorithms to assign forces to locations
	within the campaign.  A CombatZone is a collection of
	closely related sectors, and the assets contained
	within them.
*/

#include "CombatZone.h"
#include "CombatZone.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "Campaign.h"
#include "ZoneForce.h"
#include "ShipDesign.h"
//#include "Ship.h"

#include "../System/Game.h"
#include "../Foundation/DataLoader.h"
#include "../Foundation/ParseUtil.h"

CombatZone::CombatZone()
{
}

CombatZone::~CombatZone()
{
	regions.destroy();
	forces.destroy();
}

// +--------------------------------------------------------------------+

void
CombatZone::Clear()
{
	forces.destroy();
}

// +--------------------------------------------------------------------+

void
CombatZone::AddGroup(CombatGroup* group)
{
	if (group) {
		int iff = group->GetIFF();
		ZoneForce* f = FindForce(iff);
		f->AddGroup(group);
		group->SetCurrentZone(this);
	}
}

void
CombatZone::RemoveGroup(CombatGroup* group)
{
	if (group) {
		int iff = group->GetIFF();
		ZoneForce* f = FindForce(iff);
		f->RemoveGroup(group);
		group->SetCurrentZone(0);
	}
}

bool
CombatZone::HasGroup(CombatGroup* group)
{
	if (group) {
		int iff = group->GetIFF();
		ZoneForce* f = FindForce(iff);
		return f->HasGroup(group);
	}

	return false;
}

// +--------------------------------------------------------------------+

void
CombatZone::AddRegion(const char* rgn)
{
	if (rgn && *rgn) {
		regions.append(new Text(rgn));

		if (name.length() < 1)
			name = rgn;
	}
}

// +--------------------------------------------------------------------+

bool
CombatZone::HasRegion(const char* rgn)
{
	if (rgn && *rgn && regions.size()) {
		Text test(rgn);
		return regions.contains(&test);
	}

	return false;
}

// +--------------------------------------------------------------------+

ZoneForce*
CombatZone::FindForce(int iff)
{
	ListIter<ZoneForce> f = forces;
	while (++f) {
		if (f->GetIFF() == iff)
			return f.value();
	}

	return MakeForce(iff);
}

// +--------------------------------------------------------------------+

ZoneForce*
CombatZone::MakeForce(int iff)
{
	ZoneForce* f = new ZoneForce(iff);
	forces.append(f);
	return f;
}

// +--------------------------------------------------------------------+

static List<CombatZone> zonelist;

List<CombatZone>&
CombatZone::Load(const char* filename)
{
	zonelist.clear();

	DataLoader* loader = DataLoader::GetLoader();
	BYTE* block = 0;

	loader->LoadBuffer(filename, block, true);
	Parser parser(new BlockReader((const char*)block));

	Term* term = parser.ParseTerm();

	if (!term) {
		return zonelist;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "ZONES") {
			return zonelist;
		}
	}

	do {
		delete term; term = 0;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "zone") {
					if (!def->term() || !def->term()->isStruct()) {
						//::Print("WARNING: zone struct missing in '%s%s'\n", loader->GetDataPath(), filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();

						CombatZone* zone = new CombatZone();
						char        rgn[64];
						ZeroMemory(rgn, sizeof(rgn));

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "region") {
									GetDefText(rgn, pdef, filename);
									zone->AddRegion(rgn);
								}
								else if (pdef->name()->value() == "system") {
									GetDefText(rgn, pdef, filename);
									zone->system = rgn;
								}
							}
						}

						zonelist.append(zone);
					}
				}
			}
		}
	} while (term);

	loader->ReleaseBuffer(block);

	return zonelist;
}

// +--------------------------------------------------------------------+
