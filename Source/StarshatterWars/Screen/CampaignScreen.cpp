// /*  Project nGenEx  Fractal Dev Games  Copyright (C) 2024. All Rights Reserved.
//     SUBSYSTEM:    SSW
//     FILE:         CampaignScreen.cpp
//     AUTHOR:       Carlos Bott */

#include "CampaignScreen.h"

#include "TimerSubsystem.h"
#include "SSWGameInstance.h"
#include "CampaignSave.h"

#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"

void UCampaignScreen::NativeConstruct()
{
	Super::NativeConstruct();

	USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance());
	if (!GI)
	{
		return;
	}

	// Load player profile / selection
	GI->LoadGame(GI->PlayerSaveName, GI->PlayerSaveSlot);

	if (TitleText)
	{
		TitleText->SetText(FText::FromString("Dynamic Campaigns").ToUpper());
	}

	// Buttons
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UCampaignScreen::OnCancelButtonClicked);
		CancelButton->OnHovered.AddDynamic(this, &UCampaignScreen::OnCancelButtonHovered);
		CancelButton->OnUnhovered.AddDynamic(this, &UCampaignScreen::OnCancelButtonUnHovered);

		if (CancelButtonText)
		{
			CancelButtonText->SetText(FText::FromString("CANCEL"));
		}
	}

	if (PlayButton)
	{
		PlayButton->OnClicked.AddDynamic(this, &UCampaignScreen::OnPlayButtonClicked);
		PlayButton->OnHovered.AddDynamic(this, &UCampaignScreen::OnPlayButtonHovered);
		PlayButton->OnUnhovered.AddDynamic(this, &UCampaignScreen::OnPlayButtonUnHovered);

		if (PlayButtonText)
		{
			// Will be updated by UpdateCampaignButtons()
			PlayButtonText->SetText(FText::FromString("START"));
		}
	}

	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UCampaignScreen::OnRestartButtonClicked);
		RestartButton->OnHovered.AddDynamic(this, &UCampaignScreen::OnRestartButtonHovered);
		RestartButton->OnUnhovered.AddDynamic(this, &UCampaignScreen::OnRestartButtonUnHovered);

		if (RestartButtonText)
		{
			RestartButtonText->SetText(FText::FromString("RESTART"));
		}
	}

	// Dropdown
	if (CampaignSelectDD)
	{
		CampaignSelectDD->OnSelectionChanged.AddDynamic(this, &UCampaignScreen::OnSetSelected);
	}

	// Player name
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(GI->PlayerInfo.Name));
		UE_LOG(LogTemp, Log, TEXT("Player Name: %s"), *GI->PlayerInfo.Name);
	}

	// Build dropdown options
	SetCampaignDDList();

	// Restore selection: PlayerInfo.Campaign is ALWAYS 1-based campaign index
	int32 SelectedOptionIndex = 0;
	if (GI->PlayerInfo.Campaign > 0)
	{
		const int32 Found = CampaignIndexByOptionIndex.IndexOfByKey(GI->PlayerInfo.Campaign);
		if (Found != INDEX_NONE)
		{
			SelectedOptionIndex = Found;
		}
	}

	Selected = SelectedOptionIndex;

	PickedRowName = CampaignRowNamesByOptionIndex.IsValidIndex(Selected)
		? CampaignRowNamesByOptionIndex[Selected]
		: NAME_None;

	if (CampaignSelectDD)
	{
		// Programmatic selection; OnSetSelected ignores Direct
		CampaignSelectDD->SetSelectedIndex(SelectedOptionIndex);
	}

	// Update right panel and buttons
	SetSelectedData(Selected);
	UpdateCampaignButtons();
}

UTexture2D* UCampaignScreen::LoadTextureFromFile()
{
	USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance());
	if (!GI)
	{
		return nullptr;
	}

	return GI->LoadPNGTextureFromFile(ImagePath);
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

	// Resolve stable campaign index (1-based) from dropdown selection
	const int32 CampaignIndex1Based =
		CampaignIndexByOptionIndex.IsValidIndex(Selected) ? CampaignIndexByOptionIndex[Selected] : (Selected + 1);

	// Build selection metadata FIRST (authoritative)
	GI->SelectedCampaignDisplayName = CampaignSelectDD ? CampaignSelectDD->GetSelectedOption() : TEXT("");
	GI->SelectedCampaignIndex = CampaignIndex1Based;                 // 1-based
	GI->SelectedCampaignRowName = PickedRowName;

	// Persist selection: PlayerInfo.Campaign is 1-based stable id
	GI->PlayerInfo.Campaign = CampaignIndex1Based;
	GI->SaveGame(GI->PlayerSaveName, GI->PlayerSaveSlot, GI->PlayerInfo);

	// Check save existence using RowName slot convention
	const bool bHasSave = DoesSelectedCampaignSaveExist();

	// Load/create save
	if (bHasSave)
	{
		GI->LoadOrCreateSelectedCampaignSave();
	}
	else
	{
		GI->CreateNewCampaignSave(
			GI->SelectedCampaignIndex,
			GI->SelectedCampaignRowName,
			GI->SelectedCampaignDisplayName
		);
	}

	// Point timer at active save
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Timer->SetCampaignSave(GI->CampaignSave);

		// Only reset time for NEW campaign
		if (!bHasSave)
		{
			Timer->RestartCampaignClock(true);
		}
	}

	GI->ShowCampaignLoading();
}

void UCampaignScreen::OnRestartButtonClicked()
{
	PlayUISound(this, AcceptSound);

	USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance());
	if (!GI)
		return;

	if (PickedRowName.IsNone())
		return;

	// Resolve stable campaign index (1-based)
	const int32 CampaignIndex1Based =
		CampaignIndexByOptionIndex.IsValidIndex(Selected) ? CampaignIndexByOptionIndex[Selected] : (Selected + 1);

	// Authoritative selection metadata
	GI->SelectedCampaignDisplayName = CampaignSelectDD ? CampaignSelectDD->GetSelectedOption() : TEXT("");
	GI->SelectedCampaignIndex = CampaignIndex1Based;
	GI->SelectedCampaignRowName = PickedRowName;

	// Persist selection (1-based)
	GI->PlayerInfo.Campaign = CampaignIndex1Based;
	GI->SaveGame(GI->PlayerSaveName, GI->PlayerSaveSlot, GI->PlayerInfo);

	// Overwrite/create save FIRST
	GI->CreateNewCampaignSave(
		GI->SelectedCampaignIndex,
		GI->SelectedCampaignRowName,
		GI->SelectedCampaignDisplayName
	);

	// Restart campaign clock on the new save
	if (UTimerSubsystem* Timer = GetGameInstance()->GetSubsystem<UTimerSubsystem>())
	{
		Timer->SetCampaignSave(GI->CampaignSave);
		Timer->RestartCampaignClock(true);
	}

	// Restart campaign runtime

	GI->ShowCampaignLoading();
}

void UCampaignScreen::OnPlayButtonHovered()
{
	PlayUISound(this, HoverSound);
}

void UCampaignScreen::OnPlayButtonUnHovered()
{
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

	if (USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance()))
	{
		GI->ToggleCampaignScreen(false);
	}
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
		const FS_Campaign* Row = GI->CampaignDataTable->FindRow<FS_Campaign>(RowName, TEXT("CampaignScreen"));
		if (!Row || !Row->bAvailable)
			continue;

		CampaignSelectDD->AddOption(Row->Name);

		// Parallel arrays keyed by dropdown option index
		CampaignRowNamesByOptionIndex.Add(RowName);
		CampaignIndexByOptionIndex.Add(Row->Index + 1); // store 1-based stable campaign index
	}

	// NO selection here. NativeConstruct restores selection.
}

void UCampaignScreen::SetSelectedData(int32 OptionIndex)
{
	USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance());
	if (!GI)
		return;

	// OptionIndex is dropdown option index (0-based)
	Selected = OptionIndex;

	if (!GI->CampaignData.IsValidIndex(Selected))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetSelectedData: CampaignData invalid index %d (num=%d)"),
			Selected, GI->CampaignData.Num());
		return;
	}

	// Ensure Orders has 4 entries for UI
	GI->CampaignData[Selected].Orders.SetNum(4);

	GetCampaignImageFile(Selected);

	if (UTexture2D* LoadedTexture = LoadTextureFromFile())
	{
		if (CampaignImage)
		{
			const FSlateBrush Brush = CreateBrushFromTexture(
				LoadedTexture,
				FVector2D(LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY())
			);
			CampaignImage->SetBrush(Brush);
		}
	}

	if (CampaignNameText)
		CampaignNameText->SetText(FText::FromString(GI->CampaignData[Selected].Name));

	if (CampaignStartTimeText)
		CampaignStartTimeText->SetText(FText::FromString(GI->CampaignData[Selected].Start));

	if (DescriptionText)
		DescriptionText->SetText(FText::FromString(GI->CampaignData[Selected].Description));

	if (SituationText)
		SituationText->SetText(FText::FromString(GI->CampaignData[Selected].Situation));

	if (Orders1Text)
		Orders1Text->SetText(FText::FromString(GI->CampaignData[Selected].Orders[0]));

	if (Orders2Text)
		Orders2Text->SetText(FText::FromString(GI->CampaignData[Selected].Orders[1]));

	if (Orders3Text)
		Orders3Text->SetText(FText::FromString(GI->CampaignData[Selected].Orders[2]));

	if (Orders4Text)
		Orders4Text->SetText(FText::FromString(GI->CampaignData[Selected].Orders[3]));

	if (LocationSystemText)
	{
		const FString LocationText = GI->CampaignData[Selected].System + TEXT("/") + GI->CampaignData[Selected].Region;
		LocationSystemText->SetText(FText::FromString(LocationText));
	}

	// Update RowName for save-slot lookups
	PickedRowName = CampaignRowNamesByOptionIndex.IsValidIndex(Selected)
		? CampaignRowNamesByOptionIndex[Selected]
		: NAME_None;

	// Persist stable campaign selection (1-based) so opening the screen restores correctly
	const int32 CampaignIndex1Based =
		CampaignIndexByOptionIndex.IsValidIndex(Selected) ? CampaignIndexByOptionIndex[Selected] : (Selected + 1);

	GI->PlayerInfo.Campaign = CampaignIndex1Based;
	GI->SaveGame(GI->PlayerSaveName, GI->PlayerSaveSlot, GI->PlayerInfo);
}

void UCampaignScreen::OnSetSelected(FString SelectedItem, ESelectInfo::Type Type)
{
	// Ignore programmatic SetSelectedIndex calls
	if (Type == ESelectInfo::Direct)
		return;

	if (!CampaignSelectDD)
		return;

	const int32 NewIndex = CampaignSelectDD->FindOptionIndex(SelectedItem);
	if (NewIndex == INDEX_NONE)
		return;

	Selected = NewIndex;

	PickedRowName = CampaignRowNamesByOptionIndex.IsValidIndex(NewIndex)
		? CampaignRowNamesByOptionIndex[NewIndex]
		: NAME_None;

	SetSelectedData(NewIndex);
	UpdateCampaignButtons();
}

void UCampaignScreen::GetCampaignImageFile(int32 OptionIndex)
{
	USSWGameInstance* GI = Cast<USSWGameInstance>(GetGameInstance());
	if (!GI || !GI->CampaignData.IsValidIndex(OptionIndex))
	{
		ImagePath.Empty();
		return;
	}

	// This is UI folder convention based on dropdown ordering (legacy)
	// If you prefer folder to follow campaign index (1-based), switch to CampaignIndexByOptionIndex[OptionIndex].
	ImagePath = FPaths::ProjectContentDir() + TEXT("UI/Campaigns/0");
	ImagePath.Append(FString::FromInt(OptionIndex + 1));
	ImagePath.Append(TEXT("/"));
	ImagePath.Append(GI->CampaignData[OptionIndex].MainImage);
	ImagePath.Append(TEXT(".png"));

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

	if (PickedRowName.IsNone())
		return false;

	const FString GameSlot = UCampaignSave::MakeSlotNameFromRowName(PickedRowName);
	constexpr int32 UserIndex = 0;
	return UGameplayStatics::DoesSaveGameExist(GameSlot, UserIndex);
}

void UCampaignScreen::UpdateCampaignButtons()
{
	const bool bHasSave = DoesSelectedCampaignSaveExist();

	if (PlayButton)
	{
		PlayButton->SetIsEnabled(true);
	}

	if (PlayButtonText)
	{
		PlayButtonText->SetText(FText::FromString(bHasSave ? TEXT("CONTINUE") : TEXT("START")));
	}

	if (RestartButton)
	{
		RestartButton->SetIsEnabled(bHasSave);
	}
}
