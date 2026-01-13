// 
// /*  Project nGenEx
// Fractal Dev Games
// Copyright (C) 2024. All Rights Reserved.	
// 
// SUBSYSTEM:    SSW
// FILE:         TimerSubsystem.cpp	
// AUTHOR:       Carlos Bott
// 
// */


#include "TimerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UTimerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// You can auto-start here, or start from your StartGame path:
	// StartClock();
}

void UTimerSubsystem::Deinitialize()
{
	StopClock();
	Super::Deinitialize();
}

void UTimerSubsystem::StartClock()
{
	UWorld* World = GetWorld();
	if (!World) return;

	StopClock();

	World->GetTimerManager().SetTimer(
		ClockTimerHandle,
		this,
		&UTimerSubsystem::OnClockTick,
		(float)TimeStepSeconds,
		true
	);
}

void UTimerSubsystem::StopClock()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ClockTimerHandle);
	}
}

void UTimerSubsystem::OnClockTick()
{
	// Deterministic step in universe seconds
	const double DeltaUniverse = TimeStepSeconds * TimeScale;
	const uint64 AddSeconds = (uint64)FMath::Max(1.0, FMath::RoundToDouble(DeltaUniverse));

	UniverseTimeSeconds += AddSeconds;

	// Always broadcast per-second tick (lightweight subscribers only)
	OnUniverseSecond.Broadcast(UniverseTimeSeconds);

	// Cadence gates
	const uint64 CurMinute = UniverseTimeSeconds / 60ULL;
	if (CurMinute != LastMinute)
	{
		LastMinute = CurMinute;
		OnUniverseMinute.Broadcast(UniverseTimeSeconds);
	}

	const uint64 CurHour = UniverseTimeSeconds / 3600ULL;
	if (CurHour != LastHour)
	{
		LastHour = CurHour;
		OnUniverseHour.Broadcast(UniverseTimeSeconds);
	}

	const uint64 CurDay = UniverseTimeSeconds / 86400ULL;
	if (CurDay != LastDay)
	{
		LastDay = CurDay;
		OnUniverseDay.Broadcast(UniverseTimeSeconds);
	}
}

FDateTime UTimerSubsystem::GetUniverseDateTime() const
{
	if (UniverseBaseUnixSeconds <= 0)
	{
		// fallback: treat universe seconds as unix seconds (debug)
		return FDateTime::FromUnixTimestamp((int64)UniverseTimeSeconds);
	}

	const int64 Unix = UniverseBaseUnixSeconds + (int64)UniverseTimeSeconds;
	return FDateTime::FromUnixTimestamp(Unix);
}

FString UTimerSubsystem::GetUniverseDateTimeString() const
{
	return GetUniverseDateTime().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
}




