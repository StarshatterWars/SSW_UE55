// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once
#include "CoreMinimal.h"

// -------------------------------------------------
// Campaign cadence gate
// Controls how often a planner is allowed to run
// -------------------------------------------------
struct FCampaignGate
{
	// How often this planner should execute (seconds)
	int64 IntervalSeconds = 1;

	// Next allowed execution time (universe seconds)
	int64 NextRunSeconds = 0;

	FCampaignGate() = default;

	explicit FCampaignGate(int64 InIntervalSeconds)
		: IntervalSeconds(InIntervalSeconds)
	{
	}

	// Reset gate so planner can run immediately (or from NowSeconds)
	void Reset(int64 NowSeconds = 0)
	{
		NextRunSeconds = NowSeconds;
	}

	// Returns true if planner is allowed to run at NowSeconds
	bool Due(int64 NowSeconds)
	{
		if (NowSeconds < NextRunSeconds)
			return false;

		NextRunSeconds = NowSeconds + IntervalSeconds;
		return true;
	}
};
