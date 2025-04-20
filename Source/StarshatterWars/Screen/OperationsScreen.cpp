// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OperationsScreen.h"
#include "MissionListObject.h"
#include "IntelListObject.h"

void UOperationsScreen::NativeConstruct()
{
	Super::NativeConstruct();

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->LoadGame(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot);

	if (TitleText)
		TitleText->SetText(FText::FromString("Operational Command").ToUpper());
	
	if (CancelButton) {
		CancelButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnCancelButtonClicked);
		CancelButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnCancelButtonHovered);
		CancelButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnCancelButtonUnHovered);		
	}

	if (CancelButtonText) {
		CancelButtonText->SetText(FText::FromString("BACK"));
	}

	if (AudioButton) {
		AudioButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnAudioButtonClicked);
	}
	if (SelectButton) {
		SelectButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnSelectButtonClicked);
		SelectButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnSelectButtonHovered);
		SelectButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnSelectButtonUnHovered);		
	}

	if (PlayButtonText) {
		PlayButtonText->SetText(FText::FromString("SELECT"));
	}

	if (OrdersButton) {
		OrdersButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnOrdersButtonClicked);
		OrdersButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnOrdersButtonHovered);
		OrdersButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnOrdersButtonUnHovered);
	}

	if (TheaterButton) {
		TheaterButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnTheaterButtonClicked);
		TheaterButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnTheaterButtonHovered);
		TheaterButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnTheaterButtonUnHovered);
	}

	if (ForcesButton) {
		ForcesButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnForcesButtonClicked);
		ForcesButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnForcesButtonHovered);
		ForcesButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnForcesButtonUnHovered);
	}

	if (IntelButton) {
		IntelButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnIntelButtonClicked);
		IntelButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnIntelButtonHovered);
		IntelButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnIntelButtonUnHovered);
	}

	if (MissionsButton) {
		MissionsButton->OnClicked.AddDynamic(this, &UOperationsScreen::OnMissionsButtonClicked);
		MissionsButton->OnHovered.AddDynamic(this, &UOperationsScreen::OnMissionsButtonHovered);
		MissionsButton->OnUnhovered.AddDynamic(this, &UOperationsScreen::OnMissionsButtonUnHovered);
	}
	if (PlayerNameText) {
		PlayerNameText->SetText(FText::FromString(SSWInstance->PlayerInfo.Name));
	}

	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(0);
	}

	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("ORDERS"));
	}

	ActiveCampaign = SSWInstance->GetActiveCampaign();
	
	if (SSWInstance->GetActiveCampaign().Index == 0) {
		if (TheaterButton) {
			TheaterButton->SetIsEnabled(false);
		}
		if (ForcesButton) {
			ForcesButton->SetIsEnabled(false); 
		}
		if (IntelButton) {
			IntelButton->SetIsEnabled(false);
		}
	} else {
		if (TheaterButton) {
				TheaterButton->SetIsEnabled(true);
		}
		if (ForcesButton) {
			ForcesButton->SetIsEnabled(true);
		}
		if (IntelButton) {
			IntelButton->SetIsEnabled(true);
		}
	}

	SelectedMission = 0;

	SetCampaignOrders();
	PopulateMissionList();
	PopulateIntelList();
	SetCampaignMissions();
}

void UOperationsScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	
	if (SSWInstance->MissionSelectionChanged) {
		SetSelectedMissionData(SSWInstance->GetSelectedMissionNr());
	}	

	if (SSWInstance->ActionSelectionChanged) {
		SetSelectedIntelData(SSWInstance->GetSelectedActionNr());
	}
	if (GameTimeText) {
		FString CustomDate = GetCampaignTime().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
		GameTimeText->SetText(FText::FromString(*CustomDate));
	}
	AudioButton->SetIsEnabled(!SSWInstance->IsSoundPlaying());
}

void UOperationsScreen::OnSelectButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);
	SSWInstance->LoadMissionBriefingScreen();
}

void UOperationsScreen::OnSelectButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);
}

void UOperationsScreen::OnSelectButtonUnHovered()
{
}

void UOperationsScreen::OnOrdersButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);

	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(0);
	}
	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("ORDERS"));
	}
	SetCampaignOrders();
}

void UOperationsScreen::OnOrdersButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);
}

void UOperationsScreen::OnOrdersButtonUnHovered()
{
}

void UOperationsScreen::OnTheaterButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);

	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(1);
	}

	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("THEATER"));
	}
}

void UOperationsScreen::OnTheaterButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);
}

void UOperationsScreen::OnTheaterButtonUnHovered()
{
}

void UOperationsScreen::OnForcesButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);

	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(2);
	}
	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("FORCES"));
	}
}

void UOperationsScreen::OnForcesButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);
}

void UOperationsScreen::OnForcesButtonUnHovered()
{
	
}

void UOperationsScreen::OnIntelButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance(); 
	SSWInstance->PlayAcceptSound(this);

	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(3);
	}

	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("INTEL"));
	}

	PopulateIntelList();
	SetSelectedIntelData(SSWInstance->GetSelectedActionNr());
}

void UOperationsScreen::OnIntelButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);
}

void UOperationsScreen::OnIntelButtonUnHovered()
{
}

void UOperationsScreen::OnMissionsButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);

	if (OperationalSwitcher) {
		OperationalSwitcher->SetActiveWidgetIndex(4);
	}

	if (OperationsModeText) {
		OperationsModeText->SetText(FText::FromString("MISSIONS"));
	}
	SetCampaignMissions();
	PopulateMissionList(); 
	SetSelectedMissionData(SSWInstance->GetSelectedMissionNr());
}

void UOperationsScreen::OnMissionsButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);
}

void UOperationsScreen::OnMissionsButtonUnHovered()
{
}

void UOperationsScreen::OnAudioButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);

	SSWInstance->PlaySoundFromFile(AudioPath);
	//if(AudioAsset)
	//{
	//	UGameplayStatics::PlaySound2D(GetWorld(), AudioAsset);
	//}
}

void UOperationsScreen::OnCancelButtonClicked()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayAcceptSound(this);

	SSWInstance->LoadMainMenuScreen();
}

void UOperationsScreen::OnCancelButtonHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);
}

void UOperationsScreen::OnCancelButtonUnHovered()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	SSWInstance->PlayHoverSound(this);
}

void UOperationsScreen::SetCampaignOrders()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	if (CampaignNameText) {
		CampaignNameText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Name));
	}

	if (DescriptionText) {
		DescriptionText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Description));
	}

	if (SituationText) {
		SituationText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Situation));
	}

	if (Orders1Text) {

		Orders1Text->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Orders[0]));
	}
	if (Orders2Text) {

		Orders2Text->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Orders[1]));
	}
	if (Orders3Text) {

		Orders3Text->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Orders[2]));
	}
	if (Orders4Text) {

		Orders4Text->SetText(FText::FromString(SSWInstance->GetActiveCampaign().Orders[3]));
	}
	if (LocationSystemText) {
		FString LocationSystem = SSWInstance->GetActiveCampaign().System + "/" + SSWInstance->GetActiveCampaign().Region;
		LocationSystemText->SetText(FText::FromString(LocationSystem));
	}	
}

void UOperationsScreen::SetCampaignMissions()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	
	TArray<FS_CampaignMissionList> Missions = SSWInstance->GetActiveCampaign().MissionList;

	for (FS_CampaignMissionList info : Missions)
	{
		FString NewMission = info.Name;
		//UE_LOG(LogTemp, Log, TEXT("Mission Name Selected: %s"), *NewMission);
	}
}

void UOperationsScreen::PopulateMissionList()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance(); 
	
	if (!MissionList) return;

	MissionList->ClearListItems();

	for (int32 i = 0; i < SSWInstance->GetActiveCampaign().MissionList.Num(); ++i)
	{
		UMissionListObject* ListItem = NewObject<UMissionListObject>();
		ListItem->MissionName = SSWInstance->GetActiveCampaign().MissionList[i].Name;
		ListItem->MissionType = SSWInstance->GetActiveCampaign().MissionList[i].TypeName;
		ListItem->MissionTime = SSWInstance->GetActiveCampaign().MissionList[i].Start;
		ListItem->MissionRegion = SSWInstance->GetActiveCampaign().MissionList[i].Region;
		ListItem->MissionSystem = SSWInstance->GetActiveCampaign().MissionList[i].System;
		ListItem->MissionSitrep = SSWInstance->GetActiveCampaign().MissionList[i].System;
		
		if (SSWInstance->GetActiveCampaign().MissionList[i].Status == EMISSIONSTATUS::Active) {
			ListItem->MissionStatus = "Active";
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[i].Status == EMISSIONSTATUS::Complete) {
			ListItem->MissionStatus = "Complete";
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[i].Status == EMISSIONSTATUS::Ready) {
			ListItem->MissionStatus = "Ready";
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[i].Status == EMISSIONSTATUS::Available) {
			ListItem->MissionStatus = "Available";
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[i].Status == EMISSIONSTATUS::Pending) {
			ListItem->MissionStatus = "Pending";
		}

		MissionList->GetIndexForItem(ListItem);
		MissionList->AddItem(ListItem);

		//UE_LOG(LogTemp, Log, TEXT("Ops Mission Name: %s"), *ListItem->MissionName);
	}
}

void UOperationsScreen::PopulateIntelList()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	if (!IntelList) return;

	IntelList->ClearListItems();
	ActionList.Empty();

	for (int32 i = 0; i < SSWInstance->GetActiveCampaign().Action.Num(); ++i)
	{
		if (SSWInstance->GetActiveCampaign().Action[i].Type == "event") {
			UIntelListObject* ListItem = NewObject<UIntelListObject>();
			FS_CampaignAction ActiveAction;

			ListItem->NewsTitle = SSWInstance->GetActiveCampaign().Action[i].Title;
			ActiveAction.Title = SSWInstance->GetActiveCampaign().Action[i].Title;

			ListItem->NewsLocation = SSWInstance->GetActiveCampaign().Action[i].Region;
			ActiveAction.Region = SSWInstance->GetActiveCampaign().Action[i].Region;

			ListItem->NewsSource = SSWInstance->GetActiveCampaign().Action[i].Source;
			ActiveAction.Source = SSWInstance->GetActiveCampaign().Action[i].Source;

			ListItem->NewsDate = SSWInstance->GetActiveCampaign().Action[i].Date;
			ActiveAction.Date = SSWInstance->GetActiveCampaign().Action[i].Date;

			ListItem->NewsInfoText = SSWInstance->GetActiveCampaign().Action[i].Message;
			ActiveAction.Message = SSWInstance->GetActiveCampaign().Action[i].Message;

			ListItem->NewsImage = SSWInstance->GetActiveCampaign().Action[i].Image;
			ActiveAction.Image = SSWInstance->GetActiveCampaign().Action[i].Image;

			ListItem->NewsAudio = SSWInstance->GetActiveCampaign().Action[i].Audio;
			ActiveAction.Audio = SSWInstance->GetActiveCampaign().Action[i].Audio;

			ListItem->NewsVisited = false;

			IntelList->GetIndexForItem(ListItem);
			ActionList.Add(ActiveAction);
			IntelList->AddItem(ListItem);
		}
	}
}

FDateTime UOperationsScreen::GetCampaignTime()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	return FDateTime::FromUnixTimestamp(SSWInstance->GetGameTime());
}

void UOperationsScreen::SetSelectedMissionData(int Selected) 
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	FString SelectedMissionName = SSWInstance->GetActiveCampaign().MissionList[Selected].Name;
	
	if (MissionNameText) {
		MissionNameText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().MissionList[Selected].Name));
	}	
	if (MissionDescriptionText) {
		MissionDescriptionText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().MissionList[Selected].Description));
	}
	if (MissionSitrepText) {
		FString SitrepText = SSWInstance->GetActiveCampaign().MissionList[Selected].Sitrep;
		SitrepText = SitrepText.Replace(TEXT("\\n"), TEXT("\n"));

		MissionSitrepText->SetText(FText::FromString(SitrepText));
	}
	if (MissionStartText) {
		MissionStartText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().MissionList[Selected].Start));
	}
	if (MissionStatusText) {
		if (SSWInstance->GetActiveCampaign().MissionList[Selected].Status == EMISSIONSTATUS::Active) {
			MissionStatusText->SetText(FText::FromString("Active"));
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[Selected].Status == EMISSIONSTATUS::Complete) {
			MissionStatusText->SetText(FText::FromString("Complete"));
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[Selected].Status == EMISSIONSTATUS::Ready) {
			MissionStatusText->SetText(FText::FromString("Ready"));
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[Selected].Status == EMISSIONSTATUS::Available) {
			MissionStatusText->SetText(FText::FromString("Available"));
		}
		else if (SSWInstance->GetActiveCampaign().MissionList[Selected].Status == EMISSIONSTATUS::Pending) {
			MissionStatusText->SetText(FText::FromString("Pending"));
		}
	}
	if (MissionTypeText) {
		MissionTypeText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().MissionList[Selected].TypeName));
	}
	if (MissionSystemText) {
		MissionSystemText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().MissionList[Selected].System));
	}
	if (MissionRegionText) {
		MissionRegionText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().MissionList[Selected].Region));
	}
	if (MissionObjectiveText) {
		MissionObjectiveText->SetText(FText::FromString(SSWInstance->GetActiveCampaign().MissionList[Selected].Objective));
	}

	GetMissionImageFile(Selected);

	UTexture2D* LoadedTexture = LoadTextureFromFile();
	if (LoadedTexture && MissionImage)
	{
		FSlateBrush Brush = CreateBrushFromTexture(LoadedTexture, FVector2D(LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY()));
		MissionImage->SetBrush(Brush);
	}

	UE_LOG(LogTemp, Log, TEXT("Ops Mission Selected: %s"), *SelectedMissionName);
	SSWInstance->MissionSelectionChanged = false;

	SSWInstance->PlayerInfo.Mission = Selected;
	SSWInstance->SaveGame(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot, SSWInstance->PlayerInfo);
}

void UOperationsScreen::SetSelectedIntelData(int Selected)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	if (IntelNameText) {
		IntelNameText->SetText(FText::FromString(ActionList[Selected].Title));
	}

	if (IntelLocationText) {
		IntelLocationText->SetText(FText::FromString(ActionList[Selected].Region));
	}

	if (IntelSourceText) {
		IntelSourceText->SetText(FText::FromString(ActionList[Selected].Source));
	}
	if (IntelDateText) {
		IntelDateText->SetText(FText::FromString(ActionList[Selected].Date));
	}

	if (IntelMessageText) {
		FString MessageText = ActionList[Selected].Message;
		MessageText = MessageText.Replace(TEXT("\\n"), TEXT("\n"));

		IntelMessageText->SetText(FText::FromString(MessageText));
	}

	GetIntelImageFile(ActionList[Selected].Image);
	GetIntelAudioFile(ActionList[Selected].Audio);

	UTexture2D* LoadedTexture = LoadTextureFromFile();
	if (LoadedTexture && IntelImage)
	{
		FSlateBrush Brush = CreateBrushFromTexture(LoadedTexture, FVector2D(LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY()));
		IntelImage->SetBrush(Brush);
	}
	SSWInstance->ActionSelectionChanged = false;
}

void UOperationsScreen::GetIntelImageFile(FString IntelImageName)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	ImagePath = FPaths::ProjectContentDir() + TEXT("UI/Campaigns/0");
	ImagePath.Append(FString::FromInt(SSWInstance->GetActiveCampaign().Index + 1));
	ImagePath.Append("/");
	ImagePath.Append(IntelImageName);
	ImagePath.Append(".png");
	UE_LOG(LogTemp, Log, TEXT("Action Image: %s"), *ImagePath);
}

void UOperationsScreen::GetIntelAudioFile(FString IntelAudioName)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	AudioPath = "/Game/Audio/Vox/Scenes/0";
	AudioPath.Append(FString::FromInt(SSWInstance->GetActiveCampaign().Index + 1));
	AudioPath.Append("/");
	AudioPath.Append(IntelAudioName);
	AudioPath.Append(".");
	AudioPath.Append(IntelAudioName);

	//SSWInstance->PlaySoundFromFile(AudioPath);
	//AudioAsset = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, *AudioPath));

	UE_LOG(LogTemp, Log, TEXT("Action Audio: %s"), *AudioPath);
}

void UOperationsScreen::GetMissionImageFile(int selected)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	ImagePath = FPaths::ProjectContentDir() + TEXT("UI/Campaigns/0");
	ImagePath.Append(FString::FromInt(SSWInstance->GetActiveCampaign().Index + 1));
	ImagePath.Append("/");
	ImagePath.Append(SSWInstance->GetActiveCampaign().MissionList[selected].MissionImage);
	ImagePath.Append(".png");
	UE_LOG(LogTemp, Log, TEXT("Mission Image: %s"), *ImagePath);
}


UTexture2D* UOperationsScreen::LoadTextureFromFile()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	UTexture2D* LoadedTexture = SSWInstance->LoadPNGTextureFromFile(ImagePath);
	return LoadedTexture;
}

FSlateBrush UOperationsScreen::CreateBrushFromTexture(UTexture2D* Texture, FVector2D ImageSize)
{
	FSlateBrush Brush;
	Brush.SetResourceObject(Texture);
	Brush.ImageSize = ImageSize;
	Brush.DrawAs = ESlateBrushDrawType::Image;
	return Brush;
}



