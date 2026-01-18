// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CombatEvent.h"
#include "Campaign.h"      // for Campaign::Path(), GetTime(), GetPlayerGroup(), etc. (as you add them)
#include "CombatGroup.h"   // for GetDescription()/GetRegion/etc. (as your CombatGroup exposes them)

// -----------------------------------------------------------------------------
// Ctor mirrors Starshatter signature/field init
// -----------------------------------------------------------------------------
CombatEvent::CombatEvent(Campaign* InCampaign, int32 InType, int32 InTimeSeconds, int32 InTeam, int32 InSource, const FString& InRegion)
	: CampaignObj(InCampaign)
	, Type(InType)
	, TimeSeconds(InTimeSeconds)
	, Team(InTeam)
	, Source(InSource)
	, Region(InRegion)
	, Points(0)
	, bVisited(false)
{
}

// -----------------------------------------------------------------------------
// Name helpers
// -----------------------------------------------------------------------------
const TCHAR* CombatEvent::SourceName() const
{
	return SourceName(Source);
}

const TCHAR* CombatEvent::TypeName() const
{
	return TypeName(Type);
}

const TCHAR* CombatEvent::SourceName(int32 SourceId)
{
	switch (SourceId)
	{
	case FORCOM: return TEXT("FORCOM");
	case TACNET: return TEXT("TACNET");
	case INTEL:  return TEXT("SECURE");
	case MAIL:   return TEXT("Mail");
	case NEWS:   return TEXT("News");
	default:     return TEXT("Unknown");
	}
}

int32 CombatEvent::SourceFromName(const FString& Name)
{
	// Starshatter loops FORCOM..NEWS and compares case-insensitive.
	for (int32 i = FORCOM; i <= NEWS; ++i)
	{
		if (Name.Equals(SourceName(i), ESearchCase::IgnoreCase))
			return i;
	}
	return -1;
}

const TCHAR* CombatEvent::TypeName(int32 TypeId)
{
	switch (TypeId)
	{
	case ATTACK:         return TEXT("ATTACK");
	case DEFEND:         return TEXT("DEFEND");
	case MOVE_TO:        return TEXT("MOVE_TO");
	case CAPTURE:        return TEXT("CAPTURE");
	case STRATEGY:       return TEXT("STRATEGY");
	case STORY:          return TEXT("STORY");
	case CAMPAIGN_START: return TEXT("CAMPAIGN_START");
	case CAMPAIGN_END:   return TEXT("CAMPAIGN_END");
	case CAMPAIGN_FAIL:  return TEXT("CAMPAIGN_FAIL");
	default:             return TEXT("Unknown");
	}
}

int32 CombatEvent::TypeFromName(const FString& Name)
{
	for (int32 i = ATTACK; i <= CAMPAIGN_FAIL; ++i)
	{
		if (Name.Equals(TypeName(i), ESearchCase::IgnoreCase))
			return i;
	}
	return -1;
}

// -----------------------------------------------------------------------------
// Token replace helper (simple and safe)
// -----------------------------------------------------------------------------
static void ReplaceAllInline(FString& InOutText, const FString& From, const FString& To)
{
	if (From.IsEmpty())
		return;

	InOutText = InOutText.Replace(*From, *To, ESearchCase::IgnoreCase);
}

// -----------------------------------------------------------------------------
// Load() (stubbed) - preserves original behavior shape:
// - load info text from File
// - if contains '$', substitute $NAME/$RANK/$GROUP/$TIME
// - load image bitmap for events before CAMPAIGN_END
// -----------------------------------------------------------------------------
void CombatEvent::Load()
{
	// In Starshatter, this uses DataLoader + reads File contents into Info.
	// You are DT-driven right now, so keep this as a stub:
	if (!CampaignObj)
		return;

	// If you later wire disk reads: load File (relative to CampaignObj->Path()) into Info.
	// For now, if Info is already provided (DT), we just apply token replacement.
	if (Info.IsEmpty() && !File.IsEmpty())
	{
		// Stub placeholder so you can see it in UI and know the loader isn't wired yet:
		Info = FString::Printf(TEXT("[CombatEvent] TODO: load text from file: %s"), *File);
	}

	// Token replacement mirrors original trigger: "if (info.contains('$'))"
	if (Info.Contains(TEXT("$")))
	{
		// Player info not ported here; keep safe placeholders until you have Player/Profile wired.
		ReplaceAllInline(Info, TEXT("$NAME"), TEXT("Pilot"));
		ReplaceAllInline(Info, TEXT("$RANK"), TEXT("Unknown Rank"));

		// Group description (only if Campaign exposes it; otherwise safe)
		if (CombatGroup* Group = CampaignObj->GetPlayerGroup())
		{
			ReplaceAllInline(Info, TEXT("$GROUP"), Group->GetDescription());
		}
		else
		{
			ReplaceAllInline(Info, TEXT("$GROUP"), TEXT("Unknown Group"));
		}

		// Time formatting: Starshatter uses FormatDayTime(campaign->GetTime(), true).
		// You can improve later; for now we expose campaign time seconds.
		const int64 CampaignTimeSeconds = CampaignObj->GetTime(); // you added GetTime() returning int64
		const FString TimeStr = FString::Printf(TEXT("%llds"), (long long)CampaignTimeSeconds);
		ReplaceAllInline(Info, TEXT("$TIME"), TimeStr);
	}

	// Image load is stubbed. Keep the conditional so behavior matches:
	// "if (type < CAMPAIGN_END && image_file.length() > 0) load bitmap"
	if (Type < CAMPAIGN_END && !ImageFile.IsEmpty())
	{
		// TODO: load UTexture2D* or your own image handle
		// For now do nothing.
	}
}

void CombatEvent::ReplaceAllInline(FString& InOut, const FString& From, const FString& To)
{
	if (InOut.IsEmpty() || From.IsEmpty())
		return;

	// Replace all, case-sensitive to mirror Starshatter token behavior
	InOut.ReplaceInline(*From, *To, ESearchCase::CaseSensitive);
}

void CombatEvent::ReplaceAllInline(FString& InOut, const TCHAR* From, const FString& To)
{
	if (!From || !*From)
		return;

	InOut.ReplaceInline(From, *To, ESearchCase::CaseSensitive);
}
