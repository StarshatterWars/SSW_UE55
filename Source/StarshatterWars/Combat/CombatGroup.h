// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/
#pragma once

#include "CoreMinimal.h"
#include "GameStructs.h"

/**
 * UE-friendly port of Starshatter CombatGroup.
 * - Pure C++ (no UObject)
 * - Preserves key logic: hierarchy, reserve/intel gating, region, type/iff/id
 * - Stubs remain where you haven't ported dependent systems yet (units/zones/assignments).
 */
class CombatZone;
class Combatant;
class CombatUnit;

class CombatGroup
{
public:
	CombatGroup(ECOMBATGROUP_TYPE InType, int32 InId, const FString& InName, int32 InIff, EINTEL_TYPE InIntel, CombatGroup* InParent = nullptr);
	~CombatGroup();

	// ---------------------------------------------------------------------
	// Hierarchy / composition (core)
	// ---------------------------------------------------------------------
	void AddComponent(CombatGroup* Group);                 // takes ownership? (see notes below)
	CombatGroup* GetParent() const { return Parent; }
	const TArray<CombatGroup*>& GetComponents() const { return Components; }

	CombatGroup* FindGroup(ECOMBATGROUP_TYPE InType, int32 InId = -1);
	const CombatGroup* FindGroup(ECOMBATGROUP_TYPE InType, int32 InId = -1) const;

	// Clone behavior similar to original: shallow or deep copy of hierarchy.
	CombatGroup* Clone(bool bDeep = true) const;

	// ---------------------------------------------------------------------
	// Accessors
	// ---------------------------------------------------------------------
	const FString& GetName() const { return GroupName; }
	ECOMBATGROUP_TYPE GetType() const { return GroupType; }
	int32 GetID() const { return Id; }
	int32 GetIFF() const { return Iff; }

	// Optional setters (used during parsing)
	void SetId(int32 InId) { Id = InId; }
	void SetType(ECOMBATGROUP_TYPE InType) { GroupType = InType; }

	const FString& GetRegion() const { return Region; }
	void SetRegion(const FString& InRegion) { Region = InRegion; }
	void AssignRegion(const FString& InRegion); // mirrors Starshatter behavior (here: same as SetRegion)

	EINTEL_TYPE IntelLevel() const { return EnemyIntel; }
	void SetIntelLevel(EINTEL_TYPE InIntel) { EnemyIntel = InIntel; }

	int32 Value() const { return CachedValue; }
	int32 CalcValue(); // stubbed but functional: sums children + local "unit count" placeholder

	bool IsReserve() const;         // mirrors original logic
	bool IsMovable() const;         // mirrors original switch table
	bool IsFighterGroup() const;    // mirrors original switch table
	bool IsStarshipGroup() const;   // mirrors original switch table
	bool IsStrikeTarget() const;    // mirrors original logic shape
	bool IsTargetable() const;      // mirrors original logic shape
	bool IsDefensible() const;      // mirrors original logic shape
	bool IsAssignable() const;      // mirrors original logic shape

	// ---------------------------------------------------------------------
	// Future integration hooks (stubs for now)
	// ---------------------------------------------------------------------
	void SetCombatant(Combatant* InCombatant) { OwningCombatant = InCombatant; }
	Combatant* GetCombatant() const { return OwningCombatant; }

	CombatZone* GetCurrentZone() const { return CurrentZone; }
	void SetCurrentZone(CombatZone* InZone) { CurrentZone = InZone; }

	CombatZone* GetAssignedZone() const { return AssignedZone; }
	void SetAssignedZone(CombatZone* InZone) { AssignedZone = InZone; }

	bool IsZoneLocked() const { return AssignedZone != nullptr && bZoneLock; }
	void SetZoneLock(bool bLock = true) { bZoneLock = bLock; }

	bool IsSystemLocked() const { return !AssignedSystem.IsEmpty(); }
	const FString& GetAssignedSystem() const { return AssignedSystem; }
	void SetAssignedSystem(const FString& InSystem) { AssignedSystem = InSystem; }

private:
	static bool IsFighterType(ECOMBATGROUP_TYPE T);
	static bool IsStarshipType(ECOMBATGROUP_TYPE T);
	static bool IsMovableType(ECOMBATGROUP_TYPE T);

private:
	ECOMBATGROUP_TYPE   GroupType;
	int32        Id = 0;
	FString      GroupName;
	int32        Iff = 1;
	EINTEL_TYPE  EnemyIntel = EINTEL_TYPE::ACTIVE;

	CombatGroup* Parent = nullptr;

	// NOTE: For now, we do not delete Components in destructor automatically
	// because you may already be managing ownership elsewhere (UE subsystem, save state, etc.).
	// Once you decide ownership, you can convert to TUniquePtr or explicit deletes.
	TArray<CombatGroup*> Components;

	// Minimal state mirrored from original
	FString Region;

	int32 CachedValue = 0;

	Combatant* OwningCombatant = nullptr;

	CombatZone* CurrentZone = nullptr;
	CombatZone* AssignedZone = nullptr;
	bool bZoneLock = false;

	FString AssignedSystem;
};

