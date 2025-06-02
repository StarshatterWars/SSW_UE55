// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_Galaxy struct
#include "../System/SSWGameInstance.h"
#include "../Foundation/SystemMapUtils.h"
#include "Engine/SceneCapture2D.h" 
#include "SectorMap.generated.h"

class UCanvasPanel;
class UScrollBox;
class USizeBox;
class UPlanetMarkerWidget;
class UMoonMarkerWidget;
class USystemOrbitWidget;
class APlanetPanelActor;
class AMoonPanelActor;
class UOperationsScreen;
/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API USectorMap : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY()
	UOperationsScreen* OwningOperationsScreen;

	void SetOwner(UOperationsScreen* Owner) { OwningOperationsScreen = Owner; }
	
	// Called to draw all planets for the current system
	void BuildSectorView(const FS_PlanetMap* ActivePlanet);
	
	void InitSectorCanvas();

		UFUNCTION()
	void HighlightSelectedSystem();

	void HandleCentralPlanetClicked();
	void FocusAndZoomToMoon(UMoonMarkerWidget* Marker);

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* MapCanvas;

	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* OuterCanvas;

	UPROPERTY(meta = (BindWidgetOptional))
	USizeBox* MapCanvasSize;

	UPROPERTY(meta = (BindWidgetOptional))
	FS_PlanetMap PlanetData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<UMoonMarkerWidget> MoonMarkerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<USystemOrbitWidget> OrbitWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<AMoonPanelActor> MoonActorClass;

	UPROPERTY()
	TArray<AMoonPanelActor*> SpawnedMoonActors;

	UFUNCTION()
	void SetZoomLevel(float NewZoom);

private:
	TMap<FString, UPlanetMarkerWidget*> PlanetMarkers;
	TMap<FString, UMoonMarkerWidget*> MoonMarkers;
	TMap<FString, USystemOrbitWidget*> MoonOrbitMarkers;

	// Stores the most recently selected planet marker
	UMoonMarkerWidget* LastSelectedMoonMarker = nullptr;
	
	// Holds per-mmon orbit angle (randomized once per moon per session)
	TMap<FString, float> MoonOrbitAngles;

	float GetDynamicMoonOrbitScale(const TArray<FS_MoonMap>& Moons, float MaxPixelRadius) const;

	void HandleMoonClicked(const FString& MoonName);
	UFUNCTION()
	void CenterOnMoonWidget(UMoonMarkerWidget* Marker, float Zoom);

	void AddMoonOrbitalRing(const FS_MoonMap& Moon);

	void AddMoon(const FS_MoonMap& Moon, APlanetPanelActor* Parent, UPlanetMarkerWidget* ParentWidget);

	void SetMarkerVisibility(bool bVisible);

	SystemMapUtils::FSystemMapDragController DragController;
	
	UPROPERTY()
	FString SelectedMoonName;

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
	bool bIsAnimatingToMoon = false;

	UPROPERTY()
	bool bIsCenteringToMoon = false;
	
	UPROPERTY()
	bool bIsZoomingToMoon = false;

	UPROPERTY()
	float TargetTiltAmount = 0.0f; // Example: 0.2f for subtle tilt
	UPROPERTY()
	float TiltDuration = 0.6f; // time in seconds to fully tilt	
	
	UPROPERTY()
	bool bIsTiltingIn = false;
	UPROPERTY()
	float TiltTime = 0.0f;
	
	UPROPERTY()
	float MovementDelayTime = 0.0f;
	UPROPERTY()
	bool bIsWaitingToProcessMovement = false;
	UPROPERTY()
	float MovementDelayDuration = 0.3f; // seconds (adjust as needed)
	
	UPROPERTY()
	bool bPendingCanvasCenter = false;

	UPROPERTY()
	float MoonFocusTime = 0.f;

	UPROPERTY()
	float MoonFocusDuration = 0.5f; // Half second
	
	UPROPERTY()
	float ORBIT_TO_SCREEN;

	UPROPERTY()
	float OrbitRadius;

	UPROPERTY()
	bool bIsDragging = false;

	UPROPERTY()
	bool bIsDraggingConfirmed = false;

	FVector2D StartCanvasPos;
	FVector2D TargetCanvasPos;

	FVector2D CachedViewportSize = FVector2D::ZeroVector;
	UPROPERTY()
	FVector2D CachedViewportCenter = FVector2D::ZeroVector;
	
	UPROPERTY()
	FVector2D DragStartPos;

	UPROPERTY()
	FVector2D ZoomAnchorLocal = FVector2D::ZeroVector;

	UPROPERTY()
	FVector2D PreZoomCanvasPos = FVector2D::ZeroVector;
	
	UPROPERTY()
	FVector2D CachedCanvasSize = FVector2D(3000.f, 2000.f);
		
	UPROPERTY()
	FVector2D CanvasSize = FVector2D(6000.f, 6000.f);

	UPROPERTY()
	FVector2D CurrentDragOffset = FVector2D::ZeroVector;
};
