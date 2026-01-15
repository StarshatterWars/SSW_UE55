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

	// Capture selection metadata used by save and UI
	GI->SelectedCampaignDisplayName = CampaignSelectDD ? CampaignSelectDD->GetSelectedOption() : TEXT("");
	GI->SelectedCampaignIndex =
		CampaignIndexByOptionIndex.IsValidIndex(Selected) ? CampaignIndexByOptionIndex[Selected] : (Selected + 1);
	GI->SelectedCampaignRowName = PickedRowName;

	// Persist player selection
	GI->SaveGame(GI->PlayerSaveName, GI->PlayerSaveSlot, GI->PlayerInfo);

	// --- Ensure CampaignSave exists/loaded ---
	if (bHasSave)
	{
		// CONTINUE: load existing, do NOT reset
		GI->LoadOrCreateSelectedCampaignSave();
	}
	else
	{
		// START: create new save and reset timeline
		GI->CreateNewCampaignSave(
			GI->SelectedCampaignIndex,
			PickedRowName,
			GI->SelectedCampaignDisplayName
		);
	}

	// --- Timer should always point at current campaign save ---
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Timer->SetCampaignSave(GI->CampaignSave);

		// Only restart clock when starting fresh
		if (!bHasSave)
		{
			Timer->RestartCampaignClock(true);
		}
	}

	// --- CampaignSubsystem becomes authoritative for runtime campaign logic ---
	if (UCampaignSubsystem* Campaign = GetGameInstance()->GetSubsystem<UCampaignSubsystem>())
	{
		// Transitional: allow CampaignSubsystem to read the DT until you move to GameDataSubsystem
		Campaign->SetCampaignDataTable(GI->CampaignDataTable);

		// Start (or resume later once you add ResumeCampaign)
		Campaign->StartCampaign(GI->SelectedCampaignIndex);
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

	USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance());
	if (!GI)
		return;

	if (PickedRowName.IsNone())
		return;

	GI->PlayerInfo.Campaign = Selected;

	GI->SelectedCampaignDisplayName = CampaignSelectDD ? CampaignSelectDD->GetSelectedOption() : TEXT("");
	GI->SelectedCampaignIndex =
		CampaignIndexByOptionIndex.IsValidIndex(Selected) ? CampaignIndexByOptionIndex[Selected] : (Selected + 1);
	GI->SelectedCampaignRowName = PickedRowName;

	// Overwrite/create save FIRST
	GI->CreateNewCampaignSave(
		GI->SelectedCampaignIndex,
		PickedRowName,
		GI->SelectedCampaignDisplayName
	);

	// Restart campaign clock on the new save
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Timer->SetCampaignSave(GI->CampaignSave);
		Timer->RestartCampaignClock(true);
	}

	// Restart campaign runtime
	if (UCampaignSubsystem* Campaign = GetGameInstance()->GetSubsystem<UCampaignSubsystem>())
	{
		Campaign->SetCampaignDataTable(GI->CampaignDataTable);
		Campaign->StartCampaign(GI->SelectedCampaignIndex);
	}

	// Save player selection
	GI->SaveGame(GI->PlayerSaveName, GI->PlayerSaveSlot, GI->PlayerInfo);

	GI->ShowCampaignLoading();
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
	USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance());
	if (!CampaignSelectDD || !GI || !GI->CampaignDataTable)
		return;

	CampaignSelectDD->ClearOptions();
	CampaignSelectDD->ClearSelection();

	CampaignRowNamesByOptionIndex.Reset();
	CampaignIndexByOptionIndex.Reset();

	const TArray<FName> RowNames = GI->CampaignDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		const FS_Campaign* Row =
			GI->CampaignDataTable->FindRow<FS_Campaign>(RowName, TEXT("CampaignScreen"));

		if (!Row || !Row->bAvailable)
			continue;

		CampaignSelectDD->AddOption(Row->Name);

		CampaignRowNamesByOptionIndex.Add(RowName);
		CampaignIndexByOptionIndex.Add(Row->Index + 1);
	}

	const int32 MaxIndex = FMath::Max(0, CampaignRowNamesByOptionIndex.Num() - 1);
	const int32 SelectedOptionIndex =
		FMath::Clamp(GI->PlayerInfo.Campaign, 0, MaxIndex);

	CampaignSelectDD->SetSelectedIndex(SelectedOptionIndex);

	Selected = SelectedOptionIndex;
	PickedRowName =
		CampaignRowNamesByOptionIndex.IsValidIndex(Selected)
		? CampaignRowNamesByOptionIndex[Selected]
		: NAME_None;

	// -----------------------------
	// CACHE DISPLAY ROW (KEY FIX)
	// -----------------------------
	bHasCachedCampaignRow = false;

	if (!PickedRowName.IsNone())
	{
		if (const FS_Campaign* Row =
			GI->CampaignDataTable->FindRow<FS_Campaign>(PickedRowName, TEXT("CampaignScreen")))
		{
			CachedCampaignRow = *Row; // snapshot copy
			bHasCachedCampaignRow = true;
		}
	}
}

void UCampaignScreen::SetSelectedData(int selected)
{
	USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance());
	if (!GI)
		return;

	Selected = selected;

	if (!bHasCachedCampaignRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("CampaignScreen: No cached campaign row for selection %d"), selected);
		return;
	}

	// -----------------------------
	// Campaign Image
	// -----------------------------
	GetCampaignImageFile(selected);

	if (UTexture2D* LoadedTexture = LoadTextureFromFile())
	{
		if (CampaignImage)
		{
			const FVector2D ImageSize(
				LoadedTexture->GetSizeX(),
				LoadedTexture->GetSizeY()
			);

			FSlateBrush Brush = CreateBrushFromTexture(LoadedTexture, ImageSize);
			CampaignImage->SetBrush(Brush);
		}
	}

	// -----------------------------
	// Text Fields (DISPLAY ONLY)
	// -----------------------------
	if (CampaignNameText)
		CampaignNameText->SetText(FText::FromString(CachedCampaignRow.Name));

	if (CampaignStartTimeText)
		CampaignStartTimeText->SetText(FText::FromString(CachedCampaignRow.Start));

	if (DescriptionText)
		DescriptionText->SetText(FText::FromString(CachedCampaignRow.Description));

	if (SituationText)
		SituationText->SetText(FText::FromString(CachedCampaignRow.Situation));

	// -----------------------------
	// Orders (no mutation)
	// -----------------------------
	if (Orders1Text)
		Orders1Text->SetText(FText::FromString(
			CachedCampaignRow.Orders.IsValidIndex(0) ? CachedCampaignRow.Orders[0] : TEXT("")
		));

	if (Orders2Text)
		Orders2Text->SetText(FText::FromString(
			CachedCampaignRow.Orders.IsValidIndex(1) ? CachedCampaignRow.Orders[1] : TEXT("")
		));

	if (Orders3Text)
		Orders3Text->SetText(FText::FromString(
			CachedCampaignRow.Orders.IsValidIndex(2) ? CachedCampaignRow.Orders[2] : TEXT("")
		));

	if (Orders4Text)
		Orders4Text->SetText(FText::FromString(
			CachedCampaignRow.Orders.IsValidIndex(3) ? CachedCampaignRow.Orders[3] : TEXT("")
		));

	// -----------------------------
	// Location
	// -----------------------------
	if (LocationSystemText)
	{
		const FString LocationText =
			CachedCampaignRow.System + TEXT("/") + CachedCampaignRow.Region;

		LocationSystemText->SetText(FText::FromString(LocationText));
	}

	// -----------------------------
	// Optional: Persist selection
	// -----------------------------
	GI->PlayerInfo.Campaign = Selected;
	GI->SaveGame(GI->PlayerSaveName, GI->PlayerSaveSlot, GI->PlayerInfo);
}

void UCampaignScreen::OnSetSelected(FString /*SelectedItem*/, ESelectInfo::Type Type)
{
	// Ignore programmatic changes
	if (Type == ESelectInfo::Direct)
		return;

	if (!CampaignSelectDD)
		return;

	const int32 NewIndex =
		CampaignSelectDD->FindOptionIndex(CampaignSelectDD->GetSelectedOption());

	if (NewIndex == INDEX_NONE)
		return;

	Selected = NewIndex;
	PickedRowName =
		CampaignRowNamesByOptionIndex.IsValidIndex(NewIndex)
		? CampaignRowNamesByOptionIndex[NewIndex]
		: NAME_None;

	USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance());

	// -----------------------------
	// CACHE DISPLAY ROW (KEY FIX)
	// -----------------------------
	bHasCachedCampaignRow = false;

	if (GI && GI->CampaignDataTable && !PickedRowName.IsNone())
	{
		if (const FS_Campaign* Row =
			GI->CampaignDataTable->FindRow<FS_Campaign>(PickedRowName, TEXT("CampaignScreen")))
		{
			CachedCampaignRow = *Row;
			bHasCachedCampaignRow = true;
		}
	}

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


