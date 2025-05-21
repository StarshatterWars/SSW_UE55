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

void ACentralSunActor::BeginPlay()
{
	Super::BeginPlay();
	CurrentRotation = GetActorRotation();
	SceneCapture->CaptureScene();
}

void ACentralSunActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentRotation.Yaw += RotationSpeed * DeltaTime;
	SetActorRotation(CurrentRotation);
}

void ACentralSunActor::SetMaterial(ESPECTRAL_CLASS SpectalClass)
{
	UMaterialInterface* SelectedMat = nullptr;

	switch (SpectralClass)
	{
	case ESPECTRAL_CLASS::O: SelectedMat = Mat_O; break;
	case ESPECTRAL_CLASS::B: SelectedMat = Mat_B; break;
	case ESPECTRAL_CLASS::A: SelectedMat = Mat_A; break;
	case ESPECTRAL_CLASS::F: SelectedMat = Mat_F; break;
	case ESPECTRAL_CLASS::G: SelectedMat = Mat_G; break;
	case ESPECTRAL_CLASS::K: SelectedMat = Mat_K; break;
	case ESPECTRAL_CLASS::M: SelectedMat = Mat_M; break;
	case ESPECTRAL_CLASS::R: SelectedMat = Mat_R; break;
	case ESPECTRAL_CLASS::N: SelectedMat = Mat_N; break;
	case ESPECTRAL_CLASS::S: SelectedMat = Mat_S; break;
	case ESPECTRAL_CLASS::BLACK_HOLE: SelectedMat = Mat_BlackHole; break;
	case ESPECTRAL_CLASS::WHITE_DWARF: SelectedMat = Mat_WhiteDwarf; break;
	case ESPECTRAL_CLASS::RED_GIANT: SelectedMat = Mat_RedGiant; break;
	case ESPECTRAL_CLASS::UNKNOWN:
	default:
		SelectedMat = Mat_Unknown;
		break;
	}

	if (SelectedMat)
	{
		SunMesh->SetMaterial(0, SelectedMat);
	}
}
