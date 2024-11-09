/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CampaignMissionFighter.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	CampaignMissionFighter generates missions and mission
	info for the player's FIGHTER GROUP as part of a
	dynamic campaign.
*/

#pragma once

#include "CoreMinimal.h"
#include "../Foundation/Types.h"
#include "../Foundation/Geometry.h"
#include "../Foundation/List.h"
#include "../Foundation/Text.h"


// +--------------------------------------------------------------------+

class ACampaign;
class CampaignMissionRequest;
class CombatGroup;
class CombatUnit;
class CombatZone;
class ZonefORCE;
class Mission;
class MissionElement;
class MissionInfo;
class MissionTemplate;

// +--------------------------------------------------------------------+
/**
 * 
 */
class STARSHATTERWARS_API CampaignMissionFighter
{
public:
	CampaignMissionFighter();
	CampaignMissionFighter(ACampaign* c);

	virtual void   CreateMission(CampaignMissionRequest* request);

protected:
	virtual Mission* GenerateMission(int id);
	virtual void      SelectType();
	virtual void      SelectRegion();
	virtual void      GenerateStandardElements();
	virtual void      GenerateMissionElements();
	virtual void      CreateElements(CombatGroup* g);
	virtual void      CreateSquadron(CombatGroup* g);
	virtual void      CreatePlayer(CombatGroup* g);

	virtual void      CreatePatrols();
	virtual void      CreateWards();
	virtual void      CreateWardFreight();
	virtual void      CreateWardShuttle();
	virtual void      CreateWardStrike();

	virtual void      CreateEscorts();

	virtual void      CreateTargets();
	virtual void      CreateTargetsPatrol();
	virtual void      CreateTargetsSweep();
	virtual void      CreateTargetsIntercept();
	virtual void      CreateTargetsFreightEscort();
	virtual void      CreateTargetsShuttleEscort();
	virtual void      CreateTargetsStrikeEscort();
	virtual void      CreateTargetsStrike();
	virtual void      CreateTargetsAssault();
	virtual int       CreateRandomTarget(const char* rgn, Point base_loc);

	virtual bool      IsGroundObjective(CombatGroup* obj);

	virtual void      PlanetaryInsertion(MissionElement* elem);
	virtual void      OrbitalInsertion(MissionElement* elem);

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

	ACampaign* campaign;
	CampaignMissionRequest* request;
	MissionInfo* mission_info;

	CombatGroup* squadron;
	CombatGroup* strike_group;
	CombatGroup* strike_target;
	Mission* mission;
	MissionElement* player_elem;
	MissionElement* carrier_elem;
	MissionElement* ward;
	MissionElement* prime_target;
	MissionElement* escort;
	Text              air_region;
	Text              orb_region;
	bool              airborne;
	bool              airbase;
	int               ownside;
	int               enemy;
	int               mission_type;
	int				  dump_missions;
};
