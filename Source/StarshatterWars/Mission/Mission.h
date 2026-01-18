// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"

// Forward declarations (match your incremental porting approach)
class StarSystem;
class MissionElement;
class MissionEvent;

/**
 * Mission (Starshatter-style, UE stub-first)
 *
 * Goals:
 * - Keep original surface area (construct/load/parse/validate, start/active/complete, etc.)
 * - Use UE types (FString, TArray) and remain compile-safe
 * - Defer real parsing/sim integration until later
 */
class Mission
{
public:
	// Mirrors Starshatter constructors:
	// Mission(int identity, const char* fname, const char* pname)
	Mission(int32 InIdentity, const FString& InFilename = TEXT(""), const FString& InPath = TEXT("Missions/"));
	~Mission();

	// ---------------------------------------------------------------------
	// Load / Parse / Validate (stubs for now)
	// ---------------------------------------------------------------------
	bool Load();                               // load from Filename+Path (stub)
	bool ParseMission(const FString& Script);  // parse from in-memory text (stub)
	void Validate();                           // post-parse validation (stub)

	// ---------------------------------------------------------------------
	// Identity / basics
	// ---------------------------------------------------------------------
	int32 Identity() const { return Id; }

	const FString& Name() const { return NameStr; }
	void SetName(const FString& InName) { NameStr = InName; }

	int32 GetType() const { return Type; }
	void  SetType(int32 InType) { Type = InType; }

	int32 GetTeam() const { return Team; }
	void  SetTeam(int32 InTeam) { Team = InTeam; }

	// ---------------------------------------------------------------------
	// Time fields (Starshatter uses start in seconds-from-day start)
	// ---------------------------------------------------------------------
	int32 Start() const { return StartSeconds; }
	void  SetStart(int32 InStartSeconds) { StartSeconds = InStartSeconds; }

	double Stardate() const { return StardateSeconds; }
	void   SetStardate(double InStardate) { StardateSeconds = InStardate; }

	// ---------------------------------------------------------------------
	// Status flags
	// ---------------------------------------------------------------------
	bool IsOk() const { return bOk; }
	bool IsActive() const { return bActive; }
	bool IsComplete() const { return bComplete; }

	void SetOk(bool bInOk) { bOk = bInOk; }
	void SetActive(bool bInActive) { bActive = bInActive; }
	void SetComplete(bool bInComplete) { bComplete = bInComplete; }

	// ---------------------------------------------------------------------
	// Text blocks (objective/sitrep/situation) used by campaign UI
	// ---------------------------------------------------------------------
	const FString& Objective() const { return ObjectiveStr; }
	void SetObjective(const FString& InObjective) { ObjectiveStr = InObjective; }

	const FString& Sitrep() const { return SitrepStr; }
	void SetSitrep(const FString& InSitrep) { SitrepStr = InSitrep; }

	const FString& Situation() const { return SituationStr; }
	void SetSituation(const FString& InSituation) { SituationStr = InSituation; }

	const FString& Subtitles() const { return SubtitlesStr; }
	void SetSubtitles(const FString& InSubtitles) { SubtitlesStr = InSubtitles; }

	// ---------------------------------------------------------------------
	// File references
	// ---------------------------------------------------------------------
	const FString& GetFilename() const { return Filename; }
	const FString& GetPath() const { return Path; }
	void SetFilename(const FString& InFilename) { Filename = InFilename; }
	void SetPath(const FString& InPath) { Path = InPath; }

	// ---------------------------------------------------------------------
	// Star system association (stubs)
	// ---------------------------------------------------------------------
	StarSystem* GetStarSystem() const { return StarSystemPtr; }
	void SetStarSystem(StarSystem* InSystem);
	void ClearSystemList();

	// ---------------------------------------------------------------------
	// Elements / Events (stubs)
	// ---------------------------------------------------------------------
	void AddElement(MissionElement* Elem);
	MissionElement* FindElement(const FString& ElemName);

	void IncreaseElemPriority(int32 ElemIndex);
	void DecreaseElemPriority(int32 ElemIndex);

	void AddEvent(MissionEvent* Ev);
	void IncreaseEventPriority(int32 EventIndex);
	void DecreaseEventPriority(int32 EventIndex);

	const TArray<MissionElement*>& GetElements() const { return Elements; }
	const TArray<MissionEvent*>& GetEvents() const { return Events; }

	// ---------------------------------------------------------------------
	// Type mapping (stubs — fill in when you port Mission::TypeFromName)
	// ---------------------------------------------------------------------
	static int32 TypeFromName(const FString& TypeName);
	static FString NameFromType(int32 InType);

private:
	// Identity
	int32 Id = -1;

	// Type/team (Starshatter: type, team)
	int32 Type = 0;
	int32 Team = 1;

	// Status flags (Starshatter: ok, active, complete)
	bool bOk = false;
	bool bActive = false;
	bool bComplete = false;

	// Time
	int32  StartSeconds = 33 * 3600;   // mirrors original default
	double StardateSeconds = 0.0;

	// File references
	FString Filename;
	FString Path;

	// Strings (objective/sitrep/situation/subtitles/name)
	FString NameStr;
	FString ObjectiveStr;
	FString SitrepStr;
	FString SituationStr;
	FString SubtitlesStr;

	// Star system pointers (stubs)
	StarSystem* StarSystemPtr = nullptr;
	TArray<StarSystem*> SystemList;

	// Elements / events
	TArray<MissionElement*> Elements;
	TArray<MissionEvent*> Events;
};
