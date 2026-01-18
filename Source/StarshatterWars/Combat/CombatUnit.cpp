// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CombatUnit.h"
#include "CombatGroup.h"
#include "CombatZone.h"

CombatUnit::CombatUnit()
{
}

CombatUnit::~CombatUnit()
{
	// No ownership here; group/campaign manage lifetimes
}

void CombatUnit::SetGroup(CombatGroup* InGroup)
{
	Group = InGroup;
}

void CombatUnit::SetRegion(const FString& InRegion)
{
	Region = InRegion;
}

void CombatUnit::SetZone(CombatZone* InZone)
{
	Zone = InZone;
}

