// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "OperationsScreen.h"
#include "MissionListObject.h"
#include "IntelListObject.h"
#include "RosterViewObject.h"
#include "RosterTVElement.h"
#include "Components/ListView.h"

#include "OOBForceItem.h"
#include "OOBFleetItem.h"
#include "OOBCarrierGroupItem.h" 
#include "OOBBattleItem.h"
#include "OOBDestroyerItem.h"
#include "OOBWingItem.h"

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

	LoadForces();

	if(ForceListView) {
		ForceListView->OnItemClicked().AddUObject(this, &UOperationsScreen::OnForceSelected);
		ForceListView->ClearListItems();

		for (const FS_OOBForce& Force : LoadedForces)
		{
			UOOBForceItem* ForceItem = NewObject<UOOBForceItem>(this);
			ForceItem->Data = Force;
			ForceListView->AddItem(ForceItem);
			//UE_LOG(LogTemp, Log, TEXT("Force Item Name: %s"), ForceItem->Data);
		}
	}

	if (FleetListView) {
		FleetListView->ClearListItems();
		FleetListView->OnItemClicked().AddUObject(this, &UOperationsScreen::OnFleetSelected);
	}

	if (CarrierListView) {
		CarrierListView->ClearListItems();
		CarrierListView->OnItemClicked().AddUObject(this, &UOperationsScreen::OnCarrierSelected);
	}

	if (BattleListView) {
		BattleListView->ClearListItems();
		BattleListView->OnItemClicked().AddUObject(this, &UOperationsScreen::OnBattleGroupSelected);
	}

	if (DesronListView) {
		DesronListView->ClearListItems();
		DesronListView->OnItemClicked().AddUObject(this, &UOperationsScreen::OnDesronSelected);
	}

	SelectedMission = 0;

	SetCampaignOrders();
	PopulateMissionList();
	PopulateIntelList();
	PopulateCombatRoster();
	PrintGroupData(GetAllGroupsList());
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

	if (SSWInstance->RosterSelectionChanged) {
		SetSelectedRosterData(SSWInstance->GetSelectedRosterNr());
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
	SetSelectedRosterData(SSWInstance->GetSelectedRosterNr());
	if (FleetInfoBorder) FleetInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
	if (CarrierInfoBorder) CarrierInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
	if (BattleInfoBorder) BattleInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
	if (DesronInfoBorder) DesronInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
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

void UOperationsScreen::PopulateCombatRoster()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	if (!RosterView) return;;
	RosterView->ClearListItems();
	BuildHierarchy();

	for (FS_CombatGroup Item : GetBaseGroupsList())
	{
		URosterViewObject* ListItem = NewObject<URosterViewObject>();
		ListItem->Group = Item;
		RosterView->GetIndexForItem(ListItem);
		if (Item.Intel == EINTEL_TYPE::KNOWN) {
			RosterView->AddItem(ListItem);
		}
	}
}

FDateTime UOperationsScreen::GetCampaignTime()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	return FDateTime::FromUnixTimestamp(SSWInstance->GetGameTime());
}

void UOperationsScreen::OnForceSelected(UObject* SelectedItem)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	UE_LOG(LogTemp, Log, TEXT("Force Clicked"));
	UOOBForceItem* ForceItem = Cast<UOOBForceItem>(SelectedItem);
	if (!ForceItem) return;

	if (FleetInfoBorder) FleetInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
	if (CarrierInfoBorder) CarrierInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
	if (BattleInfoBorder) BattleInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
	if (DesronInfoBorder) DesronInfoBorder->SetVisibility(ESlateVisibility::Collapsed);

	if(FleetListView) {
		FleetListView->ClearListItems();

		for (const FS_OOBFleet& Fleet : ForceItem->Data.Fleet)
		{
			UOOBFleetItem* FleetItem = NewObject<UOOBFleetItem>(this);
			FleetItem->Data = Fleet;
			FleetListView->AddItem(FleetItem);
		}
		
		if (FleetListView->GetNumItems() > 0) {
			if (FleetInfoBorder) FleetInfoBorder->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			if (FleetInfoBorder) FleetInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (CarrierListView) {
		CarrierListView->ClearListItems();
	}
	if (DesronListView) {
		DesronListView->ClearListItems();
	}
	if (BattleListView) {
		BattleListView->ClearListItems();
	}

	if (UOOBForceItem* Force_Item = Cast<UOOBForceItem>(SelectedItem))
	{
		const FS_OOBForce& ForceData = Force_Item->Data;

		UE_LOG(LogTemp, Log, TEXT("Selected Force: %s"), *ForceData.Name);

		// Update UI fields
		if (GroupInfoText)
		{
			GroupInfoText->SetText(FText::FromString(ForceData.Name));
		}

		if (GroupTypeText)
		{
			GroupTypeText->SetText(FText::FromString(SSWInstance->GetNameFromType(ForceData.Type)));
		}

		if (GroupLocationText)
		{
			GroupLocationText->SetText(FText::FromString(ForceData.Location));
		}

		if (GroupEmpireText)
		{
			GroupEmpireText->SetText(FText::FromString(SSWInstance->GetEmpireTypeNameByIndex(ForceData.Empire)));
		}
		
		// Clear and populate fleets
		if (FleetListView)
		{
			FleetListView->ClearListItems();

			for (const FS_OOBFleet& Fleet : ForceData.Fleet)
			{
				UOOBFleetItem* FleetItem = NewObject<UOOBFleetItem>(this);
				FleetItem->Data = Fleet;
				FleetListView->AddItem(FleetItem);
			}

			if (FleetListView->GetNumItems() > 0) {
				if (FleetInfoBorder) FleetInfoBorder->SetVisibility(ESlateVisibility::Visible);
			}
			else {
				if (FleetInfoBorder) FleetInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

void UOperationsScreen::OnFleetSelected(UObject* SelectedItem)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	if (UOOBFleetItem* FleetItem = Cast<UOOBFleetItem>(SelectedItem))
	{
		const FS_OOBFleet& FleetData = FleetItem->Data;

		UE_LOG(LogTemp, Log, TEXT("Selected Fleet: %s"), *FleetData.Name);

		// Clear all subgroup list views first
		
		if (CarrierListView) CarrierListView->ClearListItems();
		if (BattleListView) BattleListView->ClearListItems();
		if (DesronListView) DesronListView->ClearListItems();

		if (CarrierInfoBorder) CarrierInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
		if (BattleInfoBorder) BattleInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
		if (DesronInfoBorder) DesronInfoBorder->SetVisibility(ESlateVisibility::Collapsed);

		// Update UI fields
		if (GroupInfoText)
		{
			GroupInfoText->SetText(FText::FromString(FleetData.Name));
		}

		if (GroupLocationText)
		{
			GroupLocationText->SetText(FText::FromString(FleetData.Location));
		}

		if (GroupEmpireText)
		{
			GroupEmpireText->SetText(FText::FromString(SSWInstance->GetEmpireTypeNameByIndex(FleetData.Empire)));
		}
		
		if (GroupTypeText)
		{
			GroupTypeText->SetText(FText::FromString(SSWInstance->GetNameFromType(FleetData.Type)));
		}

		// -- Carrier Groups --
		for (const FS_OOBCarrier& Carrier : FleetData.Carrier)
		{
			UOOBCarrierGroupItem* CarrierItem = NewObject<UOOBCarrierGroupItem>(this);
			CarrierItem->Data = Carrier;
			CarrierListView->AddItem(CarrierItem);
		}
		
		if (CarrierListView->GetNumItems() > 0) {
			if (CarrierInfoBorder) CarrierInfoBorder->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			if (CarrierInfoBorder) CarrierInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
		}

		// -- Battle Groups --
		for (const FS_OOBBattle& Battle : FleetData.Battle)
		{
			UOOBBattleItem* BattleItem = NewObject<UOOBBattleItem>(this);
			BattleItem->Data = Battle;
			BattleListView->AddItem(BattleItem);
		}

		if (BattleListView->GetNumItems() > 0) {
			if (BattleInfoBorder) BattleInfoBorder->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			if (BattleInfoBorder) BattleInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
		}

		// -- Desrons (Destroyer Groups) --
		for (const FS_OOBDestroyer& Destroyer : FleetData.Destroyer)
		{
			UOOBDestroyerItem* DestroyerItem = NewObject<UOOBDestroyerItem>(this);
			DestroyerItem->Data = Destroyer;
			DesronListView->AddItem(DestroyerItem);
		}
		
		if (DesronListView->GetNumItems() > 0) {
			if (DesronInfoBorder) DesronInfoBorder->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			if (DesronInfoBorder) DesronInfoBorder->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UOperationsScreen::OnCarrierSelected(UObject* SelectedItem)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	if (UOOBCarrierGroupItem* CarrierItem = Cast<UOOBCarrierGroupItem>(SelectedItem))
	{
		const FS_OOBCarrier& CarrierData = CarrierItem->Data;

		UE_LOG(LogTemp, Log, TEXT("Selected Carrier Group: %s"), *CarrierData.Name);

		// Update UI fields
		if (GroupInfoText)
		{
			GroupInfoText->SetText(FText::FromString(CarrierData.Name));
		}

		if (GroupLocationText)
		{
			GroupLocationText->SetText(FText::FromString(CarrierData.Location));
		}

		if (GroupEmpireText)
		{
			GroupEmpireText->SetText(FText::FromString(SSWInstance->GetEmpireTypeNameByIndex(CarrierData.Empire)));
		}

		if (GroupTypeText)
		{
			GroupTypeText->SetText(FText::FromString(SSWInstance->GetNameFromType(CarrierData.Type)));

		}
		
		// Clear the wing list view before adding new items
		if (WingListView)
		{
			WingListView->ClearListItems();

			// Populate WingListView with this carrier's wings
			for (const FS_OOBWing& Wing : CarrierData.Wing)
			{
				UOOBWingItem* WingItem = NewObject<UOOBWingItem>(this);
				WingItem->Data = Wing;
				WingListView->AddItem(WingItem);
			}
		}
	}
}

void UOperationsScreen::OnDesronSelected(UObject* SelectedItem)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	if (UOOBDestroyerItem* DesronItem = Cast<UOOBDestroyerItem>(SelectedItem))
	{
		const FS_OOBDestroyer& DesronData = DesronItem->Data;

		UE_LOG(LogTemp, Log, TEXT("Selected DESRON Group: %s"), *DesronData.Name);

		// Update UI fields
		if (GroupInfoText)
		{
			GroupInfoText->SetText(FText::FromString(DesronData.Name));
		}

		if (GroupLocationText)
		{
			GroupLocationText->SetText(FText::FromString(DesronData.Location));
		}

		if (GroupEmpireText)
		{
			GroupEmpireText->SetText(FText::FromString(SSWInstance->GetEmpireTypeNameByIndex(DesronData.Empire)));
		}

		if (GroupTypeText)
		{
			GroupTypeText->SetText(FText::FromString(SSWInstance->GetNameFromType(DesronData.Type)));
		}
	}
}

void UOperationsScreen::OnBattleGroupSelected(UObject* SelectedItem)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	if (UOOBBattleItem* BattleItem = Cast<UOOBBattleItem>(SelectedItem))
	{
		const FS_OOBBattle& BattleData = BattleItem->Data;

		UE_LOG(LogTemp, Log, TEXT("Selected Battle Group: %s"), *BattleData.Name);

		// Update UI fields
		if (GroupInfoText)
		{
			GroupInfoText->SetText(FText::FromString(BattleData.Name));
		}

		if (GroupLocationText)
		{
			GroupLocationText->SetText(FText::FromString(BattleData.Location));
		}

		if (GroupEmpireText)
		{
			GroupEmpireText->SetText(FText::FromString(SSWInstance->GetEmpireTypeNameByIndex(BattleData.Empire)));
		}

		if (GroupTypeText)
		{
			GroupTypeText->SetText(FText::FromString(SSWInstance->GetNameFromType(BattleData.Type)));
		}
	}
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

void UOperationsScreen::SetSelectedRosterData(int Selected)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance(); 
	if (GroupInfoText) {
		GroupInfoText->SetText(FText::FromString(SSWInstance->CombatRosterData[Selected].DisplayName));
	}

	if (GroupLocationText) {
		GroupLocationText->SetText(FText::FromString(SSWInstance->CombatRosterData[Selected].Region));
	}
	if (GroupLocationText) {
		
		GroupLocationText->SetText(FText::FromString(SSWInstance->CombatRosterData[Selected].Region));
	}
	if (GroupTypeText) {
		GroupTypeText->SetText(FText::FromString(SSWInstance->GetNameFromType(SSWInstance->CombatRosterData[Selected].Type)));
	}

	if (GroupEmpireText) {
		GroupEmpireText->SetText(FText::FromString(SSWInstance->GetEmpireTypeNameByIndex(SSWInstance->CombatRosterData[Selected].EmpireId)));
	}
	SSWInstance->RosterSelectionChanged = false;
}

void UOperationsScreen::BuildHierarchy()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	
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
FString
UOperationsScreen::GetOrdinal(int id)
{
	FString ordinal;

	int last_two_digits = id % 100;

	if (last_two_digits > 10 && last_two_digits < 20) {
		ordinal = FString::FormatAsNumber(id) + "th";
	}
	else {
		int last_digit = last_two_digits % 10;

		if (last_digit == 1)
			ordinal = FString::FormatAsNumber(id) + "st";
		else if (last_digit == 2)
			ordinal = FString::FormatAsNumber(id) + "nd";
		else if (last_digit == 3)
			ordinal = FString::FormatAsNumber(id) + "rd";
		else
			ordinal = FString::FormatAsNumber(id) + "th";
	}

	return ordinal;
}
const TArray<FS_CombatGroup>& UOperationsScreen::GetFlattenedList() const
{
	return FlattenedList;
}

void UOperationsScreen::SetFlattenedList(const TArray<FS_CombatGroup>& NewList)
{
	FlattenedList = NewList;
}

const TArray<FS_CombatGroup>& UOperationsScreen::GetAllGroupsList() const
{
	return AllGroups;
}

void UOperationsScreen::SetAllGroupsList(const TArray<FS_CombatGroup>& NewList)
{
	AllGroups = NewList;
}

const TArray<FS_CombatGroup>& UOperationsScreen::GetBaseGroupsList() const
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	return SSWInstance->CombatRosterData;
}

void UOperationsScreen::PrintGroupData(TArray<FS_CombatGroup> Group) const
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	for (FS_CombatGroup Item : Group)
	{
		UE_LOG(LogTemp, Log, TEXT("Item: %s"), *Item.DisplayName);
	}
}

void UOperationsScreen::SetBaseGroupsList(const TArray<FS_CombatGroup>& NewList)
{

}

void UOperationsScreen::LoadForces()
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	if (!SSWInstance->OrderOfBattleDataTable || !ForceListView) return;

	ForceListView->ClearListItems();
	LoadedForces.Empty();

	// Retrieve all rows
	TArray<FName> RowNames = SSWInstance->OrderOfBattleDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		if (FS_OOBForce* Force = SSWInstance->OrderOfBattleDataTable->FindRow<FS_OOBForce>(RowName, TEXT("LoadForces")))
		{
			LoadedForces.Add(*Force);

			// Wrap in UObject and add to list
			UOOBForceItem* ForceItem = NewObject<UOOBForceItem>(this);
			ForceItem->Data = *Force;
			ForceListView->AddItem(ForceItem);
		}
	}
}
