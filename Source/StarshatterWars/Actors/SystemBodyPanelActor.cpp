#include "SystemBodyPanelActor.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "../Foundation/SystemMapUtils.h"

ASystemBodyPanelActor::ASystemBodyPanelActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootScene);
	BodyMesh->SetRelativeScale3D(FVector(3.0f));
	BodyMesh->SetRelativeLocation(FVector::ZeroVector);
	BodyMesh->SetCastShadow(false);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(SphereMesh.Object);
	}

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootScene);
	SceneCapture->SetRelativeLocation(FVector(-600.f, 0.f, 0.f));
	SceneCapture->SetRelativeRotation(FRotator::ZeroRotator);
	SceneCapture->FOVAngle = 60.f;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = true;

	// Strip expensive flags for UI thumbnails
	SceneCapture->ShowFlags.SetAtmosphere(false);
	SceneCapture->ShowFlags.SetFog(false);
	SceneCapture->ShowFlags.SetSkyLighting(false);
	SceneCapture->ShowFlags.SetAmbientOcclusion(false);
	SceneCapture->ShowFlags.SetMotionBlur(false);
	SceneCapture->ShowFlags.SetScreenSpaceReflections(false);
	SceneCapture->ShowFlags.SetPostProcessing(false);
}

void ASystemBodyPanelActor::BeginPlay()
{
	Super::BeginPlay();
	InitializeBody();
}

void ASystemBodyPanelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	// Spin rotation from derived logic
	const FRotator SpinRot = ComputeSpinRotation(Time);

	// Optional tilt composition
	if (bApplyAxisTilt)
	{
		const FRotator TiltRot(0.f, 0.f, GetBodyTiltDegrees()); // if your tilt is not roll, adjust in derived
		const FQuat FinalQ = FQuat(SpinRot) * FQuat(TiltRot);
		BodyMesh->SetRelativeRotation(FinalQ.Rotator());
	}
	else
	{
		BodyMesh->SetRelativeRotation(SpinRot);
	}
}

void ASystemBodyPanelActor::InitBody()
{
	// Texture (derived decides how)
	BodyTexture = LoadBodyTexture();

	// Material
	BodyMaterialInstance = SystemMapUtils::CreatePreviewMID(
		this,
		GetBaseMaterial(),
		BodyTexture,
		GetBodyName()
	);

	if (BodyMaterialInstance)
	{
		BodyMesh->SetMaterial(0, BodyMaterialInstance);
	}

	// Render target
	const int32 Resolution = ComputeRenderTargetResolution(GetBodyRadius());
	UTextureRenderTarget2D* RT = SystemMapUtils::EnsureRenderTarget(
		this,
		GetBodyName(),
		Resolution,
		SceneCapture,
		BodyMesh
	);

	BodyRenderTarget = RT;
	SceneCapture->TextureTarget = BodyRenderTarget;

	// Capture now + safe capture next tick
	BodyMesh->MarkRenderStateDirty();
	SceneCapture->CaptureScene();
	SystemMapUtils::ScheduleSafeCapture(this, SceneCapture);

	UE_LOG(LogTemp, Warning, TEXT("InitBody() %s -> Mat: %s, Tex: %s, RT: %s"),
		*GetBodyName(),
		*GetNameSafe(BodyMaterialInstance),
		*GetNameSafe(BodyTexture),
		*GetNameSafe(BodyRenderTarget));
}
float ASystemBodyPanelActor::ComputeUIScale(float Radius) const
{
	return SystemMapUtils::GetBodyUIScale(2000.0, 150000.0, Radius);
}

void ASystemBodyPanelActor::InitializeBody()
{
	const float Radius = GetBodyRadius();

	if (bApplyUIScale)
	{
		const float ScaleFactor = ComputeUIScale(Radius);
		BodyMesh->SetRelativeScale3D(FVector(ScaleFactor));
	}

	// Tilt is applied in Tick composition when bApplyAxisTilt = true.
	// If you want one-time tilt only (no tick composition), set bApplyAxisTilt=false
	// and apply tilt here explicitly in derived.
}

FRotator ASystemBodyPanelActor::ComputeSpinRotation(float WorldTimeSeconds) const
{
	// Preserve your existing behavior:
	return SystemMapUtils::GetBodyRotation(WorldTimeSeconds, RotationSpeed, BodyTilt);
}

UTexture2D* ASystemBodyPanelActor::LoadBodyTexture()
{
	return SystemMapUtils::LoadBodyAssetTexture("Planet", TextureName);
}

