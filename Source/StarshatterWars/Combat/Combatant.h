#pragma once

#include "CoreMinimal.h"
#include "GameStructs.h"

// Forward declarations
class CombatGroup;
class CombatZone;

/*
 * Combatant
 *
 * Mirrors Starshatter Combatant exactly:
 * - Owns top-level force groups
 * - Identified by name + IFF
 * - Provides lookup and aggregation helpers
 *
 * In Starshatter:
 *   class Combatant : public SimObserver
 */
class Combatant
{
public:
	Combatant();
	explicit Combatant(const FString& InName, int32 InIFF);
	~Combatant();

	// ---------------------------------------------------------------------
	// Identity
	// ---------------------------------------------------------------------
	int32 GetIFF() const { return IFF; }

	// Starshatter-equivalent accessor
	const FString& GetName() const;

	void SetName(const FString& InName) { Name = InName; }
	void SetIFF(int32 InIFF) { IFF = InIFF; }

	// ---------------------------------------------------------------------
	// Groups
	// ---------------------------------------------------------------------
	void AddGroup(CombatGroup* Group);
	void RemoveGroup(CombatGroup* Group);

	const TArray<CombatGroup*>& GetGroups() const { return Groups; }

	// Starshatter helpers
	CombatGroup* FindGroup(ECOMBATGROUP_TYPE Type, int32 Id);
	CombatGroup* FindGroup(ECOMBATGROUP_TYPE Type, CombatGroup* NearGroup);

	// ---------------------------------------------------------------------
	// Zones / Regions
	// ---------------------------------------------------------------------
	bool HasZone(const FString& Region) const;

	// ---------------------------------------------------------------------
	// Aggregation
	// ---------------------------------------------------------------------
	int32 GetAllGroups(TArray<CombatGroup*>& OutGroups) const;

private:
	// Identity
	FString Name;
	int32   IFF = 0;

	// Top-level force groups
	TArray<CombatGroup*> Groups;
};
