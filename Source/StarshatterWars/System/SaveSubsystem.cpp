// /*  Project nGenEx
// Fractal Dev Games
// Copyright (C) 2024. All Rights Reserved.	
// 
// SUBSYSTEM:    SSW
// FILE:         SaveSubsystem.h
// 
// AUTHOR:       Carlos Bott
// */


#include "SaveSubsystem.h"

#include "TimerSubsystem.h"
#include "CampaignSave.h"
#include "Kismet/GameplayStatics.h"

// Include your Universe save type:
#include "UniverseSaveGame.h"

void USaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BindToTimer();
}

void USaveSubsystem::Deinitialize()
{
	UnbindFromTimer();
	Super::Deinitialize();
}

// ------------------------
// Timer binding
// ------------------------

void USaveSubsystem::BindToTimer()
{
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Timer->OnUniverseMinute.AddUObject(this, &USaveSubsystem::HandleUniverseMinute);
	}
}

void USaveSubsystem::UnbindFromTimer()
{
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Timer->OnUniverseMinute.RemoveAll(this);
	}
}

int32 USaveSubsystem::GetAutosaveIntervalMinutes() const
{
	switch (AutosavePolicy)
	{
	case EAutosavePolicy::EveryMinute:   return 1;
	case EAutosavePolicy::Every5Minutes: return 5;
	case EAutosavePolicy::Every10Minutes:return 10;
	default:                             return 0;
	}
}

void USaveSubsystem::HandleUniverseMinute(uint64 UniverseSecondsNow)
{
	if (!bAutosaveEnabled)
		return;

	const int32 Interval = GetAutosaveIntervalMinutes();
	if (Interval <= 0)
		return;

	const uint64 CurMinute = UniverseSecondsNow / 60ULL;
	if (CurMinute == LastAutosaveMinute)
		return;

	// throttle to interval
	if (LastAutosaveMinute != MAX_uint64)
	{
		const uint64 Delta = CurMinute - LastAutosaveMinute;
		if ((int32)Delta < Interval)
			return;
	}

	LastAutosaveMinute = CurMinute;

	// Pull authoritative time from timer (optional; UniverseSecondsNow is already authoritative)
	UniverseTimeSeconds = UniverseSecondsNow;

	SaveUniverse(false);
	SaveCampaign(false);
	SavePlayer(PlayerSlotName, false);
}

void USaveSubsystem::SetAutosaveEnabled(bool bEnabled)
{
	bAutosaveEnabled = bEnabled;
}

// ------------------------
// Player save
// ------------------------

bool USaveSubsystem::LoadPlayer(const FString& SlotName)
{
	PlayerSlotName = SlotName;

	// Replace with your actual player save class if you have one.
	// Here we assume you serialize PlayerData via some USaveGame type, or your existing SaveGame(...) wrapper.
	// If your current player save is custom, this function should wrap that exact object.

	// Placeholder: implement with your existing player save object type.
	return true;
}

bool USaveSubsystem::SavePlayer(const FString& SlotName, bool /*bForce*/)
{
	PlayerSlotName = SlotName;

	// Placeholder: implement with your existing player save type.
	return true;
}

// ------------------------
// Universe save
// ------------------------

FString USaveSubsystem::GetUniverseSlotName() const
{
	// Keep consistent with what you used before
	// Example: "Universe_<UniverseId>"
	return FString::Printf(TEXT("Universe_%s"), *UniverseId);
}

bool USaveSubsystem::LoadOrCreateUniverse(const FString& InUniverseId, int64 BaseUnixSecondsIfNew)
{
	UniverseId = InUniverseId;

	const FString Slot = GetUniverseSlotName();
	const int32 UI = UserIndex;

	if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(Slot, UI))
	{
		if (UUniverseSaveGame* U = Cast<UUniverseSaveGame>(Loaded))
		{
			UniverseBaseUnixSeconds = U->UniverseBaseUnixSeconds;
			UniverseTimeSeconds = U->UniverseTimeSeconds;

			// Push into timer subsystem (authoritative runtime time)
			if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
			{
				Timer->SetUniverseBaseUnixSeconds(UniverseBaseUnixSeconds);
				Timer->SetUniverseTimeSeconds(UniverseTimeSeconds);
			}

			return true;
		}
	}

	// Create new
	UUniverseSaveGame* NewObj = Cast<UUniverseSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UUniverseSaveGame::StaticClass())
	);
	if (!NewObj)
		return false;

	NewObj->UniverseId = UniverseId;
	NewObj->UniverseBaseUnixSeconds = BaseUnixSecondsIfNew;
	NewObj->UniverseTimeSeconds = 0;

	UniverseBaseUnixSeconds = BaseUnixSecondsIfNew;
	UniverseTimeSeconds = 0;

	// Push into timer
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Timer->SetUniverseBaseUnixSeconds(UniverseBaseUnixSeconds);
		Timer->SetUniverseTimeSeconds(UniverseTimeSeconds);
	}

	return UGameplayStatics::SaveGameToSlot(NewObj, Slot, UI);
}

bool USaveSubsystem::SaveUniverse(bool /*bForce*/)
{
	if (UniverseId.IsEmpty())
		return false;

	// Pull latest runtime time from timer
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		UniverseTimeSeconds = Timer->GetUniverseTimeSeconds();
		UniverseBaseUnixSeconds = Timer->GetUniverseDateTime().ToUnixTimestamp() - (int64)UniverseTimeSeconds;
		// If you already store UniverseBaseUnixSeconds directly in timer, use that instead.
	}

	UUniverseSaveGame* SaveObj = Cast<UUniverseSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UUniverseSaveGame::StaticClass())
	);
	if (!SaveObj)
		return false;

	SaveObj->UniverseId = UniverseId;
	SaveObj->UniverseBaseUnixSeconds = UniverseBaseUnixSeconds;
	SaveObj->UniverseTimeSeconds = UniverseTimeSeconds;

	return UGameplayStatics::SaveGameToSlot(SaveObj, GetUniverseSlotName(), UserIndex);
}

// ------------------------
// Campaign save
// ------------------------

bool USaveSubsystem::DoesCampaignSaveExist(FName RowName) const
{
	if (RowName.IsNone())
		return false;

	const FString Slot = UCampaignSave::MakeSlotNameFromRowName(RowName);
	return UGameplayStatics::DoesSaveGameExist(Slot, UserIndex);
}

UCampaignSave* USaveSubsystem::LoadOrCreateCampaignSave(int32 CampaignIndex, FName RowName, const FString& DisplayName)
{
	CampaignIndex = FMath::Max(1, CampaignIndex);
	if (RowName.IsNone())
		return nullptr;

	const FString Slot = UCampaignSave::MakeSlotNameFromRowName(RowName);

	// Load if exists
	if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(Slot, UserIndex))
	{
		if (UCampaignSave* LoadedSave = Cast<UCampaignSave>(Loaded))
		{
			LoadedSave->CampaignIndex = CampaignIndex;
			LoadedSave->CampaignRowName = RowName;
			if (!DisplayName.IsEmpty())
				LoadedSave->CampaignDisplayName = DisplayName;

			CampaignSave = LoadedSave;

			// Inject into timer (so T+ updates)
			if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
			{
				Timer->SetCampaignSave(CampaignSave);
			}

			return CampaignSave;
		}
	}

	// Create new
	return CreateNewCampaignSave(CampaignIndex, RowName, DisplayName);
}

UCampaignSave* USaveSubsystem::CreateNewCampaignSave(int32 CampaignIndex, FName RowName, const FString& DisplayName)
{
	CampaignIndex = FMath::Max(1, CampaignIndex);
	if (RowName.IsNone())
		return nullptr;

	const FString Slot = UCampaignSave::MakeSlotNameFromRowName(RowName);

	UCampaignSave* NewSave = Cast<UCampaignSave>(
		UGameplayStatics::CreateSaveGameObject(UCampaignSave::StaticClass())
	);
	if (!NewSave)
		return nullptr;

	NewSave->CampaignIndex = CampaignIndex;
	NewSave->CampaignRowName = RowName;
	NewSave->CampaignDisplayName = DisplayName;

	// Anchor to current universe time
	uint64 Now = 0;
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Now = Timer->GetUniverseTimeSeconds();
	}
	NewSave->InitializeCampaignClock(Now);

	const bool bOK = UGameplayStatics::SaveGameToSlot(NewSave, Slot, UserIndex);
	if (!bOK)
		return nullptr;

	CampaignSave = NewSave;

	// Inject into timer
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Timer->SetCampaignSave(CampaignSave);
	}

	return CampaignSave;
}

bool USaveSubsystem::SaveCampaign(bool /*bForce*/)
{
	if (!CampaignSave || CampaignSave->CampaignRowName.IsNone())
		return false;

	const FString Slot = UCampaignSave::MakeSlotNameFromRowName(CampaignSave->CampaignRowName);
	return UGameplayStatics::SaveGameToSlot(CampaignSave, Slot, UserIndex);
}


