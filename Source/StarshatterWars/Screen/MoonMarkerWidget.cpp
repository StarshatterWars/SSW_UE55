// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MoonMarkerWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "../Foundation/PlanetUtils.h"
#include "../Foundation/MoonUtils.h"
#include "../Actors/PlanetPanelActor.h"
#include "../System/SSWGameInstance.h"

void UMoonMarkerWidget::SetMoonName(const FString& InName)
{
	MoonName = InName;
	SetToolTipText(FText::FromString(InName));
	if (MoonNameText)
	{
		MoonNameText->SetText(FText::FromString(InName));
		MoonNameText->SetColorAndOpacity(FLinearColor::White);
	}
}

void UMoonMarkerWidget::SetSelected(bool bSelected)
{
	// Optional highlight logic
}

void UMoonMarkerWidget::SetMarkerMaterial(UMaterialInterface* MoonMat)
{
	MoonWidgetMaterial = MoonMat;
}

void UMoonMarkerWidget::Init(const FS_MoonMap& Moon)
{
	MoonData = Moon;

	FString IconPath = FPaths::ProjectContentDir() + TEXT("GameData/Galaxy/PlanetIcons/") + Moon.Icon + TEXT(".png");
	UTexture2D* LoadedTexture = LoadTextureFromFile(IconPath);
	if (LoadedTexture && MoonImage)
	{
		FSlateBrush Brush = CreateBrushFromTexture(LoadedTexture, FVector2D(64, 64));
		MoonImage->SetBrush(Brush);
	}
}

void UMoonMarkerWidget::InitFromMoonActor(const FS_MoonMap& Moon, AMoonPanelActor* MoonActor)
{
	MoonData = Moon;

	if (!MoonImage || !MoonActor || !MoonWidgetMaterial)
	{
		Init(Moon); // fallback
		return;
	}

	UTextureRenderTarget2D* RT = MoonActor->GetRenderTarget();
	if (!RT)
	{
		Init(Moon);
		return;
	}

	UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(MoonWidgetMaterial, this);
	DynMat->SetTextureParameterValue("InputTexture", RT);
	MoonImage->SetBrushFromMaterial(DynMat);
	float SizePx = MoonUtils::GetUISizeFromRadius(Moon.Radius) / 2;

	// Set brush and visual size
	MoonImage->SetBrushSize(FVector2D(SizePx, SizePx));
	// Optional: resize based on radius

	if (UCanvasPanelSlot* ImageSlot = Cast<UCanvasPanelSlot>(MoonImage->Slot))
	{
		ImageSlot->SetSize(FVector2D(SizePx, SizePx));
	}
}

UTexture2D* UMoonMarkerWidget::LoadTextureFromFile(FString Path)
{
	if (USSWGameInstance* SSW = GetGameInstance<USSWGameInstance>())
	{
		return SSW->LoadPNGTextureFromFile(Path);
	}
	return nullptr;
}

FSlateBrush UMoonMarkerWidget::CreateBrushFromTexture(UTexture2D* Texture, FVector2D ImageSize)
{
	FSlateBrush Brush;
	Brush.SetResourceObject(Texture);
	Brush.ImageSize = ImageSize;
	Brush.DrawAs = ESlateBrushDrawType::Image;
	return Brush;
}

FReply UMoonMarkerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnMoonClicked.Broadcast(MoonName);
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}


