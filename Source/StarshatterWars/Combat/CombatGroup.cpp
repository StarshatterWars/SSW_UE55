// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CombatGroup.h"

CombatGroup::CombatGroup(ECOMBATGROUP_TYPE InType, int32 InId, const FString& InName, int32 InIff, EINTEL_TYPE InIntel, CombatGroup* InParent)
	: GroupType(InType)
	, Id(InId)
	, GroupName(InName)
	, Iff(InIff)
	, EnemyIntel(InIntel)
	, Parent(nullptr)
{
	if (InParent)
	{
		InParent->AddComponent(this);
	}
}

CombatGroup::~CombatGroup()
{
	// IMPORTANT:
	// Do not delete children here yet; ownership needs to be defined in your UE architecture.
	// If/when you decide CombatGroup owns its tree, convert Components to TArray<TUniquePtr<CombatGroup>>.
	Components.Reset();
}

void CombatGroup::AddComponent(CombatGroup* Group)
{
	if (!Group)
		return;

	Group->Parent = this;
	Components.Add(Group);
}

CombatGroup* CombatGroup::FindGroup(ECOMBATGROUP_TYPE InType, int32 InId)
{
	if (GroupType == InType && (InId < 0 || Id == InId))
		return this;

	for (CombatGroup* Child : Components)
	{
		if (!Child) continue;
		if (CombatGroup* Found = Child->FindGroup(InType, InId))
			return Found;
	}

	return nullptr;
}

const CombatGroup* CombatGroup::FindGroup(ECOMBATGROUP_TYPE InType, int32 InId) const
{
	if (GroupType == InType && (InId < 0 || Id == InId))
		return this;

	for (const CombatGroup* Child : Components)
	{
		if (!Child) continue;
		if (const CombatGroup* Found = Child->FindGroup(InType, InId))
			return Found;
	}

	return nullptr;
}

CombatGroup* CombatGroup::Clone(bool bDeep) const
{
	CombatGroup* NewGroup = new CombatGroup(GroupType, Id, GroupName, Iff, EnemyIntel, nullptr);
	NewGroup->Region = Region;
	NewGroup->CachedValue = CachedValue;
	NewGroup->AssignedSystem = AssignedSystem;
	NewGroup->CurrentZone = CurrentZone;
	NewGroup->AssignedZone = AssignedZone;
	NewGroup->bZoneLock = bZoneLock;
	NewGroup->OwningCombatant = OwningCombatant;

	if (bDeep)
	{
		for (const CombatGroup* Child : Components)
		{
			if (!Child) continue;
			CombatGroup* ChildClone = Child->Clone(true);
			NewGroup->AddComponent(ChildClone);
		}
	}

	return NewGroup;
}

void CombatGroup::AssignRegion(const FString& InRegion)
{
	// Starshatter had some logic around zones; for now, preserve the intent:
	Region = InRegion;
}

bool CombatGroup::IsReserve() const
{
	// Mirrors Starshatter:
	// if (enemy_intel <= Intel::RESERVE) return true;
	// if (parent) return parent->IsReserve();
	if ((int32)EnemyIntel <= (int32)EINTEL_TYPE::RESERVE)
		return true;

	return Parent ? Parent->IsReserve() : false;
}

int32 CombatGroup::CalcValue()
{
	// Starshatter computes value from units and recursively from components.
	// You have not ported units yet, so we keep the recursion behavior and
	// treat "local units" as 0 until you wire them.

	int32 Sum = 0;

	for (CombatGroup* Child : Components)
	{
		if (!Child) continue;
		Sum += Child->CalcValue();
	}

	CachedValue = Sum;
	return CachedValue;
}

// ---------------------------------------------------------------------
// Capability logic (mirrors original switch tables / intent)
// ---------------------------------------------------------------------

bool CombatGroup::IsFighterType(ECOMBATGROUP_TYPE T)
{
	switch (T)
	{
	case ECOMBATGROUP_TYPE::WING:
	case ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:
	case ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:
	case ECOMBATGROUP_TYPE::ATTACK_SQUADRON:
		return true;
	default:
		return false;
	}
}

bool CombatGroup::IsStarshipType(ECOMBATGROUP_TYPE T)
{
	switch (T)
	{
	case ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:
	case ECOMBATGROUP_TYPE::BATTLE_GROUP:
	case ECOMBATGROUP_TYPE::CARRIER_GROUP:
		return true;
	default:
		return false;
	}
}

bool CombatGroup::IsMovableType(ECOMBATGROUP_TYPE T)
{
	switch (T)
	{
	case ECOMBATGROUP_TYPE::CARRIER_GROUP:
	case ECOMBATGROUP_TYPE::BATTLE_GROUP:
	case ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:
	case ECOMBATGROUP_TYPE::ATTACK_SQUADRON:
	case ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:
	case ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:
	case ECOMBATGROUP_TYPE::LCA_SQUADRON:
	case ECOMBATGROUP_TYPE::COURIER:
	case ECOMBATGROUP_TYPE::MEDICAL:
	case ECOMBATGROUP_TYPE::SUPPLY:
	case ECOMBATGROUP_TYPE::REPAIR:
	case ECOMBATGROUP_TYPE::FREIGHT:
	case ECOMBATGROUP_TYPE::PASSENGER:
	case ECOMBATGROUP_TYPE::PRIVATE:
		return true;
	default:
		return false;
	}
}

bool CombatGroup::IsAssignable() const
{
	// Mirrors Starshatter: only certain types, and must have value > 0.
	switch (GroupType)
	{
	case ECOMBATGROUP_TYPE::CARRIER_GROUP:
	case ECOMBATGROUP_TYPE::BATTLE_GROUP:
	case ECOMBATGROUP_TYPE::DESTROYER_SQUADRON:
	case ECOMBATGROUP_TYPE::ATTACK_SQUADRON:
	case ECOMBATGROUP_TYPE::FIGHTER_SQUADRON:
	case ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON:
	case ECOMBATGROUP_TYPE::LCA_SQUADRON:
		return const_cast<CombatGroup*>(this)->CalcValue() > 0;
	default:
		return false;
	}
}

bool CombatGroup::IsTargetable() const
{
	// Mirrors Starshatter intent:
	// - Neutral/non-combatants not strategic targets
	// - Some civilian categories excluded
	// - Must have units; we don't have units yet, so use value > 0 as proxy

	if (Iff < 1 || Iff >= 100)
		return false;

	switch (GroupType)
	{
	case ECOMBATGROUP_TYPE::PASSENGER:
	case ECOMBATGROUP_TYPE::PRIVATE:
	case ECOMBATGROUP_TYPE::MEDICAL:
	case ECOMBATGROUP_TYPE::HABITAT:
		return false;
	default:
		break;
	}

	return const_cast<CombatGroup*>(this)->CalcValue() > 0;
}

bool CombatGroup::IsDefensible() const
{
	// Starshatter: if (type >= SUPPORT) defensible if value > 0.
	// We preserve ordering assumption from enum layout.
	if ((int32)GroupType >= (int32)ECOMBATGROUP_TYPE::SUPPORT)
		return const_cast<CombatGroup*>(this)->CalcValue() > 0;

	return false;
}

bool CombatGroup::IsStrikeTarget() const
{
	// Mirrors Starshatter:
	// if (type < BATTALION || type == MINEFIELD || type == PASSENGER/PRIVATE/MEDICAL/HABITAT) return false;
	if ((int32)GroupType < (int32)ECOMBATGROUP_TYPE::BATTALION)
		return false;

	switch (GroupType)
	{
	case ECOMBATGROUP_TYPE::MINEFIELD:
	case ECOMBATGROUP_TYPE::PASSENGER:
	case ECOMBATGROUP_TYPE::PRIVATE:
	case ECOMBATGROUP_TYPE::MEDICAL:
	case ECOMBATGROUP_TYPE::HABITAT:
		return false;
	default:
		break;
	}

	return const_cast<CombatGroup*>(this)->CalcValue() > 0;
}

bool CombatGroup::IsMovable() const
{
	return IsMovableType(GroupType);
}

bool CombatGroup::IsFighterGroup() const
{
	return IsFighterType(GroupType);
}

bool CombatGroup::IsStarshipGroup() const
{
	return IsStarshipType(GroupType);
}
