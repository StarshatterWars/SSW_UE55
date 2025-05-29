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

	if (UCanvasPanelSlot* MainSlot = Cast<UCanvasPanelSlot>(OuterCanvas->Slot))
	{
		MainSlot->SetPosition(FVector2D::ZeroVector);
	}

	if (!MapCanvas) 
	{
		return;
	}

	InitMapCanvas();

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

	// Handle movement delay timer
	if (bIsWaitingToProcessMovement)
	{
		MovementDelayTime += InDeltaTime;

		if (MovementDelayTime >= MovementDelayDuration)
		{
			bIsWaitingToProcessMovement = false;
			MovementDelayTime = 0.0f;

			// Trigger any post-drag action here
			if (LastSelectedMarker)
			{
				CenterOnPlanetWidget(LastSelectedMarker, ZoomLevel);
				LastSelectedMarker = nullptr;
			}
		}
	}
	
	// Phase 1: Center to planet
	if (bIsCenteringToPlanet)
	{
		CenterAnimTime += InDeltaTime;
		float Progress = FMath::Clamp(CenterAnimTime / CenterAnimDuration, 0.0f, 1.0f);
		float SmoothT = SystemMapUtils::EaseInOut(Progress);

		FVector2D NewPos = FMath::Lerp(StartCanvasPosition, TargetCanvasPosition, SmoothT);
		CanvasSlot->SetPosition(NewPos);
		CurrentDragOffset = NewPos;

		if (Progress >= 1.0f)
		{
			CanvasSlot->SetPosition(TargetCanvasPosition);
			bIsCenteringToPlanet = false;
			ZoomAnimTime = 0.0f;
			bIsZoomingToPlanet = true;
		}

		SystemMapUtils::ApplyZoomAndTilt(MapCanvas, ZoomLevel, TargetTiltAmount);
		return;
	}

	// Phase 2: Zoom in/out
	if (bIsZoomingToPlanet)
	{
		ZoomAnimTime += InDeltaTime;
		float Progress = FMath::Clamp(ZoomAnimTime / ZoomAnimDuration, 0.0f, 1.0f);
		float SmoothT = SystemMapUtils::EaseInOut(Progress);

		ZoomLevel = FMath::Lerp(StartZoomLevel, TargetZoomLevel, SmoothT);

		SystemMapUtils::ApplyZoomAndTilt(MapCanvas, ZoomLevel, TargetTiltAmount);

		if (Progress >= 1.0f)
		{
			ZoomLevel = TargetZoomLevel;
			bIsZoomingToPlanet = false;

			if (LastSelectedMarker)
			{
				CenterOnPlanetWidget(LastSelectedMarker, ZoomLevel); // recenter after zoom
				//LastSelectedMarker = nullptr;
			}

			SystemMapUtils::ApplyZoomAndTilt(MapCanvas, ZoomLevel, TargetTiltAmount);
		}
	}

	// Phase 3: Tilt on load
	if (bIsTiltingIn)
	{
		TiltTime += InDeltaTime;
		float Progress = FMath::Clamp(TiltTime / TiltDuration, 0.0f, 1.0f);
		float SmoothT = SystemMapUtils::EaseInOut(Progress);

		float CurrentTilt = FMath::Lerp(0.0f, TargetTiltAmount, SmoothT);
		SystemMapUtils::ApplyZoomAndTilt(MapCanvas, ZoomLevel, CurrentTilt);

		if (Progress >= 1.0f)
		{
			bIsTiltingIn = false;
			SystemMapUtils::ApplyZoomAndTilt(MapCanvas, ZoomLevel, TargetTiltAmount);
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

	bPendingCanvasCenter = true;

	MapCanvas->ClearChildren();
	PlanetMarkers.Empty();

	const FVector2D Center = MapCanvas->GetCachedGeometry().GetLocalSize() * 0.5f;
	
	AddCentralStar(ActiveSystem);

	// Convert to screen scale
	ORBIT_TO_SCREEN = GetDynamicOrbitScale(ActiveSystem->Stellar[0].Planet, 480.f);

	// Build planet markers and orbit rings
	for (const FS_PlanetMap& Planet : ActiveSystem->Stellar[0].Planet)
	{
		AddPlanet(Planet);		
	}
		
	AssignRenderTargetsToPlanets();
	HighlightSelectedSystem();

	FTimerHandle LayoutCheck;
	GetWorld()->GetTimerManager().SetTimer(LayoutCheck, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (MapCanvas)
			{
				FVector2D Size = MapCanvas->GetDesiredSize();
				UE_LOG(LogTemp, Warning, TEXT("MapCanvas DesiredSize: %s"), *Size.ToString());
			}
		}), 0.05f, false);

	// Now that content is added and bounds are valid
	//CenterCanvasOnMapBounds(CachedViewportSize);
}

void USystemMap::HandleCentralSunClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Central sun clicked — requesting galaxy map view"));

	if (OwningOperationsScreen)
	{
		//ClearMapCanvas();
		OwningOperationsScreen->ShowGalaxyMap(); // or equivalent
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No owner set for USystemMap"));
	}
}

void USystemMap::SetZoomLevel(float NewZoom)
{
	ZoomLevel = FMath::Clamp(NewZoom, 0.25f, 4.0f);

	if (MapCanvas)
	{
		SystemMapUtils::ApplyZoomAndTilt(MapCanvas, ZoomLevel, TargetTiltAmount);
	}
}

void USystemMap::ZoomToFitAllPlanets()
{

}

FReply USystemMap::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	float Delta = InMouseEvent.GetWheelDelta();
	// In NativeTick or MouseWheel:
	float OldZoom = ZoomLevel;
	ZoomLevel = FMath::Clamp(ZoomLevel + Delta * ZoomStep, MinZoomLevel, MaxZoomLevel);

	// Maintain center by adjusting position
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
	{
		const FVector2D ViewSize = GetCachedGeometry().GetLocalSize();
		const FVector2D OldSize = MapCanvas->GetDesiredSize() * OldZoom;
		const FVector2D NewSize = MapCanvas->GetDesiredSize() * ZoomLevel;

		// Calculate shift in scale
		const FVector2D SizeDiff = (NewSize - OldSize) * 0.5f;

		// Update canvas position to preserve center
		const FVector2D OldPos = CanvasSlot->GetPosition();
		const FVector2D NewPos = OldPos - SizeDiff;

		CanvasSlot->SetPosition(NewPos);
		CurrentDragOffset = NewPos;
	}

	// Then apply transform
	SystemMapUtils::ApplyZoomAndTilt(MapCanvas, ZoomLevel, TargetTiltAmount);
	
	return FReply::Handled();
}

FReply USystemMap::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;

		// Store the mouse position in screen space at the moment of click
		DragStartPos = InMouseEvent.GetScreenSpacePosition();

		// Store the canvas offset at the start of the drag (including MapCenterOffset)
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
		{
			InitialCanvasOffset = CanvasSlot->GetPosition();
		}

		// Optional: Cancel any ongoing centering/zooming
		bIsCenteringToPlanet = false;
		bIsZoomingToPlanet = false;
		LastSelectedMarker = nullptr;

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

	// 1. Mouse delta since drag started
	const FVector2D CurrentMousePos = InMouseEvent.GetScreenSpacePosition();
	const FVector2D Delta = CurrentMousePos - DragStartPos;

	// 2. Apply delta to the position where drag started
	const FVector2D Proposed = InitialCanvasOffset + Delta;

	// 3. Clamp the new position based on zoomed canvas size and viewport
	const FVector2D CanvasSize = MapCanvas->GetDesiredSize() * ZoomLevel;
	const FVector2D ViewportSize = GetCachedGeometry().GetLocalSize();

	const FVector2D Clamped = SystemMapUtils::ClampCanvasDragOffset(
		Proposed, CanvasSize, ViewportSize, 50.0f // offset-aware clamp
	);

	// 4. Apply to canvas
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
	{
		CanvasSlot->SetPosition(Clamped);
		CurrentDragOffset = Clamped;
	}

	return FReply::Handled();
}

FReply USystemMap::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDragging)
	{
		bIsDragging = false;

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
		{
			// Recalculate and clamp position relative to center + offset
			const FVector2D CanvasSize = MapCanvas->GetDesiredSize() * ZoomLevel;
			const FVector2D ViewportSize = GetCachedGeometry().GetLocalSize();

			const FVector2D ClampedOffset = SystemMapUtils::ClampCanvasDragOffset(
				CanvasSlot->GetPosition(),
				CanvasSize,
				ViewportSize,
				50.0f // 
			);

			CanvasSlot->SetPosition(ClampedOffset);
			InitialCanvasOffset = ClampedOffset;
			CurrentDragOffset = ClampedOffset;
		}

		return FReply::Handled().ReleaseMouseCapture();
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
	if (!Marker || !MapCanvas) return;

	if (bIsWaitingToProcessMovement)
		return;

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot);
	UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(Marker->Slot);
	if (!CanvasSlot || !MarkerSlot) return;

	// 1. Current canvas position from drag
	const FVector2D CanvasPos = CurrentDragOffset;

	// 2. Marker position in canvas (local)
	const FVector2D MarkerCenter = MarkerSlot->GetPosition() + MarkerSlot->GetSize() * MarkerSlot->GetAlignment();

	// 3. Visual screen-space position of the marker (after zoom and drag)
	const FVector2D MarkerVisual = MarkerCenter * ZoomLevel + CanvasPos;

	// 4. Desired screen-space center (0,0 base + your offset)
	const FVector2D DesiredScreenCenter = MapCenterOffset;

	// 5. How much to move canvas to put marker at desired offset
	const FVector2D Delta = DesiredScreenCenter - MarkerVisual;

	// 6. New canvas position
	const FVector2D NewCanvasPos = CanvasPos + Delta;

	// 7. Optional: Clamp the result
	const FVector2D CanvasSize = MapCanvas->GetDesiredSize() * ZoomLevel;
	const FVector2D ViewportSize = GetCachedGeometry().GetLocalSize();
	const FVector2D Clamped = SystemMapUtils::ClampCanvasDragOffset(NewCanvasPos, CanvasSize, ViewportSize, 50.0f);

	// 8. Animate
	StartCanvasPosition = CanvasPos;
	TargetCanvasPosition = Clamped;
	CenterAnimTime = 0.0f;
	bIsCenteringToPlanet = true;

	// 9. Zoom
	StartZoomLevel = ZoomLevel;
	TargetZoomLevel = Zoom;
	ZoomAnimTime = 0.0f;

	LastSelectedMarker = Marker;
}

void USystemMap::ApplyTiltToMapCanvas(float TiltAmount)
{
	TargetTiltAmount = TiltAmount;
	SystemMapUtils::ApplyZoomAndTilt(MapCanvas, ZoomLevel, TargetTiltAmount);
}

void USystemMap::AddCentralStar(const FS_Galaxy* Star)
{
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

		FVector Location = FVector(-500, 0, 200);
		FRotator Rotation = FRotator::ZeroRotator;

		SunActor = ACentralSunActor::SpawnWithSpectralClass(
			GetWorld(),
			Location,
			Rotation,
			SunActorClass,
			Star->Stellar[0].Class,     // Spectral class
			Star->Stellar[0].Radius,    // Pass in radius from Galaxy.def
			Star->Stellar[0].Name
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
				StarSlot->SetSize(FVector2D(64.f, 64.f));
				TrackedMapWidgets.Add(StarWidget);

				if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
				{
					FVector2D CanvasPos = CanvasSlot->GetPosition();
					FVector2D SlotPos = StarSlot->GetPosition();
					UE_LOG(LogTemp, Warning, TEXT("CanvasSlot Pos: %s | StarSlot Pos: %s"),
						*CanvasPos.ToString(),
						*SlotPos.ToString());
				}
			}
		}
	}
}

void USystemMap::AddPlanet(const FS_PlanetMap& Planet)
{
	float Perihelion = 0.f;
	float Aphelion = 0.f;

	PlanetOrbitUtils::CalculateOrbitExtremes(Planet.Orbit, Planet.Eccentricity, Perihelion, Aphelion);

	// Get ellipse axes
	float a = (Perihelion + Aphelion) * 0.5f; // semi-major
	float b = a * FMath::Sqrt(1.0f - FMath::Square(Planet.Eccentricity)); // semi-minor

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
}

void USystemMap::AssignRenderTargetsToPlanets()
{
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

void USystemMap::HighlightSelectedSystem()
{
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

void USystemMap::FinalizeCanvasLayoutFromContentBounds()
{

}

void USystemMap::DeferredFinalizeLayout()
{

}

void USystemMap::InitMapCanvas()
{
	ZoomLevel = 1.0f;
	TargetZoomLevel = 1.0f;
	TargetTiltAmount = 0.0f;

	bIsCenteringToPlanet = false;
	bIsZoomingToPlanet = false;
	bIsDragging = false;

	bIsTiltingIn = true;
	TiltTime = 0.0f;

	CurrentDragOffset = FVector2D::ZeroVector;
	bPendingCanvasCenter = true;

	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Visible);
	SetIsEnabled(true);

	if(OuterCanvas) 
		OuterCanvas->SetClipping(EWidgetClipping::ClipToBoundsAlways);  // strict
	
	if (MapCanvas) {
		MapCanvas->ClearChildren();
		
		// Delay layout-dependent logic like centering
		FTimerHandle LayoutTimer;
		GetWorld()->GetTimerManager().SetTimer(LayoutTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
				{
					CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
					CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
					CanvasSlot->SetSize(FVector2D(3000.f, 2000.f));	
					CanvasSlot->SetPosition(MapCenterOffset);
					CanvasSlot->SetPosition(FVector2D(0.f, 0.f));

					SystemMapUtils::ApplyZoomAndTilt(MapCanvas, ZoomLevel, TargetTiltAmount);
				}
			}), 0.05f, false); 
	}
}

