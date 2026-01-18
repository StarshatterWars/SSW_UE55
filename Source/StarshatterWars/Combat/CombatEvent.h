// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameStructs.h"

// Forward declarations (keep light; avoid circular includes)
class Campaign;
class CombatGroup;

/**
 * CombatEvent
 * Mirror of Starshatter 4.5 CombatEvent, UE-friendly.
 * - Keeps Source/Type enums + Name<->Id mapping
 * - Keeps fields: campaign, type, time, team, source, region, points, visited, file, image_file, info
 * - Load() is stubbed (you will wire to your data tables / file loader later)
 */
class CombatEvent
{
public:
	// ---------------------------------------------------------------------
	// Source constants (match original ordering intent)
	// ---------------------------------------------------------------------
	enum ESource : int32
	{
		FORCOM = 1,
		TACNET = 2,
		INTEL = 3,
		MAIL = 4,
		NEWS = 5
	};

	// ---------------------------------------------------------------------
	// Type constants (match original ordering intent)
	// ---------------------------------------------------------------------
	enum EType : int32
	{
		ATTACK = 1,
		DEFEND = 2,
		MOVE_TO = 3,
		CAPTURE = 4,
		STRATEGY = 5,
		STORY = 6,
		CAMPAIGN_START = 7,
		CAMPAIGN_END = 8,
		CAMPAIGN_FAIL = 9
	};

public:
	CombatEvent(Campaign* InCampaign, int32 InType, int32 InTimeSeconds, int32 InTeam, int32 InSource, const FString& InRegion);
	~CombatEvent() = default;

	// ---------------------------------------------------------------------
	// Core API (Starshatter-compatible)
	// ---------------------------------------------------------------------
	const TCHAR* SourceName() const;
	const TCHAR* TypeName() const;

	static const TCHAR* SourceName(int32 SourceId);
	static int32        SourceFromName(const FString& Name);

	static const TCHAR* TypeName(int32 TypeId);
	static int32        TypeFromName(const FString& Name);

	// ---------------------------------------------------------------------
	// Load (stubbed). In Starshatter, this loaded info text + image bitmap.
	// Here: you will wire to your DT / filesystem later.
	// ---------------------------------------------------------------------
	void Load();

	// ---------------------------------------------------------------------
	// Accessors / mutators
	// ---------------------------------------------------------------------
	Campaign* GetCampaign() const { return CampaignObj; }

	int32  GetType()   const { return Type; }
	int32  GetTime()   const { return TimeSeconds; }
	int32  GetTeam()   const { return Team; }
	int32  GetSource() const { return Source; }

	const FString& GetRegion() const { return Region; }

	int32  GetPoints() const { return Points; }
	void   SetPoints(int32 InPoints) { Points = InPoints; }

	bool   Visited() const { return bVisited; }
	void   SetVisited(bool bInVisited) { bVisited = bInVisited; }

	const FString& GetFile() const { return File; }
	void SetFile(const FString& InFile) { File = InFile; }

	const FString& GetImageFile() const { return ImageFile; }
	void SetImageFile(const FString& InImageFile) { ImageFile = InImageFile; }

	const FString& GetInfo() const { return Info; }
	void SetInfo(const FString& InInfo) { Info = InInfo; }

private:
	Campaign* CampaignObj = nullptr;

	int32 Type = 0;
	int32 TimeSeconds = 0;
	int32 Team = 0;
	int32 Source = 0;

	FString Region;

	int32 Points = 0;
	bool  bVisited = false;

	// In Starshatter: file contains the body text filename; image_file contains bitmap filename.
	FString File;
	FString ImageFile;

	// In Starshatter: loaded from File; token-substituted.
	FString Info;

	static void ReplaceAllInline(FString& InOut, const FString& From, const FString& To);
	static void ReplaceAllInline(FString& InOut, const TCHAR* From, const FString& To);
};
