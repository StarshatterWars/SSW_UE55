// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once
#include "CoreMinimal.h"

class CombatGroup;
class CombatZone;

/*
 * CombatUnit
 * ----------
 * Leaf-level unit (ship, fighter, facility, etc.)
 * Mirrors Starshatter CombatUnit interface, stripped to essentials.
 */
class CombatUnit
{
public:
	CombatUnit();
	virtual ~CombatUnit();

	// Identity
	int32        GetId() const { return Id; }
	const FString& GetName() const { return Name; }

	// Ownership / hierarchy
	CombatGroup* GetGroup() const { return Group; }
	void         SetGroup(CombatGroup* InGroup);

	// Location / zone
	const FString& GetRegion() const { return Region; }
	void           SetRegion(const FString& InRegion);

	CombatZone* GetZone() const { return Zone; }
	void        SetZone(CombatZone* InZone);

	// IFF / team
	int32 GetIFF() const { return Iff; }
	void  SetIFF(int32 InIff) { Iff = InIff; }

	// Status flags (minimal but expandable)
	bool IsDestroyed() const { return bDestroyed; }
	void SetDestroyed(bool bInDestroyed) { bDestroyed = bInDestroyed; }

	// Lifecycle (stubs)
	virtual void ExecFrame(double /*DeltaSeconds*/) {}
	virtual void Reset() {}

private:
	// Identity
	int32   Id = 0;
	FString Name;

	// Hierarchy
	CombatGroup* Group = nullptr;

	// Location
	FString     Region;
	CombatZone* Zone = nullptr;

	// Team
	int32 Iff = 0;

	// State
	bool bDestroyed = false;
};
