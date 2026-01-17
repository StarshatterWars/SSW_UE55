// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CombatZone.h"
#include "CombatGroup.h"

// Your CombatGroup port should expose these (Starshatter-equivalent):
//   int32 GetIFF() const;
//   void  SetCurrentZone(CombatZone*);
// If your signatures differ, adjust these two call sites only.

void CombatZone::Clear()
{
	// Starshatter: forces.destroy();
	for (ZoneForce* F : Forces)
	{
		delete F;
	}
	Forces.Reset();
}

void CombatZone::AddGroup(CombatGroup* Group)
{
	if (!Group)
		return;

	const int32 Iff = Group->GetIFF();
	ZoneForce* F = FindForce(Iff);
	if (F)
	{
		F->AddGroup(Group);
		Group->SetCurrentZone(this);
	}
}

void CombatZone::RemoveGroup(CombatGroup* Group)
{
	if (!Group)
		return;

	const int32 Iff = Group->GetIFF();
	ZoneForce* F = FindForce(Iff);
	if (F)
	{
		F->RemoveGroup(Group);
		Group->SetCurrentZone(nullptr);
	}
}

bool CombatZone::HasGroup(CombatGroup* Group)
{
	if (!Group)
		return false;

	const int32 Iff = Group->GetIFF();
	ZoneForce* F = FindForce(Iff);
	return F ? F->HasGroup(Group) : false;
}

void CombatZone::AddRegion(const FString& Region)
{
	if (Region.IsEmpty())
		return;

	Regions.Add(Region);

	// Starshatter: if (name.length() < 1) name = rgn;
	if (NameStr.IsEmpty())
	{
		NameStr = Region;
	}
}

bool CombatZone::HasRegion(const FString& Region) const
{
	if (Region.IsEmpty() || Regions.Num() == 0)
		return false;

	// Starshatter used Text contains; we do case-insensitive match for safety
	for (const FString& R : Regions)
	{
		if (R.Equals(Region, ESearchCase::IgnoreCase))
			return true;
	}

	return false;
}

void CombatZone::AddRegion(const char* RegionAnsi)
{
	if (!RegionAnsi || !*RegionAnsi)
		return;

	AddRegion(FString(UTF8_TO_TCHAR(RegionAnsi)));
}

bool CombatZone::HasRegion(const char* RegionAnsi) const
{
	if (!RegionAnsi || !*RegionAnsi)
		return false;

	return HasRegion(FString(UTF8_TO_TCHAR(RegionAnsi)));
}

ZoneForce* CombatZone::FindForce(int32 Iff)
{
	// Starshatter: iterate forces and return match else MakeForce(iff)
	for (ZoneForce* F : Forces)
	{
		if (F && F->GetIFF() == Iff)
			return F;
	}

	return MakeForce(Iff);
}

ZoneForce* CombatZone::MakeForce(int32 Iff)
{
	ZoneForce* F = new ZoneForce(Iff);
	Forces.Add(F);
	return F;
}

// -----------------------------------------------------------------------------
// ZoneForce
// -----------------------------------------------------------------------------
ZoneForce::ZoneForce(int32 InIff)
	: Iff(InIff)
{
	for (int32 i = 0; i < 8; ++i)
		Need[i] = 0;
}

void ZoneForce::AddGroup(CombatGroup* Group)
{
	if (!Group)
		return;

	Groups.Add(Group);
}

void ZoneForce::RemoveGroup(CombatGroup* Group)
{
	if (!Group)
		return;

	Groups.RemoveSingle(Group);
}

bool ZoneForce::HasGroup(CombatGroup* Group) const
{
	if (!Group)
		return false;

	return Groups.Contains(Group);
}

int32 ZoneForce::GetNeed(int32 GroupTypeIndex) const
{
	if (GroupTypeIndex < 0 || GroupTypeIndex >= 8)
		return 0;

	return Need[GroupTypeIndex];
}

void ZoneForce::SetNeed(int32 GroupTypeIndex, int32 Needed)
{
	if (GroupTypeIndex < 0 || GroupTypeIndex >= 8)
		return;

	Need[GroupTypeIndex] = Needed;
}

void ZoneForce::AddNeed(int32 GroupTypeIndex, int32 Needed)
{
	if (GroupTypeIndex < 0 || GroupTypeIndex >= 8)
		return;

	Need[GroupTypeIndex] += Needed;
}
