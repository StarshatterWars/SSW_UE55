// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MoonPanelActor.h"#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/ConstructorHelpers.h"
#include "../Foundation/PlanetUtils.h"
#include "../Game/GalaxyManager.h"
#include "Windows/MinWindows.h"

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
	FRotator Spin = PlanetUtils::GetPlanetRotation(Time, RotationSpeed, MoonData.Tilt);
	MoonMesh->SetRelativeRotation(Spin);
}

void AMoonPanelActor::AssignScreenCapture()
{
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (SceneCapture)
			{
				MoonMesh->MarkRenderStateDirty();
				SceneCapture->CaptureScene();

				// Log confirmation
				UE_LOG(LogTemp, Warning, TEXT("Moon RenderTarget used: %s [%p] for moon %s"),
					*GetNameSafe(MoonRenderTarget),
					MoonRenderTarget,
					*MoonData.Name);
			}
		}), 0.05f, false); // 50ms delay — adjust as needed
}

void AMoonPanelActor::EnsureRenderTarget()
{
	// Create the render target
	int32 Resolution = PlanetUtils::GetRenderTargetResolutionForRadius(MoonData.Radius);
	UGalaxyManager* Galaxy = UGalaxyManager::Get(this); // use your accessor
	MoonRenderTarget = Galaxy->GetOrCreatePlanetRenderTarget(MoonData.Name, Resolution);

	if (!MoonRenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create RenderTarget for moon %s"), *MoonData.Name);
		return;
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("RenderTarget created for moon %s"), *MoonData.Name);
	}

	SceneCapture->TextureTarget = MoonRenderTarget;
}

void AMoonPanelActor::InitMoon()
{
	// Texture
	MoonTexture = PlanetUtils::LoadPlanetAssetTexture(MoonData.Texture);

	// Ensure a unique render target before using SceneCapture
	EnsureRenderTarget();

	// Create dynamic material
	UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(MoonBaseMaterial, MoonMesh);
	DynMat->Rename(*FString::Printf(TEXT("MID_%s_%d"), *MoonData.Name, FMath::RandRange(1000, 9999)));
	if (MoonTexture)
	{
		#define UpdateResource UpdateResource
		MoonTexture->UpdateResource();
		DynMat->SetTextureParameterValue("BaseTexture", MoonTexture);
	}

	// Apply to mesh
	MoonMesh->SetMaterial(0, DynMat);
	MoonMaterialInstance = DynMat;

	// Capture scene (now safe!)
	MoonMesh->MarkRenderStateDirty();
	SceneCapture->CaptureScene();

	UE_LOG(LogTemp, Warning, TEXT("InitMoon() Moon: %s -> Mat: %s, Tex: %s, RT: %s"),
		*MoonData.Name,
		*GetNameSafe(DynMat),
		*GetNameSafe(MoonTexture),
		*GetNameSafe(MoonRenderTarget));
}

void AMoonPanelActor::InitializeMoon()
{
	float ScaleFactor = PlanetUtils::GetPlanetUIScale(MoonData.Radius);
	//PlanetMesh->SetRelativeScale3D(FVector(1.0f));

	FRotator AxisTilt = PlanetUtils::GetPlanetAxisTilt(MoonData.Tilt);
	MoonMesh->SetRelativeRotation(AxisTilt);
}

