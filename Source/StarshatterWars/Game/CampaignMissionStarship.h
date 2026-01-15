/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CampaignMissionStarship.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	CampaignMissionStarship generates missions and mission
	info for the player's STARSHIP GROUP as part of a
	dynamic campaign.
*/


#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "Geometry.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class UCampaign;
class CampaignMissionRequest;
class CombatGroup;
class CombatUnit;
class CombatZone;
class ZoneForce;
class Mission;
class MissionElement;
class MissionInfo;
class MissionTemplate;

// +--------------------------------------------------------------------+

/**
 * 
 */
class STARSHATTERWARS_API CampaignMissionStarship
{
public:
	static const char* TYPENAME() { return "CampaignMissionStarship"; }
	
	CampaignMissionStarship();
	CampaignMissionStarship(UCampaign* c);

	virtual void   CreateMission(CampaignMissionRequest* request);

protected:
	virtual Mission* GenerateMission(int id);
	virtual void      SelectType();
	virtual void      SelectRegion();
	virtual void      GenerateStandardElements();
	virtual void      GenerateMissionElements();
	virtual void      CreateElements(CombatGroup* g);
	virtual void      CreateSquadron(CombatGroup* g);
	virtual void      CreatePlayer();

	virtual void      CreateWards();
	virtual void      CreateWardFreight();

	virtual void      CreateEscorts();

	virtual void      CreateTargets();
	virtual void      CreateTargetsAssault();
	virtual void      CreateTargetsPatrol();
	virtual void      CreateTargetsCarrier();
	virtual void      CreateTargetsFreightEscort();
	virtual int       CreateRandomTarget(const char* rgn, Point base_loc);

	virtual MissionElement*
		CreateSingleElement(CombatGroup* g,
			CombatUnit* u);
	virtual MissionElement*
		CreateFighterPackage(CombatGroup* squadron,
			int            count,
			int            role);

	virtual CombatGroup* FindSquadron(int iff, int type);
	virtual CombatUnit* FindCarrier(CombatGroup* g);

	virtual void         DefineMissionObjectives();
	virtual MissionInfo* DescribeMission();
	virtual void         Exit();

	UCampaign* campaign;
	CampaignMissionRequest* request;
	MissionInfo* mission_info;

	CombatUnit* player_unit;
	CombatGroup* player_group;
	CombatGroup* strike_group;
	CombatGroup* strike_target;
	Mission* mission;
	List<MissionElement> player_group_elements;
	MissionElement* player;
	MissionElement* ward;
	MissionElement* prime_target;
	MissionElement* escort;

	int               ownside;
	int               enemy;
	int               mission_type;

	int dump_missions;
	int pkg_id = 1000;
};
