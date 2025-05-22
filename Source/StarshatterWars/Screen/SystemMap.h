// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/GameStructs.h" // FS_Galaxy struct
#include "../System/SSWGameInstance.h"
#include "SystemMap.generated.h"

class UCanvasPanel;
class UPlanetMarkerWidget;
class USystemOrbitWidget;
class UCentralSunWidget;
class ACentralSunActor;

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API USystemMap : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called to draw all planets for the current system
	void BuildSystemView(const FS_Galaxy* ActiveSystem);


protected:
	void NativeConstruct() override;
	void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MapCanvas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<UPlanetMarkerWidget> PlanetMarkerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<USystemOrbitWidget> OrbitWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<UCentralSunWidget> StarWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
	TSubclassOf<ACentralSunActor> SunActorClass;

private:
	TMap<FString, UPlanetMarkerWidget*> PlanetMarkers;

	// Holds per-planet orbit angle (randomized once per planet per session)
	TMap<FString, float> PlanetOrbitAngles;

	float GetDynamicOrbitScale(const TArray<FS_PlanetMap>& Planets, float MaxPixelRadius) const;
	const float OrbitTiltY = 0.6f; // 60% vertical scale for orbital ellipse
	UPROPERTY()
    FVector2D ScreenOffset;
	UPROPERTY()
	ACentralSunActor* SunActor;
	UPROPERTY()
	UCentralSunWidget* StarWidget;
};