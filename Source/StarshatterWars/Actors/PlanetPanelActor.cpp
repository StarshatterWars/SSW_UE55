// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlanetPanelActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/ConstructorHelpers.h"
#include "../Foundation/PlanetUtils.h"
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
}

void APlanetPanelActor::BeginPlay()
{
	Super::BeginPlay();

	if(PlanetRenderTarget) 
		PlanetRenderTarget = nullptr;

	EnsureRenderTarget();

	if (SceneCapture && !SceneCapture->TextureTarget)
	{
		SceneCapture->TextureTarget = PlanetRenderTarget;
	}

	if (!PlanetMaterialInstance && PlanetBaseMaterial && PlanetMesh)
	{
		PlanetMaterialInstance = UMaterialInstanceDynamic::Create(PlanetBaseMaterial, this);
		PlanetMesh->SetMaterial(0, PlanetMaterialInstance);
	}

	RefreshSceneCapture();
}

void APlanetPanelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));
	RefreshSceneCapture();
}

void APlanetPanelActor::RefreshSceneCapture()
{
	FTimerHandle DelayHandle;
	GetWorldTimerManager().SetTimer(DelayHandle, [this]()
		{
			if (SceneCapture)
			{
				SceneCapture->CaptureScene();
			}
		}, 0.0f, false);

	PlanetMesh->MarkRenderStateDirty();
	SceneCapture->CaptureScene();
}

void APlanetPanelActor::EnsureRenderTarget()
{
	if (!PlanetRenderTarget)
	{
		FString BaseName = PlanetData.Name.IsEmpty() ? TEXT("Unnamed") : PlanetData.Name;
		FString RTName = FString::Printf(TEXT("PlanetRT_%s_%d"), *BaseName, FMath::RandRange(1000, 9999));
		PlanetRenderTarget = PlanetUtils::CreatePlanetRenderTarget(RTName, this);
	}

	if (SceneCapture && SceneCapture->TextureTarget != PlanetRenderTarget)
	{
		SceneCapture->TextureTarget = PlanetRenderTarget;
	}
}

void APlanetPanelActor::InitializePlanet(double InRadius, UMaterialInterface* BaseMaterial, const FString& TextureName, FS_PlanetMap InData)
{
	PlanetData = InData;
	Radius = InRadius;

	PlanetTexture = PlanetUtils::LoadPlanetTexture(TextureName);
	PlanetMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (PlanetMaterialInstance)
	{
		PlanetMaterialInstance->SetTextureParameterValue("BaseTexture", PlanetTexture);
		PlanetMesh->SetMaterial(0, PlanetMaterialInstance);

		UE_LOG(LogTemp, Warning, TEXT("Applied material with texture: %s"), *GetNameSafe(PlanetTexture));
	}

	float Scale = FMath::Clamp((float)(Radius / 1.0e7f), 0.4f, 1.8f);
	PlanetMesh->SetRelativeScale3D(FVector(Scale));

	RefreshSceneCapture();
}
