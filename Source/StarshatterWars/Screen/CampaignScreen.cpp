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
			PlayButtonText->SetText(FText::FromString("CONTINUE"));
		}

		if (RestartButton) {
			RestartButton->OnClicked.AddDynamic(this, &UCampaignScreen::OnRestartButtonClicked);
			RestartButton->OnHovered.AddDynamic(this, &UCampaignScreen::OnRestartButtonHovered);
			RestartButton->OnUnhovered.AddDynamic(this, &UCampaignScreen::OnRestartButtonUnHovered);

			if (SSWInstance->PlayerInfo.Campaign >= 0) {
				RestartButton->SetIsEnabled(true);
			}
			else {
				RestartButton->SetIsEnabled(false);
			}

			if (RestartButtonText) {
				RestartButtonText->SetText(FText::FromString("RESTART"));
			}
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
	UpdateCampaignButtons();
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

/*void UCampaignScreen::OnPlayButtonClicked()
{
	PlayUISound(this, AcceptSound);

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	if (!SSWInstance)
		return;

	const bool bHasSave = DoesSelectedCampaignSaveExist();

	SSWInstance->PlayerInfo.Campaign = Selected;
	SSWInstance->SetActiveCampaign(SSWInstance->CampaignData[Selected]);

	SSWInstance->SelectedCampaignDisplayName = CampaignSelectDD ? CampaignSelectDD->GetSelectedOption() : TEXT("");
	SSWInstance->SelectedCampaignIndex =
		CampaignIndexByOptionIndex.IsValidIndex(Selected) ? CampaignIndexByOptionIndex[Selected] : (Selected + 1);
	SSWInstance->SelectedCampaignRowName = PickedRowName;

	// Persist player selection (optional but consistent with your flow)
	SSWInstance->SaveGame(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot, SSWInstance->PlayerInfo);

	if (bHasSave)
	{
		// CONTINUE: load existing, do NOT reset
		SSWInstance->LoadOrCreateSelectedCampaignSave();
	}
	else
	{
		// START: create new save and reset timeline
		SSWInstance->CreateNewCampaignSave(
			SSWInstance->SelectedCampaignIndex,
			PickedRowName,
			SSWInstance->SelectedCampaignDisplayName
		);
	}

	SSWInstance->ShowCampaignLoading();
}*/

void UCampaignScreen::OnPlayButtonClicked()
{
	PlayUISound(this, AcceptSound);

	USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance());
	if (!GI)
		return;

	if (PickedRowName.IsNone())
		return;

	const bool bHasSave = DoesSelectedCampaignSaveExist();

	// Persist selection in PlayerInfo (fine to keep)
	GI->PlayerInfo.Campaign = Selected;
	GI->SaveGame(GI->PlayerSaveName, GI->PlayerSaveSlot, GI->PlayerInfo);

	// Resolve the numeric campaign index you use for naming/legacy (you already compute it)
	GI->SelectedCampaignDisplayName = CampaignSelectDD ? CampaignSelectDD->GetSelectedOption() : TEXT("");
	GI->SelectedCampaignIndex =
		CampaignIndexByOptionIndex.IsValidIndex(Selected) ? CampaignIndexByOptionIndex[Selected] : (Selected + 1);
	GI->SelectedCampaignRowName = PickedRowName;

	// --- Save workflow ---
	if (bHasSave)
	{
		GI->LoadOrCreateSelectedCampaignSave();   // should load into GI->CampaignSave
	}
	else
	{
		GI->CreateNewCampaignSave(
			GI->SelectedCampaignIndex,
			PickedRowName,
			GI->SelectedCampaignDisplayName
		);
	}

	// Point timer at the active campaign save (required)
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Timer->SetCampaignSave(GI->CampaignSave);

		// If you want CONTINUE to keep its time, do not restart here.
		// If you want START to reset timeline, restart only for new campaigns.
		if (!bHasSave)
		{
			Timer->RestartCampaignClock(true);
		}
	}

	// --- Campaign subsystem now becomes authoritative ---
	if (UCampaignSubsystem* Campaign = GetGameInstance()->GetSubsystem<UCampaignSubsystem>())
	{
		// Transitional: ensure CampaignSubsystem knows where the DT is (until you move to GameDataSubsystem)
		Campaign->SetCampaignDataTable(GI->CampaignDataTable);

		if (bHasSave)
		{
			// Resume campaign runtime state from save (if you have it, else just StartCampaign)
			Campaign->StartCampaign(GI->SelectedCampaignIndex);
			// Later: Campaign->ResumeCampaign(GI->CampaignSave);
		}
		else
		{
			Campaign->StartCampaign(GI->SelectedCampaignIndex);
		}
	}

	GI->ShowCampaignLoading();
}

void UCampaignScreen::OnPlayButtonHovered()
{
	PlayUISound(this, HoverSound);
}

void UCampaignScreen::OnPlayButtonUnHovered()
{
}
void UCampaignScreen::OnRestartButtonClicked()
{
	PlayUISound(this, AcceptSound);

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	if (!SSWInstance)
		return;

	SSWInstance->PlayerInfo.Campaign = Selected;
	SSWInstance->SetActiveCampaign(SSWInstance->CampaignData[Selected]);

	SSWInstance->SelectedCampaignDisplayName = SSWInstance->GetActiveCampaign().Name;
	SSWInstance->SelectedCampaignIndex =
		CampaignIndexByOptionIndex.IsValidIndex(Selected) ? CampaignIndexByOptionIndex[Selected] : (Selected + 1);
	SSWInstance->SelectedCampaignRowName = PickedRowName;

	// Create/overwrite save FIRST (so timer points at correct object)
	SSWInstance->CreateNewCampaignSave(
		SSWInstance->SelectedCampaignIndex,
		PickedRowName,
		SSWInstance->SelectedCampaignDisplayName
	);

	// Then restart campaign clock on the newly injected save
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Timer->SetCampaignSave(SSWInstance->CampaignSave);
		Timer->RestartCampaignClock(true);
	}

	if (UCampaignSubsystem* Campaign = GetGameInstance()->GetSubsystem<UCampaignSubsystem>())
	{
		Campaign->SetCampaignDataTable(SSWInstance->CampaignDataTable);
		Campaign->StartCampaign(SSWInstance->SelectedCampaignIndex); // Start fresh
	}

	SSWInstance->SaveGame(SSWInstance->PlayerSaveName, SSWInstance->PlayerSaveSlot, SSWInstance->PlayerInfo);
	SSWInstance->ShowCampaignLoading();
}

void UCampaignScreen::OnRestartButtonHovered()
{
	PlayUISound(this, HoverSound);
}

void UCampaignScreen::OnRestartButtonUnHovered()
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
	USSWGameInstance* GI = (USSWGameInstance*)GetGameInstance();

	if (!CampaignSelectDD || !GI || !GI->CampaignDataTable)
		return;

	CampaignSelectDD->ClearOptions();
	CampaignSelectDD->ClearSelection();

	CampaignRowNamesByOptionIndex.Reset();
	CampaignIndexByOptionIndex.Reset();

	const TArray<FName> RowNames = GI->CampaignDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		const FS_Campaign* Row = GI->CampaignDataTable->FindRow<FS_Campaign>(RowName, TEXT("CampaignScreen"));
		if (!Row)
			continue;

		if (!Row->bAvailable)
			continue;

		// Add visible option text
		CampaignSelectDD->AddOption(Row->Name);

		// Store RowName at same option index
		CampaignRowNamesByOptionIndex.Add(RowName);

		// IMPORTANT: your FS_Campaign has "Index" (0-based in your current usage)
		// If your SaveGame slot naming expects 1-based campaign index, store Index+1.
		CampaignIndexByOptionIndex.Add(Row->Index + 1);
	}

	const int32 MaxIndex = FMath::Max(0, CampaignRowNamesByOptionIndex.Num() - 1);
	const int32 SelectedOptionIndex = FMath::Clamp(GI->PlayerInfo.Campaign, 0, MaxIndex);

	CampaignSelectDD->SetSelectedIndex(SelectedOptionIndex);

	Selected = SelectedOptionIndex;
	PickedRowName = CampaignRowNamesByOptionIndex.IsValidIndex(Selected)
		? CampaignRowNamesByOptionIndex[Selected]
		: NAME_None;
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

	if (CampaignNameText) {
		CampaignNameText->SetText(FText::FromString(SSWInstance->CampaignData[selected].Name));
	}
	if (CampaignStartTimeText) {
		CampaignStartTimeText->SetText(FText::FromString(SSWInstance->CampaignData[selected].Start));
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

void UCampaignScreen::OnSetSelected(FString /*SelectedItem*/, ESelectInfo::Type Type)
{
	// You probably want this to respond to all user-driven changes:
	if (Type == ESelectInfo::Direct) // ignore programmatic SetSelectedIndex
	{
		return;
	}

	if (!CampaignSelectDD)
		return;

	const int32 NewIndex = CampaignSelectDD->FindOptionIndex(CampaignSelectDD->GetSelectedOption());
	if (NewIndex == INDEX_NONE)
		return;

	// Update our picked row name from the option index arrays you already maintain
	Selected = NewIndex;
	PickedRowName = CampaignRowNamesByOptionIndex.IsValidIndex(NewIndex)
		? CampaignRowNamesByOptionIndex[NewIndex]
		: NAME_None;

	// Update the right-panel details and save player selection
	SetSelectedData(NewIndex);
	UpdateCampaignButtons();
}

void UCampaignScreen::GetCampaignImageFile(int selected)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	ImagePath = FPaths::ProjectContentDir() + TEXT("UI/Campaigns/0");
	ImagePath.Append(FString::FromInt(selected + 1));
	ImagePath.Append("/");
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

bool UCampaignScreen::DoesSelectedCampaignSaveExist() const
{
	const USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance());
	if (!GI)
		return false;

	// You need the selected campaign RowName at this point.
	// If you store it on selection, use that:
	if (PickedRowName.IsNone())
		return false;

	const FString GameSlot = UCampaignSave::MakeSlotNameFromRowName(PickedRowName);
	constexpr int32 UserIndex = 0;
	return UGameplayStatics::DoesSaveGameExist(GameSlot, UserIndex);
}

void UCampaignScreen::UpdateCampaignButtons()
{
	const bool bHasSave = DoesSelectedCampaignSaveExist();

	// PLAY button is always enabled when a campaign is selected
	if (PlayButton)
	{
		PlayButton->SetIsEnabled(true);
	}

	// Label depends on save existence
	if (PlayButtonText)
	{
		PlayButtonText->SetText(FText::FromString(bHasSave ? TEXT("CONTINUE") : TEXT("START")));
	}

	// Restart only makes sense if there is something to restart
	if (RestartButton)
	{
		RestartButton->SetIsEnabled(bHasSave);
	}
}


