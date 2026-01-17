// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once
#include "CoreMinimal.h"
#include "GameStructs.h"

// Forward declarations (match Starshatter intent)
class CombatGroup;
class ZoneForce;

class CombatZone
{
public:
	static const TCHAR* TYPENAME() { return TEXT("CombatZone"); }

	CombatZone() = default;
	~CombatZone() = default;

	bool operator==(const CombatZone& Other) const { return this == &Other; }

	// ---- Starshatter-style accessors ----
	const FString& Name()   const { return NameStr; }
	const FString& System() const { return SystemStr; }

	// ---- Group membership (Starshatter behavior) ----
	void AddGroup(CombatGroup* Group);
	void RemoveGroup(CombatGroup* Group);
	bool HasGroup(CombatGroup* Group);

	// ---- Regions (Starshatter behavior) ----
	void AddRegion(const FString& Region);
	bool HasRegion(const FString& Region) const;

	// Optional convenience overloads (useful if some callsites still pass char*)
	void AddRegion(const char* RegionAnsi);
	bool HasRegion(const char* RegionAnsi) const;

	// ---- Collections ----
	TArray<FString>& GetRegions() { return Regions; }
	TArray<ZoneForce*>& GetForces() { return Forces; }

	// ---- Forces per IFF ----
	ZoneForce* FindForce(int32 Iff);
	ZoneForce* MakeForce(int32 Iff);

	// ---- Reset ----
	void Clear();

private:
	// Attributes (same concept as Starshatter)
	FString             NameStr;
	FString             SystemStr;
	TArray<FString>     Regions;

	// Starshatter stored ZoneForce objects in a list; we keep heap objects for stable pointers.
	TArray<ZoneForce*>  Forces;
};

// -----------------------------------------------------------------------------
// ZoneForce (Starshatter behavior preserved)
// -----------------------------------------------------------------------------
class ZoneForce
{
public:
	explicit ZoneForce(int32 InIff);

	int32 GetIFF() const { return Iff; }

	TArray<CombatGroup*>& GetGroups() { return Groups; }
	TArray<CombatGroup*>& GetTargetList() { return TargetList; }
	TArray<CombatGroup*>& GetDefendList() { return DefendList; }

	void AddGroup(CombatGroup* Group);
	void RemoveGroup(CombatGroup* Group);
	bool HasGroup(CombatGroup* Group) const;

	int32 GetNeed(int32 GroupTypeIndex) const;
	void  SetNeed(int32 GroupTypeIndex, int32 Needed);
	void  AddNeed(int32 GroupTypeIndex, int32 Needed);

private:
	int32 Iff = 0;

	TArray<CombatGroup*> Groups;
	TArray<CombatGroup*> DefendList;
	TArray<CombatGroup*> TargetList;

	// Starshatter used a fixed array need[8]
	int32 Need[8] = { 0,0,0,0,0,0,0,0 };
};
