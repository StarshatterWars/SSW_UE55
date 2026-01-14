#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerSubsystem.generated.h"

// -------- Universe Delegates --------
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUniverseSecond, uint64 /*UniverseSeconds*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUniverseMinute, uint64 /*UniverseSeconds*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUniverseHour, uint64 /*UniverseSeconds*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUniverseDay, uint64 /*UniverseSeconds*/);

// -------- Mission/Cutscene Timeline Delegates --------
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMissionSecond, int32 /*MissionSeconds*/);

UENUM(BlueprintType)
enum class EMissionClockState : uint8
{
	Stopped,
	Running,
	Paused
};

UCLASS()
class STARSHATTERWARS_API UTimerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ---- Universe Events ----
	FOnUniverseSecond OnUniverseSecond;
	FOnUniverseMinute OnUniverseMinute;
	FOnUniverseHour   OnUniverseHour;
	FOnUniverseDay    OnUniverseDay;

	// ---- Mission/Cutscene Events ----
	FOnMissionSecond OnMissionSecond;

	// ---- Universe Config ----
	/** 1.0 = real time, 60 = 1 min/sec, 3600 = 1 hr/sec */
	UPROPERTY(EditAnywhere, Category = "Time|Universe")
	double TimeScale = 60.0;

	/** The cadence of the real-time timer that advances universe time */
	UPROPERTY(EditAnywhere, Category = "Time|Universe")
	double TimeStepSeconds = 1.0;

	// ---- Mission Config ----
	/** 1.0 = real seconds; you can speed up/slow down cutscene scheduling if needed */
	UPROPERTY(EditAnywhere, Category = "Time|Mission")
	double MissionTimeScale = 1.0;

	// ---- Universe API ----
	void StartClock();
	void StopClock();

	uint64 GetUniverseTimeSeconds() const { return UniverseTimeSeconds; }

	void SetUniverseBaseUnixSeconds(int64 InBaseUnixSeconds) { UniverseBaseUnixSeconds = InBaseUnixSeconds; }
	FDateTime GetUniverseDateTime() const;
	FString GetUniverseDateTimeString() const;

	void SetUniverseTimeSeconds(uint64 InSeconds) { UniverseTimeSeconds = InSeconds; }

	// ---- Mission/Cutscene Timeline API ----
	UFUNCTION(BlueprintCallable, Category = "Time|Mission")
	void StartMissionRun(bool bResetToZero = true);

	UFUNCTION(BlueprintCallable, Category = "Time|Mission")
	void StopMissionRun();

	UFUNCTION(BlueprintCallable, Category = "Time|Mission")
	void PauseMissionClock();

	UFUNCTION(BlueprintCallable, Category = "Time|Mission")
	void ResumeMissionClock();

	UFUNCTION(BlueprintCallable, Category = "Time|Mission")
	void ResetMissionClock();

	UFUNCTION(BlueprintPure, Category = "Time|Mission")
	EMissionClockState GetMissionClockState() const { return MissionClockState; }

	UFUNCTION(BlueprintPure, Category = "Time|Mission")
	double GetMissionTimeSeconds() const { return MissionTimelineSeconds; }

	UFUNCTION(BlueprintPure, Category = "Time|Mission")
	int32 GetMissionTimeSecondsInt() const { return (int32)FMath::FloorToInt((float)MissionTimelineSeconds); }

	UFUNCTION(BlueprintPure, Category = "Time|Mission")
	FText GetMissionTimerTextMMSS() const;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	FTimerHandle ClockTimerHandle;

	// ---- Universe State ----
	uint64 UniverseTimeSeconds = 0;
	int64  UniverseBaseUnixSeconds = 0;

	uint64 LastMinute = MAX_uint64;
	uint64 LastHour = MAX_uint64;
	uint64 LastDay = MAX_uint64;

	// ---- Mission/Cutscene State ----
	double MissionTimelineSeconds = 0.0;
	EMissionClockState MissionClockState = EMissionClockState::Stopped;
	int32 LastMissionSecondBroadcast = TNumericLimits<int32>::Min();

	void OnClockTick();
};
