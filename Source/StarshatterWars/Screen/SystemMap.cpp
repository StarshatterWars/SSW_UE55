// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemMap.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "PlanetMarkerWidget.h"
#include "SystemOrbitWidget.h"
#include "CentralSunWidget.h"
#include "OperationsScreen.h"
#include "../Game/GameStructs.h" // FS_Galaxy struct
#include "../System/SSWGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "../Game/GalaxyManager.h"
#include "../Actors/CentralSunActor.h"
#include "../Foundation/PlanetOrbitUtils.h"
#include "TimerManager.h"
#include "EngineUtils.h" 

void USystemMap::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	PanelSize = FVector2D(1450.f, 640.f);

	USSWGameInstance* SSWInstance = (USSWGameInstance*)GetGameInstance();

	ScreenOffset.X = 700;
	ScreenOffset.Y = 300;

	const FString& SelectedSystem = SSWInstance->SelectedSystem;
	const FS_Galaxy* System = UGalaxyManager::Get(this)->FindSystemByName(SelectedSystem);
	if (System)
	{
		UE_LOG(LogTemp, Log, TEXT("USystemMap::NativeConstruct() System Found: %s"), *SelectedSystem);
		BuildSystemView(System);
	}
}
void USystemMap::NativeDestruct()
{
	Super::NativeDestruct();
}

float USystemMap::GetDynamicOrbitScale(const TArray<FS_PlanetMap>& Planets, float MaxPixelRadius) const
{
	float MaxOrbit = 0.f;
	for (const FS_PlanetMap& Planet : Planets)
	{
		MaxOrbit = FMath::Max(MaxOrbit, Planet.Orbit);
	}

	if (MaxOrbit <= 0.f)
	{
		return 1.0f; // fallback to avoid divide-by-zero
	}

	return MaxOrbit / MaxPixelRadius; // e.g. 1e9 km mapped to 500 px = 2e6 scale
}

void USystemMap::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{

}


void USystemMap::BuildSystemView(const FS_Galaxy* ActiveSystem)
{
	if (!MapCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("USystemMap::BuildSystemView(): Missing MapCanvas"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("USystemMap::BuildSystemView(): System: %s has %d planets"), *ActiveSystem->Name, ActiveSystem->Stellar[0].Planet.Num());

	UE_LOG(LogTemp, Warning, TEXT("USystemMap::BuildSystemView(): Stellar Classification: %u"), static_cast<uint8>(ActiveSystem->Stellar[0].Class));

	MapCanvas->ClearChildren();
	PlanetMarkers.Empty();

	const FVector2D Center(960.f, 540.f); // Screen center
	
	if (SunActorClass)
	{
		if (SunActor)
		{
			SunActor->Destroy();
			SunActor = nullptr;
		}

		// 2. Destroy previous widget (if it exists)
		if (StarWidget)
		{
			StarWidget->RemoveFromParent();
			StarWidget = nullptr;
		}

		// Pull the stellar radius from the active system
		const float StellarRadius = ActiveSystem->Stellar[0].Radius;

		FVector Location = FVector(-500, 0, 200);
		FRotator Rotation = FRotator::ZeroRotator;

		SunActor = ACentralSunActor::SpawnWithSpectralClass(
			GetWorld(),
			Location,
			Rotation,
			SunActorClass,
			ActiveSystem->Stellar[0].Class,     // Spectral class
			ActiveSystem->Stellar[0].Radius     // Pass in radius from Galaxy.def
		);
	}

	// Add central star
	if (StarWidgetClass)
	{
		StarWidget = CreateWidget<UCentralSunWidget>(this, StarWidgetClass);
		// Bind the click delegate to handle galaxy map return
		StarWidget->OnSunClicked.AddDynamic(this, &USystemMap::HandleCentralSunClicked);

		if (StarWidget && SunActor)
		{
			StarWidget->InitializeFromSunActor(SunActor);
			if (UCanvasPanelSlot* StarSlot = MapCanvas->AddChildToCanvas(StarWidget))
			{
				StarSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				StarSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				StarSlot->SetPosition(FVector2D(32.f, 0.f));
				StarSlot->SetZOrder(15);
			}
		}
	}

	// Build planet markers and orbit rings
	for (const FS_PlanetMap& Planet : ActiveSystem->Stellar[0].Planet)
	{
		float PanelWidth = MapCanvas->GetCachedGeometry().GetLocalSize().X;
		float PanelHeight = MapCanvas->GetCachedGeometry().GetLocalSize().Y;
		
		float Perihelion = 0.f;
		float Aphelion = 0.f;

		PlanetOrbitUtils::CalculateOrbitExtremes(Planet.Orbit, Planet.Eccentricity, Perihelion, Aphelion);

		// Get ellipse axes
		float a = (Perihelion + Aphelion) * 0.5f; // semi-major
		float b = a * FMath::Sqrt(1.0f - FMath::Square(Planet.Eccentricity)); // semi-minor

		// Convert to screen scale
		float ORBIT_TO_SCREEN = GetDynamicOrbitScale(ActiveSystem->Stellar[0].Planet, 480.f);
		a /= ORBIT_TO_SCREEN;
		b /= ORBIT_TO_SCREEN;

		float Radius = Planet.Orbit / ORBIT_TO_SCREEN;

		Radius = PlanetOrbitUtils::FitOrbitRadiusToPanel(
			Radius,
			Planet.Inclination,
			PanelSize.X,  // Fixed panel width
			PanelSize.Y,  // Fixed panel height
			50.f     // Optional padding (tweakable)
		);

		UE_LOG(LogTemp, Warning, TEXT("Planet %s -> Orbit radius: %f"), *Planet.Name, Radius);

		if (OrbitWidgetClass)
		{
			auto* Orbit = CreateWidget<USystemOrbitWidget>(this, OrbitWidgetClass);
	
			//OrbitCenter += Center;
			// Apply visual configuration
			Orbit->SetOrbitRadius(Radius);
			const float InclinationVisual = PlanetOrbitUtils::AmplifyInclination(Planet.Inclination);
			Orbit->SetOrbitInclination(InclinationVisual);

			// Position orbit ring widget
			if (UCanvasPanelSlot* OrbitSlot = MapCanvas->AddChildToCanvas(Orbit))
			{
				OrbitSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				OrbitSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				OrbitSlot->SetPosition(FVector2D(0.f, 0.f)); // use panel center
				OrbitSlot->SetZOrder(0);
			}

			Orbit->SetVisibility(ESlateVisibility::Visible);
		}

		// Planet marker
		if (PlanetMarkerClass)
		{
			auto* Marker = CreateWidget<UPlanetMarkerWidget>(this, PlanetMarkerClass);
			if (Marker)
			{
				float& OrbitAngle = PlanetOrbitAngles.FindOrAdd(Planet.Name);
				if (OrbitAngle == 0.0f)
				{
					OrbitAngle = FMath::FRandRange(0.0f, 360.0f);
				}
				
				float VisualInclination = PlanetOrbitUtils::AmplifyInclination(Planet.Inclination, 2.0f);

				FVector2D TiltedPos = PlanetOrbitUtils::Get2DOrbitPositionWithInclination(
					Radius,
					OrbitAngle,
					VisualInclination
				);

				if (UCanvasPanelSlot* PlanetSlot = MapCanvas->AddChildToCanvas(Marker))
				{
					PlanetSlot->SetAnchors(FAnchors(0.5f, 0.5f));
					PlanetSlot->SetAlignment(FVector2D(0.5f, 0.5f));
					PlanetSlot->SetPosition(TiltedPos);
					PlanetSlot->SetAutoSize(true);
					PlanetSlot->SetZOrder(10);
				}
				Marker->SetVisibility(ESlateVisibility::Visible);
				Marker->SetPlanetName(Planet.Name);
				Marker->Init(Planet);

				PlanetMarkers.Add(Planet.Name, Marker);
				
			}
		}
	}

	// Highlight current system
	if (const USSWGameInstance* GI = GetWorld()->GetGameInstance<USSWGameInstance>())
	{
		const FString& CurrentSystem = GI->SelectedSystem;

		if (UPlanetMarkerWidget** Found = PlanetMarkers.Find(CurrentSystem))
		{
			UPlanetMarkerWidget* Marker = *Found;
			Marker->SetSelected(true);

			FTimerHandle Delay;
			GetWorld()->GetTimerManager().SetTimer(Delay, FTimerDelegate::CreateWeakLambda(this, [this, Marker]()
				{
					if (!Marker) return;

					FVector2D Center;
					if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Marker->Slot))
					{
						Center = Slot->GetPosition() + Slot->GetSize() * Slot->GetAlignment();
					}

					// Optionally center camera or draw selection lines here
					UE_LOG(LogTemp, Log, TEXT("Selected system: %s at %s"), *Marker->GetPlanetName(), *Center.ToString());

				}), 0.01f, false);
		}
	}
}

void USystemMap::HandleCentralSunClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Central sun clicked — requesting galaxy map view"));

	if (OwningOperationsScreen)
	{
		OwningOperationsScreen->ShowGalaxyMap(); // or equivalent
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No owner set for USystemMap"));
	}
}

/*void USystemMap::BuildSystemView(const FS_Galaxy* ActiveSystem)
{
	if (!MapCanvas) return;

	MapCanvas->ClearChildren();
	PlanetMarkers.Empty();
	const FVector2D Center(960.f, 540.f);

	if (SunActorClass)
	{
		if (SunActor) { SunActor->Destroy(); SunActor = nullptr; }
		if (StarWidget) { StarWidget->RemoveFromParent(); StarWidget = nullptr; }

		SunActor = ACentralSunActor::SpawnWithSpectralClass(
			GetWorld(), FVector(-500, 0, 200), FRotator::ZeroRotator, SunActorClass, ActiveSystem->Class
		);
	}

	if (StarWidgetClass && SunActor)
	{
		StarWidget = CreateWidget<UCentralSunWidget>(this, StarWidgetClass);
		StarWidget->InitializeFromSunActor(SunActor);
		if (auto* StarSlot = MapCanvas->AddChildToCanvas(StarWidget))
		{
			StarSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			StarSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			StarSlot->SetPosition(FVector2D(32.f, 0.f));
			StarSlot->SetZOrder(15);
		}
	}

	for (const FS_PlanetMap& Planet : ActiveSystem->Stellar[0].Planet)
	{
		float Perihelion = 0, Aphelion = 0;
		PlanetOrbitUtils::CalculateOrbitExtremes(Planet.Orbit, Planet.Eccentricity, Perihelion, Aphelion);

		float a = (Perihelion + Aphelion) * 0.5f;
		float b = a * FMath::Sqrt(1.0f - FMath::Square(Planet.Eccentricity));
		float ORBIT_TO_SCREEN = GetDynamicOrbitScale(ActiveSystem->Stellar[0].Planet, 480.f);
		a /= ORBIT_TO_SCREEN;
		b /= ORBIT_TO_SCREEN;

		// Orbit ring
		if (OrbitWidgetClass)
		{
			auto* Orbit = CreateWidget<USystemOrbitWidget>(this, OrbitWidgetClass);
			Orbit->SetOrbitAxes(a, b);
			Orbit->SetOrbitInclination(Planet.Inclination);

			if (auto* OrbitSlot = MapCanvas->AddChildToCanvas(Orbit))
			{
				OrbitSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				OrbitSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				OrbitSlot->SetPosition(FVector2D(0.f, 0.f));
				OrbitSlot->SetZOrder(0);
			}
		}

		// Planet marker
		if (PlanetMarkerClass)
		{
			auto* Marker = CreateWidget<UPlanetMarkerWidget>(this, PlanetMarkerClass);
			if (Marker)
			{
				float& OrbitAngle = PlanetOrbitAngles.FindOrAdd(Planet.Name);
				if (OrbitAngle == 0.f) OrbitAngle = FMath::FRandRange(0.0f, 360.0f);

				// Orbit position with inclination applied as rotation
				float AngleRad = FMath::DegreesToRadians(OrbitAngle);
				FVector2D EllipticalPos = FVector2D(a * FMath::Cos(AngleRad), b * FMath::Sin(AngleRad));
				FVector2D Rotated = EllipticalPos.GetRotated(Planet.Inclination);
				FVector2D Final = Center + Rotated;

				if (auto* PlanetSlot = MapCanvas->AddChildToCanvas(Marker))
				{
					PlanetSlot->SetAnchors(FAnchors(0.5f, 0.5f));
					PlanetSlot->SetAlignment(FVector2D(0.5f, 0.5f));
					PlanetSlot->SetPosition(Final);
					PlanetSlot->SetAutoSize(true);
					PlanetSlot->SetZOrder(10);
				}
				Marker->SetVisibility(ESlateVisibility::Visible);
				Marker->SetPlanetName(Planet.Name);
				Marker->Init(Planet);
				PlanetMarkers.Add(Planet.Name, Marker);
			}
		}
	}
}*/

