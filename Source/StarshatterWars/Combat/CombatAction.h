// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameStructs.h"

// Forward declarations (match your port structure)
class Campaign;
class Combatant;
class CombatGroup;

/**
 * Mirror of Starshatter CombatActionReq.
 * In Starshatter this supported:
 *  - action dependency (action id + status + not)
 *  - group dependency (combatant + group_type + group_id + comp + score + intel)
 *  - score dependency (c1 + c2 + comp + score)
 */
struct FCombatActionReq
{
	// ----- Requirement "kinds" -----
	// If ActionId != 0, it is an action dependency.
	int32 ActionId = 0;
	int32 ActionStatus = 0;
	bool  bNot = false;

	// If GroupType != 0, it is a group dependency.
	Combatant* C1 = nullptr;
	Combatant* C2 = nullptr;

	int32 GroupType = 0;
	int32 GroupId = 0;

	// Generic comparator for score/intel/etc.
	int32 Comp = 0;
	int32 Score = 0;
	int32 Intel = 0;

	// ---- Comparators (Starshatter: CombatActionReq::CompFromName) ----
	static int32 CompFromName(const FString& In);
	static bool  Compare(int32 L, int32 Comp, int32 R);
};


// ------------------------------------------------------------
// CombatAction
// ------------------------------------------------------------
class CombatAction
{
public:
	// ---- Mirror enums (int-based to match original Starshatter style) ----
	enum : int32
	{
		// Types (CombatAction::TypeFromName)
		// NOTE: Keep these stable because campaign.def uses names, not numbers.
		// You can extend later without breaking.
		UNKNOWN_ACTION = 0,
		INTEL_EVENT = 1,
		COMBAT_EVENT = 2,
		MISSION_TEMPLATE = 3,
		MOVE_GROUP = 4,   // common scripted action in Starshatter campaigns
		SET_INTEL = 5,   // optional / helper
	};

	enum : int32
	{
		// Status (CombatAction::StatusFromName)
		INCOMPLETE = 0,
		ACTIVE = 1,
		COMPLETE = 2,
		FAILED = 3,
	};

public:
	CombatAction();
	CombatAction(int32 InId, int32 InType, int32 InSubtype, int32 InTeam);
	~CombatAction();

	// Identity
	int32 Identity() const { return Id; }
	int32 Type() const { return ActionType; }
	int32 Subtype() const { return ActionSubtype; }

	// Status
	int32  Status() const { return ActionStatus; }
	void   SetStatus(int32 InStatus) { ActionStatus = InStatus; }

	// Timing / gating
	bool   IsAvailable(Campaign* CampaignObj) const;     // mirrors Starshatter usage
	bool   IsAvailable(int64 CampaignTimeSeconds) const; // convenience for UE
	bool   IsActive(Campaign* CampaignObj) const;

	int32  StartAfter()  const { return StartAfterSeconds; }
	int32  StartBefore() const { return StartBeforeSeconds; }

	void   SetStartAfter(int32 In) { StartAfterSeconds = In; }
	void   SetStartBefore(int32 In) { StartBeforeSeconds = In; }

	// Probability / delay
	int32  Delay() const { return DelaySeconds; }
	int32  Probability() const { return ProbabilityPct; }

	void   SetDelay(int32 In) { DelaySeconds = In; }
	void   SetProbability(int32 In) { ProbabilityPct = In; }

	// Rank gates
	int32  MinRank() const { return MinRankValue; }
	int32  MaxRank() const { return MaxRankValue; }

	void   SetMinRank(int32 In) { MinRankValue = In; }
	void   SetMaxRank(int32 In) { MaxRankValue = In; }

	// Location / system / region
	const FVector& GetLocation() const { return Location; }
	const FString& GetSystem() const { return System; }
	const FString& GetRegion() const { return Region; }

	void SetLocation(const FVector& In) { Location = In; }
	void SetSystem(const FString& In) { System = In; }
	void SetRegion(const FString& In) { Region = In; }

	// Text / files (briefings, intel, scenes)
	const FString& GetFilename() const { return Filename; }
	const FString& GetImageFile() const { return ImageFile; }
	const FString& GetSceneFile() const { return SceneFile; }
	const FString& GetText() const { return Text; }
	const FString& GetTitle() const { return Title; }

	void SetFilename(const FString& In) { Filename = In; }
	void SetImageFile(const FString& In) { ImageFile = In; }
	void SetSceneFile(const FString& In) { SceneFile = In; }
	void SetText(const FString& In) { Text = In; }
	void SetTitle(const FString& In) { Title = In; }

	// Team / IFF / source
	int32 GetTeam() const { return Team; }
	int32 GetIFF() const { return Iff; }
	int32 GetSource() const { return Source; }

	void SetTeam(int32 In) { Team = In; }
	void SetIFF(int32 In) { Iff = In; }
	void SetSource(int32 In) { Source = In; }

	// Opposing type / count (Starshatter fields)
	int32 OpposingType() const { return OppType; }
	void  SetOpposingType(int32 In) { OppType = In; }

	int32 Count() const { return CountValue; }
	void  SetCount(int32 In) { CountValue = In; }

	// Asset/Target fields (used in campaign.def actions)
	int32 AssetType() const { return AssetTypeValue; }
	int32 AssetId()   const { return AssetIdValue; }
	int32 TargetType() const { return TargetTypeValue; }
	int32 TargetId()   const { return TargetIdValue; }
	int32 TargetIFF()  const { return TargetIffValue; }

	void SetAssetType(int32 In) { AssetTypeValue = In; }
	void SetAssetId(int32 In) { AssetIdValue = In; }
	void SetTargetType(int32 In) { TargetTypeValue = In; }
	void SetTargetId(int32 In) { TargetIdValue = In; }
	void SetTargetIFF(int32 In) { TargetIffValue = In; }

	// Kills lists (Starshatter: Text list)
	TArray<FString>& AssetKills() { return AssetKillNames; }
	TArray<FString>& TargetKills() { return TargetKillNames; }
	const TArray<FString>& AssetKills() const { return AssetKillNames; }
	const TArray<FString>& TargetKills() const { return TargetKillNames; }

	// Requirements
	void AddRequirement(int32 InActionId, int32 InStatus, bool bInNot);
	void AddRequirement(Combatant* InC1, int32 InGroupType, int32 InGroupId, int32 InComp, int32 InScore, int32 InIntel);
	void AddRequirement(Combatant* InC1, Combatant* InC2, int32 InComp, int32 InScore);

	const TArray<FCombatActionReq>& Requirements() const { return Req; }

	// Evaluate requirements (stub-safe; uses campaign lookups if available)
	bool RequirementsMet(Campaign* CampaignObj) const;

	// Name parsing helpers (mirrors Starshatter API)
	static int32 TypeFromName(const FString& In);
	static int32 StatusFromName(const FString& In);

private:
	// Core identity
	int32 Id = 0;
	int32 ActionType = UNKNOWN_ACTION;
	int32 ActionSubtype = 0;
	int32 Team = 0;
	int32 Iff = 0;

	// Source / opposing / counts
	int32 Source = 0;
	int32 OppType = -1;
	int32 CountValue = 1;

	// Time gate
	int32 StartBeforeSeconds = 1000000000; // TIME_NEVER
	int32 StartAfterSeconds = 0;

	// Rank gate
	int32 MinRankValue = 0;
	int32 MaxRankValue = 100;

	// Delay / probability
	int32 DelaySeconds = 0;
	int32 ProbabilityPct = 100;

	// Location / region info
	FVector Location = FVector::ZeroVector;
	FString System;
	FString Region;

	// Files / text
	FString Filename;
	FString ImageFile;
	FString SceneFile;
	FString Text;
	FString Title;

	// Asset/target
	int32 AssetTypeValue = 0;
	int32 AssetIdValue = 0;
	int32 TargetTypeValue = 0;
	int32 TargetIdValue = 0;
	int32 TargetIffValue = 0;

	// Kill lists
	TArray<FString> AssetKillNames;
	TArray<FString> TargetKillNames;

	// Requirements
	TArray<FCombatActionReq> Req;

	// Current status
	int32 ActionStatus = INCOMPLETE;
};
