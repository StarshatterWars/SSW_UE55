// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlanetPanelActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "../System/SSWGameInstance.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
APlanetPanelActor::APlanetPanelActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Root Component
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	// Sun Mesh
	PlanetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SunMesh"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		PlanetMesh->SetStaticMesh(SphereMesh.Object);
	}

	PlanetMesh->SetupAttachment(RootScene);
	PlanetMesh->SetRelativeScale3D(FVector(3.0f));
	PlanetMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	PlanetMesh->SetCastShadow(false);

	// Scene Capture
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootComponent);
	SceneCapture->SetRelativeLocation(FVector(-600.f, 0.f, 0.f));
	SceneCapture->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	SceneCapture->FOVAngle = 60.f;

	// Use final color with alpha
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	// Flags to avoid black sky/background
	SceneCapture->ShowFlags.SetAtmosphere(false);
	SceneCapture->ShowFlags.SetFog(false);
	SceneCapture->ShowFlags.SetSkyLighting(false);
	SceneCapture->ShowFlags.SetAmbientOcclusion(false);
	SceneCapture->ShowFlags.SetMotionBlur(false);
	SceneCapture->ShowFlags.SetScreenSpaceReflections(false);
	SceneCapture->ShowFlags.SetDirectionalLights(true);
	SceneCapture->ShowFlags.SetPostProcessing(false);
	SceneCapture->ShowFlags.SetMaterials(true);
	SceneCapture->bCaptureEveryFrame = false;      // recommended for manual capture
	SceneCapture->bCaptureOnMovement = true;       // helps with refresh on transform change
	SceneCapture->TextureTarget = PlanetRenderTarget;
}


APlanetPanelActor* APlanetPanelActor::SpawnWithPlanetData(UWorld* World, const FVector& Location, const FRotator& Rotation, TSubclassOf<APlanetPanelActor> ActorClass, FS_PlanetMap& PlanetInfo)
{
	if (!World || !*ActorClass) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Defer construction to allow property assignment before BeginPlay
	Params.bDeferConstruction = true;

	// Spawn actor, but defer BeginPlay
	APlanetPanelActor* NewActor = World->SpawnActor<APlanetPanelActor>(ActorClass, Location, Rotation, Params);
	if (!NewActor) return nullptr;

	NewActor->PlanetInfo = PlanetInfo;
	// Resume construction, now BeginPlay will see the correct value
	NewActor->FinishSpawning(FTransform(Rotation, Location));

	return NewActor;
}

// Called when the game starts or when spawned
void APlanetPanelActor::BeginPlay()
{
	Super::BeginPlay();

	CurrentRotation = GetActorRotation();
	UE_LOG(LogTemp, Warning, TEXT("APlanetPanelActor::BeginPlay() called"));

	EnsureRenderTarget();

	// Assign render target if missing
	if (SceneCapture && !SceneCapture->TextureTarget)
	{
		SceneCapture->TextureTarget = PlanetRenderTarget;
	}

	if (!PlanetMaterialInstance && PlanetBaseMaterial && PlanetMesh)
	{
		PlanetMaterialInstance = UMaterialInstanceDynamic::Create(PlanetBaseMaterial, this);
		PlanetMesh->SetMaterial(0, PlanetMaterialInstance);

		// Force update
		SceneCapture->bCaptureEveryFrame = false;
		SceneCapture->bCaptureOnMovement = true;

		// Nudge to force re-render

		UE_LOG(LogTemp, Warning, TEXT("PlanetMaterialInstance: %s"), *GetNameSafe(PlanetMaterialInstance));
		UE_LOG(LogTemp, Warning, TEXT("Mesh Material: %s"), *GetNameSafe(PlanetMesh->GetMaterial(0)));

		// Force redraw
		PlanetMesh->SetVisibility(false, true);
		PlanetMesh->SetVisibility(true, true);
		RefreshSceneCapture();
	}
}

// Called every frame
void APlanetPanelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));

	RefreshSceneCapture();
}

void APlanetPanelActor::RefreshSceneCapture()
{
	// Delay CaptureScene by one frame
	FTimerHandle DelayHandle;
	GetWorldTimerManager().SetTimer(DelayHandle, [this]()
		{
			if (SceneCapture)
			{
				SceneCapture->CaptureScene();
			}
		}, 0.0f, false); // 0.0 = delay to next tick

	PlanetMesh->MarkRenderStateDirty();
	SceneCapture->SetRelativeRotation(SceneCapture->GetRelativeRotation() + FRotator(1, 0, 0));
	SceneCapture->SetRelativeRotation(SceneCapture->GetRelativeRotation() - FRotator(1, 0, 0));
	UE_LOG(LogTemp, Warning, TEXT("Refreshing SceneCapture"));
	SceneCapture->CaptureScene();
}

void APlanetPanelActor::EnsureRenderTarget()
{
	// 1. Create if null
	if (!PlanetRenderTarget)
	{
		PlanetRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("RT_SunRenderTarget"));
		PlanetRenderTarget->RenderTargetFormat = RTF_RGBA8;
		PlanetRenderTarget->ClearColor = FLinearColor::Black;
		PlanetRenderTarget->InitAutoFormat(64, 64); // or your preferred size
		PlanetRenderTarget->UTextureRenderTarget2D::UpdateResourceImmediate();

		UE_LOG(LogTemp, Log, TEXT("PlanetRenderTarget created."));
	}

	// 2. Assign to SceneCapture if not already
	if (SceneCapture && SceneCapture->TextureTarget != PlanetRenderTarget)
	{
		SceneCapture->TextureTarget = PlanetRenderTarget;
		UE_LOG(LogTemp, Log, TEXT("PlanetRenderTarget assigned to SceneCapture."));
	}
}

void APlanetPanelActor::ApplyPlanetVisuals()
{
}

void APlanetPanelActor::InitializePlanet(float InRadius, UMaterialInterface* InMaterial)
{
	Radius = InRadius;

	if (InMaterial)
	{
		PlanetMesh->SetMaterial(0, InMaterial);
	}

	// Uniform scale adjustment (optional visual scale — not real units)
	float Scale = FMath::Clamp(Radius / 1.6e9f, 0.8f, 1.5f);
	PlanetMesh->SetRelativeScale3D(FVector(Scale));
}


