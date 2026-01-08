// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CentralPlanetActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/ConstructorHelpers.h"
#include "../Foundation/PlanetUtils.h"
#include "../Foundation/SystemMapUtils.h"
#include "../Game/GalaxyManager.h"


ACentralPlanetActor* ACentralPlanetActor::SpawnWithPlanetData(
	UWorld* World,
	const FVector& Location,
	const FRotator& Rotation,
	TSubclassOf<ACentralPlanetActor> ActorClass,
	FS_PlanetMap PlanetInfo
)
{
	if (!World || !*ActorClass) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bDeferConstruction = true;  // Defer BeginPlay

	ACentralPlanetActor* NewActor = World->SpawnActor<ACentralPlanetActor>(ActorClass, Location, Rotation, Params);
	if (!NewActor) return nullptr;

	// Assign planet data before BeginPlay
	NewActor->PlanetData = PlanetInfo;
	NewActor->InitPlanet();

	// Complete spawn
	NewActor->FinishSpawning(FTransform(Rotation, Location));

	return NewActor;
}

ACentralPlanetActor::ACentralPlanetActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	PlanetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlanetMesh"));
	PlanetMesh->SetupAttachment(RootScene);
	PlanetMesh->SetRelativeScale3D(FVector(3.0f));
	PlanetMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	PlanetMesh->SetCastShadow(false);
	PlanetMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		PlanetMesh->SetStaticMesh(SphereMesh.Object);
	}

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootScene);
	SceneCapture->SetRelativeLocation(FVector(-600.f, 0.f, 0.f));
	SceneCapture->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	SceneCapture->FOVAngle = 60.f;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = true;
	SceneCapture->ShowFlags.SetAtmosphere(false);
	SceneCapture->ShowFlags.SetFog(false);
	SceneCapture->ShowFlags.SetSkyLighting(false);
	SceneCapture->ShowFlags.SetAmbientOcclusion(false);
	SceneCapture->ShowFlags.SetMotionBlur(false);
	SceneCapture->ShowFlags.SetScreenSpaceReflections(false);
	SceneCapture->ShowFlags.SetPostProcessing(false);
	isSceneDelay = false;
}

void ACentralPlanetActor::BeginPlay()
{
	Super::BeginPlay();
	InitializePlanet();
}

void ACentralPlanetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();
	FRotator Spin = SystemMapUtils::GetBodyRotation(Time, RotationSpeed, PlanetData.Tilt);
	PlanetMesh->SetRelativeRotation(Spin);
}

void ACentralPlanetActor::InitPlanet()
{
	// Texture
	PlanetTexture = SystemMapUtils::LoadBodyAssetTexture("Planet", PlanetData.Texture);

	PlanetRenderTarget = SystemMapUtils::EnsureRenderTarget(
		this,
		PlanetData.Name +".Planet",
		SystemMapUtils::GetRenderTargetResolutionForRadius(1000.0, 150000.0, PlanetData.Radius),
		SceneCapture,
		PlanetMesh
	);

	UMaterialInstanceDynamic* DynMat = SystemMapUtils::CreatePreviewMID(
		this,
		PlanetBaseMaterial,
		PlanetTexture,
		PlanetData.Name + ".Planet"
	);

	// Apply to mesh
	PlanetMesh->SetMaterial(0, DynMat);
	PlanetMaterialInstance = DynMat;

	if (bUseSystemOverviewOnly == false)
	{
		SceneCapture->TextureTarget = PlanetRenderTarget;

		// Capture scene (now safe!)
		PlanetMesh->MarkRenderStateDirty();
		SceneCapture->CaptureScene();

		UE_LOG(LogTemp, Warning, TEXT("InitPlanet() Planet: %s -> Mat: %s, Tex: %s, RT: %s"),
			*PlanetData.Name,
			*GetNameSafe(DynMat),
			*GetNameSafe(PlanetTexture),
			*GetNameSafe(PlanetRenderTarget));

		// Delay CaptureScene by one tick (safe on all platforms)
		SystemMapUtils::ScheduleSafeCapture(this, SceneCapture);
	}
}

void ACentralPlanetActor::InitializePlanet()
{
	float ScaleFactor = SystemMapUtils::GetBodyUIScale(2000.0, 150000.0, PlanetData.Radius);
	//PlanetMesh->SetRelativeScale3D(FVector(1.0f));

	FRotator AxisTilt = PlanetUtils::GetPlanetAxisTilt(PlanetData.Tilt);
	PlanetMesh->SetRelativeRotation(AxisTilt);
}