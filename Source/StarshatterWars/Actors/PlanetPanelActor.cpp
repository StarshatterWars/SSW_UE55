// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlanetPanelActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/ConstructorHelpers.h"
#include "../Foundation/PlanetUtils.h"
#include "../Game/GalaxyManager.h"
#include "Windows/MinWindows.h"


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
	Params.bDeferConstruction = true;  // Defer BeginPlay

	APlanetPanelActor* NewActor = World->SpawnActor<APlanetPanelActor>(ActorClass, Location, Rotation, Params);
	if (!NewActor) return nullptr;

	// Assign planet data before BeginPlay
	NewActor->PlanetData = PlanetInfo;
	NewActor->InitPlanet();

	// Complete spawn
	NewActor->FinishSpawning(FTransform(Rotation, Location));

	return NewActor;
}

APlanetPanelActor::APlanetPanelActor()
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

void APlanetPanelActor::BeginPlay()
{
	Super::BeginPlay();
	InitializePlanet();
}

void APlanetPanelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();
	FRotator Spin = PlanetUtils::GetPlanetRotation(Time, RotationSpeed, PlanetData.Tilt);
	PlanetMesh->SetRelativeRotation(Spin);
}

void APlanetPanelActor::AssignScreenCapture()
{
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (SceneCapture)
		{
			PlanetMesh->MarkRenderStateDirty();
			SceneCapture->CaptureScene();
			
			// Log confirmation
			UE_LOG(LogTemp, Warning, TEXT("RenderTarget used: %s [%p] for planet %s"),
				*RTName,
				PlanetRenderTarget,
				*PlanetData.Name);
		}
	}), 0.05f, false); // 50ms delay — adjust as needed
}

void APlanetPanelActor::EnsureRenderTarget()
{
	// Generate unique name based on planet name and a random ID
	RTName = FString::Printf(TEXT("RT_Planet_%s_%d"), *PlanetData.Name, FMath::RandRange(1000, 9999));

	// Create the render target
	int32 Resolution = PlanetUtils::GetRenderTargetResolutionForRadius(PlanetData.Radius);
	UGalaxyManager* Galaxy = UGalaxyManager::Get(this); // use your accessor
	PlanetRenderTarget = Galaxy->GetOrCreateRenderTarget(PlanetData.Name, Resolution);
	//PlanetRenderTarget = PlanetUtils::CreatePlanetRenderTarget(RTName, PlanetMesh, Resolution);

	if (!PlanetRenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create RenderTarget for planet %s"), *PlanetData.Name);
		return;
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("RenderTarget created for planet %s"), *PlanetData.Name);
	}

	SceneCapture->TextureTarget = PlanetRenderTarget;
}

void APlanetPanelActor::InitPlanet() 
{
	// Texture
	PlanetTexture = PlanetUtils::LoadPlanetAssetTexture(PlanetData.Texture);

	// Ensure a unique render target before using SceneCapture
	EnsureRenderTarget();

	// Create dynamic material
	UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(PlanetBaseMaterial, PlanetMesh);
	DynMat->Rename(*FString::Printf(TEXT("MID_%s_%d"), *PlanetData.Name, FMath::RandRange(1000, 9999)));
	if (PlanetTexture)
	{
		#define UpdateResource UpdateResource
		PlanetTexture->UpdateResource();
		DynMat->SetTextureParameterValue("BaseTexture", PlanetTexture);
	}

	// Apply to mesh
	PlanetMesh->SetMaterial(0, DynMat);
	PlanetMaterialInstance = DynMat;

	// Capture scene (now safe!)
	PlanetMesh->MarkRenderStateDirty();
	SceneCapture->CaptureScene();

	UE_LOG(LogTemp, Warning, TEXT("InitPlanet() Planet: %s -> Mat: %s, Tex: %s, RT: %s"),
		*PlanetData.Name,
		*GetNameSafe(DynMat),
		*GetNameSafe(PlanetTexture),
		*GetNameSafe(PlanetRenderTarget));
}

void APlanetPanelActor::InitializePlanet()
{
	float ScaleFactor = PlanetUtils::GetPlanetUIScale(PlanetData.Radius);
	//PlanetMesh->SetRelativeScale3D(FVector(1.0f));

	FRotator AxisTilt = PlanetUtils::GetPlanetAxisTilt(PlanetData.Tilt);
	PlanetMesh->SetRelativeRotation(AxisTilt);
}

