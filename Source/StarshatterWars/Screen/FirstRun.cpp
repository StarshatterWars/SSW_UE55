// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "FirstRun.h"
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
	PlayerData.Campaign = 0;
	SSWInstance->SaveGame("PlayerSaveSlot", 0, PlayerData);
	SSWInstance->ToggleFirstRunDlg(false);
}

void UFirstRun::OnCancelClicked()
{
	GEngine->ForceGarbageCollection();

	APlayerController* player = GetOwningPlayer();
	UKismetSystemLibrary::QuitGame(this, player, EQuitPreference::Quit, true);
}




