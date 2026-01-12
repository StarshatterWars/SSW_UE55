#include "CampaignSave.h"

FString UCampaignSave::MakeSlotNameFromCampaignIndex(int32 InCampaignIndex)
{
	// Enforce 1-based campaign index
	const int32 SafeIndex = FMath::Max(1, InCampaignIndex);

	// Campaign 1 -> Slot 0
	const int32 SlotNumber = SafeIndex - 1;

	return FString::Printf(TEXT("CampaignSave_%d"), SlotNumber);
}

void UCampaignSave::InitializeCampaignClock(uint64 UniverseTimeSecondsNow)
{
	CampaignStartUniverseSeconds = (int64)UniverseTimeSecondsNow;
	bInitialized = true;
}

uint64 UCampaignSave::GetTPlusSeconds(uint64 UniverseTimeSecondsNow) const
{
	if (!bInitialized)
	{
		return NULL;
	}

	const int64 Now = (int64)UniverseTimeSecondsNow;

	// Guard against underflow / weird load ordering
	if (Now <= CampaignStartUniverseSeconds)
	{
		return NULL;
	}

	return (uint64)(Now - CampaignStartUniverseSeconds);
}

FString UCampaignSave::FormatTPlus_DD_HHMMSS(uint64 TPlusSeconds)
{
	const uint64 Days0 = TPlusSeconds / 86400ULL;
	const uint64 RemD = TPlusSeconds % 86400ULL;

	const uint64 Hours = RemD / 3600ULL;
	const uint64 RemH = RemD % 3600ULL;

	const uint64 Mins = RemH / 60ULL;
	const uint64 Secs = RemH % 60ULL;

	// Starshatter-like 1-based day display:
	// T+0 => 01/00:00:00
	const uint64 DayDisplay = Days0 + 1ULL;

	return FString::Printf(TEXT("%02llu/%02llu:%02llu:%02llu"),
		DayDisplay, Hours, Mins, Secs);
}

FString UCampaignSave::GetTPlusDisplay(uint64 UniverseTimeSecondsNow) const
{
	return FormatTPlus_DD_HHMMSS(GetTPlusSeconds(UniverseTimeSecondsNow));
}
