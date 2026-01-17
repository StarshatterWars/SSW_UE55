#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatZone.h"

Combatant::Combatant()
{
}

Combatant::Combatant(const FString& InName, int32 InIFF)
	: Name(InName)
	, IFF(InIFF)
{
}

Combatant::~Combatant()
{
	// IMPORTANT:
	// Starshatter Combatant does NOT own CombatGroups.
	// Ownership remains with Campaign / simulation layer.
	Groups.Reset();
}


const FString& Combatant::GetName() const
{
	return Name;
}
// ---------------------------------------------------------------------
// Groups
// ---------------------------------------------------------------------

void Combatant::AddGroup(CombatGroup* Group)
{
	if (!Group)
		return;

	if (!Groups.Contains(Group))
	{
		Groups.Add(Group);
	}
}

void Combatant::RemoveGroup(CombatGroup* Group)
{
	if (!Group)
		return;

	Groups.Remove(Group);
}

CombatGroup* Combatant::FindGroup(ECOMBATGROUP_TYPE Type, int32 Id)
{
	for (CombatGroup* G : Groups)
	{
		if (!G)
			continue;

		if (G->GetType() == Type && G->GetID() == Id)
			return G;
	}

	return nullptr;
}

CombatGroup* Combatant::FindGroup(ECOMBATGROUP_TYPE Type, CombatGroup* NearGroup)
{
	if (!NearGroup)
		return nullptr;

	const FString& Region = NearGroup->GetRegion();

	for (CombatGroup* G : Groups)
	{
		if (!G)
			continue;

		if (G->GetType() == Type && G->GetRegion().Equals(Region, ESearchCase::IgnoreCase))
			return G;
	}

	return nullptr;
}

// ---------------------------------------------------------------------
// Zones / Regions
// ---------------------------------------------------------------------

bool Combatant::HasZone(const FString& Region) const
{
	for (CombatGroup* G : Groups)
	{
		if (!G)
			continue;

		if (G->GetRegion().Equals(Region, ESearchCase::IgnoreCase))
			return true;
	}

	return false;
}

// ---------------------------------------------------------------------
// Aggregation
// ---------------------------------------------------------------------

int32 Combatant::GetAllGroups(TArray<CombatGroup*>& OutGroups) const
{
	OutGroups.Reset();

	for (CombatGroup* G : Groups)
	{
		if (G)
			OutGroups.Add(G);
	}

	return OutGroups.Num();
}
