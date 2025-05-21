// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CentralSunActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/ConstructorHelpers.h"

ACentralSunActor::ACentralSunActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Root Component
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	// Sun Mesh
	SunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SunMesh"));

	// Background Mesh
	BackgroundMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackgroundMesh"));
	

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		SunMesh->SetStaticMesh(SphereMesh.Object);
	}

	SunMesh->SetupAttachment(RootScene);
	SunMesh->SetRelativeScale3D(FVector(3.0f));
	SunMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	SunMesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		BackgroundMesh->SetStaticMesh(CubeMesh.Object);
	}

	BackgroundMesh->SetupAttachment(RootScene);
	BackgroundMesh->SetRelativeScale3D(FVector(0.1f, 15.f, 15.f));
	BackgroundMesh->SetRelativeLocation(FVector(300.f, 0.f, 0.f));
	BackgroundMesh->SetCastShadow(false);

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

	// Create and configure render target
	SunRenderTarget = NewObject<UTextureRenderTarget2D>();
	SunRenderTarget->RenderTargetFormat = RTF_RGBA16f;
	SunRenderTarget->InitAutoFormat(64, 64); // or your desired size
	SunRenderTarget->ClearColor = FLinearColor(0, 0, 0, 0); // Fully transparent
	SunRenderTarget->UpdateResourceImmediate(true);

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RTAsset(TEXT("/Game/Screens/Operations/Widgets/RT_SunRenderTarget.RT_SunRenderTarget"));

	if (RTAsset.Succeeded())
	{
		SunRenderTarget = RTAsset.Object;
		SceneCapture->TextureTarget = SunRenderTarget;
	}
}

ACentralSunActor* ACentralSunActor::SpawnWithSpectralClass(
	UWorld* World,
	const FVector& Location,
	const FRotator& Rotation,
	TSubclassOf<ACentralSunActor> ActorClass,
	ESPECTRAL_CLASS InSpectralClass)
{
	if (!World || !*ActorClass) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Defer construction to allow property assignment before BeginPlay
	Params.bDeferConstruction = true;

	// Spawn actor, but defer BeginPlay
	ACentralSunActor* NewActor = World->SpawnActor<ACentralSunActor>(ActorClass, Location, Rotation, Params);
	if (!NewActor) return nullptr;

	// Set SpectralClass before BeginPlay
	NewActor->SpectralClass = InSpectralClass;

	// Resume construction, now BeginPlay will see the correct value
	NewActor->FinishSpawning(FTransform(Rotation, Location));

	return NewActor;
}

void ACentralSunActor::BeginPlay()
{
	Super::BeginPlay();
	CurrentRotation = GetActorRotation();
	
	if (!StarMaterialInstance && StarBaseMaterial && SunMesh)
	{
		StarMaterialInstance = UMaterialInstanceDynamic::Create(StarBaseMaterial, this);
		SunMesh->SetMaterial(0, StarMaterialInstance);
	}

	UE_LOG(LogTemp, Warning, TEXT("ACentralSunActor::BeginPlay() Spectral Class: %u"), static_cast<uint8>(SpectralClass));
	
	SetMaterial(SpectralClass);

	if (SceneCapture)
	{
		SceneCapture->CaptureScene(); // MUST come after material assignment
	}
}

void ACentralSunActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentRotation.Yaw += RotationSpeed * DeltaTime;
	SetActorRotation(CurrentRotation);
}

void ACentralSunActor::SetMaterial(ESPECTRAL_CLASS Class)
{
	UE_LOG(LogTemp, Warning, TEXT("ACentralSunActor::SetMaterial() called"));

	// Set color based on spectral class
	FLinearColor StarColor;

	switch (Class)
	{
	case ESPECTRAL_CLASS::O: StarColor = FLinearColor(0.5f, 0.6f, 1.0f); break;
	case ESPECTRAL_CLASS::B: StarColor = FLinearColor(0.7f, 0.7f, 1.0f); break;
	case ESPECTRAL_CLASS::A: StarColor = FLinearColor(0.9f, 0.9f, 1.0f); break;
	case ESPECTRAL_CLASS::F: StarColor = FLinearColor(1.0f, 1.0f, 0.9f); break;
	case ESPECTRAL_CLASS::G: StarColor = FLinearColor(1.0f, 0.95f, 0.6f); break;
	case ESPECTRAL_CLASS::K: StarColor = FLinearColor(1.0f, 0.6f, 0.3f); break;
	case ESPECTRAL_CLASS::M: StarColor = FLinearColor(1.0f, 0.2f, 0.1f); break;
	case ESPECTRAL_CLASS::R: StarColor = FLinearColor(0.9f, 0.1f, 0.1f); break;
	case ESPECTRAL_CLASS::N: StarColor = FLinearColor(0.75f, 0.05f, 0.05f); break;
	case ESPECTRAL_CLASS::S: StarColor = FLinearColor(0.95f, 0.4f, 0.2f); break;
	case ESPECTRAL_CLASS::BLACK_HOLE: StarColor = FLinearColor::Black; break;
	case ESPECTRAL_CLASS::WHITE_DWARF: StarColor = FLinearColor::White; break;
	case ESPECTRAL_CLASS::RED_GIANT: StarColor = FLinearColor(1.0f, 0.5f, 0.3f); break;
	default: StarColor = FLinearColor::Gray; break;
	}

	//DynMat->SetVectorParameterValue("StarColor", FLinearColor::Red);
	if (StarMaterialInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("StarColor set to: R=%.2f G=%.2f B=%.2f"),
			StarColor.R, StarColor.G, StarColor.B);

		StarMaterialInstance->SetVectorParameterValue("StarColor", StarColor);
		StarMaterialInstance->SetScalarParameterValue("GlowStrength", 15.0f); // optional
	}
	// Refresh capture
	if (SceneCapture)
	{
		SceneCapture->CaptureScene();
	}
}

void ACentralSunActor::Init(ESPECTRAL_CLASS InClass)
{
	
	UE_LOG(LogTemp, Warning, TEXT("ACentralSunActor::Init() Spectral Class: %u"), static_cast<uint8>(InClass)); 
	SpectralClass = InClass;
}
