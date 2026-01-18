/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CampaignMissionStarship.h
    AUTHOR:       John DiCamillo

    UNREAL PORT:
    - Maintains all variables and methods (names, signatures, members).
    - Uses UE-compatible shims for Geometry/List/Text and UE basic types.
*/

#pragma once

#include "Types.h"
#include "Geometry.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Campaign;
class CampaignMissionRequest;
class CombatGroup;
class CombatUnit;
class CombatZone;
class Mission;
class MissionElement;
class MissionInfo;
class MissionTemplate;

// +--------------------------------------------------------------------+

class CampaignMissionStarship
{
public:
    static const char* TYPENAME() { return "CampaignMissionStarship"; }

    CampaignMissionStarship(Campaign* c);
    virtual ~CampaignMissionStarship();

    virtual void   CreateMission(CampaignMissionRequest* request);

protected:
    virtual Mission* GenerateMission(int id);
    virtual void            SelectType();
    virtual void            SelectRegion();
    virtual void            GenerateStandardElements();
    virtual void            GenerateMissionElements();
    virtual void            CreateElements(CombatGroup* g);
    virtual void            CreateSquadron(CombatGroup* g);
    virtual void            CreatePlayer();

    virtual void            CreateWards();
    virtual void            CreateWardFreight();

    virtual void            CreateEscorts();

    virtual void            CreateTargets();
    virtual void            CreateTargetsAssault();
    virtual void            CreateTargetsPatrol();
    virtual void            CreateTargetsCarrier();
    virtual void            CreateTargetsFreightEscort();
    virtual int             CreateRandomTarget(const char* rgn, Point base_loc);

    virtual MissionElement* CreateSingleElement(CombatGroup* g, CombatUnit* u);
    virtual MissionElement* CreateFighterPackage(CombatGroup* squadron, int count, int role);

    virtual CombatGroup* FindSquadron(int iff, int type);
    virtual CombatUnit* FindCarrier(CombatGroup* g);

    virtual void            DefineMissionObjectives();
    virtual MissionInfo* DescribeMission();
    virtual void            Exit();

protected:
    Campaign* campaign = nullptr;
    CampaignMissionRequest* request = nullptr;
    MissionInfo* mission_info = nullptr;

    CombatUnit* player_unit = nullptr;
    CombatGroup* player_group = nullptr;
    CombatGroup* strike_group = nullptr;
    CombatGroup* strike_target = nullptr;

    Mission* mission = nullptr;

    List<MissionElement>    player_group_elements;

    MissionElement* player = nullptr;
    MissionElement* ward = nullptr;
    MissionElement* prime_target = nullptr;
    MissionElement* escort = nullptr;

    int                     ownside = 0;
    int                     enemy = 0;
    int                     mission_type = 0;
};
