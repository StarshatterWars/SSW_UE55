// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "FirstRun.h"
#include "Misc/DateTime.h"
#include "Misc/TimeSpan.h"
#include "../System/SSWGameInstance.h"

void UFirstRun::NativeConstruct()
{
	Super::NativeConstruct();

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	if (btn_apply) {
		btn_apply->OnClicked.AddDynamic(this, &UFirstRun::OnApplyClicked);
	}
	if (btn_cancel) {
		btn_cancel->OnClicked.AddDynamic(this, &UFirstRun::OnCancelClicked);
	}
	if (FirstRunTitle) {
		FirstRunTitle->SetText(FText::FromString("NEW PLAYER"));
	}

	if (FirstRunPrompt) {
		FirstRunPrompt->SetText(FText::FromString("Create a new player account. \nEnter your name in the box provided. \nThe user name may be a nickname, callsign, or last name."));
	}
}

void UFirstRun::OnApplyClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	FString PlayerName = PlayerNameBox ? PlayerNameBox->GetText().ToString() : TEXT("DefaultPlayer");
	PlayerData.Name = PlayerName;
	PlayerData.Campaign = -1;
	FDateTime Now = FDateTime::Now();
	FDateTime Future = Now + FTimespan::FromDays(200 * 365.2425);
	PlayerData.CreateTime = Now.ToUnixTimestamp();
	PlayerData.GameTime = 0;
	PlayerData.CampaignTime = 0;
	PlayerData.PlayerForce = -1;
	PlayerData.PlayerFleet = -1;
	PlayerData.PlayerWing = -1;
	PlayerData.PlayerCarrier = -1;
	PlayerData.PlayerBattleGroup = -1;
	PlayerData.PlayerDesronGroup = -1;
	PlayerData.PlayerSquadron = -1;
	PlayerData.PlayerShip = "Unassigned";
	PlayerData.PlayerSystem = "Borova";
	PlayerData.PlayerRegion = "Borova";
	SSWInstance->SaveGame(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot, PlayerData);
	SSWInstance->ToggleFirstRunDlg(false);
}

void UFirstRun::OnCancelClicked()
{
	GEngine->ForceGarbageCollection();

	APlayerController* player = GetOwningPlayer();
	UKismetSystemLibrary::QuitGame(this, player, EQuitPreference::Quit, true);
}





