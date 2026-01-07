// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         SystemMap.h	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_Galaxy, FS_PlanetMap, etc.
#include "../Foundation/SystemMapUtils.h"
#include "../Space/SystemOverview.h"
#include "SystemMap.generated.h"

class UCanvasPanel;
class UCanvasPanelSlot;
class USizeBox;
class UImage;
class UTextureRenderTarget2D;

class UPlanetMarkerWidget;
class UMoonMarkerWidget;
class USystemOrbitWidget;
class UCentralSunWidget;

class ACentralSunActor;
class APlanetPanelActor;
class AMoonPanelActor;

class UOperationsScreen;

// Overview body struct lives in SystemOverview.h; forward-declare only
struct FOverviewBody;

/**
 * System map widget: builds orbits + markers AND renders a system overview RT.
 */
UCLASS()
class STARSHATTERWARS_API USystemMap : public UUserWidget
{
	GENERATED_BODY()

public:
	// ====== Bound widgets (optional) ======
	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* MapCanvas = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* OuterCanvas = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	USizeBox* MapCanvasSize = nullptr;

	// Image used to show the overview RT (either bound in BP or created at runtime)
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* OverviewImage = nullptr;

	// ====== External owner ======
	UPROPERTY()
	UOperationsScreen* OwningOperationsScreen = nullptr;

	void SetOwner(UOperationsScreen* Owner) { OwningOperationsScreen = Owner; }

	// ====== System build ======
	void BuildSystemView(const FS_Galaxy* ActiveSystem);

	// ====== Navigation ======
	UFUNCTION()
	void HandleCentralSunClicked();

	UFUNCTION()
	void HandlePlanetSelected(FS_PlanetMap Planet);

	// ====== Canvas init / cleanup ======
	UFUNCTION()
	void InitMapCanvas();

	void ClearSystemView();

	// ====== Marker interaction ======
	void FocusAndZoomToPlanet(UPlanetMarkerWidget* Marker);
	void SetMarkerVisibility(bool bVisible);

	// ====== Overview RT pipeline ======
	// Ensures OverviewImage exists (bound or created) and is placed in the canvas
	void EnsureOverviewImage();

	// Creates RT if missing (owned by SystemMap)
	void EnsureOverviewRenderTarget(int32 Size);

	// Spawns overview actor if missing (owned by SystemMap)
	void EnsureOverviewActor();

	// Builds diorama + capture once + updates UMG brush
	void RebuildAndCaptureOverview(const FS_Galaxy* ActiveSystem);

	// Push RT into OverviewImage brush
	void UpdateOverviewBrush();

	// ====== Maps ======
	TMap<FString, UPlanetMarkerWidget*> PlanetMarkers;
	TMap<FString, USystemOrbitWidget*> PlanetOrbitMarkers;

	// Stores the most recently selected planet marker
	UPROPERTY()
	UPlanetMarkerWidget* LastSelectedMarker = nullptr;

	// Holds per-planet orbit angle (randomized once per planet per session)
	UPROPERTY()
	TMap<FString, float> PlanetOrbitAngles;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// ====== BP-configurable classes ======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<UMoonMarkerWidget> MoonMarkerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<UPlanetMarkerWidget> PlanetMarkerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<USystemOrbitWidget> OrbitWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<UCentralSunWidget> StarWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
	TSubclassOf<ACentralSunActor> SunActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<APlanetPanelActor> PlanetActorClass;

	// NEW: overview actor class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overview")
	TSubclassOf<ASystemOverview> SystemOverviewActorClass;

	// NEW: RT resolution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overview")
	int32 OverviewRTSize = 1024;

	UPROPERTY()
	TArray<APlanetPanelActor*> SpawnedPlanetActors;

	// ====== Zoom / Fit (optional) ======
	UFUNCTION()
	void SetZoomLevel(float NewZoom);

	UFUNCTION()
	void ZoomToFitAllPlanets();

private:
	// ====== Central star ======
	UPROPERTY()
	UCentralSunWidget* CentralStarMarker = nullptr;

	UPROPERTY()
	ACentralSunActor* SunActor = nullptr;

	UPROPERTY()
	UCentralSunWidget* StarWidget = nullptr;

	// ====== Overview RT owned by this widget ======
	UPROPERTY(Transient)
	UTextureRenderTarget2D* SystemOverviewRT = nullptr;

	UPROPERTY(Transient)
	ASystemOverview* SystemOverviewActor = nullptr;

	UPROPERTY()
	bool bOverviewImageAdded = false;

	FVector2D GetViewportCenterInCanvasSpace() const;

	// ====== Helpers ======
	float GetDynamicOrbitScale(const TArray<FS_PlanetMap>& Planets, float MaxPixelRadius) const;

	void AddCentralStar(const FS_Galaxy* Star);
	void AddPlanetOrbitalRing(const FS_PlanetMap& Planet);
	void AddPlanet(const FS_PlanetMap& Planet);

	// Build overview bodies from FS_Galaxy into the diorama format
	void BuildOverviewBodiesFromSystem(const FS_Galaxy* ActiveSystem, TArray<FOverviewBody>& OutBodies) const;

	UFUNCTION()
	void HighlightSelectedSystem();

	UFUNCTION()
	void HandlePlanetClicked(const FString& PlanetName);

	UFUNCTION()
	void CenterOnPlanetWidget(UPlanetMarkerWidget* Marker, float Zoom);

	UFUNCTION()
	void ApplyTiltToMapCanvas(float TiltAmount);

	// ====== Layout / drag / animation state ======
	UPROPERTY()
	FVector2D CachedCanvasSize = FVector2D(3000.f, 2000.f);

	UPROPERTY()
	FVector2D CachedViewportSize = FVector2D::ZeroVector;

	UPROPERTY()
	FVector2D DragStartPos = FVector2D::ZeroVector;

	UPROPERTY()
	FVector2D CanvasSize = FVector2D(6000.f, 6000.f);

	SystemMapUtils::FSystemMapDragController DragController;

	UPROPERTY()
	float MinZoom = 0.5f;

	UPROPERTY()
	float MaxZoom = 4.0f;

	UPROPERTY()
	float ZoomStep = 0.1f;

	UPROPERTY()
	float ZoomLevel = 1.0f;

	UPROPERTY()
	bool bIsDragging = false;

	UPROPERTY()
	bool bIsAnimatingToPlanet = false;

	UPROPERTY()
	float PlanetFocusTime = 0.f;

	UPROPERTY()
	float PlanetFocusDuration = 0.5f;

	FVector2D StartCanvasPos = FVector2D::ZeroVector;
	FVector2D TargetCanvasPos = FVector2D::ZeroVector;

	UPROPERTY()
	float TargetTiltAmount = 0.0f;

	UPROPERTY()
	bool bIsTiltingIn = false;

	UPROPERTY()
	float TiltTime = 0.0f;

	UPROPERTY()
	float TiltDuration = 0.6f;

	UPROPERTY()
	float MovementDelayTime = 0.0f;

	UPROPERTY()
	bool bIsWaitingToProcessMovement = false;

	UPROPERTY()
	float MovementDelayDuration = 0.3f;

	UPROPERTY()
	bool bPendingCanvasCenter = false;

	UPROPERTY()
	float ORBIT_TO_SCREEN = 1.0f;

	UPROPERTY()
	float OrbitRadius = 0.f;
};
