// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CombatAction.h"
#include "Campaign.h" // your ported Campaign class (for FindAction / GetTime etc.)

// ------------------------------------------------------------
// FCombatActionReq
// ------------------------------------------------------------
int32 FCombatActionReq::CompFromName(const FString& In)
{
	// Mirror typical Starshatter comparators:
	// LT, LE, EQ, GE, GT, NE
	const FString S = In.TrimStartAndEnd();

	if (S.Equals(TEXT("LT"), ESearchCase::IgnoreCase)) return -2;
	if (S.Equals(TEXT("LE"), ESearchCase::IgnoreCase)) return -1;
	if (S.Equals(TEXT("EQ"), ESearchCase::IgnoreCase)) return 0;
	if (S.Equals(TEXT("GE"), ESearchCase::IgnoreCase)) return 1;
	if (S.Equals(TEXT("GT"), ESearchCase::IgnoreCase)) return 2;
	if (S.Equals(TEXT("NE"), ESearchCase::IgnoreCase)) return 3;

	// Default EQ
	return 0;
}

bool FCombatActionReq::Compare(int32 L, int32 Comp, int32 R)
{
	switch (Comp)
	{
	case -2: return L < R; // LT
	case -1: return L <= R; // LE
	case  0: return L == R; // EQ
	case  1: return L >= R; // GE
	case  2: return L > R; // GT
	case  3: return L != R; // NE
	default: return L == R;
	}
}

// ------------------------------------------------------------
// CombatAction
// ------------------------------------------------------------
CombatAction::CombatAction() = default;

CombatAction::CombatAction(int32 InId, int32 InType, int32 InSubtype, int32 InTeam)
	: Id(InId)
	, ActionType(InType)
	, ActionSubtype(InSubtype)
	, Team(InTeam)
{
}

CombatAction::~CombatAction() = default;

bool CombatAction::IsAvailable(int64 CampaignTimeSeconds) const
{
	// Mirrors Starshatter:
	// - within [StartAfter, StartBefore]
	// - status not already COMPLETE/FAILED depending on design
	// Starshatter uses action availability as a time/req gate; status tracked separately.
	if (CampaignTimeSeconds < (int64)StartAfterSeconds)
		return false;

	if (CampaignTimeSeconds > (int64)StartBeforeSeconds)
		return false;

	return true;
}

bool CombatAction::IsAvailable(Campaign* CampaignObj) const
{
	if (!CampaignObj)
		return false;

	return IsAvailable(CampaignObj->GetTime());
}

bool CombatAction::IsActive(Campaign* CampaignObj) const
{
	// In Starshatter, “active” is not strongly distinct for all action types,
	// but it’s frequently used as: available + requirements met + not complete.
	if (!CampaignObj)
		return false;

	if (!IsAvailable(CampaignObj))
		return false;

	if (ActionStatus == COMPLETE || ActionStatus == FAILED)
		return false;

	return RequirementsMet(CampaignObj);
}

void CombatAction::AddRequirement(int32 InActionId, int32 InStatus, bool bInNot)
{
	FCombatActionReq R;
	R.ActionId = InActionId;
	R.ActionStatus = InStatus;
	R.bNot = bInNot;

	Req.Add(R);
}

void CombatAction::AddRequirement(Combatant* InC1, int32 InGroupType, int32 InGroupId, int32 InComp, int32 InScore, int32 InIntel)
{
	FCombatActionReq R;
	R.C1 = InC1;
	R.GroupType = InGroupType;
	R.GroupId = InGroupId;
	R.Comp = InComp;
	R.Score = InScore;
	R.Intel = InIntel;

	Req.Add(R);
}

void CombatAction::AddRequirement(Combatant* InC1, Combatant* InC2, int32 InComp, int32 InScore)
{
	FCombatActionReq R;
	R.C1 = InC1;
	R.C2 = InC2;
	R.Comp = InComp;
	R.Score = InScore;

	Req.Add(R);
}

bool CombatAction::RequirementsMet(Campaign* CampaignObj) const
{
	// This is intentionally conservative:
	// - If you have missing systems (combatant scoring, intel levels, group lookup),
	//   it will not crash; it will simply treat missing info as “not met”.
	if (!CampaignObj)
		return false;

	for (const FCombatActionReq& R : Req)
	{
		// 1) Action dependency: action id + status + optional NOT
		if (R.ActionId != 0)
		{
			CombatAction* Dep = CampaignObj->FindAction(R.ActionId);
			const bool bMatch = Dep && (Dep->Status() == R.ActionStatus);
			const bool bPass = R.bNot ? !bMatch : bMatch;

			if (!bPass)
				return false;

			continue;
		}

		// 2) Group dependency: (c1, group_type, group_id, comp, score, intel)
		if (R.GroupType != 0)
		{
			// You will wire this once Combatant/CombatGroup provide:
			// - Combatant::FindGroup(type,id)
			// - CombatGroup::IntelLevel(), CombatGroup::CalcValue()/Value(), etc.
			//
			// For now: if you don’t have it yet, fail safely.
			// If you DO have it already, implement here.
			return false;
		}

		// 3) Score dependency: (c1, c2, comp, score)
		if (R.C1 || R.C2)
		{
			// Same story: once Combatant::Score exists, implement it.
			// Fail-safe until then.
			return false;
		}
	}

	// No requirements means “met”
	return true;
}

int32 CombatAction::TypeFromName(const FString& In)
{
	// Must match campaign.def strings you are using.
	if (In.Equals(TEXT("INTEL_EVENT"), ESearchCase::IgnoreCase) || In.Equals(TEXT("intel"), ESearchCase::IgnoreCase))
		return INTEL_EVENT;

	if (In.Equals(TEXT("COMBAT_EVENT"), ESearchCase::IgnoreCase) || In.Equals(TEXT("combat"), ESearchCase::IgnoreCase))
		return COMBAT_EVENT;

	if (In.Equals(TEXT("MISSION_TEMPLATE"), ESearchCase::IgnoreCase) || In.Equals(TEXT("mission_template"), ESearchCase::IgnoreCase))
		return MISSION_TEMPLATE;

	if (In.Equals(TEXT("MOVE_GROUP"), ESearchCase::IgnoreCase) || In.Equals(TEXT("move"), ESearchCase::IgnoreCase))
		return MOVE_GROUP;

	if (In.Equals(TEXT("SET_INTEL"), ESearchCase::IgnoreCase) || In.Equals(TEXT("set_intel"), ESearchCase::IgnoreCase))
		return SET_INTEL;

	return UNKNOWN_ACTION;
}

int32 CombatAction::StatusFromName(const FString& In)
{
	if (In.Equals(TEXT("INCOMPLETE"), ESearchCase::IgnoreCase)) return INCOMPLETE;
	if (In.Equals(TEXT("ACTIVE"), ESearchCase::IgnoreCase)) return ACTIVE;
	if (In.Equals(TEXT("COMPLETE"), ESearchCase::IgnoreCase)) return COMPLETE;
	if (In.Equals(TEXT("FAILED"), ESearchCase::IgnoreCase)) return FAILED;

	// Defaults to incomplete
	return INCOMPLETE;
}
