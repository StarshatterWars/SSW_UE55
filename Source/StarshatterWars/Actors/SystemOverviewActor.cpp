// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "SystemOverviewActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "../Foundation/SystemMapUtils.h"


void ASystemOverviewActor::BeginPlay()
{
	Super::BeginPlay();
}

ASystemOverviewActor* ASystemOverviewActor::SpawnWithSystemData(UWorld* World, const FVector& Location, const FRotator& Rotation, TSubclassOf<ASystemOverviewActor> ActorClass, UTextureRenderTarget2D* RT)
{
	if (!World || !*ActorClass) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bDeferConstruction = true;  // Defer BeginPlay

	ASystemOverviewActor* NewActor = World->SpawnActor<ASystemOverviewActor>(ActorClass, Location, Rotation, Params);
	if (!NewActor) return nullptr;

	FVector CaptureLocation(0.f, 0.f, 2000.f);
	FRotator CaptureRotation(-90.f, 0.f, 0.f);

	// Assign planet data before BeginPlay
	NewActor->InitSystem(RT, CaptureLocation, CaptureRotation);

	// Complete spawn
	NewActor->FinishSpawning(FTransform(Rotation, Location));

	return NewActor;
}

// Sets default values
ASystemOverviewActor::ASystemOverviewActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	RootComponent = SceneCapture;

	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
}


void ASystemOverviewActor::InitSystem(UTextureRenderTarget2D* Target, const FVector& LookFrom, const FRotator& LookAt)
{	
	OverviewRT = Target;
	SceneCapture->TextureTarget = Target;
	SceneCapture->SetWorldLocation(FVector(0.f, 0.f, 2000.f)); // Z = height
	SceneCapture->SetWorldRotation(FRotator(-90.f, 0.f, 0.f)); // Look straight down

	//SceneCapture->SetWorldLocation(LookFrom);
	//SceneCapture->SetWorldRotation(LookAt);

	// Optional orthographic for 2D systems
	SceneCapture->ProjectionType = ECameraProjectionMode::Orthographic;
	SceneCapture->OrthoWidth = 3000.0f;

	// Delay CaptureScene by one tick (safe on all platforms)
	SystemMapUtils::ScheduleSafeCapture(this, SceneCapture);
}
