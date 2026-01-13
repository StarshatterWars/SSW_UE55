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


#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerSubsystem.generated.h"


// -------- Delegates --------
// Dynamic not required (you’re C++), use native multicast:
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUniverseSecond, uint64 /*UniverseSeconds*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUniverseMinute, uint64 /*UniverseSeconds*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUniverseHour, uint64 /*UniverseSeconds*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUniverseDay, uint64 /*UniverseSeconds*/);
/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UTimerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// ---- Events ----
	FOnUniverseSecond OnUniverseSecond;
	FOnUniverseMinute OnUniverseMinute;
	FOnUniverseHour   OnUniverseHour;
	FOnUniverseDay    OnUniverseDay;

	// ---- Config ----
	/** 1.0 = real time, 60 = 1 min/sec, 3600 = 1 hr/sec */
	UPROPERTY(EditAnywhere, Category = "Time")
	double TimeScale = 60.0;

	/** The cadence of the real-time timer that advances universe time */
	UPROPERTY(EditAnywhere, Category = "Time")
	double TimeStepSeconds = 1.0;

	// ---- API ----
	void StartClock();
	void StopClock();

	uint64 GetUniverseTimeSeconds() const { return UniverseTimeSeconds; }

	// Anchor base (your 2228-01-01 mapping)
	void SetUniverseBaseUnixSeconds(int64 InBaseUnixSeconds) { UniverseBaseUnixSeconds = InBaseUnixSeconds; }

	FDateTime GetUniverseDateTime() const;
	FString GetUniverseDateTimeString() const;

	// Optional: external injection for loaded save state
	void SetUniverseTimeSeconds(uint64 InSeconds) { UniverseTimeSeconds = InSeconds; }

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	FTimerHandle ClockTimerHandle;

	uint64 UniverseTimeSeconds = 0;
	int64 UniverseBaseUnixSeconds = 0;

	// “last broadcast” guards:
	uint64 LastMinute = MAX_uint64;
	uint64 LastHour = MAX_uint64;
	uint64 LastDay = MAX_uint64;

	void OnClockTick();
};
	
	