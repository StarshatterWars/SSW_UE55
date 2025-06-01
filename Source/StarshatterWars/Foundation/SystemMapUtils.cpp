// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemMapUtils.h"

#include "Components/Widget.h"
#include "Components/CanvasPanel.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/WidgetComponent.h"

#include "../Screen/PlanetMarkerWidget.h"

#include "PlanetOrbitUtils.h"
#include "TimerManager.h"

#include "Materials/MaterialInstanceDynamic.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/World.h"

#include "../Game/GalaxyManager.h"
#include "../Screen/SystemOrbitWidget.h"

#include "Slate/WidgetRenderer.h"
#include "Widgets/SViewport.h"
#include "Kismet/KismetRenderingLibrary.h"


UTextureRenderTarget2D* SystemMapUtils::CreateUniqueRenderTargetForActor(
	const FString& Name,
	AActor* OwnerActor,
	int32 Resolution
)
{
	if (!OwnerActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateUniqueRenderTargetForActor failed: null owner for %s"), *Name);
		return nullptr;
	}

	const FName UniqueRTName = FName(*FString::Printf(TEXT("RT_%s_%s"), *OwnerActor->GetName(), *Name));

	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(
		OwnerActor,
		UTextureRenderTarget2D::StaticClass(),
		UniqueRTName,
		RF_Transient
	);

	if (!RenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create render target for %s"), *Name);
		return nullptr;
	}

	RenderTarget->RenderTargetFormat = RTF_RGBA16f;
	RenderTarget->ClearColor = FLinearColor::Black;
	RenderTarget->bAutoGenerateMips = false;
	RenderTarget->InitAutoFormat(Resolution, Resolution);
	RenderTarget->UpdateResourceImmediate(true);

	UE_LOG(LogTemp, Log, TEXT("Created unique render target: %s [Addr: %p] for actor %s"),
		*UniqueRTName.ToString(), RenderTarget, *OwnerActor->GetName());

	return RenderTarget;
}


float SystemMapUtils::ClampZoomLevel(float ProposedZoom, float MinZoom, float MaxZoom)
{
	return FMath::Clamp(ProposedZoom, MinZoom, MaxZoom);
}

float SystemMapUtils::GetCenteredCanvasPosition(float ContentWidth, float ViewportWidth)
{
	return (ViewportWidth - ContentWidth) * 0.5f;
}

float SystemMapUtils::GetCenteredScrollOffset(float ContentHeight, float ViewportHeight)
{
	const float MaxScrollY = FMath::Max(ContentHeight - ViewportHeight, 0.f);
	return MaxScrollY * 0.5f;
}

float SystemMapUtils::InterpolateScrollOffset(float CurrentOffset, float TargetOffset, float DeltaTime, float Speed)
{
	return FMath::FInterpTo(CurrentOffset, TargetOffset, DeltaTime, Speed);
}

float SystemMapUtils::SnapZoomToSteps(float Zoom, float StepSize, float MinZoom, float MaxZoom)
{
	float Clamped = FMath::Clamp(Zoom, MinZoom, MaxZoom);
	float Steps = FMath::RoundToFloat(Clamped / StepSize);
	return FMath::Clamp(Steps * StepSize, MinZoom, MaxZoom);
}

float SystemMapUtils::ClampHorizontalPosition(float ProposedX, float ContentWidth, float ViewportWidth, float Margin)
{
	const float MinVisible = 32.f; // ensure part of the central sun stays on-screen

	// Prevent the left edge from moving too far right (star disappearing)
	const float MaxX = Margin;

	// Prevent the right edge from dragging too far left (entire canvas offscreen)
	const float MinX = FMath::Min(ViewportWidth - ContentWidth + Margin, Margin - MinVisible);
	//const float MinX = ViewportWidth - ContentWidth - Margin;

	return FMath::Clamp(ProposedX, MinX, MaxX);
}

float SystemMapUtils::ClampVerticalPosition(float ProposedY, float ContentHeight, float ViewportHeight, float Margin)
{
	const float MinY = Margin; // Prevents top from going offscreen
	const float MaxY = FMath::Max(ContentHeight - ViewportHeight - Margin, 0.f); // Prevents bottom overscroll

	return FMath::Clamp(ProposedY, MinY, MaxY);
}

float SystemMapUtils::ClampVerticalScroll(float ProposedOffset, float ContentHeight, float ViewportHeight, float Margin)
{
	const float MaxScrollY = FMath::Max(ContentHeight - ViewportHeight - Margin, 0.f);
	const float MinScrollY = Margin;

	return FMath::Clamp(ProposedOffset, MinScrollY, MaxScrollY);
}

float SystemMapUtils::ClampHorizontalScroll(float ProposedOffset, float ContentWidth, float ViewportWidth, float Margin)
{
	const float MaxScrollX = FMath::Max(ContentWidth - ViewportWidth - Margin, 0.f);
	const float MinScrollX = Margin;

	return FMath::Clamp(ProposedOffset, MinScrollX, MaxScrollX);
}

float SystemMapUtils::EaseInOut(float t)
{
	return t * t * (3.f - 2.f * t);
}

void SystemMapUtils::ApplyZoomAndTilt(UCanvasPanel* MapCanvas, float Zoom, float Tilt)
{
	if (!MapCanvas) return;

	// Apply zoom and tilt to the canvas itself
	FWidgetTransform CanvasTransform;
	CanvasTransform.Scale = FVector2D(Zoom, Zoom);
	CanvasTransform.Shear = FVector2D(0.0f, -Tilt); // Vertical tilt on canvas
	MapCanvas->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	MapCanvas->SetRenderTransform(CanvasTransform);

	// Apply tilt/parallax to children individually (for visual depth)
	for (UWidget* Child : MapCanvas->GetAllChildren())
	{
		if (!Child) continue;

		// Optional: only apply to specific types
		if (Child->IsA(UPlanetMarkerWidget::StaticClass()) ||
			Child->IsA(USystemOrbitWidget::StaticClass()))
		{
			FWidgetTransform ChildTransform;
			ChildTransform.Shear = FVector2D(0.0f, -Tilt * 0.5f); // Shallower parallax
			Child->SetRenderTransform(ChildTransform);
		}
	}
}

FPlanetFocusResult SystemMapUtils::CenterOnPlanet(
	const FVector2D& MarkerPosition,
	const FVector2D& MarkerSize,
	const FVector2D& ViewportSize,
	const FVector2D& ContentSize,
	float CurrentZoom,
	float RequestedZoom,
	float Margin)
{
	FPlanetFocusResult Result;

	const FVector2D MarkerCenter = MarkerPosition + MarkerSize * 0.5f;

	UE_LOG(LogTemp, Log, TEXT("CenterOnPlanet:"));
	UE_LOG(LogTemp, Log, TEXT("  MarkerPos: %s, MarkerSize: %s, MarkerCenter: %s"), *MarkerPosition.ToString(), *MarkerSize.ToString(), *MarkerCenter.ToString());
	UE_LOG(LogTemp, Log, TEXT("  ViewportSize: %s, ContentSize: %s, CurrentZoom: %.2f, RequestedZoom: %.2f"), *ViewportSize.ToString(), *ContentSize.ToString(), CurrentZoom, RequestedZoom);

	// Scroll Y clamp
	const float ProposedScrollY = MarkerCenter.Y - ViewportSize.Y * 0.5f;
	const float MaxScrollY = FMath::Max(ContentSize.Y - ViewportSize.Y, 0.f);
	const float MinScrollY = 0.f;
	Result.ScrollOffsetY = FMath::Clamp(ProposedScrollY, MinScrollY, MaxScrollY);

	// Canvas X clamp
	const float ProposedCanvasX = ViewportSize.X * 0.5f - MarkerCenter.X;
	const float MaxCanvasX = Margin;
	const float MinCanvasX = ViewportSize.X - ContentSize.X;
	Result.CanvasOffsetX = FMath::Clamp(ProposedCanvasX, MinCanvasX, MaxCanvasX);

	UE_LOG(LogTemp, Log, TEXT("  ProposedCanvasX: %.1f, ClampedCanvasX: %.1f (MinX: %.1f, MaxX: %.1f)"), ProposedCanvasX, Result.CanvasOffsetX, MinCanvasX, MaxCanvasX);

	// Zoom level
	Result.ZoomLevel = (RequestedZoom > 0.f)
		? ClampZoomLevel(RequestedZoom)
		: CurrentZoom;

	UE_LOG(LogTemp, Log, TEXT("  Final ZoomLevel: %.2f"), Result.ZoomLevel);

	return Result;
}

FVector2D SystemMapUtils::ConvertTopLeftToCenterAnchored(const FVector2D& TopLeftPos, const FVector2D& CanvasSize)
{
	return TopLeftPos - (CanvasSize * 0.5f);
}

UTextureRenderTarget2D* SystemMapUtils::CreateRenderTarget(const FString& Name, int32 Resolution, UObject* Outer)
{
	if (!Outer)
	{
		Outer = GetTransientPackage(); // fallback
	}

	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(Outer, *Name);
	if (!RenderTarget)
	{
		return nullptr;
	}

	RenderTarget->RenderTargetFormat = RTF_RGBA16f; // good for lit previews
	RenderTarget->ClearColor = FLinearColor::Black;
	RenderTarget->bAutoGenerateMips = false;
	RenderTarget->InitAutoFormat(Resolution, Resolution);
	RenderTarget->UpdateResourceImmediate(true);

	return RenderTarget;
}

FBox2D SystemMapUtils::ComputeContentBounds(const TSet<UWidget*>& ContentWidgets, UCanvasPanel* Canvas)
{
	FBox2D Bounds(EForceInit::ForceInit);

	if (!Canvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("ComputeContentBounds: Canvas is null"));
		return Bounds;
	}

	int32 Count = 0;

	for (UWidget* Widget : ContentWidgets)
	{
		if (!Widget)
		{
			UE_LOG(LogTemp, Warning, TEXT("ComputeContentBounds: Null widget skipped"));
			continue;
		}

		if (!Canvas->GetAllChildren().Contains(Widget))
		{
			UE_LOG(LogTemp, Warning, TEXT("ComputeContentBounds: Widget %s not found in canvas"), *Widget->GetName());
			continue;
		}

		if (const UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Widget->Slot))
		{
			const FVector2D Pos = Slot->GetPosition();
			const FVector2D Size = Slot->GetSize();
			const FVector2D Align = Slot->GetAlignment();

			const FVector2D TopLeft = Pos - Size * Align;
			const FVector2D BottomRight = TopLeft + Size;

			UE_LOG(LogTemp, Warning,
				TEXT("[Bounds] Widget=%s | Pos=%s | Size=%s | Align=%s | TL=%s | BR=%s"),
				*Widget->GetName(),
				*Pos.ToString(),
				*Size.ToString(),
				*Align.ToString(),
				*TopLeft.ToString(),
				*BottomRight.ToString());

			Bounds += TopLeft;
			Bounds += BottomRight;
			Count++;
		}
		else
		{
			UE_LOG(LogTemp, Warning,
				TEXT("ComputeContentBounds: Widget %s is not in a CanvasPanelSlot"),
				*Widget->GetName());
		}
	}
	Bounds = Bounds.ExpandBy(FVector2D(400.f, 300.f)); // pad outward

	UE_LOG(LogTemp, Warning,
		TEXT("[ComputeContentBounds] %d valid widgets | Final Bounds: Min=%s Max=%s Size=%s"),
		Count,
		*Bounds.Min.ToString(),
		*Bounds.Max.ToString(),
		*(Bounds.GetSize().ToString()));

	return Bounds;
}

FVector2D SystemMapUtils::ComputeMoonOrbitOffset(
	float OrbitKm,
	float OrbitAngleDegrees,
	float Inclination,
	float OrbitToScreen
)
{
	const float AngleRad = FMath::DegreesToRadians(FMath::Fmod(OrbitAngleDegrees, 360.0f));
	const float Inclined = PlanetOrbitUtils::AmplifyInclination(Inclination, 2.0f);
	const float Radius = OrbitKm / OrbitToScreen;

	return PlanetOrbitUtils::Get2DOrbitPositionWithInclination(Radius, AngleRad, Inclined);
}

void SystemMapUtils::ScheduleSafeCapture(UObject* WorldContext, USceneCaptureComponent2D* Capture)
{
	if (!WorldContext || !Capture) return;

	UWorld* World = WorldContext->GetWorld();
	if (!World) return;

	FTimerHandle TempHandle;
	World->GetTimerManager().SetTimer(TempHandle, [=]()
		{
			if (Capture)
			{
				Capture->CaptureScene();
				UE_LOG(LogTemp, Log, TEXT("Scheduled CaptureScene() fired for %s"), *Capture->GetName());
			}
		}, 0.0f, false);
}

UTextureRenderTarget2D* SystemMapUtils::EnsureRenderTarget(
	UObject* Context,
	const FString& Name,
	int32 Resolution,
	USceneCaptureComponent2D* SceneCapture,
	UObject* Owner)
{
	if (!Context || !SceneCapture)
	{
		UE_LOG(LogTemp, Error, TEXT("EnsureRenderTarget: Missing context or capture"));
		return nullptr;
	}

	UGalaxyManager* Galaxy = UGalaxyManager::Get(Context);
	if (!Galaxy)
	{
		UE_LOG(LogTemp, Error, TEXT("EnsureRenderTarget: GalaxyManager not found"));
		return nullptr;
	}

	// Default to SceneCapture or Transient if no mesh or owner provided
	UObject* RTOuter = Owner ? Owner : SceneCapture;

	UTextureRenderTarget2D* RT = Galaxy->GetOrCreateRenderTarget(Name, Resolution, RTOuter);
	if (!RT)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create RenderTarget for %s"), *Name);
		return nullptr;
	}

	SceneCapture->TextureTarget = RT;

	UE_LOG(LogTemp, Log, TEXT("RenderTarget created for %s | Resolution=%d | Outer=%s"),
		*Name, Resolution, *GetNameSafe(RTOuter));

	return RT;
}

UMaterialInstanceDynamic* SystemMapUtils::CreatePreviewMID(
	UObject* Outer,
	UMaterialInterface* BaseMaterial,
	UTexture* BaseTexture,
	const FString& Label)
{
	if (!BaseMaterial || !Outer)
	{
		UE_LOG(LogTemp, Error, TEXT("CreatePreviewMID failed: Missing base material or outer"));
		return nullptr;
	}

	// Create dynamic instance
	UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMaterial, Outer);
	if (!DynMat)
	{
		UE_LOG(LogTemp, Error, TEXT("CreatePreviewMID failed: UMaterialInstanceDynamic::Create returned null"));
		return nullptr;
	}

	// Give it a unique name for debugging
	const FString MIDName = FString::Printf(TEXT("MID_%s"), *Label);
	DynMat->Rename(*MIDName);

	// Optionally assign the base texture
	if (BaseTexture)
	{
		BaseTexture->UpdateResource();
		DynMat->SetTextureParameterValue("BaseTexture", BaseTexture);
	}

	UE_LOG(LogTemp, Log, TEXT("Created PreviewMID: %s (Label: %s, Texture: %s)"),
		*MIDName,
		*Label,
		*GetNameSafe(BaseTexture));

	return DynMat;
}

UTextureRenderTarget2D* SystemMapUtils::CreateSystemOverviewRenderTarget(
	UWorld* World,
	FVector CaptureLocation,
	FVector CaptureTarget,
	int32 Resolution,
	const FString& Name)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSystemOverviewRenderTarget: World is null"));
		return nullptr;
	}

	// Create the render target
	UTextureRenderTarget2D* RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(
		World,
		Resolution,
		Resolution,
		RTF_RGBA16f
	);

	if (!RenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create system render target"));
		return nullptr;
	}

	RenderTarget->ClearColor = FLinearColor::Black;
	RenderTarget->UpdateResourceImmediate(true);

	// Spawn a temporary SceneCapture actor
	ASceneCapture2D* CaptureActor = World->SpawnActor<ASceneCapture2D>(ASceneCapture2D::StaticClass(), CaptureLocation, FRotator::ZeroRotator);
	if (!CaptureActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn SceneCapture2D"));
		return nullptr;
	}

	USceneCaptureComponent2D* Capture = CaptureActor->GetCaptureComponent2D();
	Capture->SetWorldLocation(CaptureLocation);
	Capture->SetWorldRotation((CaptureTarget - CaptureLocation).Rotation());
	Capture->FOVAngle = 30.f;
	Capture->TextureTarget = RenderTarget;
	Capture->bCaptureEveryFrame = false;
	Capture->bCaptureOnMovement = false;

	// Optional: remove unnecessary render features
	Capture->ShowFlags.SetAtmosphere(false);
	Capture->ShowFlags.SetFog(false);
	Capture->ShowFlags.SetSkyLighting(false);
	Capture->ShowFlags.SetMotionBlur(false);
	Capture->ShowFlags.SetPostProcessing(false);
	
	UE_LOG(LogTemp, Warning, TEXT("SceneCapture using RT: %s"), *GetNameSafe(Capture->TextureTarget));


	// Trigger the capture
	Capture->CaptureScene();

	UE_LOG(LogTemp, Log, TEXT("System overview captured to %s (Size: %dx%d)"), *Name, Resolution, Resolution);
	return RenderTarget;
}

void SystemMapUtils::ApplyRenderTargetToImage(
	UObject* Outer,
	UImage* Image,
	UMaterialInterface* BaseMaterial,
	UTextureRenderTarget2D* RenderTarget,
	FVector2D BrushSize)
{
	if (!Image || !BaseMaterial || !RenderTarget || !Outer)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyRenderTargetToImage: Invalid input"));
		return;
	}

	UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMaterial, Outer);
	if (!DynMat)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create dynamic material"));
		return;
	}

	DynMat->SetTextureParameterValue("InputTexture", RenderTarget);
	Image->SetBrushFromMaterial(DynMat);
	Image->SetBrushSize(BrushSize);

	if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Image->Slot))
	{
		Slot->SetSize(BrushSize);
	}

	UE_LOG(LogTemp, Log, TEXT("RenderTarget applied to image: %s"), *RenderTarget->GetName());
}

FVector SystemMapUtils::ComputePlanetWorldPosition(
	const FVector& StarLocation,
	float OrbitKm,
	float OrbitAngleDeg,
	float InclinationDeg,
	float OrbitToWorldScale
)
{
	const float OrbitRadius = OrbitKm / OrbitToWorldScale;
	const float AngleRad = FMath::DegreesToRadians(OrbitAngleDeg);
	const float InclinationRad = FMath::DegreesToRadians(InclinationDeg);

	const float X = FMath::Cos(AngleRad) * OrbitRadius;
	const float Y = FMath::Sin(AngleRad) * OrbitRadius;
	const float Z = OrbitRadius * FMath::Sin(InclinationRad); // simple vertical tilt

	return StarLocation + FVector(X, Y, Z);
}

UTextureRenderTarget2D* SystemMapUtils::RenderWidgetToTexture(UUserWidget* Widget, int32 Width, int32 Height, float Scale)
{
	if (!Widget)
	{
		UE_LOG(LogTemp, Error, TEXT("RenderWidgetToTexture: Widget is null"));
		return nullptr;
	}

	// Create temporary shared renderer
	TSharedRef<FWidgetRenderer> Renderer = MakeShared<FWidgetRenderer>(true);

	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->RenderTargetFormat = RTF_RGBA8;
	RenderTarget->InitAutoFormat(Width, Height);
	RenderTarget->ClearColor = FLinearColor::Transparent;
	RenderTarget->UpdateResourceImmediate();

	// Draw the widget directly into the RT
	Renderer->DrawWidget(
		RenderTarget,
		Widget->TakeWidget(),
		FVector2D(Width, Height),
		Scale
	);

	UE_LOG(LogTemp, Log, TEXT("Widget rendered to RenderTarget: %dx%d"), Width, Height);
	return RenderTarget;
}
