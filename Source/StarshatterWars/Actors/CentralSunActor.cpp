// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "CentralSunActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/ConstructorHelpers.h"
#include "../Game/GalaxyManager.h"
#include "../Foundation/StarUtils.h"

ACentralSunActor::ACentralSunActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Root Component
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	// Sun Mesh
	SunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SunMesh"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		SunMesh->SetStaticMesh(SphereMesh.Object);
	}

	SunMesh->SetupAttachment(RootScene);
	SunMesh->SetRelativeScale3D(FVector(3.0f));
	SunMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	SunMesh->SetCastShadow(false);	
	
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
	ESPECTRAL_CLASS InSpectralClass,
	float InRadius,
	FString InName)
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
	NewActor->Radius = InRadius;
	NewActor->StarName = InName;

	// Resume construction, now BeginPlay will see the correct value
	NewActor->FinishSpawning(FTransform(Rotation, Location));

	return NewActor;
}

void ACentralSunActor::BeginPlay()
{
	Super::BeginPlay();

	CurrentRotation = GetActorRotation();

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

		ApplyStarVisuals(SpectralClass);

		// Force update
		SceneCapture->bCaptureEveryFrame = false;
		SceneCapture->bCaptureOnMovement = true;

		// Nudge to force re-render
		
		UE_LOG(LogTemp, Warning, TEXT("StarMaterialInstance: %s"), *GetNameSafe(StarMaterialInstance));
		UE_LOG(LogTemp, Warning, TEXT("Mesh Material: %s"), *GetNameSafe(SunMesh->GetMaterial(0)));
		
		// Force redraw
		SunMesh->SetVisibility(false, true);
		SunMesh->SetVisibility(true, true);
	}
}

void ACentralSunActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));
}

void ACentralSunActor::EnsureRenderTarget()
{
	// Create the render target
	int32 Resolution = StarUtils::GetRenderTargetResolutionForRadius(Radius);
	UGalaxyManager* Galaxy = UGalaxyManager::Get(this); // use your accessor
	SunRenderTarget = Galaxy->GetOrCreateStarRenderTarget(StarName, Resolution);

	if (!SunRenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create RenderTarget for star %s"), *StarName);
		return;
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("RenderTarget created for star %s"), *StarName);
	}

	SceneCapture->TextureTarget = SunRenderTarget;
}

void ACentralSunActor::ApplyStarVisuals(ESPECTRAL_CLASS Class)
{
	if (!StarMaterialInstance || !SunMesh) return;

	UE_LOG(LogTemp, Warning, TEXT("ACentralSunActor::ApplyStarVisuals(): SpectralClass = %s, StarColor = R=%.2f G=%.2f B=%.2f"),
		*UEnum::GetValueAsString(Class),
		StarColor.R, StarColor.G, StarColor.B);

	// Use StarUtils for consistent visuals
	StarColor = StarUtils::GetColor(Class);
	float GlowStrength = StarUtils::GetGlowStrength(Class);
	float SunspotStrength = StarUtils::GetSunspotStrength(Class);

	//StarMaterialInstance->SetVectorParameterValue("StarColor", StarColor);
	//StarMaterialInstance->SetScalarParameterValue("GlowStrength", GlowStrength/10);
	//StarMaterialInstance->SetTextureParameterValue("Sunspots", SunspotTexture);
	//StarMaterialInstance->SetScalarParameterValue("SunspotStrength", SunspotStrength);
}

void ACentralSunActor::AssignScreenCapture()
{
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (SceneCapture)
			{
				SunMesh->MarkRenderStateDirty();
				SceneCapture->CaptureScene();

				// Log confirmation
				UE_LOG(LogTemp, Warning, TEXT("RenderTarget used: %s [%p] for star %s"),
					*GetNameSafe(SunRenderTarget),
					SunRenderTarget,
					*StarName);
			}
		}), 0.05f, false); // 50ms delay — adjust as needed
}
