// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MoonPanelActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/ConstructorHelpers.h"
#include "../Foundation/SystemMapUtils.h"
#include "../Foundation/MoonUtils.h"
#include "../Game/GalaxyManager.h"
#include "../Actors/PlanetPanelActor.h"

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
	Params.bDeferConstruction = true;  // Defer BeginPlay

	AMoonPanelActor* NewActor = World->SpawnActor<AMoonPanelActor>(ActorClass, Location, Rotation, Params);
	if (!NewActor) return nullptr;

	// Assign moon data before BeginPlay
	NewActor->MoonData = MoonInfo;
	NewActor->InitMoon();

	// Complete spawn
	NewActor->FinishSpawning(FTransform(Rotation, Location));

	return NewActor;
}

// Sets default values
AMoonPanelActor::AMoonPanelActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	MoonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoonMesh"));
	MoonMesh->SetupAttachment(RootScene);
	MoonMesh->SetRelativeScale3D(FVector(3.0f));
	MoonMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	MoonMesh->SetCastShadow(false);
	MoonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MoonMesh->SetStaticMesh(SphereMesh.Object);
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


// Called when the game starts or when spawned
void AMoonPanelActor::BeginPlay()
{
	Super::BeginPlay();
	InitializeMoon();
}

// Called every frame
void AMoonPanelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float Time = GetWorld()->GetTimeSeconds();
	FRotator Spin = MoonUtils::GetMoonRotation(Time, RotationSpeed, MoonData.Tilt);
	MoonMesh->SetRelativeRotation(Spin);
}

void AMoonPanelActor::SetParentPlanet(APlanetPanelActor* InParent)
{
	UE_LOG(LogTemp, Warning, TEXT("SetParentPlanet() Parent: %s -> Moon: %s"), *InParent->PlanetData.Name, *MoonData.Name);
	ParentPlanet = InParent;
}

void AMoonPanelActor::InitMoon()
{
	// Texture
	MoonTexture = MoonUtils::LoadMoonAssetTexture(MoonData.Texture);

	MoonMaterialInstance = SystemMapUtils::CreatePreviewMID(
		this,
		MoonBaseMaterial,
		MoonTexture,
		MoonData.Name
	);

	// Apply to mesh
	MoonMesh->SetMaterial(0, MoonMaterialInstance);

	int32 Resolution = FMath::Clamp(MoonUtils::GetRenderTargetResolutionForRadius(MoonData.Radius), 256, 2048);
	MoonRenderTarget = SystemMapUtils::EnsureRenderTarget(
		this,
		MoonData.Name,
		Resolution,
		SceneCapture,
		MoonMesh
	);

	SceneCapture->TextureTarget = MoonRenderTarget;

	// Capture scene (now safe!)
	MoonMesh->MarkRenderStateDirty();
	SceneCapture->CaptureScene();

	UE_LOG(LogTemp, Warning, TEXT("InitMoon() Moon: %s -> Mat: %s, Tex: %s, RT: %s"),
		*MoonData.Name,
		*GetNameSafe(MoonMaterialInstance),
		*GetNameSafe(MoonTexture),
		*GetNameSafe(MoonRenderTarget));

	// Delay CaptureScene by one tick (safe on all platforms)
	SystemMapUtils::ScheduleSafeCapture(this, SceneCapture);
}

void AMoonPanelActor::InitializeMoon()
{
	float ScaleFactor = MoonUtils::GetMoonUIScale(MoonData.Radius);
	//PlanetMesh->SetRelativeScale3D(FVector(1.0f));

	FRotator AxisTilt = MoonUtils::GetMoonAxisTilt(MoonData.Tilt);
	MoonMesh->SetRelativeRotation(AxisTilt);
}

