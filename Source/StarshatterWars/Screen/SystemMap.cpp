// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemMap.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "PlanetMarkerWidget.h"
#include "SystemOrbitWidget.h"
#include "CentralSunWidget.h"
#include "OperationsScreen.h"
#include "../Game/GameStructs.h" // FS_Galaxy struct
#include "../System/SSWGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "../Game/GalaxyManager.h"
#include "../Actors/CentralSunActor.h"
#include "../Actors/PlanetPanelActor.h"
#include "../Foundation/PlanetOrbitUtils.h"
#include "../Foundation/SystemMapUtils.h"
#include "TimerManager.h"
#include "EngineUtils.h" 


void USystemMap::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Visible);
	SetIsEnabled(true);
	InitialMapCanvasOffset = FVector2D(50.f, -200.f);

	ZoomLevel = 1.0f;

	// Canvas transform setup
	if (MapCanvas)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetPosition(InitialMapCanvasOffset);
		}

		// Apply zoom
		FWidgetTransform ZoomTransform;
		ZoomTransform.Scale = FVector2D(ZoomLevel, ZoomLevel);
		MapCanvas->SetRenderTransform(ZoomTransform);

		// Add dummy to force layout size
		UImage* DummyImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());

		FSlateBrush DummyBrush;
		DummyBrush.ImageSize = FVector2D(1920.f, 1080.f);
		DummyBrush.TintColor = FLinearColor::Transparent;
		DummyImage->SetBrush(DummyBrush);
		DummyImage->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (UCanvasPanelSlot* DummySlot = MapCanvas->AddChildToCanvas(DummyImage))
		{
			DummySlot->SetAutoSize(false);
			DummySlot->SetPosition(FVector2D(0.f, 0.f));
		}
	}

	// Delay layout-dependent logic like centering
	FTimerHandle LayoutTimer;
	GetWorld()->GetTimerManager().SetTimer(LayoutTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (!MapCanvas) return;

			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
			{
				CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				CanvasSlot->SetPosition(InitialMapCanvasOffset);
			}
		}), 0.05f, false);

	// Optional: build the system after everything is ready
	if (USSWGameInstance* GI = GetGameInstance<USSWGameInstance>())
	{
		const FString& SelectedSystem = GI->SelectedSystem;
		const FS_Galaxy* System = UGalaxyManager::Get(this)->FindSystemByName(SelectedSystem);

		if (System)
		{
			UE_LOG(LogTemp, Log, TEXT("USystemMap::NativeConstruct() Found system: %s"), *SelectedSystem);
			BuildSystemView(System);
		}
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
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!MapCanvas)
		return;

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot);
	if (!CanvasSlot)
		return;

	// ----- PHASE 1: Canvas Movement -----
	if (bIsCenteringToPlanet)
	{
		CenterAnimTime += InDeltaTime;
		float Progress = FMath::Clamp(CenterAnimTime / CenterAnimDuration, 0.0f, 1.0f);
		float SmoothT = SystemMapUtils::EaseInOut(Progress);

		FVector2D Pos = FMath::Lerp(StartCanvasPosition, TargetCanvasPosition, SmoothT);
		CanvasSlot->SetPosition(Pos);

		if (Progress >= 1.0f)
		{
			CanvasSlot->SetPosition(TargetCanvasPosition);
			bIsCenteringToPlanet = false;

			// Begin zoom phase
			ZoomAnimTime = 0.0f;
			bIsZoomingToPlanet = true;

			UE_LOG(LogTemp, Log, TEXT("[CENTER COMPLETE] Now zooming..."));
		}

		return;
	}

	// ----- PHASE 2: Zoom In/Out -----
	if (bIsZoomingToPlanet)
	{
		ZoomAnimTime += InDeltaTime;
		float Progress = FMath::Clamp(ZoomAnimTime / ZoomAnimDuration, 0.0f, 1.0f);
		float SmoothT = SystemMapUtils::EaseInOut(Progress);

		ZoomLevel = FMath::Lerp(StartZoomLevel, TargetZoomLevel, SmoothT);

		FWidgetTransform Transform;
		Transform.Scale = FVector2D(ZoomLevel, ZoomLevel);
		MapCanvas->SetRenderTransform(Transform);

		if (Progress >= 1.0f)
		{
			ZoomLevel = TargetZoomLevel;
			bIsZoomingToPlanet = false;

			Transform.Scale = FVector2D(ZoomLevel, ZoomLevel);
			MapCanvas->SetRenderTransform(Transform);

			UE_LOG(LogTemp, Log, TEXT("[ZOOM COMPLETE] Final Zoom = %.2f"), ZoomLevel);
		}
	}
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
	AddLayoutExtender();

	//const FVector2D Center(960.f, 540.f); // Screen center
	const FVector2D Center = MapCanvas->GetCachedGeometry().GetLocalSize() * 0.5f;
	
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
			ActiveSystem->Stellar[0].Radius,    // Pass in radius from Galaxy.def
			ActiveSystem->Stellar[0].Name
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
				StarSlot->SetAutoSize(true);
				StarSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				StarSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				StarSlot->SetPosition(FVector2D(0.f, 0.f));
				StarSlot->SetZOrder(15);
			}
		}
	}

	// Build planet markers and orbit rings
	for (const FS_PlanetMap& Planet : ActiveSystem->Stellar[0].Planet)
	{
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
				OrbitSlot->SetAutoSize(true);
			}

			Orbit->SetVisibility(ESlateVisibility::Visible);
		}
		if (PlanetActorClass)
		{
			if (PlanetActor)
			{
				PlanetActor->Destroy();
				PlanetActor = nullptr;
			}

			FVector ActorLocation = FVector(-1000, 0, 200);  // Adjust position as needed
			FRotator ActorRotation = FRotator::ZeroRotator;

			PlanetActor = APlanetPanelActor::SpawnWithPlanetData(
				GetWorld(),
				ActorLocation,
				ActorRotation,
				PlanetActorClass,
				Planet
			);

			if (PlanetActor)
			{
				SpawnedPlanetActors.Add(PlanetActor);
			}	
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
				//Marker->Init(Planet);
				Marker->InitFromPlanetActor(Planet, PlanetActor); // pass data and actor
				Marker->OnPlanetClicked.AddDynamic(this, &USystemMap::HandlePlanetClicked);

				PlanetMarkers.Add(Planet.Name, Marker);
				
			}
		}
		
		FTimerHandle CaptureDelay;
		GetWorld()->GetTimerManager().SetTimer(CaptureDelay, FTimerDelegate::CreateWeakLambda(this, [this]()
	 {
				for (APlanetPanelActor* Actor : SpawnedPlanetActors)
				{
					if (Actor)
					{
						Actor->AssignScreenCapture();
					}
				}
				SpawnedPlanetActors.Empty();
			}), 0.05f, false); // Adjust delay as needed (50ms)
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
	
	FTimerHandle LayoutCheck;
	GetWorld()->GetTimerManager().SetTimer(LayoutCheck, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (MapCanvas)
			{
				FVector2D Size = MapCanvas->GetDesiredSize();
				UE_LOG(LogTemp, Warning, TEXT("MapCanvas DesiredSize: %s"), *Size.ToString());
			}
		}), 0.05f, false);
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

void USystemMap::AddLayoutExtender() {
	UImage* DummyImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	// Force the image to take space in layout
	FSlateBrush DummyBrush;
	DummyBrush.ImageSize = FVector2D(3000.f, 3000.f);  // Force size
	DummyBrush.TintColor = FLinearColor::Transparent; // Fully invisible
	DummyImage->SetBrush(DummyBrush);
	DummyImage->SetVisibility(ESlateVisibility::HitTestInvisible);

	if (UCanvasPanelSlot* DummySlot = MapCanvas->AddChildToCanvas(DummyImage))
	{
		DummySlot->SetAutoSize(true);                     // Size comes from brush
		DummySlot->SetPosition(FVector2D(0.f, 0.f));        // Position doesn't matter
	}
}

void USystemMap::SetZoomLevel(float NewZoom)
{
	ZoomLevel = FMath::Clamp(NewZoom, 0.25f, 4.0f); // prevent too small or too big

	if (MapCanvas)
	{
		FWidgetTransform ZoomTransform;
		ZoomTransform.Scale = FVector2D(ZoomLevel, ZoomLevel);
		MapCanvas->SetRenderTransform(ZoomTransform);
	}
}

void USystemMap::ZoomToFitAllPlanets()
{

}

FReply USystemMap::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	float Delta = InMouseEvent.GetWheelDelta();
	SetZoomLevel(ZoomLevel + Delta * 0.1f);
	return FReply::Handled();
}

FReply USystemMap::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("USystemMap::MouseButtonDown triggered")); 
	
	bIsCenteringToPlanet = false;
	bIsZoomingToPlanet = false;

	// Inside MouseButtonDown, if desired:
	if (bIsCenteringToPlanet || bIsZoomingToPlanet)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
		{
			CanvasSlot->SetPosition(TargetCanvasPosition);
		}
		ZoomLevel = TargetZoomLevel;
		FWidgetTransform Transform;
		Transform.Scale = FVector2D(ZoomLevel, ZoomLevel);
		MapCanvas->SetRenderTransform(Transform);
	}

	FVector2D Size = MapCanvas->GetDesiredSize();
	FVector2D Viewport = GetCachedGeometry().GetLocalSize();

	UE_LOG(LogTemp, Warning, TEXT("ContentSize: %s, Viewport: %s"), *Size.ToString(), *Viewport.ToString());
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		DragStartPos = InMouseEvent.GetScreenSpacePosition();

		if (MapCanvas)
		{
			if (UCanvasPanelSlot* MouseSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
			{
				InitialMapCanvasOffset = MouseSlot->GetPosition();
			}
		}

		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}


FReply USystemMap::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bIsDragging || !MapCanvas)
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

	FVector2D CurrentMousePos = InMouseEvent.GetScreenSpacePosition();
	FVector2D Delta = CurrentMousePos - DragStartPos;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
	{
		FVector2D Proposed = InitialMapCanvasOffset + Delta;
		FVector2D ContentSize = MapCanvas->GetDesiredSize() * ZoomLevel;
		FVector2D ViewportSize = OuterCanvas->GetCachedGeometry().GetLocalSize();

		FVector2D Clamped = SystemMapUtils::ClampCanvasDragOffset(Proposed, ContentSize, ViewportSize);

		CanvasSlot->SetPosition(Clamped);

		UE_LOG(LogTemp, Verbose,
			TEXT("[DRAG CANVAS] Delta=%s | Proposed=%s | Clamped=%s"),
			*Delta.ToString(), *Proposed.ToString(), *Clamped.ToString());
	}

	return FReply::Handled();
}

FReply USystemMap::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("USystemMap::MouseButtonUp triggered"));

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = false;
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}


void USystemMap::HandlePlanetClicked(const FString& PlanetName)
{
	if (UPlanetMarkerWidget** Found = PlanetMarkers.Find(PlanetName))
	{
		// Optional: center or highlight
		UE_LOG(LogTemp, Log, TEXT("Planet selected: %s"), *PlanetName);
		CenterOnPlanetWidget(*Found, 1.0f);
	}
}

void USystemMap::CenterOnPlanetWidget(UPlanetMarkerWidget* Marker, float Zoom)
{
	if (!Marker || !MapCanvas)
		return;

	UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(Marker->Slot);
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot);
	if (!MarkerSlot || !CanvasSlot)
		return;

	float ClampedZoom = FMath::Clamp(Zoom, MinZoomLevel, MaxZoomLevel);

	FVector2D MarkerCenter = MarkerSlot->GetPosition() + MarkerSlot->GetSize() * MarkerSlot->GetAlignment();

	// Calculate center offset in layout space (panel-centered, 0,0 origin)
	FVector2D UnclampedOffset = -MarkerCenter + InitialMapCanvasOffset;

	// Recalculate content size using *target zoom*
	FVector2D ContentSize = MapCanvas->GetDesiredSize() * ClampedZoom;
	FVector2D ViewportSize = OuterCanvas->GetCachedGeometry().GetLocalSize();

	TargetCanvasPosition = SystemMapUtils::ClampCanvasDragOffset(
		UnclampedOffset, ContentSize, ViewportSize
	);

	StartCanvasPosition = CanvasSlot->GetPosition();

	StartZoomLevel = ZoomLevel;
	TargetZoomLevel = ClampedZoom;

	CenterAnimTime = 0.0f;
	bIsCenteringToPlanet = true;
	bIsZoomingToPlanet = false;

	UE_LOG(LogTemp, Warning,
		TEXT("[CENTER PLANET] Marker=%s | Zoom=%.2f -> %.2f | CanvasFrom=%s -> To=%s"),
		*MarkerCenter.ToString(), StartZoomLevel, TargetZoomLevel,
		*StartCanvasPosition.ToString(), *TargetCanvasPosition.ToString());
}
