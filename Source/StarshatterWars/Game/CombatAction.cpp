/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         CombatAction.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	A planned action (mission/story/strategy) in a dynamic campaign.
*/


#include "CombatAction.h"
#include "CombatGroup.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatActionReq.h"
#include "PlayerData.h"
#include "../Foundation/Random.h"

// +----------------------------------------------------------------------+

CombatAction::CombatAction(int n, int typ, int sub, int iff)
	: id(n), type(typ), subtype(sub), opp_type(-1), team(iff),
	status(PENDING), count(0), rval(-1), source(0), time(0),
	start_before((int)1e9), start_after(0),
	min_rank(0), max_rank(100),
	delay(0), probability(100), asset_type(0), target_type(0)
{ }

CombatAction::~CombatAction()
{
	requirements.destroy();
	asset_kills.destroy();
	target_kills.destroy();
}

// +----------------------------------------------------------------------+

bool
CombatAction::IsAvailable() const
{
	CombatAction * pThis = (CombatAction*)this;

	if (rval < 0) {
		pThis->rval = (int)RandomDouble(0, 100);

		if (rval > probability)
			pThis->status = SKIPPED;
	}

	if (status != PENDING)
		return false;

	if (min_rank > 0 || max_rank < 100) {
		PlayerData* player = PlayerData::GetCurrentPlayer();

		if (player->Rank() < min_rank || player->Rank() > max_rank)
			return false;
	}

	ACampaign* campaign = ACampaign::GetCampaign();
	if (campaign) {
		if (campaign->GetTime() < start_after) {
			return false;
		}

		if (campaign->GetTime() > start_before) {
			pThis->status = FAILED; // too late!
			return false;
		}

		// check requirements against actions in current campaign:
		ListIter<CombatActionReq> iter = pThis->requirements;
		while (++iter) {
			CombatActionReq* r = iter.value();
			bool             ok = false;

			if (r->action > 0) {
				ListIter<CombatAction> action = campaign->GetActions();
				while (++action) {
					CombatAction* a = action.value();

					if (a->Identity() == r->action) {
						if (r->notreq) {
							if (a->Status() == r->stat)
								return false;
						}
						else {
							if (a->Status() != r->stat)
								return false;
						}
					}
				}
			}

			// group-based requirement
			else if (r->group_type > 0) {
				if (r->c1) {
					CombatGroup* group = r->c1->FindGroup(r->group_type, r->group_id);

					if (group) {
						int test = 0;
						int comp = 0;

						if (r->intel) {
							test = group->IntelLevel();
							comp = r->intel;
						}

						else {
							test = group->CalcValue();
							comp = r->score;
						}

						switch (r->comp) {
						case COMPARISON_OPERATOR::LT:  
							ok = (test < comp); 
							break;

						case COMPARISON_OPERATOR::LE: 
							ok = (test <= comp);
							break;

						case COMPARISON_OPERATOR::GT: 
							ok = (test > comp); 
							break;

						case COMPARISON_OPERATOR::GE:  
							ok = (test >= comp); 
							break;

						case COMPARISON_OPERATOR::EQ: 
							ok = (test == comp); 
							break;
						}
					}

					if (!ok)
						return false;
				}
			}

			// score-based requirement
			else {
				int test = 0;

				if (r->comp <= COMPARISON_OPERATOR::EQ) {  // absolute
					if (r->c1) {
						int testop = r->c1->Score();

						switch (r->comp) {
						case COMPARISON_OPERATOR::LT: 
							ok = (testop < r->score);
							break;

						case COMPARISON_OPERATOR::LE:  
							ok = (testop <= r->score);
							break;

						case COMPARISON_OPERATOR::GT: 
							ok = (testop > r->score);
							break;

						case COMPARISON_OPERATOR::GE: 
							ok = (testop >= r->score);
							break;

						case COMPARISON_OPERATOR::EQ:  
							ok = (testop == r->score);
							break;
						}
					}
				}

				else {                                 // relative
					if (r->c1 && r->c2) {
						int testop = r->c1->Score() - r->c2->Score();

						switch (r->comp) {
						case COMPARISON_OPERATOR::RLT: 
							ok = (testop < r->score);
							break;

						case COMPARISON_OPERATOR::RLE: 
							ok = (testop <= r->score);
							break;

						case COMPARISON_OPERATOR::RGT: 
							ok = (testop > r->score);
							break;

						case COMPARISON_OPERATOR::RGE: 
							ok = (testop >= r->score);
							break;

						case COMPARISON_OPERATOR::REQ:
							ok = (testop == r->score);
							break;
						}
					}
				}

				if (!ok)
					return false;
			}

			if (delay > 0) {
				pThis->start_after = (int)campaign->GetTime() + delay;
				pThis->delay = 0;
				return IsAvailable();
			}
		}
	}

	return true;
}

// +----------------------------------------------------------------------+

void
CombatAction::FireAction()
{
	ACampaign* campaign = ACampaign::GetCampaign();
	if (campaign)
		time = (int)campaign->GetTime();

	if (count >= 1)
		count--;

	if (count < 1)
		status = COMPLETE;
}

void
CombatAction::FailAction()
{
	ACampaign* campaign = ACampaign::GetCampaign();
	if (campaign)
		time = (int)campaign->GetTime();

	count = 0;
	status = FAILED;
}

// +----------------------------------------------------------------------+

void
CombatAction::AddRequirement(int action, int stat, bool notreq)
{
	requirements.append(new CombatActionReq(action, stat, notreq));
}

void
CombatAction::AddRequirement(Combatant* c1, Combatant* c2, int comp, int score)
{
	requirements.append(new CombatActionReq(c1, c2, comp, score));
}

void
CombatAction::AddRequirement(Combatant* c1, int group_type, int group_id, int comp, int score, int intel)
{
	requirements.append(new CombatActionReq(c1, group_type, group_id, comp, score, intel));
}

// +----------------------------------------------------------------------+

int
CombatAction::TypeFromName(const char* n)
{
	int type = 0;

	if (!_stricmp(n, "NO_ACTION"))
		type = (int) ECOMBATACTION_TYPE::NO_ACTION;

	else if (!_stricmp(n, "MARKER"))
		type = (int)ECOMBATACTION_TYPE::NO_ACTION;

	else if (!_stricmp(n, "STRATEGIC_DIRECTIVE"))
		type = (int)ECOMBATACTION_TYPE::STRATEGIC_DIRECTIVE;

	else if (!_stricmp(n, "STRATEGIC"))
		type = (int)ECOMBATACTION_TYPE::STRATEGIC_DIRECTIVE;

	else if (!_stricmp(n, "ZONE_ASSIGNMENT"))
		type = (int)ECOMBATACTION_TYPE::ZONE_ASSIGNMENT;

	else if (!_stricmp(n, "ZONE"))
		type = (int)ECOMBATACTION_TYPE::ZONE_ASSIGNMENT;

	else if (!_stricmp(n, "SYSTEM_ASSIGNMENT"))
		type = (int)ECOMBATACTION_TYPE::SYSTEM_ASSIGNMENT;

	else if (!_stricmp(n, "SYSTEM"))
		type = (int)ECOMBATACTION_TYPE::SYSTEM_ASSIGNMENT;

	else if (!_stricmp(n, "MISSION_TEMPLATE"))
		type = (int)ECOMBATACTION_TYPE::MISSION_TEMPLATE;

	else if (!_stricmp(n, "MISSION"))
		type = (int)ECOMBATACTION_TYPE::MISSION_TEMPLATE;

	else if (!_stricmp(n, "COMBAT_EVENT"))
		type = (int)ECOMBATACTION_TYPE::COMBAT_EVENT;

	else if (!_stricmp(n, "EVENT"))
		type = (int)ECOMBATACTION_TYPE::COMBAT_EVENT;

	else if (!_stricmp(n, "INTEL_EVENT"))
		type = (int)ECOMBATACTION_TYPE::INTEL_EVENT;

	else if (!_stricmp(n, "INTEL"))
		type = (int)ECOMBATACTION_TYPE::INTEL_EVENT;

	else if (!_stricmp(n, "CAMPAIGN_SITUATION"))
		type = (int)ECOMBATACTION_TYPE::CAMPAIGN_SITUATION;

	else if (!_stricmp(n, "SITREP"))
		type = (int)ECOMBATACTION_TYPE::CAMPAIGN_SITUATION;

	else if (!_stricmp(n, "CAMPAIGN_ORDERS"))
		type = (int)ECOMBATACTION_TYPE::CAMPAIGN_ORDERS;

	else if (!_stricmp(n, "ORDERS"))
		type = (int)ECOMBATACTION_TYPE::CAMPAIGN_ORDERS;

	return type;
}

int
CombatAction::StatusFromName(const char* n)
{
	int stat = 0;

	if (!_stricmp(n, "PENDING"))
		stat = (int)ECOMBATACTION_STATUS::PENDING;

	else if (!_stricmp(n, "ACTIVE"))
		stat = (int)ECOMBATACTION_STATUS::ACTIVE;

	else if (!_stricmp(n, "SKIPPED"))
		stat = (int)ECOMBATACTION_STATUS::SKIPPED;

	else if (!_stricmp(n, "FAILED"))
		stat = (int)ECOMBATACTION_STATUS::FAILED;

	else if (!_stricmp(n, "COMPLETE"))
		stat = (int)ECOMBATACTION_STATUS::COMPLETE;

	return stat;
}

