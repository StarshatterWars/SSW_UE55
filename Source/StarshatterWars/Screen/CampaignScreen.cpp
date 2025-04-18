// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CampaignScreen.h"
#include "Engine/Font.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateBrush.h"

void UCampaignScreen::NativeConstruct()
{
	Super::NativeConstruct();

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->LoadGame(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot);
	
	if (TitleText) {
		TitleText->SetText(FText::FromString("Dynamic Campaigns").ToUpper());
	}

	if (CancelButton) {
		CancelButton->OnClicked.AddDynamic(this, &UCampaignScreen::OnCancelButtonClicked);
		CancelButton->OnHovered.AddDynamic(this, &UCampaignScreen::OnCancelButtonHovered);
		CancelButton->OnUnhovered.AddDynamic(this, &UCampaignScreen::OnCancelButtonUnHovered);
		if (CancelButtonText) {
			CancelButtonText->SetText(FText::FromString("CANCEL"));
		}
	}
	if (PlayButton) {
		PlayButton->OnClicked.AddDynamic(this, &UCampaignScreen::OnPlayButtonClicked);
		PlayButton->OnHovered.AddDynamic(this, &UCampaignScreen::OnPlayButtonHovered);
		PlayButton->OnUnhovered.AddDynamic(this, &UCampaignScreen::OnPlayButtonUnHovered);
		
		if(SSWInstance->PlayerInfo.Campaign >= 0) {
			PlayButton->SetIsEnabled(true);
		}
		else {
			PlayButton->SetIsEnabled(false);
		}
		
		if (PlayButtonText) {
			PlayButtonText->SetText(FText::FromString("APPLY"));
		}
	}

	if (CampaignSelectDD) {
		CampaignSelectDD->OnSelectionChanged.AddDynamic(this, &UCampaignScreen::OnSetSelected);
	}

	if (PlayerNameText) {
		PlayerNameText->SetText(FText::FromString(SSWInstance->PlayerInfo.Name));
		UE_LOG(LogTemp, Log, TEXT("Player Name: %s"), *SSWInstance->PlayerInfo.Name);
	}

	SetCampaignDDList();
	if (SSWInstance->PlayerInfo.Campaign == -1) {
		SetSelectedData(0);
	}
	else {
		SetSelectedData(SSWInstance->PlayerInfo.Campaign);
	}
	
}

void UCampaignScreen::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

}

UTexture2D* UCampaignScreen::LoadTextureFromFile()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();	
	UTexture2D* LoadedTexture = SSWInstance->LoadPNGTextureFromFile(ImagePath);
	return LoadedTexture;
}

FSlateBrush UCampaignScreen::CreateBrushFromTexture(UTexture2D* Texture, FVector2D ImageSize)
{
	FSlateBrush Brush;
	Brush.SetResourceObject(Texture);
	Brush.ImageSize = ImageSize;
	Brush.DrawAs = ESlateBrushDrawType::Image;
	return Brush;
}

void UCampaignScreen::OnPlayButtonClicked()
{
	PlayUISound(this, AcceptSound);
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayerInfo.Campaign = Selected;
	SSWInstance->SetActiveCampaign(SSWInstance->CampaignData[Selected]);
	FString NewCampaign = SSWInstance->GetActiveCampaign().Name;
	UE_LOG(LogTemp, Log, TEXT("Campaign Name Selected: %s"), *NewCampaign);
	FDateTime GameDate(2228 + Selected, 1, 1);
	SSWInstance->SetCampaignTime(GameDate.ToUnixTimestamp());
	SSWInstance->SaveGame(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot, SSWInstance->PlayerInfo);
	SSWInstance->ShowCampaignLoading();
}

void UCampaignScreen::OnPlayButtonHovered()
{
	PlayUISound(this, HoverSound);
}

void UCampaignScreen::OnPlayButtonUnHovered()
{
}

void UCampaignScreen::OnCancelButtonClicked()
{
	PlayUISound(this, AcceptSound);
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->ToggleCampaignScreen(false);
}

void UCampaignScreen::OnCancelButtonHovered()
{
	PlayUISound(this, HoverSound);
}

void UCampaignScreen::OnCancelButtonUnHovered()
{
}

void UCampaignScreen::SetCampaignDDList()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	if (CampaignSelectDD) {
		CampaignSelectDD->ClearOptions();
		CampaignSelectDD->ClearSelection();

		for (int index = 0; index < SSWInstance->CampaignData.Num(); index++) {
		
			if(SSWInstance->CampaignData[index].Available) {
				CampaignSelectDD->AddOption(SSWInstance->CampaignData[index].Name);
			}
		}

		CampaignSelectDD->SetSelectedIndex(SSWInstance->PlayerInfo.Campaign);
	}

	if (CampaignNameText) {

		if (SSWInstance->PlayerInfo.Campaign == -1) {
			CampaignNameText->SetText(FText::FromString("Campaign Not Set"));
			Selected = 0;
		} else {
			CampaignNameText->SetText(FText::FromString(SSWInstance->CampaignData[SSWInstance->PlayerInfo.Campaign].Name));
			Selected = SSWInstance->PlayerInfo.Campaign;
		}
	}
}

void UCampaignScreen::SetSelectedData(int selected)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	SSWInstance->CampaignData[selected].Orders.SetNum(4);
	
	GetCampaignImageFile(selected);

	UTexture2D* LoadedTexture = LoadTextureFromFile();
	if (LoadedTexture && CampaignImage)
	{
		FSlateBrush Brush = CreateBrushFromTexture(LoadedTexture, FVector2D(LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY()));
		CampaignImage->SetBrush(Brush);
	}

	if (DescriptionText) {
		DescriptionText->SetText(FText::FromString(SSWInstance->CampaignData[selected].Description));
	}

	if (SituationText) {
		SituationText->SetText(FText::FromString(SSWInstance->CampaignData[selected].Situation));
	}

	if(Orders1Text) {
		
		Orders1Text->SetText(FText::FromString(SSWInstance->CampaignData[selected].Orders[0]));
	}
	if (Orders2Text) {

		Orders2Text->SetText(FText::FromString(SSWInstance->CampaignData[selected].Orders[1]));
	}
	if (Orders3Text) {

		Orders3Text->SetText(FText::FromString(SSWInstance->CampaignData[selected].Orders[2]));
	}
	if (Orders4Text) {

		Orders4Text->SetText(FText::FromString(SSWInstance->CampaignData[selected].Orders[3]));
	}
	if (LocationSystemText) {
		FString LocationText = SSWInstance->CampaignData[selected].System + "/" + SSWInstance->CampaignData[selected].Region;
		LocationSystemText->SetText(FText::FromString(LocationText));
	}

	Selected = selected;
	SSWInstance->PlayerInfo.Campaign = Selected;
	SSWInstance->SaveGame(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot, SSWInstance->PlayerInfo);
}

void UCampaignScreen::OnSetSelected(FString dropDownInt, ESelectInfo::Type type)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	
	if (type == ESelectInfo::OnMouseClick) {
		int value;

		if (dropDownInt == SSWInstance->CampaignData[0].Name) {
			value = 0;
		} 
		else if (dropDownInt == SSWInstance->CampaignData[1].Name) {
			value = 1;
		}
		else if (dropDownInt == SSWInstance->CampaignData[2].Name) {
			value = 2;
		}
		else if (dropDownInt == SSWInstance->CampaignData[3].Name) {
			value = 3;
		}
		else if (dropDownInt == SSWInstance->CampaignData[4].Name) {
			value = 4;
		}

		SetSelectedData(value);
		PlayButton->SetIsEnabled(true);
	}
}

void UCampaignScreen::GetCampaignImageFile(int selected)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	ImagePath = FPaths::ProjectContentDir() + TEXT("UI/Campaigns/0");
	ImagePath.Append(FString::FromInt(selected + 1));
	ImagePath.Append("/");
	UE_LOG(LogTemp, Log, TEXT("Campaign File: %s"), *SSWInstance->CampaignData[selected].MainImage);
	ImagePath.Append(SSWInstance->CampaignData[selected].MainImage);
	ImagePath.Append(".png");
	UE_LOG(LogTemp, Log, TEXT("Campaign Image: %s"), *ImagePath);
}

void UCampaignScreen::PlayUISound(UObject* WorldContext, USoundBase* UISound)
{
	if (UISound)
	{
		UGameplayStatics::PlaySound2D(WorldContext, UISound);
	}
}
