// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemMap.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "PlanetMarkerWidget.h"
#include "MoonMarkerWidget.h"
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
#include "../Foundation/StarUtils.h"
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

float USystemMap::GetDynamicMoonOrbitScale(const TArray<FS_MoonMap>& Moons, float MaxPixelRadius) const
{
	float MaxOrbit = 0.f;
	for (const FS_MoonMap& Moon : Moons)
	{
		MaxOrbit = FMath::Max(MaxOrbit, Moon.Orbit);
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
	
	if (bIsAnimatingToPlanet)
	{
		PlanetFocusTime += InDeltaTime;
		float Alpha = FMath::Clamp(PlanetFocusTime / PlanetFocusDuration, 0.f, 1.f);
		float Smoothed = SystemMapUtils::EaseInOut(Alpha); // ease = t * t * (3 - 2t)

		const FVector2D InterpPos = FMath::Lerp(StartCanvasPos, TargetCanvasPos, Smoothed);

		if (UCanvasPanelSlot* PlanetSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
		{
			PlanetSlot->SetPosition(InterpPos);
		}

		if (Alpha >= 1.f)
		{
			bIsAnimatingToPlanet = false;
		}
	}

	// Phase 2: Zoom in/out
	/*if (!FMath::IsNearlyEqual(ZoomLevel, TargetZoom))
	{
		const float DeltaZoom = TargetZoom - ZoomLevel;
		ZoomLevel += DeltaZoom * FMath::Clamp(InDeltaTime * ZoomInterpSpeed, 0.f, 1.f);

		// Zoom at anchor point
		if (UCanvasPanelSlot* ZoomSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
		{
			const FVector2D ScaleDelta = FVector2D(ZoomLevel / FMath::Max(0.01f, ZoomLevel - DeltaZoom));
			const FVector2D AnchorOffset = ZoomAnchorLocal - PreZoomCanvasPos;
			const FVector2D ScaledOffset = AnchorOffset * (1.f - ZoomLevel / TargetZoom);

			const FVector2D NewCanvasPos = CanvasSlot->GetPosition() + ScaledOffset;
			ZoomSlot->SetPosition(NewCanvasPos);
		}

		// Apply transform scale
		SystemMapUtils::ApplyZoomToCanvas(MapCanvas, ZoomLevel);
	}*/

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
	PlanetOrbitMarkers.Empty();
	MoonMarkers.Empty();
	MoonOrbitMarkers.Empty();

	const FVector2D Center = MapCanvas->GetCachedGeometry().GetLocalSize() * 0.5f;
	
	AddCentralStar(ActiveSystem);

	// Convert to screen scale
	ORBIT_TO_SCREEN = GetDynamicOrbitScale(ActiveSystem->Stellar[0].Planet, 480.f);

	// Build planet markers and orbit rings
	for (const FS_PlanetMap& Planet : ActiveSystem->Stellar[0].Planet)
	{
		AddPlanet(Planet);		
	}
		
	HighlightSelectedSystem();
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
	const float ScrollDelta = InMouseEvent.GetWheelDelta();

	// Store previous zoom
	const float PreviousZoom = ZoomLevel;

	// Apply delta
	ZoomLevel = FMath::Clamp(ZoomLevel + ScrollDelta * ZoomStep, MinZoom, MaxZoom);

	if (ZoomLevel == PreviousZoom)
		return FReply::Handled(); // no change

	// Reapply transform
	SystemMapUtils::ApplyZoomToCanvas(MapCanvas, ZoomLevel);

	// Optional: recenter on last selected planet
	if (LastSelectedMarker)
	{
		CenterOnPlanetWidget(LastSelectedMarker, 1.5f); // will animate based on your setup
	}

	return FReply::Handled();
}

FReply USystemMap::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// Find planet under cursor (optional — you could just zoom to LastSelectedMarker)
		if (LastSelectedMarker && MapCanvas)
		{
			if (LastSelectedMarker)
			{
				CenterOnPlanetWidget(LastSelectedMarker, 1.5f); // will animate based on your setup
				SystemMapUtils::ApplyZoomToCanvas(MapCanvas, 2.0f);
			}
		}
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

FReply USystemMap::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;

		// Store the mouse position in screen space at the moment of click
		DragStartPos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
		{
			DragController.BeginDrag(InGeometry, InMouseEvent, CanvasSlot->GetPosition());
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
	if (DragController.bIsDragging && MapCanvas)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
		{
			FVector2D Delta = DragController.ComputeDragDelta(InGeometry, InMouseEvent);
			FVector2D Proposed = DragController.InitialCanvasOffset + Delta;

			FVector2D Clamped = SystemMapUtils::ClampCanvasToSafeMargin(
				Proposed,
				CachedCanvasSize,
				CachedViewportSize,
				100.f // margin in pixels
			);

			//FVector2D Final = SystemMapUtils::ClampCanvasDragOffset(Clamped, CachedCanvasSize, CachedViewportSize, 50.f);
			CanvasSlot->SetPosition(Proposed);
		}
	}

	return FReply::Handled();
}

FReply USystemMap::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDragging)
	{
		DragController.EndDrag();

		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}


void USystemMap::HandlePlanetClicked(const FString& PlanetName)
{
	if (UPlanetMarkerWidget** Found = PlanetMarkers.Find(PlanetName))
	{
		LastSelectedMarker = *Found;
		// Optional: center or highlight
		UE_LOG(LogTemp, Log, TEXT("Planet selected: %s"), *PlanetName);
		CenterOnPlanetWidget(*Found, 1.0f);
	}
}

void USystemMap::HandleMoonClicked(const FString& MoonName)
{
	if (UMoonMarkerWidget** Found = MoonMarkers.Find(MoonName))
	{
		LastMoonSelectedMarker = *Found;
		// Optional: center or highlight
		UE_LOG(LogTemp, Log, TEXT("Moon selected: %s"), *MoonName);
		CenterOnMoonWidget(*Found, 1.0f);
	}
}

void USystemMap::CenterOnPlanetWidget(UPlanetMarkerWidget* Marker, float Zoom)
{
	if (!Marker || !MapCanvas) return;

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot);
	UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(Marker->Slot);
	if (!CanvasSlot || !MarkerSlot) return;

	const FVector2D MarkerCenter = MarkerSlot->GetPosition() + MarkerSlot->GetSize() * MarkerSlot->GetAlignment();
	const FVector2D ViewportCenter = CachedViewportSize * 0.5f;

	StartCanvasPos = CanvasSlot->GetPosition();
	TargetCanvasPos = ViewportCenter - MarkerCenter;

	PlanetFocusTime = 0.f;
	bIsAnimatingToPlanet = true;
}

void USystemMap::CenterOnMoonWidget(UMoonMarkerWidget* Marker, float Zoom)
{
	if (!Marker || !MapCanvas) return;

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot);
	UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(Marker->Slot);
	if (!CanvasSlot || !MarkerSlot) return;

	const FVector2D MarkerCenter = MarkerSlot->GetPosition() + MarkerSlot->GetSize() * MarkerSlot->GetAlignment();
	const FVector2D ViewportCenter = CachedViewportSize * 0.5f;

	StartCanvasPos = CanvasSlot->GetPosition();
	TargetCanvasPos = ViewportCenter - MarkerCenter;

	MoonFocusTime = 0.f;
	bIsAnimatingToPlanet = true;
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

void USystemMap::AddPlanetOrbitalRing(const FS_PlanetMap& Planet)
{
	float Perihelion = 0.f;
	float Aphelion = 0.f;

	PlanetOrbitUtils::CalculateOrbitExtremes(Planet.Orbit, Planet.Eccentricity, Perihelion, Aphelion);

	// Get ellipse axes
	float a = (Perihelion + Aphelion) * 0.5f; // semi-major
	float b = a * FMath::Sqrt(1.0f - FMath::Square(Planet.Eccentricity)); // semi-minor

	a /= ORBIT_TO_SCREEN;
	b /= ORBIT_TO_SCREEN;

	OrbitRadius = Planet.Orbit / ORBIT_TO_SCREEN;

	UE_LOG(LogTemp, Warning, TEXT("Planet %s -> Orbit radius: %f"), *Planet.Name, OrbitRadius);

	if (OrbitWidgetClass)
	{
		auto* PlanetOrbitRing = CreateWidget<USystemOrbitWidget>(this, OrbitWidgetClass);

		//OrbitCenter += Center;
		// Apply visual configuration
		PlanetOrbitRing->SetOrbitRadius(OrbitRadius);
		const float InclinationVisual = PlanetOrbitUtils::AmplifyInclination(Planet.Inclination);
		PlanetOrbitRing->SetOrbitInclination(InclinationVisual);

		// Position orbit ring widget
		if (UCanvasPanelSlot* OrbitSlot = MapCanvas->AddChildToCanvas(PlanetOrbitRing))
		{
			OrbitSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			OrbitSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			OrbitSlot->SetPosition(FVector2D(0.f, 0.f)); // use panel center
			OrbitSlot->SetZOrder(0);
			OrbitSlot->SetAutoSize(true);

			PlanetOrbitMarkers.Add(Planet.Name, PlanetOrbitRing);
		}

		PlanetOrbitRing->SetVisibility(ESlateVisibility::Visible);
	}
}

void USystemMap::AddMoonOrbitalRing(const FS_MoonMap& Moon)
{
	OrbitRadius = Moon.Orbit / ORBIT_TO_SCREEN;

	UE_LOG(LogTemp, Warning, TEXT("Moon: %s -> Orbit radius: %f"), *Moon.Name, OrbitRadius);

	if (OrbitWidgetClass)
	{
		auto* MoonOrbitRing = CreateWidget<USystemOrbitWidget>(this, OrbitWidgetClass);

		// Apply visual configuration
		MoonOrbitRing->SetOrbitRadius(OrbitRadius);
		//const float InclinationVisual = PlanetOrbitUtils::AmplifyInclination(Planet.Inclination);
		//Orbit->SetOrbitInclination(InclinationVisual);

		// Position orbit ring widget
		if (UCanvasPanelSlot* OrbitSlot = MapCanvas->AddChildToCanvas(MoonOrbitRing))
		{
			OrbitSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			OrbitSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			OrbitSlot->SetPosition(FVector2D(0.f, 0.f)); // use panel center
			OrbitSlot->SetZOrder(0);
			OrbitSlot->SetAutoSize(true);

			MoonOrbitMarkers.Add(Moon.Name, MoonOrbitRing);
		}

		MoonOrbitRing->SetVisibility(ESlateVisibility::Visible);
	}
}

void USystemMap::AddPlanet(const FS_PlanetMap& Planet)
{
	AddPlanetOrbitalRing(Planet);

	MoonMarkers.Empty();
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
			UTextureRenderTarget2D* PlanetRT = SystemMapUtils::CreateUniqueRenderTargetForActor(Planet.Name, PlanetActor);
			SpawnedPlanetActors.Add(PlanetActor);
		}
	}
	// Planet marker
	if (PlanetMarkerClass)
	{
		auto* PlanetMarker = CreateWidget<UPlanetMarkerWidget>(this, PlanetMarkerClass);
		if (PlanetMarker)
		{
			float& OrbitAngle = PlanetOrbitAngles.FindOrAdd(Planet.Name);
			if (OrbitAngle == 0.0f)
			{
				OrbitAngle = FMath::FRandRange(0.0f, 360.0f);
			}

			float VisualInclination = PlanetOrbitUtils::AmplifyInclination(Planet.Inclination, 2.0f);

			OrbitRadius = Planet.Orbit / ORBIT_TO_SCREEN;

			FVector2D TiltedPos = PlanetOrbitUtils::Get2DOrbitPositionWithInclination(
				OrbitRadius,
				OrbitAngle,
				VisualInclination
			);

			if (UCanvasPanelSlot* PlanetSlot = MapCanvas->AddChildToCanvas(PlanetMarker))
			{
				PlanetSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				PlanetSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				PlanetSlot->SetPosition(TiltedPos);
				PlanetSlot->SetAutoSize(true);
				PlanetSlot->SetZOrder(10);
			}

			PlanetMarker->SetVisibility(ESlateVisibility::Visible);
			PlanetMarker->SetPlanetName(Planet.Name);
			PlanetMarker->InitFromPlanetActor(Planet, PlanetActor); // pass data and actor
			PlanetMarker->OnPlanetClicked.AddDynamic(this, &USystemMap::HandlePlanetClicked);


			PlanetMarkers.Add(Planet.Name, PlanetMarker);

			for (const FS_MoonMap& Moon : Planet.Moon)
			{
				//AddMoon(Moon, PlanetActor, PlanetMarker);
			}
		}
	}
}

void USystemMap::AddMoon(const FS_MoonMap& Moon, APlanetPanelActor* Parent, UPlanetMarkerWidget* ParentMarker)
{
	if (!Parent || !MapCanvas || !MoonActorClass) return;

	AddMoonOrbitalRing(Moon);

	// --- Get or assign a stable orbit angle ---
	float& OrbitAngle = MoonOrbitAngles.FindOrAdd(Moon.Name);
	if (OrbitAngle == 0.0f)
	{
		OrbitAngle = FMath::FRandRange(0.0f, 360.0f);
	}

	// --- Compute orbit offset from parent ---
	
	constexpr float MoonOrbitBoost = 10.0f;

	FVector2D OrbitOffset2D = SystemMapUtils::ComputeMoonOrbitOffset(
		Moon.Orbit * MoonOrbitBoost,
		OrbitAngle,
		Moon.Inclination,
		ORBIT_TO_SCREEN
	);

	// --- Spawn moon actor ---
	const FVector MoonWorldPos = Parent->GetActorLocation() + FVector(OrbitOffset2D.X, OrbitOffset2D.Y, 0);

	AMoonPanelActor* NewMoon = AMoonPanelActor::SpawnWithMoonData(
		GetWorld(),
		MoonWorldPos,
		FRotator::ZeroRotator,
		MoonActorClass,
		Moon
	);

	if (NewMoon)
	{
		NewMoon->SetParentPlanet(Parent);		
		NewMoon->AttachToActor(Parent, FAttachmentTransformRules::KeepWorldTransform);
		SpawnedMoonActors.Add(NewMoon);

		UE_LOG(LogTemp, Log, TEXT("SetParentPlanet() Parent: %s -> Moon: %s"), *Parent->PlanetData.Name, *Moon.Name);
	}

	// --- Spawn moon marker (UI) ---
	if (MoonMarkerClass)
	{
		UMoonMarkerWidget* Marker = CreateWidget<UMoonMarkerWidget>(this, MoonMarkerClass);
		if (Marker)
		{
			FVector2D MarkerPos = OrbitOffset2D;

			if (UCanvasPanelSlot* ParentSlot = Cast<UCanvasPanelSlot>(ParentMarker->Slot))
			{
				FVector2D ParentCenter = ParentSlot->GetPosition() + ParentSlot->GetSize() * ParentSlot->GetAlignment();
				MarkerPos += ParentCenter;
			}

			if (UCanvasPanelSlot* MoonSlot = MapCanvas->AddChildToCanvas(Marker))
			{
				MoonSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				MoonSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				MoonSlot->SetPosition(MarkerPos);
				MoonSlot->SetAutoSize(true);
				MoonSlot->SetZOrder(10);
			}

			Marker->SetVisibility(ESlateVisibility::Visible);
			Marker->SetMoonName(Moon.Name);
			Marker->InitFromMoonActor(Moon, NewMoon);
			Marker->OnMoonClicked.AddDynamic(this, &USystemMap::HandleMoonClicked);

			MoonMarkers.Add(Moon.Name, Marker);
		}
	}
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
	TargetZoom = 1.0f;
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
					CanvasSlot->SetSize(FVector2D(6000.f, 6000.f));	
					CanvasSlot->SetPosition(FVector2D(0.f, 0.f));

					SystemMapUtils::ApplyZoomAndTilt(MapCanvas, ZoomLevel, TargetTiltAmount);
				}
			}), 0.05f, false); 
	}
}

void USystemMap::FocusAndZoomToPlanet(UPlanetMarkerWidget* Marker)
{
	if (!Marker || !MapCanvas) return;

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot);
	UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(Marker->Slot);
	if (!CanvasSlot || !MarkerSlot) return;

	const FVector2D MarkerCenter = MarkerSlot->GetPosition() + MarkerSlot->GetSize() * MarkerSlot->GetAlignment();
	const FVector2D ViewportCenter = CachedViewportSize * 0.5f;

	// Set zoom level instantly or interpolate target
	TargetZoom = FMath::Clamp(1.5f, MinZoom, MaxZoom); // Change 1.5f to desired zoom
	ZoomAnchorLocal = ViewportCenter;
	PreZoomCanvasPos = CanvasSlot->GetPosition();

	// Begin centering animation
	StartCanvasPos = CanvasSlot->GetPosition();
	TargetCanvasPos = ViewportCenter - MarkerCenter;
	PlanetFocusTime = 0.f;
	bIsAnimatingToPlanet = true;
}
