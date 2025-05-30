// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlanetMarkerWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "../Foundation/PlanetUtils.h"
#include "../Actors/PlanetPanelActor.h"
#include "../System/SSWGameInstance.h"

void UPlanetMarkerWidget::SetPlanetName(const FString& InName)
{
	SetToolTipText(FText::FromString(InName));

	if (PlanetNameText)
	{
		PlanetNameText->SetText(FText::FromString(InName));
		PlanetNameText->SetColorAndOpacity(FLinearColor::White);
	}
}

void UPlanetMarkerWidget::SetSelected(bool bSelected)
{
	// Optional highlight logic
}

void UPlanetMarkerWidget::SetMarkerMaterial(UMaterialInterface* PlanetMat)
{
	PlanetWidgetMaterial = PlanetMat;
}

void UPlanetMarkerWidget::Init(const FS_PlanetMap& Planet)
{
	PlanetData = Planet;

	FString IconPath = FPaths::ProjectContentDir() + TEXT("GameData/Galaxy/PlanetIcons/") + Planet.Icon + TEXT(".png");
	UTexture2D* LoadedTexture = LoadTextureFromFile(IconPath);
	if (LoadedTexture && PlanetImage)
	{
		FSlateBrush Brush = CreateBrushFromTexture(LoadedTexture, FVector2D(64, 64));
		PlanetImage->SetBrush(Brush);
	}
}

void UPlanetMarkerWidget::InitFromPlanetActor(const FS_PlanetMap& Planet, APlanetPanelActor* PlanetActor)
{
	PlanetData = Planet;

	if (!PlanetImage || !PlanetActor || !PlanetWidgetMaterial)
	{
		Init(Planet); // fallback
		return;
	}

	UTextureRenderTarget2D* RT = PlanetActor->GetRenderTarget();
	if (!RT)
	{
		Init(Planet);
		return;
	}

	UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(PlanetWidgetMaterial, this);
	DynMat->SetTextureParameterValue("InputTexture", RT);
	PlanetImage->SetBrushFromMaterial(DynMat);
	float SizePx = PlanetUtils::GetUISizeFromRadius(Planet.Radius)/2;
	
	// Set brush and visual size
	PlanetImage->SetBrushSize(FVector2D(SizePx, SizePx));
	// Optional: resize based on radius
	
	if (UCanvasPanelSlot* ImageSlot = Cast<UCanvasPanelSlot>(PlanetImage->Slot))
	{
		ImageSlot->SetSize(FVector2D(SizePx, SizePx));
	}
}

UTexture2D* UPlanetMarkerWidget::LoadTextureFromFile(FString Path)
{
	if (USSWGameInstance* SSW = GetGameInstance<USSWGameInstance>())
	{
		return SSW->LoadPNGTextureFromFile(Path);
	}
	return nullptr;
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
		OnPlanetClicked.Broadcast(PlanetName);
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
