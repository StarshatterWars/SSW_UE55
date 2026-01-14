#include "TimerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UTimerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// StartClock(); // optional
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

// -------------------- Mission API --------------------

void UTimerSubsystem::StartMissionRun(bool bResetToZero)
{
	// Ensure the master clock is running (so mission timeline advances)
	StartClock();

	if (bResetToZero)
	{
		ResetMissionClock();
	}

	MissionClockState = EMissionClockState::Running;
}

void UTimerSubsystem::StopMissionRun()
{
	MissionClockState = EMissionClockState::Stopped;
}

void UTimerSubsystem::PauseMissionClock()
{
	if (MissionClockState == EMissionClockState::Running)
	{
		MissionClockState = EMissionClockState::Paused;
	}
}

void UTimerSubsystem::ResumeMissionClock()
{
	if (MissionClockState == EMissionClockState::Paused)
	{
		MissionClockState = EMissionClockState::Running;
	}
}

void UTimerSubsystem::ResetMissionClock()
{
	MissionTimelineSeconds = 0.0;
	LastMissionSecondBroadcast = TNumericLimits<int32>::Min();

	// Immediately broadcast 0 so UI/cutscene systems snap to 00:00 on restart
	OnMissionSecond.Broadcast(0);
}

FText UTimerSubsystem::GetMissionTimerTextMMSS() const
{
	const int32 TotalSeconds = GetMissionTimeSecondsInt();
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
}

// -------------------- Clock Tick --------------------

void UTimerSubsystem::OnClockTick()
{
	// ---- Universe time ----
	const double DeltaUniverse = TimeStepSeconds * TimeScale;
	const uint64 AddSeconds = (uint64)FMath::Max(1.0, FMath::RoundToDouble(DeltaUniverse));
	UniverseTimeSeconds += AddSeconds;

	OnUniverseSecond.Broadcast(UniverseTimeSeconds);

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

	// ---- Mission/Cutscene timeline (shared) ----
	if (MissionClockState == EMissionClockState::Running)
	{
		// Mission timeline advances in real seconds by default
		MissionTimelineSeconds += (TimeStepSeconds * MissionTimeScale);

		const int32 CurMissionSec = GetMissionTimeSecondsInt();
		if (CurMissionSec != LastMissionSecondBroadcast)
		{
			LastMissionSecondBroadcast = CurMissionSec;
			OnMissionSecond.Broadcast(CurMissionSec);
		}
	}
}

FDateTime UTimerSubsystem::GetUniverseDateTime() const
{
	if (UniverseBaseUnixSeconds <= 0)
	{
		return FDateTime::FromUnixTimestamp((int64)UniverseTimeSeconds);
	}

	const int64 Unix = UniverseBaseUnixSeconds + (int64)UniverseTimeSeconds;
	return FDateTime::FromUnixTimestamp(Unix);
}

FString UTimerSubsystem::GetUniverseDateTimeString() const
{
	return GetUniverseDateTime().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
}
