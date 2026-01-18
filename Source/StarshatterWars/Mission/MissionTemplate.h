#pragma once

#include "CoreMinimal.h"

class Mission;
class MissionTemplate
{
public:
	static constexpr int32 TIME_NEVER = 1000000000;

	MissionTemplate() = default;
	~MissionTemplate() = default;

	// Identity
	int32 Id = 0;
	FString Name;
	FString Script;

	// Gating
	int32 MissionType = 0;
	int32 GroupType = 0;
	int32 MinRank = 0;
	int32 MaxRank = 0;
	int32 StartAfter = 0;
	int32 StartBefore = TIME_NEVER;
	int32 ExecOnce = 0;

	// Optional linkage
	int32 ActionId = 0;
	int32 ActionStatus = 0;

	// Runtime
	bool bExecuted = false;

	// Starshatter-style helpers
	bool IsAvailable(int64 NowSeconds, int32 PlayerRank) const;
	Mission* CreateMission() const;
};
