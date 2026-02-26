// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameStructs.h"

class Campaign;
class CampaignMissionRequest;
class CombatGroup;
class CombatUnit;
class CombatZone;
class Mission;
class MissionElement;
struct FMissionInfo;
class MissionTemplate;

/**
 * CampaignMissionFighter
 *
 * UE stub port of Starshatter 4.5 CampaignMissionFighter.
 * Generates missions / mission info for the player's fighter squadron in a dynamic campaign.
 *
 * This is stubs-only: it preserves the original method inventory and state fields,
 * so you can fill it in gradually without losing parity with the original design.
 */
class CampaignMissionFighter
{
public:
	static const TCHAR* TYPENAME() { return TEXT("CampaignMissionFighter"); }

	explicit CampaignMissionFighter(Campaign* InCampaign);
	virtual ~CampaignMissionFighter();

	/** Entry point: build a mission using request data */
	virtual void CreateMission(CampaignMissionRequest* InRequest);

protected:
	// ---- Original Starshatter method inventory (stubs) ----
	virtual Mission* GenerateMission(int32 Id);
	virtual void            SelectType();
	virtual void            SelectRegion();
	virtual void            GenerateStandardElements();
	virtual void            GenerateMissionElements();
	virtual void            CreateElements(CombatGroup* G);
	virtual void            CreateSquadron(CombatGroup* G);
	virtual void            CreatePlayer(CombatGroup* G);

	virtual void            CreatePatrols();
	virtual void            CreateWards();
	virtual void            CreateWardFreight();
	virtual void            CreateWardShuttle();
	virtual void            CreateWardStrike();

	virtual void            CreateEscorts();

	virtual void            CreateTargets();
	virtual void            CreateTargetsPatrol();
	virtual void            CreateTargetsSweep();
	virtual void            CreateTargetsIntercept();
	virtual void            CreateTargetsFreightEscort();
	virtual void            CreateTargetsShuttleEscort();
	virtual void            CreateTargetsStrikeEscort();
	virtual void            CreateTargetsStrike();
	virtual void            CreateTargetsAssault();
	virtual int32           CreateRandomTarget(const FString& Region, const FVector& BaseLoc);

	virtual bool            IsGroundObjective(CombatGroup* Obj);

	virtual void            PlanetaryInsertion(MissionElement* Elem);
	virtual void            OrbitalInsertion(MissionElement* Elem);

	virtual MissionElement* CreateSingleElement(CombatGroup* G, CombatUnit* U);
	virtual MissionElement* CreateFighterPackage(CombatGroup* Squadron, int32 Count, int32 Role);

	virtual CombatGroup* FindSquadron(int32 Iff, int32 Type);
	virtual CombatUnit* FindCarrier(CombatGroup* G);

	virtual void            DefineMissionObjectives();
	virtual FMissionInfo* DescribeMission();
	virtual void            Exit();

protected:
	// ---- Runtime state (mirrors original fields) ----
	Campaign* CampaignObj = nullptr;
	CampaignMissionRequest* Request = nullptr;
	FMissionInfo* MissionInfoPtr = nullptr;

	CombatGroup* Squadron = nullptr;
	CombatGroup* StrikeGroup = nullptr;
	CombatGroup* StrikeTarget = nullptr;

	Mission* MissionPtr = nullptr;

	MissionElement* PlayerElem = nullptr;
	MissionElement* CarrierElem = nullptr;
	MissionElement* Ward = nullptr;
	MissionElement* PrimeTarget = nullptr;
	MissionElement* Escort = nullptr;

	// Starshatter "Text" => FString
	FString                  AirRegion;
	FString                  OrbRegion;

	bool                     bAirborne = false;
	bool                     bAirbase = false;

	int32                    OwnSide = 0;
	int32                    Enemy = 0;
	int32                    MissionType = 0;
};
