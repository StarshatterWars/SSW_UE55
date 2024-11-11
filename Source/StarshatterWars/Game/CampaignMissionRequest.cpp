// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CampaignMissionRequest.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatAssignment.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Mission.h"
//#include "Instruction.h"
#include "ShipDesign.h"
#include "../Space/StarSystem.h"
#include "../Foundation/Random.h"

CampaignMissionRequest::CampaignMissionRequest()
{
}

CampaignMissionRequest::CampaignMissionRequest(UCampaign* c,
	int          t,
	int          s,
	CombatGroup* p,
	CombatGroup* tgt) {

	campaign = c;
	type = t;
	opp_type = -1;
	start = s;
	primary_group = p;
	secondary_group = 0;
	objective = tgt;
	use_loc = false;
}
