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
	SceneCapture->ShowFlags.SetMaterials(true);
	SceneCapture->bCaptureEveryFrame = false;      // recommended for manual capture
	SceneCapture->bCaptureOnMovement = true;       // helps with refresh on transform change
	SceneCapture->TextureTarget = SunRenderTarget;

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
	SetStarColor(SpectralClass);
	
	UE_LOG(LogTemp, Warning, TEXT("ACentralSunActor::BeginPlay() SpectralClass: %u"), static_cast<uint8>(SpectralClass));
	
	EnsureRenderTarget();
	
	// Assign render target if missing
	if (SceneCapture && !SceneCapture->TextureTarget)
	{
		SceneCapture->TextureTarget = SunRenderTarget;
	}

	if (!StarMaterialInstance && StarBaseMaterial && SunMesh)
	{
		StarMaterialInstance = UMaterialInstanceDynamic::Create(StarBaseMaterial, this);
		SunMesh->SetMaterial(0, StarMaterialInstance);

		UE_LOG(LogTemp, Warning, TEXT("ACentralSunActor::BeginPlay(): SpectralClass = %s, StarColor = R=%.2f G=%.2f B=%.2f"),
			*UEnum::GetValueAsString(SpectralClass),
			StarColor.R, StarColor.G, StarColor.B);

		StarMaterialInstance->SetVectorParameterValue("StarColor", StarColor);
		StarMaterialInstance->SetScalarParameterValue("GlowStrength", 2.5f);
		StarMaterialInstance->SetTextureParameterValue("Sunspots", SunspotTexture);
		StarMaterialInstance->SetScalarParameterValue("SunspotStrength", 0.5f); // 0 = off, 1 = full

		// Force update
		SceneCapture->bCaptureEveryFrame = false;
		SceneCapture->bCaptureOnMovement = true;

		// Nudge to force re-render
		
		UE_LOG(LogTemp, Warning, TEXT("StarMaterialInstance: %s"), *GetNameSafe(StarMaterialInstance));
		UE_LOG(LogTemp, Warning, TEXT("Mesh Material: %s"), *GetNameSafe(SunMesh->GetMaterial(0)));
		
		// Force redraw
		SunMesh->SetVisibility(false, true);
		SunMesh->SetVisibility(true, true);
		RefreshSceneCapture();
	}
}

void ACentralSunActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentRotation = GetActorRotation();
	CurrentRotation.Yaw += RotationSpeed * DeltaTime;
	SetActorRotation(CurrentRotation);

	RefreshSceneCapture();
}

void ACentralSunActor::SetStarColor(ESPECTRAL_CLASS Class) {
	// Set color based on spectral class

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

	UE_LOG(LogTemp, Warning, TEXT("SpectralClass = %s, StarColor = R=%.2f G=%.2f B=%.2f"),
		*UEnum::GetValueAsString(SpectralClass),
		StarColor.R, StarColor.G, StarColor.B);
}


FLinearColor ACentralSunActor::GetStarColor()
{
	return StarColor;
}


void ACentralSunActor::RefreshSceneCapture()
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

	SunMesh->MarkRenderStateDirty();
	SceneCapture->SetRelativeRotation(SceneCapture->GetRelativeRotation() + FRotator(1, 0, 0));
	SceneCapture->SetRelativeRotation(SceneCapture->GetRelativeRotation() - FRotator(1, 0, 0));
	UE_LOG(LogTemp, Warning, TEXT("Refreshing SceneCapture"));
	SceneCapture->CaptureScene();
}

void ACentralSunActor::EnsureRenderTarget()
{
	// 1. Create if null
	if (!SunRenderTarget)
	{
		SunRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("RT_SunRenderTarget"));
		SunRenderTarget->RenderTargetFormat = RTF_RGBA8;
		SunRenderTarget->ClearColor = FLinearColor::Black;
		SunRenderTarget->InitAutoFormat(64, 64); // or your preferred size
		SunRenderTarget->UTextureRenderTarget2D::UpdateResourceImmediate();

		UE_LOG(LogTemp, Log, TEXT("SunRenderTarget created."));
	}

	// 2. Assign to SceneCapture if not already
	if (SceneCapture && SceneCapture->TextureTarget != SunRenderTarget)
	{
		SceneCapture->TextureTarget = SunRenderTarget;
		UE_LOG(LogTemp, Log, TEXT("SunRenderTarget assigned to SceneCapture."));
	}
}


	
