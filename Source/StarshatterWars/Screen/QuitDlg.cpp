// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "QuitDlg.h"
#include "../System/SSWGameInstance.h"

void UQuitDlg::OnApplyClicked()
{
	GEngine->ForceGarbageCollection();

	APlayerController* player = GetOwningPlayer();
	UKismetSystemLibrary::QuitGame(this, player, EQuitPreference::Quit, true);
}

void UQuitDlg::OnCancelClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->ToggleQuitDlg(false);
}

void UQuitDlg::NativeConstruct()
{
	Super::NativeConstruct();

	if (btn_apply) {
		btn_apply->OnClicked.AddDynamic(this, &UQuitDlg::OnApplyClicked);
	}
	if (btn_cancel) {
		btn_cancel->OnClicked.AddDynamic(this, &UQuitDlg::OnCancelClicked);
	}
	if(ExitTitle) {
		ExitTitle->SetText(FText::FromString("EXIT STARSHATTER?"));
	}

	if (ExitPrompt) {
		ExitPrompt->SetText(FText::FromString("Are you sure you want to exit Starshatter and return to Windows?"));
	}
}