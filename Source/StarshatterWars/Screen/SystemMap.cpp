// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         SystemMap.cpp	AUTHOR:       Carlos Bott*/

#include "SystemMap.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include "PlanetMarkerWidget.h"
#include "MoonMarkerWidget.h"
#include "SystemOrbitWidget.h"
#include "CentralSunWidget.h"
#include "OperationsScreen.h"

#include "../Game/GalaxyManager.h"

#include "../Actors/CentralSunActor.h"
#include "../Actors/PlanetPanelActor.h"

#include "../Foundation/PlanetOrbitUtils.h"
#include "../Foundation/SystemMapUtils.h"

void USystemMap::NativeConstruct()
{
	Super::NativeConstruct();

	InitMapCanvas();

	// Ensure the image exists early (either bound or created)
	EnsureOverviewImage();

	// Ensure RT + actor exist (lazy creation, safe)
	EnsureOverviewRenderTarget(OverviewRTSize);
	EnsureOverviewActor();
	UpdateOverviewBrush();
}

void USystemMap::NativeDestruct()
{
	ClearSystemView();

	// Destroy overview actor (owned by widget)
	if (SystemOverviewActor)
	{
		SystemOverviewActor->Destroy();
		SystemOverviewActor = nullptr;
	}

	// Release RT (owned by widget)
	SystemOverviewRT = nullptr;

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
		return 1.0f;
	}

	return MaxOrbit / MaxPixelRadius;
}

void USystemMap::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!MapCanvas)
		return;

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot);
	if (!CanvasSlot)
		return;

	if (bIsWaitingToProcessMovement)
	{
		MovementDelayTime += InDeltaTime;
		if (MovementDelayTime >= MovementDelayDuration)
		{
			bIsWaitingToProcessMovement = false;
			MovementDelayTime = 0.0f;

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
		const float Alpha = FMath::Clamp(PlanetFocusTime / PlanetFocusDuration, 0.f, 1.f);
		const float Smoothed = SystemMapUtils::EaseInOut(Alpha);

		const FVector2D InterpPos = FMath::Lerp(StartCanvasPos, TargetCanvasPos, Smoothed);
		CanvasSlot->SetPosition(InterpPos);

		if (Alpha >= 1.f)
		{
			bIsAnimatingToPlanet = false;
		}
	}

	if (bIsTiltingIn)
	{
		TiltTime += InDeltaTime;
		const float Progress = FMath::Clamp(TiltTime / TiltDuration, 0.0f, 1.0f);
		const float SmoothT = SystemMapUtils::EaseInOut(Progress);

		const float CurrentTilt = FMath::Lerp(0.0f, TargetTiltAmount, SmoothT);
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
	if (!ActiveSystem || ActiveSystem->Stellar.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("USystemMap::BuildSystemView(): ActiveSystem missing Stellar[0]"));
		return;
	}

	if (!MapCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("USystemMap::BuildSystemView(): Missing MapCanvas"));
		return;
	}

	bPendingCanvasCenter = true;

	MapCanvas->ClearChildren();
	PlanetMarkers.Empty();
	PlanetOrbitMarkers.Empty();

	// Put overview image behind rings/markers (re-add if needed)
	EnsureOverviewImage();

	AddCentralStar(ActiveSystem);

	ORBIT_TO_SCREEN = GetDynamicOrbitScale(ActiveSystem->Stellar[0].Planet, 480.f);

	for (const FS_PlanetMap& Planet : ActiveSystem->Stellar[0].Planet)
	{
		AddPlanet(Planet);
	}

	HighlightSelectedSystem();
	SetMarkerVisibility(true);

	// NEW: widget-owned overview capture
	RebuildAndCaptureOverview(ActiveSystem);
}

void USystemMap::HandleCentralSunClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Central sun clicked — requesting galaxy map view"));

	if (OwningOperationsScreen)
	{
		OwningOperationsScreen->ShowGalaxyMap();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No owner set for USystemMap"));
	}
}

void USystemMap::HandlePlanetSelected(FS_PlanetMap Planet)
{
	UE_LOG(LogTemp, Warning, TEXT("Planet %s clicked — requesting sector map view"), *Planet.Name);

	if (OwningOperationsScreen)
	{
		if (USSWGameInstance* SSWInstance = GetGameInstance<USSWGameInstance>())
		{
			SSWInstance->SelectedSector = Planet;
			SSWInstance->SelectedSectorName = Planet.Name;
		}

		OwningOperationsScreen->ShowSectorMap(Planet);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No owner set for USystemMap"));
	}
}

void USystemMap::InitMapCanvas()
{
	ZoomLevel = 1.0f;
	TargetTiltAmount = 0.0f;

	bIsDragging = false;
	bIsTiltingIn = true;
	TiltTime = 0.0f;

	bPendingCanvasCenter = true;

	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Visible);
	SetIsEnabled(true);

	// Clear system UI elements / actors you already manage
	if (UGalaxyManager::Get(this))
	{
		UGalaxyManager::Get(this)->ClearAllRenderTargets();
	}
	SystemMapUtils::ClearAllSystemUIElements(this);
	SystemMapUtils::DestroyAllSystemActors(GetWorld());

	if (!OuterCanvas || !MapCanvas)
	{
		return;
	}

	if (UCanvasPanelSlot* MainSlot = Cast<UCanvasPanelSlot>(OuterCanvas->Slot))
	{
		MainSlot->SetPosition(FVector2D::ZeroVector);
	}

	FTimerHandle LayoutTimer;
	GetWorld()->GetTimerManager().SetTimer(
		LayoutTimer,
		FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
				{
					CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
					CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
					CanvasSlot->SetSize(CanvasSize);
					CanvasSlot->SetPosition(FVector2D(0.f, 0.f));
				}

				// Cache viewport size (if your project sets this elsewhere, keep that)
				CachedViewportSize = CanvasSize; // fallback
			}),
		0.05f,
		false
	);

	// Optional: auto-build current selected system
	if (USSWGameInstance* GI = GetGameInstance<USSWGameInstance>())
	{
		const FString& SelectedSystem = GI->SelectedSystem;
		if (const FS_Galaxy* System = UGalaxyManager::Get(this)->FindSystemByName(SelectedSystem))
		{
			UE_LOG(LogTemp, Log, TEXT("USystemMap::InitMapCanvas() Found system: %s"), *SelectedSystem);
			BuildSystemView(System);
		}
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
	// Your future implementation
}

FReply USystemMap::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const float ScrollDelta = InMouseEvent.GetWheelDelta();
	const float PreviousZoom = ZoomLevel;

	ZoomLevel = FMath::Clamp(ZoomLevel + ScrollDelta * ZoomStep, MinZoom, MaxZoom);
	if (ZoomLevel == PreviousZoom)
		return FReply::Handled();

	SystemMapUtils::ApplyZoomToCanvas(MapCanvas, ZoomLevel);

	if (LastSelectedMarker)
	{
		CenterOnPlanetWidget(LastSelectedMarker, 1.5f);
	}

	return FReply::Handled();
}

FReply USystemMap::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

FReply USystemMap::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;

		DragStartPos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
		{
			DragController.BeginDrag(InGeometry, InMouseEvent, CanvasSlot->GetPosition());
		}

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
			const FVector2D Delta = DragController.ComputeDragDelta(InGeometry, InMouseEvent);
			const FVector2D Proposed = DragController.InitialCanvasOffset + Delta;

			// NOTE: you compute Clamped but you were applying Proposed.
			// If you want clamping, set Clamped.
			const FVector2D Clamped = SystemMapUtils::ClampCanvasToSafeMargin(
				Proposed,
				CachedCanvasSize,
				CachedViewportSize,
				100.f
			);

			CanvasSlot->SetPosition(Clamped);
		}
	}

	return FReply::Handled();
}

FReply USystemMap::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDragging)
	{
		bIsDragging = false;
		DragController.EndDrag();
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void USystemMap::HandlePlanetClicked(const FString& PlanetName)
{
	if (UPlanetMarkerWidget** Found = PlanetMarkers.Find(PlanetName))
	{
		if (LastSelectedMarker == *Found)
		{
			UE_LOG(LogTemp, Log, TEXT("Planet already selected: %s, loading sector view"), *PlanetName);
			HandlePlanetSelected(LastSelectedMarker->PlanetData);
		}
		else
		{
			LastSelectedMarker = *Found;
			UE_LOG(LogTemp, Log, TEXT("Planet selected: %s"), *PlanetName);
			CenterOnPlanetWidget(*Found, 1.0f);
		}
	}
}

void USystemMap::CenterOnPlanetWidget(UPlanetMarkerWidget* Marker, float Zoom)
{
	if (!Marker || !MapCanvas) return;

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot);
	UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(Marker->Slot);
	if (!CanvasSlot || !MarkerSlot) return;

	// Marker position is in MapCanvas local space.
	// IMPORTANT: AutoSize widgets often report 0 size at runtime; use DesiredSize as fallback.
	FVector2D MarkerSize = MarkerSlot->GetSize();
	if (MarkerSize.IsNearlyZero())
	{
		MarkerSize = Marker->GetDesiredSize();
	}

	const FVector2D MarkerCenter =
		MarkerSlot->GetPosition() + (MarkerSize * MarkerSlot->GetAlignment());

	const FVector2D ViewportCenter = GetViewportCenterInCanvasSpace();

	StartCanvasPos = CanvasSlot->GetPosition();
	TargetCanvasPos = ViewportCenter - MarkerCenter;

	PlanetFocusTime = 0.f;
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

		if (StarWidget)
		{
			StarWidget->RemoveFromParent();
			StarWidget = nullptr;
		}

		const FVector Location(-500, 0, 200);
		const FRotator Rotation = FRotator::ZeroRotator;

		SunActor = ACentralSunActor::SpawnWithSpectralClass(
			GetWorld(),
			Location,
			Rotation,
			SunActorClass,
			Star->Stellar[0].Class,
			Star->Stellar[0].Radius,
			Star->Stellar[0].Name
		);
	}

	if (StarWidgetClass)
	{
		StarWidget = CreateWidget<UCentralSunWidget>(this, StarWidgetClass);
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
				CentralStarMarker = StarWidget;
			}
		}
	}
}

void USystemMap::AddPlanetOrbitalRing(const FS_PlanetMap& Planet)
{
	float Perihelion = 0.f;
	float Aphelion = 0.f;

	PlanetOrbitUtils::CalculateOrbitExtremes(Planet.Orbit, Planet.Eccentricity, Perihelion, Aphelion);

	float a = (Perihelion + Aphelion) * 0.5f;
	float b = a * FMath::Sqrt(1.0f - FMath::Square(Planet.Eccentricity));

	a /= ORBIT_TO_SCREEN;
	b /= ORBIT_TO_SCREEN;

	OrbitRadius = Planet.Orbit / ORBIT_TO_SCREEN;

	if (OrbitWidgetClass)
	{
		USystemOrbitWidget* PlanetOrbitRing = CreateWidget<USystemOrbitWidget>(this, OrbitWidgetClass);
		PlanetOrbitRing->SetOrbitRadius(OrbitRadius);

		const float InclinationVisual = PlanetOrbitUtils::AmplifyInclination(Planet.Inclination);
		PlanetOrbitRing->SetOrbitInclination(InclinationVisual);

		if (UCanvasPanelSlot* OrbitSlot = MapCanvas->AddChildToCanvas(PlanetOrbitRing))
		{
			OrbitSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			OrbitSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			OrbitSlot->SetPosition(FVector2D(0.f, 0.f));
			OrbitSlot->SetZOrder(0);
			OrbitSlot->SetAutoSize(true);

			PlanetOrbitMarkers.Add(Planet.Name, PlanetOrbitRing);
		}

		PlanetOrbitRing->SetVisibility(ESlateVisibility::Visible);
	}
}

void USystemMap::AddPlanet(const FS_PlanetMap& Planet)
{
	AddPlanetOrbitalRing(Planet);

	APlanetPanelActor* NewPlanetActor = nullptr;

	if (PlanetActorClass)
	{
		const FVector ActorLocation(-1000, 0, 200);
		const FRotator ActorRotation = FRotator::ZeroRotator;

		NewPlanetActor = APlanetPanelActor::SpawnWithPlanetData(
			GetWorld(),
			ActorLocation,
			ActorRotation,
			PlanetActorClass,
			Planet
		);

		if (NewPlanetActor)
		{
			UTextureRenderTarget2D* PlanetRT =
				SystemMapUtils::CreateUniqueRenderTargetForActor(Planet.Name, NewPlanetActor);

			SpawnedPlanetActors.Add(NewPlanetActor);

			UE_LOG(LogTemp, Log, TEXT("Spawned PlanetActor for %s (RT=%s)"),
				*Planet.Name,
				PlanetRT ? *PlanetRT->GetName() : TEXT("NULL"));
		}
	}

	if (PlanetMarkerClass)
	{
		UPlanetMarkerWidget* PlanetMarker = CreateWidget<UPlanetMarkerWidget>(this, PlanetMarkerClass);
		if (PlanetMarker)
		{
			float& OrbitAngle = PlanetOrbitAngles.FindOrAdd(Planet.Name);
			if (FMath::IsNearlyZero(OrbitAngle))
			{
				OrbitAngle = FMath::FRandRange(0.0f, 360.0f);
			}

			const float VisualInclination = PlanetOrbitUtils::AmplifyInclination(Planet.Inclination, 2.0f);
			OrbitRadius = Planet.Orbit / ORBIT_TO_SCREEN;

			const FVector2D TiltedPos = PlanetOrbitUtils::Get2DOrbitPositionWithInclination(
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

			PlanetMarker->InitFromPlanetActor(Planet, NewPlanetActor);
			PlanetMarker->OnPlanetClicked.AddDynamic(this, &USystemMap::HandlePlanetClicked);

			PlanetMarkers.Add(Planet.Name, PlanetMarker);
		}
	}
}

void USystemMap::HighlightSelectedSystem()
{
	if (const USSWGameInstance* GI = GetWorld()->GetGameInstance<USSWGameInstance>())
	{
		const FString& CurrentSystem = GI->SelectedSystem;

		if (UPlanetMarkerWidget** Found = PlanetMarkers.Find(CurrentSystem))
		{
			UPlanetMarkerWidget* Marker = *Found;
			Marker->SetSelected(true);
		}
	}
}

void USystemMap::SetMarkerVisibility(bool bVisible)
{
	const ESlateVisibility Vis = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	if (CentralStarMarker)
		CentralStarMarker->SetVisibility(Vis);

	for (auto& Elem : PlanetMarkers)
	{
		if (Elem.Value)
			Elem.Value->SetVisibility(Vis);
	}
}

void USystemMap::ClearSystemView()
{
	if (MapCanvas)
	{
		MapCanvas->ClearChildren();
	}

	PlanetMarkers.Empty();
	PlanetOrbitMarkers.Empty();
	LastSelectedMarker = nullptr;

	SystemMapUtils::DestroyAllSystemActors(GetWorld());

	// Overview actor remains owned by widget; keep it and reuse.
	// If you want to destroy it on clear, uncomment:
	/*
	if (SystemOverviewActor)
	{
		SystemOverviewActor->Destroy();
		SystemOverviewActor = nullptr;
	}
	*/

	UE_LOG(LogTemp, Warning, TEXT("[SystemMap] Cleared all markers and actors"));
}

// =====================
// Overview RT pipeline
// =====================

void USystemMap::EnsureOverviewImage()
{
	if (OverviewImage)
	{
		// Already bound in widget BP
		return;
	}

	if (!MapCanvas || bOverviewImageAdded)
	{
		return;
	}

	OverviewImage = NewObject<UImage>(this, UImage::StaticClass(), TEXT("SystemOverviewImage"));
	if (!OverviewImage)
		return;

	if (UCanvasPanelSlot* OverviewSlot = MapCanvas->AddChildToCanvas(OverviewImage))
	{
		OverviewSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		OverviewSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		OverviewSlot->SetPosition(FVector2D(0.f, 0.f));

		// Large enough behind your system; tune as needed
		OverviewSlot->SetSize(FVector2D(2000.f, 2000.f));

		// Behind orbits/planets but above canvas background
		OverviewSlot->SetZOrder(1);

		bOverviewImageAdded = true;
	}

	OverviewImage->SetVisibility(ESlateVisibility::Visible);
}

void USystemMap::EnsureOverviewRenderTarget(int32 Size)
{
	if (SystemOverviewRT && SystemOverviewRT->SizeX == Size && SystemOverviewRT->SizeY == Size)
		return;

	SystemOverviewRT = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass(), TEXT("RT_SystemOverview"));
	if (!SystemOverviewRT)
		return;

	SystemOverviewRT->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	SystemOverviewRT->ClearColor = FLinearColor::Black;
	SystemOverviewRT->bAutoGenerateMips = false;
	SystemOverviewRT->InitAutoFormat(Size, Size);
	SystemOverviewRT->UpdateResourceImmediate(true);
}

void USystemMap::EnsureOverviewActor()
{
	if (SystemOverviewActor)
		return;

	if (!GetWorld())
		return;

	TSubclassOf<ASystemOverview> SpawnClass = SystemOverviewActorClass;
	if (!SpawnClass)
	{
		// If you do not want to expose a BP class, you can spawn native:
		SpawnClass = ASystemOverview::StaticClass();
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.ObjectFlags |= RF_Transient;

	// Spawn off-screen; it only captures to RT
	SystemOverviewActor = GetWorld()->SpawnActor<ASystemOverview>(
		SpawnClass,
		FVector(100000.f, 100000.f, 100000.f),
		FRotator::ZeroRotator,
		Params
	);

	if (SystemOverviewActor)
	{
		SystemOverviewActor->ConfigureCaptureForUI();
		if (SystemOverviewRT)
		{
			SystemOverviewActor->SetRenderTarget(SystemOverviewRT);
		}
	}
}

void USystemMap::BuildOverviewBodiesFromSystem(const FS_Galaxy* ActiveSystem, TArray<FOverviewBody>& OutBodies) const
{
	OutBodies.Reset();

	if (!ActiveSystem || ActiveSystem->Stellar.Num() == 0)
		return;

	const auto& Star = ActiveSystem->Stellar[0];

	// Root star
	{
		FOverviewBody StarBody;
		StarBody.Name = Star.Name;
		StarBody.ParentIndex = INDEX_NONE;
		StarBody.OrbitKm = 0.f;
		StarBody.InclinationDeg = 0.f;
		StarBody.OrbitAngleDeg = 0.f;

		// Star radius: use your star radius field (you used Star.Radius already in sun spawn)
		StarBody.RadiusKm = Star.Radius;

		OutBodies.Add(StarBody);
	}

	// Planets (parent = star index 0)
	for (const FS_PlanetMap& Planet : Star.Planet)
	{
		FOverviewBody P;
		P.Name = Planet.Name;
		P.ParentIndex = 0;
		P.OrbitKm = Planet.Orbit;

		// If you store inclination in degrees already, keep it.
		// If it's radians, convert here.
		P.InclinationDeg = Planet.Inclination;

		// Use the same random angle map you already keep, so the overview matches 2D map layout.
		const float Angle = PlanetOrbitAngles.Contains(Planet.Name) ? PlanetOrbitAngles[Planet.Name] : 0.f;
		P.OrbitAngleDeg = Angle;

		// IMPORTANT: adjust this line to your struct
		P.RadiusKm = Planet.Radius; // <-- rename if your field differs (e.g. Planet.Radius)

		OutBodies.Add(P);
	}

	// If you later add moons, extend here (ParentIndex = that planet's index in OutBodies)
}

void USystemMap::RebuildAndCaptureOverview(const FS_Galaxy* ActiveSystem)
{
	EnsureOverviewImage();
	EnsureOverviewRenderTarget(OverviewRTSize);
	EnsureOverviewActor();

	if (!SystemOverviewActor || !SystemOverviewRT)
		return;

	// Ensure actor has our RT
	SystemOverviewActor->SetRenderTarget(SystemOverviewRT);

	TArray<FOverviewBody> Bodies;
	BuildOverviewBodiesFromSystem(ActiveSystem, Bodies);

	SystemOverviewActor->BuildDiorama(Bodies);

	// Capture once after build
	SystemOverviewActor->CaptureOnce();

	// Push RT into UMG image
	UpdateOverviewBrush();
}

void USystemMap::UpdateOverviewBrush()
{
	if (!OverviewImage || !SystemOverviewRT)
		return;

	FSlateBrush Brush;
	Brush.SetResourceObject(SystemOverviewRT);
	Brush.ImageSize = FVector2D(SystemOverviewRT->SizeX, SystemOverviewRT->SizeY);
	OverviewImage->SetBrush(Brush);
	OverviewImage->SetOpacity(1.0f);
	OverviewImage->SetVisibility(ESlateVisibility::Visible);
}

FVector2D USystemMap::GetViewportCenterInCanvasSpace() const
{
	// Prefer OuterCanvas geometry; fall back to this widget geometry.
	const FVector2D ViewSize =
		OuterCanvas ? OuterCanvas->GetCachedGeometry().GetLocalSize()
		: GetCachedGeometry().GetLocalSize();

	// If the MapCanvas slot is center-anchored, its local origin is already at "center".
	if (MapCanvas)
	{
		if (const UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapCanvas->Slot))
		{
			const FAnchors A = CanvasSlot->GetAnchors();

			// Common case in your code: anchored at exact center.
			const bool bCenterAnchored =
				FMath::IsNearlyEqual(A.Minimum.X, 0.5f) &&
				FMath::IsNearlyEqual(A.Minimum.Y, 0.5f) &&
				FMath::IsNearlyEqual(A.Maximum.X, 0.5f) &&
				FMath::IsNearlyEqual(A.Maximum.Y, 0.5f);

			if (bCenterAnchored)
			{
				return FVector2D::ZeroVector; // center-origin space
			}
		}
	}

	// Otherwise treat as top-left origin space
	return ViewSize * 0.5f;
}
