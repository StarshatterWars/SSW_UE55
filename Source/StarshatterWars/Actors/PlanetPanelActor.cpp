#include "PlanetPanelActor.h"
#include "../Foundation/PlanetUtils.h"
#include "../Foundation/SystemMapUtils.h"

APlanetPanelActor::APlanetPanelActor()
{
	// Use the base tick/tilt composition policy
	RotationSpeed = 20.f;
	bApplyUIScale = false;

	// Since PlanetUtils likely already bakes tilt into rotation, you may set:
	// bApplyAxisTilt = false;
	// and handle tilt in ComputeSpinRotation().
	//
	// For safety and to avoid double-tilt, default to false and use your existing util rotation:
	bApplyAxisTilt = false;
}

APlanetPanelActor* APlanetPanelActor::SpawnWithPlanetData(
	UWorld* World,
	const FVector& Location,
	const FRotator& Rotation,
	TSubclassOf<APlanetPanelActor> ActorClass,
	FS_PlanetMap PlanetInfo
)
{
	if (!World || !*ActorClass) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bDeferConstruction = true;

	APlanetPanelActor* NewActor = World->SpawnActor<APlanetPanelActor>(ActorClass, Location, Rotation, Params);
	if (!NewActor) return nullptr;

	NewActor->PlanetData = PlanetInfo;
	NewActor->BodyTilt = PlanetInfo.Tilt;
	NewActor->BodyRadius = PlanetInfo.Radius;
	NewActor->BodyName = PlanetInfo.Name;
	NewActor->TextureName = PlanetInfo.Texture;
	NewActor->InitBody("Planet");
	NewActor->FinishSpawning(FTransform(Rotation, Location));

	return NewActor;
}



