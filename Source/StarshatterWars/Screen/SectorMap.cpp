// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SectorMap.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/SceneCaptureComponent2D.h"

#include "Engine/SceneCapture2D.h"

#include "PlanetMarkerWidget.h"
#include "MoonMarkerWidget.h"
#include "SystemOrbitWidget.h"
#include "OperationsScreen.h"

#include "../Game/GameStructs.h" // FS_Galaxy struct
#include "../Game/GalaxyManager.h"

#include "../System/SSWGameInstance.h"
#include "Kismet/GameplayStatics.h"

#include "../Actors/PlanetPanelActor.h"

#include "../Foundation/PlanetOrbitUtils.h"
#include "../Foundation/SystemMapUtils.h"
#include "../Foundation/StarUtils.h"

#include "TimerManager.h"
#include "EngineUtils.h" 


void USectorMap::NativeConstruct()
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

	InitSectorCanvas();

	// Optional: build the system after everything is ready
	if (USSWGameInstance* GI = GetGameInstance<USSWGameInstance>())
	{
		const FString& SelectedSector = GI->SelectedSector;
		const FS_PlanetMap* Sector = UGalaxyManager::Get(this)->FindSectorByName(SelectedSector);

		if (Sector)
		{
			UE_LOG(LogTemp, Log, TEXT("USectorMap::NativeConstruct() Found planet: %s"), *SelectedSector);
			BuildSectorView(Sector);
		}
	}
}

void USectorMap::NativeDestruct()
{
	Super::NativeDestruct();
}

void USectorMap::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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
			if (LastSelectedMoonMarker)
			{
				CenterOnMoonWidget(LastSelectedMoonMarker, ZoomLevel);
				LastSelectedMoonMarker = nullptr;
			}
		}
	}

	if (bIsAnimatingToMoon)
	{
		MoonFocusTime += InDeltaTime;
		float Alpha = FMath::Clamp(MoonFocusTime / MoonFocusDuration, 0.f, 1.f);
		float Smoothed = SystemMapUtils::EaseInOut(Alpha); // ease = t * t * (3 - 2t)

		const FVector2D InterpPos = FMath::Lerp(StartCanvasPos, TargetCanvasPos, Smoothed);

		if (UCanvasPanelSlot* PlanetSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
		{
			PlanetSlot->SetPosition(InterpPos);
		}

		if (Alpha >= 1.f)
		{
			bIsAnimatingToMoon = false;
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
void USectorMap::AddMoonOrbitalRing(const FS_MoonMap& Moon)
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

void USectorMap::HandleMoonClicked(const FString& MoonName)
{
	if (UMoonMarkerWidget** Found = MoonMarkers.Find(MoonName))
	{
		LastSelectedMoonMarker = *Found;
		// Optional: center or highlight
		UE_LOG(LogTemp, Log, TEXT("Moon selected: %s"), *MoonName);
		CenterOnMoonWidget(*Found, 1.0f);
	}
}

void USectorMap::CenterOnMoonWidget(UMoonMarkerWidget* Marker, float Zoom)
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
	bIsAnimatingToMoon = true;
}


void USectorMap::BuildSectorView(const FS_PlanetMap* ActivePlanet)
{
	MoonMarkers.Empty();
	MoonOrbitMarkers.Empty();
}


void USectorMap::InitSectorCanvas()
{

}

float USectorMap::GetDynamicMoonOrbitScale(const TArray<FS_MoonMap>& Moons, float MaxPixelRadius) const
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

void USectorMap::AddMoon(const FS_MoonMap& Moon, APlanetPanelActor* Parent, UPlanetMarkerWidget* ParentMarker)
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

			Marker->SetVisibility(ESlateVisibility::Visible);;
			Marker->InitFromMoonActor(Moon, NewMoon);
			Marker->OnMoonClicked.AddDynamic(this, &USectorMap::HandleMoonClicked);

			MoonMarkers.Add(Moon.Name, Marker);
		}
	}
}

void USectorMap::SetMarkerVisibility(bool bVisible)
{
	const ESlateVisibility Vis = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	for (auto& Elem : PlanetMarkers)
		if (Elem.Value) Elem.Value->SetVisibility(Vis);

	for (auto& Elem : MoonMarkers)
		if (Elem.Value) Elem.Value->SetVisibility(Vis);
}

void USectorMap::FocusAndZoomToMoon(UMoonMarkerWidget* Marker)
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
	MoonFocusTime = 0.f;
	bIsAnimatingToMoon = true;
}


FReply USectorMap::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
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
	if (LastSelectedMoonMarker)
	{
		CenterOnMoonWidget(LastSelectedMoonMarker, 1.5f); // will animate based on your setup
	}

	return FReply::Handled();
}

FReply USectorMap::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// Find planet under cursor (optional — you could just zoom to LastSelectedMarker)
		if (LastSelectedMoonMarker && MapCanvas)
		{
			if (LastSelectedMoonMarker)
			{
				CenterOnMoonWidget(LastSelectedMoonMarker, 1.5f); // will animate based on your setup
				SystemMapUtils::ApplyZoomToCanvas(MapCanvas, 2.0f);
			}
		}
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

FReply USectorMap::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
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
		bIsCenteringToMoon = false;
		bIsZoomingToMoon = false;
		LastSelectedMoonMarker = nullptr;

		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}


FReply USectorMap::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
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

FReply USectorMap::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDragging)
	{
		DragController.EndDrag();

		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void USectorMap::SetZoomLevel(float NewZoom)
{
	ZoomLevel = FMath::Clamp(NewZoom, 0.25f, 4.0f);

	if (MapCanvas)
	{
		SystemMapUtils::ApplyZoomAndTilt(MapCanvas, ZoomLevel, TargetTiltAmount);
	}
}




