#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveSubsystem.generated.h"

class UTimerSubsystem;
class UCampaignSave;
class UUniverseSaveGame;

// Optional: wrap PlayerInfo in your existing struct type
USTRUCT(BlueprintType)
struct FPlayerSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Campaign = -1;

	// Add whatever else you store in PlayerInfo
};

UENUM(BlueprintType)
enum class EAutosavePolicy : uint8
{
	Disabled,
	EveryMinute,
	Every5Minutes,
	Every10Minutes
};

UCLASS()
class STARSHATTERWARS_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ------------------------
	// Lifecycle
	// ------------------------
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ------------------------
	// Config
	// ------------------------
	UPROPERTY(EditAnywhere, Category = "Save")
	EAutosavePolicy AutosavePolicy = EAutosavePolicy::EveryMinute;

	UPROPERTY(EditAnywhere, Category = "Save")
	int32 UserIndex = 0;

	// ------------------------
	// Player Save API
	// ------------------------
	UFUNCTION(BlueprintCallable, Category = "Save|Player")
	bool LoadPlayer(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "Save|Player")
	bool SavePlayer(const FString& SlotName, bool bForce = false);

	UFUNCTION(BlueprintCallable, Category = "Save|Player")
	const FPlayerSaveData& GetPlayerData() const { return PlayerData; }

	UFUNCTION(BlueprintCallable, Category = "Save|Player")
	void SetPlayerData(const FPlayerSaveData& InData) { PlayerData = InData; }

	// ------------------------
	// Universe Save API
	// ------------------------
	UFUNCTION()
	bool LoadOrCreateUniverse(const FString& UniverseId, int64 BaseUnixSecondsIfNew);

	UFUNCTION()
	bool SaveUniverse(bool bForce = false);

	UFUNCTION()
	uint64 GetUniverseTimeSeconds() const { return UniverseTimeSeconds; }

	UFUNCTION()
	int64 GetUniverseBaseUnixSeconds() const { return UniverseBaseUnixSeconds; }

	// Set from TimerSubsystem tick
	void SetUniverseTimeSeconds(uint64 InSeconds) { UniverseTimeSeconds = InSeconds; }

	// ------------------------
	// Campaign Save API
	// ------------------------
	UFUNCTION()
	UCampaignSave* LoadOrCreateCampaignSave(int32 CampaignIndex, FName RowName, const FString& DisplayName);

	UFUNCTION()
	UCampaignSave* CreateNewCampaignSave(int32 CampaignIndex, FName RowName, const FString& DisplayName);

	UFUNCTION()
	bool SaveCampaign(bool bForce = false);

	UFUNCTION()
	bool DoesCampaignSaveExist(FName RowName) const;

	UFUNCTION()
	UCampaignSave* GetCampaignSave() const { return CampaignSave; }

	// ------------------------
	// Autosave
	// ------------------------
	UFUNCTION()
	void SetAutosaveEnabled(bool bEnabled);

private:
	// ------------------------
	// Internal - autosave wiring
	// ------------------------
	void BindToTimer();
	void UnbindFromTimer();
	void HandleUniverseMinute(uint64 UniverseSecondsNow);

	int32 GetAutosaveIntervalMinutes() const;

	// ------------------------
	// Internal - slot naming
	// ------------------------
	FString GetUniverseSlotName() const;

private:
	// Cached slot context
	UPROPERTY() FString PlayerSlotName;
	UPROPERTY() FString UniverseId;

	// Stored player data
	UPROPERTY() FPlayerSaveData PlayerData;

	// Universe state mirrored for saving
	uint64 UniverseTimeSeconds = 0;
	int64  UniverseBaseUnixSeconds = 0;

	// Active campaign save
	UPROPERTY() UCampaignSave* CampaignSave = nullptr;

	// Autosave
	bool bAutosaveEnabled = true;
	uint64 LastAutosaveMinute = MAX_uint64;
};