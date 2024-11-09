/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CampaignSaveGame.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	CampaignSaveGame contains the logic needed to save and load
	campaign games in progress.
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/Text.h"
#include "../Foundation/Term.h"
#include "../Foundation/List.h"

// +--------------------------------------------------------------------+

class ACampaign;
class CampaignPlan;
class Combatant;
class CombatGroup;
class CombatZone;
class DataLoader;
class Mission;
class PlayerData;
class AStarSystem;

// +--------------------------------------------------------------------+
/**
 * 
 */
class STARSHATTERWARS_API CampaignSaveGame
{
public:
	
	static const char* TYPENAME() { return "CampaignSaveGame"; }
	
	CampaignSaveGame();
	~CampaignSaveGame();	

	void SetCampaign(ACampaign* c);

	ACampaign* GetCampaign() { return campaign; }

	void Load(const char* name);
	void Save(const char* name);
	static void Delete(const char* name);
	static void RemovePlayer(PlayerData* p);

	void LoadAuto();
	void SaveAuto();

	static Text GetResumeFile();
	static int GetSaveGameList(List<Text>& save_list);

private:
	static Text GetSaveDirectory();
	static Text GetSaveDirectory(PlayerData* p);
	static void CreateSaveDirectory();

	ACampaign* campaign;
};
