#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Game/GameStructs.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SystemBodyPanelActor.generated.h"

class UStaticMeshComponent;
class USceneCaptureComponent2D;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTexture2D;

UCLASS(Abstract)
class STARSHATTERWARS_API ASystemBodyPanelActor : public AActor
{
	GENERATED_BODY()

public:
	ASystemBodyPanelActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	float ComputeUIScale(float Radius) const;
	
	// --- Public accessors for UI ---
	UFUNCTION(BlueprintCallable, Category = "Render")
	UTextureRenderTarget2D* GetRenderTarget() const { return BodyRenderTarget; }

	UFUNCTION(BlueprintCallable, Category = "Render")
	UMaterialInstanceDynamic* GetMaterialInstance() const { return BodyMaterialInstance; }

	// --- Optional: call after setting derived data ---
	UFUNCTION(BlueprintCallable, Category = "Body")
	void InitializeBody(); // Applies scale + tilt (derived provided)

	FRotator ComputeSpinRotation(float WorldTimeSeconds) const;
	UTexture2D* LoadBodyTexture(FString AssetType);

	bool bApplyAxisTilt = true;
	float RotationSpeed = 20.f;
	float BodyTilt;
	double BodyRadius;
	FString BodyName;
	EBodyUISizeClass BodyType;

protected:
	// --- One-time init called during deferred spawn before FinishSpawning ---
	void InitBody(FString AssetType);

	// --- Derived class hooks ---
	virtual FString GetBodyName() const PURE_VIRTUAL(ASystemBodyPanelActor::GetBodyName, return TEXT("Body"););
	virtual float GetBodyRadius() const PURE_VIRTUAL(ASystemBodyPanelActor::GetBodyRadius, return 1.0f;);
	virtual float GetBodyTiltDegrees() const PURE_VIRTUAL(ASystemBodyPanelActor::GetBodyTiltDegrees, return 0.0f;);
	virtual UMaterialInterface* GetBaseMaterial() const PURE_VIRTUAL(ASystemBodyPanelActor::GetBaseMaterial, return nullptr;);
	
	void EndPlay(const EEndPlayReason::Type EndPlayReason);

	// Render target resolution computation: derived implements (planet vs moon)
	int32 ComputeRenderTargetResolution(float Radius) const;
	// Whether to apply computed scale (sometimes you may want fixed mesh scale)
	UPROPERTY(EditAnywhere, Category = "Body")
	bool bApplyUIScale = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Body")
	USceneComponent* RootScene = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Body")
	UStaticMeshComponent* BodyMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Body")
	USceneCaptureComponent2D* SceneCapture = nullptr;

	// Render state
	UPROPERTY()
	UMaterialInstanceDynamic* BodyMaterialInstance = nullptr;

	UPROPERTY()
	UTexture2D* BodyTexture = nullptr;

	UPROPERTY()
	FString TextureName;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Render", meta = (AllowPrivateAccess = true))
	UTextureRenderTarget2D* BodyRenderTarget = nullptr;
};
