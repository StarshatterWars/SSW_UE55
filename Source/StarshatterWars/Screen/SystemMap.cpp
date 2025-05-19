// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemMap.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "PlanetMarkerWidget.h"
#include "SystemOrbitWidget.h"
#include "TimerManager.h"



void USystemMap::BuildSystemView(const TArray<FS_PlanetMap>& Planets, const FString& CurrentSystemName)
{
	if (!MapCanvas) return;

	MapCanvas->ClearChildren();
	PlanetMarkers.Empty();

	const FVector2D Center(960.f, 540.f); // Screen center, adjust if needed

	for (const FS_PlanetMap& Planet : Planets)
	{
		float Radius = Planet.Orbit / KM_TO_SCREEN;

		// Draw orbit
		if (OrbitWidgetClass)
		{
			USystemOrbitWidget* Orbit = CreateWidget<USystemOrbitWidget>(this, OrbitWidgetClass);
			Orbit->SetOrbitRadius(Radius);

			if (UCanvasPanelSlot* OrbitSlot = MapCanvas->AddChildToCanvas(Orbit))
			{
				OrbitSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				OrbitSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				OrbitSlot->SetPosition(Center);
				OrbitSlot->SetZOrder(0);
			}
		}

		// Add planet marker
		if (PlanetMarkerClass)
		{
			UPlanetMarkerWidget* Marker = CreateWidget<UPlanetMarkerWidget>(this, PlanetMarkerClass);
			if (Marker)
			{
				FVector2D Position = Center + FVector2D(Radius, 0); // right side of orbit

				if (UCanvasPanelSlot* MarkerSlot = MapCanvas->AddChildToCanvas(Marker))
				{
					MarkerSlot->SetPosition(Position);
					MarkerSlot->SetAutoSize(true);
					MarkerSlot->SetZOrder(10);
				}

				Marker->SetPlanetName(Planet.Name);
				PlanetMarkers.Add(Planet.Name, Marker);
			}
		}
	}

	// Highlight current system if found
	if (UPlanetMarkerWidget** Found = PlanetMarkers.Find(CurrentSystemName))
	{
		UPlanetMarkerWidget* Marker = *Found;
		Marker->SetSelected(true);

		FTimerHandle Delay;
		GetWorld()->GetTimerManager().SetTimer(Delay, FTimerDelegate::CreateWeakLambda(this,
			[this, Marker]()
			{
				if (!Marker) return;

				FVector2D MarkerCenter = FVector2D::ZeroVector;
				if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Marker->Slot))
				{
					MarkerCenter = Slot->GetPosition() + Slot->GetSize() * Slot->GetAlignment();
				}

				// Optional: pan or highlight
				UE_LOG(LogTemp, Log, TEXT("Centered on planet: %s at %s"), *Marker->GetPlanetName(), *MarkerCenter.ToString());

			}), 0.01f, false);
	}
}

void USystemMap::NativeConstruct()
{

}

void USystemMap::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{

}
