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

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API USystemMap : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called to build the system view
	void BuildSystemView(const TArray<FS_PlanetMap>& Planets, const FString& CurrentSystemName);

protected:
	void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MapCanvas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<UPlanetMarkerWidget> PlanetMarkerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
	TSubclassOf<USystemOrbitWidget> OrbitWidgetClass;

private:
	TMap<FString, UPlanetMarkerWidget*> PlanetMarkers;
	const float KM_TO_SCREEN = 1000.f; // 1000 km = 1 pixel (adjust as needed)
};
