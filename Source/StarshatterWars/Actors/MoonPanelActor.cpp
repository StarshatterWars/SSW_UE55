#include "MoonPanelActor.h"
#include "PlanetPanelActor.h"
#include "../Foundation/SystemMapUtils.h"

AMoonPanelActor::AMoonPanelActor()
{
	RotationSpeed = 20.f;
	bApplyUIScale = true;

	// Avoid double-tilt if MoonUtils already includes tilt in rotation:
	bApplyAxisTilt = false;
}

AMoonPanelActor* AMoonPanelActor::SpawnWithMoonData(
	UWorld* World,
	const FVector& Location,
	const FRotator& Rotation,
	TSubclassOf<AMoonPanelActor> ActorClass,
	FS_MoonMap MoonInfo
)
{
	if (!World || !*ActorClass) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bDeferConstruction = true;

	AMoonPanelActor* NewActor = World->SpawnActor<AMoonPanelActor>(ActorClass, Location, Rotation, Params);
	if (!NewActor) return nullptr;

	NewActor->MoonData = MoonInfo;
	NewActor->BodyTilt = MoonInfo.Tilt;
	NewActor->BodyRadius = MoonInfo.Radius;
	NewActor->BodyName = MoonInfo.Name;
	NewActor->BodyType = EBodyUISizeClass::Moon;
	NewActor->TextureName = MoonInfo.Texture;
	NewActor->InitBody("Moon");
	NewActor->FinishSpawning(FTransform(Rotation, Location));

	return NewActor;
}

void AMoonPanelActor::SetParentPlanet(APlanetPanelActor* InParent)
{
	ParentPlanet = InParent;

	const FString ParentName = InParent ? InParent->PlanetData.Name : TEXT("None");
	UE_LOG(LogTemp, Warning, TEXT("SetParentPlanet() Parent: %s -> Moon: %s"), *ParentName, *MoonData.Name);
}


