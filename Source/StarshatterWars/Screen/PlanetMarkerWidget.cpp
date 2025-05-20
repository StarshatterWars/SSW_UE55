// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlanetMarkerWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UPlanetMarkerWidget::SetPlanetName(const FString& InName)
{
	PlanetName = InName;
	SetToolTipText(FText::FromString(InName));
}

void UPlanetMarkerWidget::SetSelected(bool bSelected)
{
	if (HighlightBorder)
	{
		if (bSelected) 
			HighlightBorder->SetVisibility(ESlateVisibility::Visible);
		else 
			HighlightBorder->SetVisibility(ESlateVisibility::Hidden);
	}
}


void UPlanetMarkerWidget::Init(const FS_PlanetMap& Planet)
{
	UE_LOG(LogTemp, Log, TEXT("UPlanetMarkerWidget::Init() Creating Widget: %s"), *Planet.Name);

	PlanetData = Planet;

	SetToolTipText(FText::FromString(Planet.Name));

	PlanetName = Planet.Name;

	if (PlanetNameText) {
		PlanetNameText->SetText(FText::FromString(Planet.Name));
		PlanetNameText->SetColorAndOpacity(FLinearColor::White);
	}

	FString ImagePath = FPaths::ProjectContentDir();
	ImagePath.Append(TEXT("GameData/Galaxy/PlanetIcons/"));
	ImagePath.Append(PlanetData.Icon);
	ImagePath.Append(".png");

	UTexture2D* LoadedTexture = LoadTextureFromFile(ImagePath);
	if (LoadedTexture)
	{
		UE_LOG(LogTemp, Log, TEXT("UPlanetMarkerWidget::Init() Creating Image: %s"), *ImagePath);

		FSlateBrush Brush = CreateBrushFromTexture(LoadedTexture, FVector2D(LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY()));

		if (!PlanetImage) {
			UE_LOG(LogTemp, Log, TEXT("UPlanetMarkerWidget::Init() PlanetImage Not Found"));
		}
		else {
			PlanetImage->SetBrush(Brush);
		}

		SetSelected(false);
	}
}

UTexture2D* UPlanetMarkerWidget::LoadTextureFromFile(FString Path)
{
	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();
	UTexture2D* LoadedTexture = SSWInstance->LoadPNGTextureFromFile(Path);
	return LoadedTexture;
}

FSlateBrush UPlanetMarkerWidget::CreateBrushFromTexture(UTexture2D* Texture, FVector2D ImageSize)
{
	FSlateBrush Brush;
	Brush.SetResourceObject(Texture);
	Brush.ImageSize = ImageSize;
	Brush.DrawAs = ESlateBrushDrawType::Image;
	return Brush;
}

FReply UPlanetMarkerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnClicked.ExecuteIfBound(PlanetData.Name);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

