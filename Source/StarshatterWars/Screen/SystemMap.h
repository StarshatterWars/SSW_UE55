// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_Galaxy struct
#include "../Foundation/SystemMapUtils.h"
#include "Engine/SceneCapture2D.h" 
#include "SystemMap.generated.h"

class UCanvasPanel;
class UImage;
class UTextureRenderTarget2D;
class UScrollBox;
class USizeBox;
class UPlanetMarkerWidget;
class UMoonMarkerWidget;
class USystemOrbitWidget;
class USystemOverviewWidget;
class UCentralSunWidget;
class ACentralSunActor;
class APlanetPanelActor;
class AMoonPanelActor;
class ASystemOverviewActor;
class UOperationsScreen;

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API USystemMap : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* MapCanvas;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* OverviewImage = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* OuterCanvas;

	UPROPERTY(meta = (BindWidgetOptional))
	USizeBox* MapCanvasSize;

	UPROPERTY(meta = (BindWidgetOptional))
	FS_Galaxy SystemData;
	
	// Called to draw all planets for the current system
	void BuildSystemView(const FS_Galaxy* ActiveSystem);

	UPROPERTY()
	UOperationsScreen* OwningOperationsScreen;

	void SetOwner(UOperationsScreen* Owner) { OwningOperationsScreen = Owner; }

	UFUNCTION()
	void HandleCentralSunClicked();
	UFUNCTION()
	void HandlePlanetSelected(FS_PlanetMap Planet);

	UPROPERTY()
	UMaterialInterface* OverlayMaterial;

	UFUNCTION()
	void InitMapCanvas();

	void FocusAndZoomToPlanet(UPlanetMarkerWidget* Marker);

	void SetOverlay();
	void SetMarkerVisibility(bool bVisible);

	void ClearSystemView();
	void EnsureOverviewImage();
	void UpdateOverviewBrush();

	TMap<FString, UPlanetMarkerWidget*> PlanetMarkers;
	TMap<FString, USystemOrbitWidget*> PlanetOrbitMarkers;

	// Stores the most recently selected planet marker
	UPlanetMarkerWidget* LastSelectedMarker = nullptr;

	// Holds per-planet orbit angle (randomized once per planet per session)
	TMap<FString, float> PlanetOrbitAngles;

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;	
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;


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

	UPROPERTY()
	TArray<APlanetPanelActor*> SpawnedPlanetActors;
	
	UFUNCTION()
	void SetZoomLevel(float NewZoom);
	UFUNCTION()
	void ZoomToFitAllPlanets();

	UPROPERTY()
	FVector2D ScreenRenderSize = FVector2D(4096.f, 4096.f);

private:	
	UCentralSunWidget* CentralStarMarker;
	
	float GetDynamicOrbitScale(const TArray<FS_PlanetMap>& Planets, float MaxPixelRadius) const;

	const float OrbitTiltY = 0.6f; // 60% vertical scale for orbital ellipse
	UPROPERTY()
    FVector2D ScreenOffset;
	UPROPERTY()
    FVector2D PanelSize;
	UPROPERTY()
	ACentralSunActor* SunActor;
	UPROPERTY()
	UCentralSunWidget* StarWidget;

	UPROPERTY()
	APlanetPanelActor* PlanetActor;

	UPROPERTY()
	UPlanetMarkerWidget* PlanetWidget;

	UPROPERTY()
	AMoonPanelActor* MoonActor;

	UPROPERTY()
	UMoonMarkerWidget* MoonWidget;

	UPROPERTY()
	FVector2D ZoomCenter = FVector2D(0.5f, 0.5f); // Default center

	// Store a persistent system render overview texture
	UPROPERTY()
	UTextureRenderTarget2D* SystemOverviewRT = nullptr;

	// For tracking drag state
	
	UPROPERTY()
	float InitialScrollOffset = 0.f;
	
	UPROPERTY()
	FString SelectedPlanetName;

	UFUNCTION()
	void HandlePlanetClicked(const FString& PlanetName);

	UFUNCTION()
	void CenterOnPlanetWidget(UPlanetMarkerWidget* Marker, float Zoom);
	
	UFUNCTION()
	void ApplyTiltToMapCanvas(float TiltAmount);
	
	void AddCentralStar(const FS_Galaxy* Star);
	void AddPlanetOrbitalRing(const FS_PlanetMap& Planet);	
	void AddPlanet(const FS_PlanetMap& Planet);


	UFUNCTION()
	void HighlightSelectedSystem();

	UPROPERTY()
	FVector2D StartCanvasPosition = FVector2D::ZeroVector;

	UPROPERTY()
	FVector2D CachedCanvasSize = FVector2D(3000.f, 2000.f);

	UPROPERTY()
	FVector2D TargetCanvasPosition = FVector2D::ZeroVector;
	UPROPERTY()
	FVector2D MapCenterOffset = FVector2D(50.f, -100.f);
	UPROPERTY()
	FVector2D CurrentDragOffset = FVector2D::ZeroVector;
	UPROPERTY()
	FVector2D InitialCanvasOffset = FVector2D::ZeroVector;
	UPROPERTY()
	FVector2D CachedViewportSize = FVector2D::ZeroVector;
	UPROPERTY()
	FVector2D CachedViewportCenter = FVector2D::ZeroVector;

	// Center of zoom (in screen space ? converted to canvas local)
	UPROPERTY()
	FVector2D ZoomAnchorLocal = FVector2D::ZeroVector;

	// Cached position before zoom begins
	UPROPERTY()
	FVector2D PreZoomCanvasPos = FVector2D::ZeroVector;
	UPROPERTY()
	FVector2D DragStartPos;

	UPROPERTY()
	FVector2D CanvasSize = FVector2D(6000.f, 6000.f);

	UPROPERTY()
	TSet<UWidget*> TrackedMapWidgets;

	SystemMapUtils::FSystemMapDragController DragController;

	UPROPERTY()
	float StartZoomLevel = 1.0f;
	UPROPERTY()
	float TargetZoom = 1.0f;
	UPROPERTY()
	float ZoomInterpSpeed = 5.f;
	UPROPERTY()
	float MinZoom = 0.5f;
	UPROPERTY()
	float MaxZoom = 4.0f;
	UPROPERTY()
	float ZoomStep = 0.1f;
	UPROPERTY()
	float ZoomLevel = 1.0f; // 1.0 = 100%
	
	UPROPERTY()
	float CenterLerpSpeed = 10.0f; // adjustable
	UPROPERTY()
	bool bIsCenteringToPlanet = false;

	UPROPERTY()
	bool bIsAnimatingToPlanet = false;

	UPROPERTY()
	bool bOverviewImageAdded = false;
	
	UPROPERTY()
	float PlanetFocusTime = 0.f;
	
	UPROPERTY()
	float PlanetFocusDuration = 0.5f; // Half second

	FVector2D StartCanvasPos;
	FVector2D TargetCanvasPos;
	
	UPROPERTY()
	bool bIsZoomingToPlanet = false;
	
	UPROPERTY()
	float CenterAnimTime = 0.0f;
	UPROPERTY()
	float ZoomAnimTime = 0.0f;
	
	UPROPERTY()
	float CenterAnimDuration = 0.5f;
	UPROPERTY()
	float ZoomAnimDuration = 0.5f;
	
	UPROPERTY()
	float TargetTiltAmount = 0.0f; // Example: 0.2f for subtle tilt
	UPROPERTY()
	bool bIsTiltingIn = false;
	UPROPERTY()
	float TiltTime = 0.0f;
	UPROPERTY()
	float TiltDuration = 0.6f; // time in seconds to fully tilt	
	UPROPERTY()
	float MovementDelayTime = 0.0f;
	UPROPERTY()
	bool bIsWaitingToProcessMovement = false;
	UPROPERTY()
	float MovementDelayDuration = 0.3f; // seconds (adjust as needed)
	UPROPERTY()
	bool bPendingCanvasCenter = false;
	
	UPROPERTY()
	bool bPendingLayoutFinalize = false;
	
	UPROPERTY()
	bool bPendingInitialLayout = false;
	
	UPROPERTY()
	bool bIsDragging = false;

	UPROPERTY()
	bool bIsDraggingConfirmed = false;

	UPROPERTY()
	bool bLayoutInitialized = false;

	FTimerHandle LayoutRetryTimer;

	UPROPERTY()
	float ORBIT_TO_SCREEN;

	UPROPERTY()
	float OrbitRadius;
	
	UPROPERTY()
	ASceneCapture2D* SceneCaptureActor = nullptr;
};