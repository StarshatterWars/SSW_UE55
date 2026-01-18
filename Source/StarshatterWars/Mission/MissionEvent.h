// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameStructs.h"

// Forward declarations (wire later)
class Mission;

/**
 * MissionEvent
 *  - UE port stub of Starshatter MissionEvent
 *  - Preserves method inventory and gating structure
 *  - Uses UE containers/types; no hard dependencies on other subsystems yet
 */
class MissionEvent
{
public:
	static const TCHAR* TYPENAME() { return TEXT("MissionEvent"); }

	MissionEvent();
	virtual ~MissionEvent() = default;

	// operations:
	void         ExecFrame(double Seconds);
	void         Activate();

	virtual bool CheckTrigger();
	virtual void Execute(bool bSilent = false);
	virtual void Skip();

	// accessors:
	int32        EventID()      const { return Id; }
	MISSIONEVENT_STATUS Status() const { return StatusValue; }
	bool         IsPending()    const { return StatusValue == MISSIONEVENT_STATUS::PENDING; }
	bool         IsActive()     const { return StatusValue == MISSIONEVENT_STATUS::ACTIVE; }
	bool         IsComplete()   const { return StatusValue == MISSIONEVENT_STATUS::COMPLETE; }
	bool         IsSkipped()    const { return StatusValue == MISSIONEVENT_STATUS::SKIPPED; }

	double       Time()         const { return TimeSeconds; }
	double       Delay()        const { return DelaySeconds; }

	MISSIONEVENT_TYPE Event()   const { return EventType; }
	const TCHAR* EventName()    const;

	const FString& EventShip()    const { return EventShipName; }
	const FString& EventSource()  const { return EventSourceName; }
	const FString& EventTarget()  const { return EventTargetName; }
	const FString& EventMessage() const { return EventMessageText; }
	const FString& EventSound()   const { return EventSoundName; }

	int32        EventParam(int32 Index = 0) const;
	int32        NumEventParams() const { return EventParamCount; }

	int32        EventChance()  const { return EventChancePct; }

	const FVector2D& EventPoint() const { return EventPoint2D; }
	const FBox2D&    EventRect()  const { return EventRect2D; }

	MISSIONEVENT_TRIGGER Trigger() const { return TriggerType; }
	const TCHAR* TriggerName()  const;

	const FString& TriggerShip()   const { return TriggerShipName; }
	const FString& TriggerTarget() const { return TriggerTargetName; }

	FString      TriggerParamStr() const;
	int32        TriggerParam(int32 Index = 0) const;
	int32        NumTriggerParams() const { return TriggerParamCount; }

	static const TCHAR* EventName(int32 N);
	static int32        EventForName(const FString& Name);
	static const TCHAR* TriggerName(int32 N);
	static int32        TriggerForName(const FString& Name);

	// Optional wiring hooks (later)
	void SetMission(Mission* InMission) { OwnerMission = InMission; }

	// Setters for parsing/DT hydration (safe, optional)
	void SetId(int32 InId) { Id = InId; }
	void SetEvent(MISSIONEVENT_TYPE InEventType) { EventType = InEventType; }
	void SetTrigger(MISSIONEVENT_TRIGGER InTriggerType) { TriggerType = InTriggerType; }

	void SetTime(double InTimeSeconds) { TimeSeconds = InTimeSeconds; }
	void SetDelay(double InDelaySeconds) { DelaySeconds = InDelaySeconds; }

	void SetEventShip(const FString& In) { EventShipName = In; }
	void SetEventSource(const FString& In) { EventSourceName = In; }
	void SetEventTarget(const FString& In) { EventTargetName = In; }
	void SetEventMessage(const FString& In) { EventMessageText = In; }
	void SetEventSound(const FString& In) { EventSoundName = In; }

	void SetEventChance(int32 InChancePct) { EventChancePct = FMath::Clamp(InChancePct, 0, 100); }

	void SetEventPoint(const FVector2D& In) { EventPoint2D = In; }
	void SetEventRect(const FBox2D& In) { EventRect2D = In; }

	void SetTriggerShip(const FString& In) { TriggerShipName = In; }
	void SetTriggerTarget(const FString& In) { TriggerTargetName = In; }

	void SetEventParams(const TArray<int32>& InParams);
	void SetTriggerParams(const TArray<int32>& InParams);

protected:
	// Internal helpers
	bool PassChanceGate() const;

protected:
	// Identity + state
	int32   Id = 0;
	MISSIONEVENT_STATUS StatusValue = MISSIONEVENT_STATUS::PENDING;

	// Time
	double  TimeSeconds = 0.0;
	double  DelaySeconds = 0.0;

	// Event
	MISSIONEVENT_TYPE EventType = MISSIONEVENT_TYPE::MESSAGE;
	int32   EventChancePct = 100;

	FString EventShipName;
	FString EventSourceName;
	FString EventTargetName;
	FString EventMessageText;
	FString EventSoundName;

	static constexpr int32 MaxEventParams = 6;
	int32   EventParams[MaxEventParams];
	int32   EventParamCount = 0;

	FVector2D EventPoint2D = FVector2D::ZeroVector;
	FBox2D    EventRect2D = FBox2D(EForceInit::ForceInitToZero);

	// Trigger
	MISSIONEVENT_TRIGGER TriggerType = MISSIONEVENT_TRIGGER::TRIGGER_TIME;

	FString TriggerShipName;
	FString TriggerTargetName;

	static constexpr int32 MaxTriggerParams = 10;
	int32   TriggerParams[MaxTriggerParams];
	int32   TriggerParamCount = 0;

	// Owner (optional)
	Mission* OwnerMission = nullptr;

	// Basic frame bookkeeping
	double  AccumulatedSeconds = 0.0;
};

