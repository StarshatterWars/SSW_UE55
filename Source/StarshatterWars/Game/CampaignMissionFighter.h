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

class UCampaign;
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
	CampaignMissionFighter(UCampaign* c);

	virtual void   CreateMission(CampaignMissionRequest* request);

protected:
	Mission* GenerateMission(int id);
	void		SelectType();
	void		SelectRegion();
	void		GenerateStandardElements();
	void		GenerateMissionElements();
	void		CreateElements(CombatGroup* g);
	void		CreateSquadron(CombatGroup* g);
	void		CreatePlayer(CombatGroup* g);

	void		CreatePatrols();
	void		CreateWards();
	void		CreateWardFreight();
	void		CreateWardShuttle();
	void		CreateWardStrike();

	void		CreateEscorts();

	void	    CreateTargets();
	void		CreateTargetsPatrol();
	void	    CreateTargetsSweep();
	void	    CreateTargetsIntercept();
	void	    CreateTargetsFreightEscort();
	void	    CreateTargetsShuttleEscort();
	void		CreateTargetsStrikeEscort();
	void	    CreateTargetsStrike();
	void	    CreateTargetsAssault();
	int			CreateRandomTarget(const char* rgn, Point base_loc);

	bool		IsGroundObjective(CombatGroup* obj);

	void	    PlanetaryInsertion(MissionElement* elem);
	void	    OrbitalInsertion(MissionElement* elem);

	virtual MissionElement* CreateSingleElement(CombatGroup* g,
			CombatUnit* u);
	virtual MissionElement* CreateFighterPackage(CombatGroup* squadron,
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
	int				  pkg_id;
};
