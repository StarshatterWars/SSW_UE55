#pragma once

#include "CoreMinimal.h"
#include "../Game/GameStructs.h"
#include "SystemBodyPanelActor.h"
#include "PlanetPanelActor.generated.h"

class UMaterialInterface;

UCLASS()
class STARSHATTERWARS_API APlanetPanelActor : public ASystemBodyPanelActor
{
	GENERATED_BODY()

public:
	APlanetPanelActor();

	static APlanetPanelActor* SpawnWithPlanetData(
		UWorld* World,
		const FVector& Location,
		const FRotator& Rotation,
		TSubclassOf<APlanetPanelActor> ActorClass,
		FS_PlanetMap PlanetInfo
	);

	// Data payload
	UPROPERTY()
	FS_PlanetMap PlanetData;

	UPROPERTY(EditDefaultsOnly, Category = "Planet")
	UMaterialInterface* PlanetBaseMaterial = nullptr;

protected:
	// Base hooks
	virtual FString GetBodyName() const override { return PlanetData.Name; }
	virtual float GetBodyRadius() const override { return PlanetData.Radius; }
	virtual float GetBodyTiltDegrees() const override { return PlanetData.Tilt; }
	virtual UMaterialInterface* GetBaseMaterial() const override { return PlanetBaseMaterial; }
};
