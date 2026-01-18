// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "Mission.h"

Mission::Mission(int32 InIdentity, const FString& InFilename, const FString& InPath)
	: Id(InIdentity)
	, Filename(InFilename)
	, Path(InPath)
{
	// Keep defaults consistent with Starshatter’s intent:
	// objective/sitrep are normally localized keys; here we keep simple placeholders.
	ObjectiveStr = TEXT("Mission.unspecified");
	SitrepStr = TEXT("Mission.unknown");
	SituationStr = TEXT("Unknown");

	// NOTE: bOk remains false until Load/Parse/Validate sets it true.
}

Mission::~Mission()
{
	// Stub ownership rules:
	// Starshatter deleted elements/events (custom list containers).
	// In your UE port, decide ownership later (TUniquePtr, UObject, etc.).
	// For now: do not delete raw pointers here.
	Elements.Reset();
	Events.Reset();
	SystemList.Reset();

	StarSystemPtr = nullptr;
}

bool Mission::Load()
{
	// Stub:
	// Later: load file Path/Filename (DataLoader equivalent), parse terms, etc.
	// For now: consider “load ok” if filename is provided OR if this is a net mission.
	bOk = !Filename.IsEmpty();
	return bOk;
}

bool Mission::ParseMission(const FString& /*Script*/)
{
	// Stub:
	// Later: parse from in-memory mission script text (net mission / editor path).
	// For now: mark ok.
	bOk = true;
	return true;
}

void Mission::Validate()
{
	// Stub:
	// Later: validate required fields (player element, objectives, etc.)
}

void Mission::SetStarSystem(StarSystem* InSystem)
{
	if (StarSystemPtr != InSystem)
	{
		StarSystemPtr = InSystem;

		if (InSystem && !SystemList.Contains(InSystem))
		{
			SystemList.Add(InSystem);
		}
	}
}

void Mission::ClearSystemList()
{
	StarSystemPtr = nullptr;
	SystemList.Reset();
}

void Mission::AddElement(MissionElement* Elem)
{
	if (Elem)
	{
		Elements.Add(Elem);
	}
}

MissionElement* Mission::FindElement(const FString& /*ElemName*/)
{
	// Stub:
	// When MissionElement is ported, implement by name:
	// for (MissionElement* E : Elements) { if (E && E->Name().Equals(ElemName,...)) return E; }
	return nullptr;
}

void Mission::IncreaseElemPriority(int32 ElemIndex)
{
	if (ElemIndex > 0 && Elements.IsValidIndex(ElemIndex))
	{
		Elements.Swap(ElemIndex - 1, ElemIndex);
	}
}

void Mission::DecreaseElemPriority(int32 ElemIndex)
{
	if (Elements.IsValidIndex(ElemIndex) && Elements.IsValidIndex(ElemIndex + 1))
	{
		Elements.Swap(ElemIndex, ElemIndex + 1);
	}
}

void Mission::AddEvent(MissionEvent* Ev)
{
	if (Ev)
	{
		Events.Add(Ev);
	}
}

void Mission::IncreaseEventPriority(int32 EventIndex)
{
	if (EventIndex > 0 && Events.IsValidIndex(EventIndex))
	{
		Events.Swap(EventIndex - 1, EventIndex);
	}
}

void Mission::DecreaseEventPriority(int32 EventIndex)
{
	if (Events.IsValidIndex(EventIndex) && Events.IsValidIndex(EventIndex + 1))
	{
		Events.Swap(EventIndex, EventIndex + 1);
	}
}

int32 Mission::TypeFromName(const FString& TypeName)
{
	// Stub mapping. Expand later with your canonical mission types.
	// Keep deterministic behavior (unknown -> 0).
	if (TypeName.IsEmpty())
		return 0;

	const FString T = TypeName.ToLower();

	if (T == TEXT("training")) return 1;
	if (T == TEXT("patrol"))   return 2;
	if (T == TEXT("strike"))   return 3;
	if (T == TEXT("escort"))   return 4;
	if (T == TEXT("defend"))   return 5;

	return 0;
}

FString Mission::NameFromType(int32 InType)
{
	switch (InType)
	{
	case 1:  return TEXT("training");
	case 2:  return TEXT("patrol");
	case 3:  return TEXT("strike");
	case 4:  return TEXT("escort");
	case 5:  return TEXT("defend");
	default: return TEXT("unknown");
	}
}
