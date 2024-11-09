/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatRoster.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	The complete roster of all known persistent entities
	for all combatants in the game.
*/


#include "CombatRoster.h"
#include "CombatGroup.h"

#include "../System/Game.h"
#include "../Foundation/DataLoader.h"
#include "../Foundation/Text.h"

// +--------------------------------------------------------------------+

static CombatRoster* roster = 0;

// +--------------------------------------------------------------------+

CombatRoster::CombatRoster()
{
	DataLoader* loader = DataLoader::GetLoader();
	loader->SetDataPath("Campaigns/");

	List<Text> files;
	loader->ListFiles("*.def", files);

	for (int i = 0; i < files.size(); i++) {
		Text filename = *files[i];

		if (!filename.contains("/") && !filename.contains("\\")) {
			loader->SetDataPath("Campaigns/");
			CombatGroup* g = CombatGroup::LoadOrderOfBattle(filename, -1, 0);
			forces.append(g);
		}
	}

	files.destroy();
}


// +--------------------------------------------------------------------+

CombatRoster::~CombatRoster()
{
	forces.destroy();
}

// +--------------------------------------------------------------------+

CombatGroup*
CombatRoster::GetForce(const char* name)
{
	ListIter<CombatGroup> iter = forces;
	while (++iter) {
		CombatGroup* f = iter.value();

		if (f->Name() == name)
			return f;
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
CombatRoster::Initialize()
{
	roster = new CombatRoster();
}

void
CombatRoster::Close()
{
	delete roster;
	roster = 0;
}

CombatRoster*
CombatRoster::GetInstance()
{
	return roster;
}