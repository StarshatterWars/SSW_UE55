// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MissionEvent.h"

namespace
{
	static const TCHAR* GetTypeNameToTChar(MISSIONEVENT_TYPE Value)
	{
		const UEnum* Enum = StaticEnum<MISSIONEVENT_TYPE>();
		if (!Enum)
		{
			return TEXT("Invalid");
		}

		thread_local FString Temp;
		Temp = Enum->GetDisplayNameTextByValue(static_cast<int64>(Value)).ToString();
		return *Temp;
	}

	static const TCHAR* GetTriggerNameToTChar(MISSIONEVENT_TRIGGER Value)
	{
		const UEnum* Enum = StaticEnum<MISSIONEVENT_TRIGGER>();
		if (!Enum)
		{
			return TEXT("Invalid");
		}

		thread_local FString Temp;
		Temp = Enum->GetDisplayNameTextByValue(static_cast<int64>(Value)).ToString();
		return *Temp;
	}

	// Keep names stable for debug tooling and parity with Starshatter
	static const TCHAR* GEventNames[(int8)MISSIONEVENT_TYPE::NUM_EVENTS] =
	{
		TEXT("MESSAGE"),
		TEXT("OBJECTIVE"),
		TEXT("INSTRUCTION"),
		TEXT("IFF"),
		TEXT("DAMAGE"),
		TEXT("JUMP"),
		TEXT("HOLD"),
		TEXT("SKIP"),
		TEXT("END_MISSION"),
		TEXT("BEGIN_SCENE"),
		TEXT("CAMERA"),
		TEXT("VOLUME"),
		TEXT("DISPLAY"),
		TEXT("FIRE_WEAPON"),
		TEXT("END_SCENE"),
	};

	static const TCHAR* GTriggerNames[(int8)MISSIONEVENT_TRIGGER::NUM_TRIGGERS] =
	{
		TEXT("TIME"),
		TEXT("DAMAGE"),
		TEXT("DESTROYED"),
		TEXT("JUMP"),
		TEXT("LAUNCH"),
		TEXT("DOCK"),
		TEXT("NAVPT"),
		TEXT("EVENT"),
		TEXT("SKIPPED"),
		TEXT("TARGET"),
		TEXT("SHIPS_LEFT"),
		TEXT("DETECT"),
		TEXT("RANGE"),
		TEXT("EVENT_ALL"),
		TEXT("EVENT_ANY"),
	};

	static int32 FindNameIndex(const TCHAR* const* Names, int32 Count, const FString& Query)
	{
		if (Query.IsEmpty())
			return -1;

		for (int32 i = 0; i < Count; ++i)
		{
			if (Query.Equals(Names[i], ESearchCase::IgnoreCase))
				return i;
		}
		return -1;
	}
}

MissionEvent::MissionEvent()
{
	// Initialize param arrays deterministically
	for (int32 i = 0; i < MaxEventParams; ++i)
		EventParams[i] = 0;

	for (int32 i = 0; i < MaxTriggerParams; ++i)
		TriggerParams[i] = 0;

	// Defaults mirror Starshatter intent:
	StatusValue = MISSIONEVENT_STATUS::PENDING;
	EventType = MISSIONEVENT_TYPE::MESSAGE;
	TriggerType = MISSIONEVENT_TRIGGER::TRIGGER_TIME;
	EventChancePct = 100;
	TimeSeconds = 0.0;
	DelaySeconds = 0.0;
}

void MissionEvent::ExecFrame(double Seconds)
{
	if (Seconds <= 0.0)
		return;

	AccumulatedSeconds += Seconds;

	// Stub behavior:
	// - If pending, test triggers
	// - If active, execute once (or keep active for HOLD/SCENE later)
	if (IsPending())
	{
		if (CheckTrigger())
		{
			Activate();
		}
	}

	if (IsActive())
	{
		Execute(/*bSilent=*/false);
	}
}

void MissionEvent::Activate()
{
	if (!IsPending())
		return;

	StatusValue = MISSIONEVENT_STATUS::ACTIVE;
}

bool MissionEvent::CheckTrigger()
{
	// This is intentionally conservative:
	// - TRIGGER_TIME: activates when accumulated time >= (TimeSeconds + DelaySeconds)
	// - Other triggers: not wired yet, return false
	switch (TriggerType)
	{
	case MISSIONEVENT_TRIGGER::TRIGGER_TIME:
	{
		const double Gate = TimeSeconds + DelaySeconds;
		return AccumulatedSeconds >= Gate;
	}
	default:
		return false;
	}
}

bool MissionEvent::PassChanceGate() const
{
	// Starshatter default: probability 100
	if (EventChancePct <= 0)
		return false;

	if (EventChancePct >= 100)
		return true;

	// Deterministic-ish: use rand for now; replace with your campaign RNG later
	const int32 Roll = FMath::RandRange(1, 100);
	return Roll <= EventChancePct;
}

void MissionEvent::Execute(bool bSilent)
{
	// Mirror Starshatter “one shot” behavior for most events:
	// - gate on chance
	// - mark COMPLETE unless you later model HOLD/SCENE with persistent active state
	if (!IsActive())
		return;

	if (!PassChanceGate())
	{
		StatusValue = MISSIONEVENT_STATUS::COMPLETE;
		return;
	}

	// Stub: later, fire message/intel/objective hooks into UI, audio, etc.
	// We keep bSilent to match original signature.
	(void)bSilent;

	StatusValue = MISSIONEVENT_STATUS::COMPLETE;
}

void MissionEvent::Skip()
{
	if (IsComplete())
		return;

	StatusValue = MISSIONEVENT_STATUS::SKIPPED;
}

const TCHAR* MissionEvent::EventName() const
{
	return GetTypeNameToTChar(EventType);
}

const TCHAR* MissionEvent::TriggerName() const
{
	return GetTriggerNameToTChar(TriggerType);
}

int32 MissionEvent::EventParam(int32 Index) const
{
	if (Index < 0 || Index >= EventParamCount || Index >= MaxEventParams)
		return 0;

	return EventParams[Index];
}

int32 MissionEvent::TriggerParam(int32 Index) const
{
	if (Index < 0 || Index >= TriggerParamCount || Index >= MaxTriggerParams)
		return 0;

	return TriggerParams[Index];
}

FString MissionEvent::TriggerParamStr() const
{
	if (TriggerParamCount <= 0)
		return FString();

	FString Out;
	for (int32 i = 0; i < TriggerParamCount; ++i)
	{
		if (i > 0)
			Out += TEXT(",");

		Out += FString::FromInt(TriggerParams[i]);
	}
	return Out;
}

const TCHAR* MissionEvent::EventName(int32 N)
{
	if (N < 0 || N >= (int32)MISSIONEVENT_TYPE::NUM_EVENTS)
		return TEXT("UNKNOWN_EVENT");

	return GEventNames[N];
}

int32 MissionEvent::EventForName(const FString& Name)
{
	return FindNameIndex(GEventNames, (int32)MISSIONEVENT_TYPE::NUM_EVENTS, Name);
}

const TCHAR* MissionEvent::TriggerName(int32 N)
{
	if (N < 0 || N >= (int32)MISSIONEVENT_TRIGGER::NUM_TRIGGERS)
		return TEXT("UNKNOWN_TRIGGER");

	return GTriggerNames[N];
}

int32 MissionEvent::TriggerForName(const FString& Name)
{
	return FindNameIndex(GTriggerNames, (int32)MISSIONEVENT_TRIGGER::NUM_TRIGGERS, Name);
}

void MissionEvent::SetEventParams(const TArray<int32>& InParams)
{
	EventParamCount = FMath::Clamp(InParams.Num(), 0, MaxEventParams);
	for (int32 i = 0; i < EventParamCount; ++i)
		EventParams[i] = InParams[i];

	// Clear remainder for determinism
	for (int32 i = EventParamCount; i < MaxEventParams; ++i)
		EventParams[i] = 0;
}

void MissionEvent::SetTriggerParams(const TArray<int32>& InParams)
{
	TriggerParamCount = FMath::Clamp(InParams.Num(), 0, MaxTriggerParams);
	for (int32 i = 0; i < TriggerParamCount; ++i)
		TriggerParams[i] = InParams[i];

	for (int32 i = TriggerParamCount; i < MaxTriggerParams; ++i)
		TriggerParams[i] = 0;
}
