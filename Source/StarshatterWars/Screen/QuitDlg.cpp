// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "QuitDlg.h"
#include "../System/SSWGameInstance.h"

void UQuitDlg::OnApplyClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
	SSWInstance->ToggleQuitDlg(false);
	SSWInstance->ExitGame(this);
}

void UQuitDlg::OnCancelClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
	SSWInstance->ToggleQuitDlg(false);
}

void UQuitDlg::OnApplyHovered() 
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);
}

void UQuitDlg::OnCancelHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);
}

void UQuitDlg::NativeConstruct()
{
	Super::NativeConstruct();

	if (btn_apply) {
		btn_apply->OnClicked.AddDynamic(this, &UQuitDlg::OnApplyClicked);
		btn_apply->OnHovered.AddDynamic(this, &UQuitDlg::OnApplyHovered);
	}
	if (btn_cancel) {
		btn_cancel->OnClicked.AddDynamic(this, &UQuitDlg::OnCancelClicked);
		btn_cancel->OnHovered.AddDynamic(this, &UQuitDlg::OnCancelHovered);
	}
	if(ExitTitle) {
		ExitTitle->SetText(FText::FromString("EXIT STARSHATTER?"));
	}

	if (ExitPrompt) {
		ExitPrompt->SetText(FText::FromString("Are you sure you want to exit Starshatter and return to Windows?"));
	}
}
