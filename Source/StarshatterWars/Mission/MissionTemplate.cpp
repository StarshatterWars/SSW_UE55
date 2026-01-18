// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MissionTemplate.h"
#include "Mission.h"

bool MissionTemplate::IsAvailable(int64 NowSeconds, int32 PlayerRank) const
{
	if (StartAfter > 0 && NowSeconds < StartAfter)
		return false;

	if (StartBefore > 0 && NowSeconds > StartBefore)
		return false;

	if (MinRank > 0 && PlayerRank < MinRank)
		return false;

	if (MaxRank > 0 && PlayerRank > MaxRank)
		return false;

	if (ExecOnce && bExecuted)
		return false;

	return true;
}

Mission* MissionTemplate::CreateMission() const
{
	// Stub: actual creation later
	return nullptr;
}

