// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once
#include "CoreMinimal.h"

// -------------------------------------------------
// Campaign tick context
// Passed from CampaignSubsystem -> planners
// -------------------------------------------------
struct FCampaignTickContext
{
	// Authoritative universe / campaign time (seconds)
	int64 NowSeconds = 0;

	// Delta since last tick (seconds)
	int64 DeltaSeconds = 0;

	// Shared deterministic RNG (owned by CampaignSubsystem)
	FRandomStream* Rng = nullptr;
};
