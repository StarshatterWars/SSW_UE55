// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Game/GameStructs.h"
#include "Engine/TextureRenderTarget2D.h" 
#include "CentralSunActor.generated.h"

class UStaticMeshComponent;
class USceneCaptureComponent2D;

UCLASS()
class STARSHATTERWARS_API ACentralSunActor : public AActor
{
	GENERATED_BODY()
	
public:
	ACentralSunActor();

	static ACentralSunActor* SpawnWithSpectralClass(
		UWorld* World,
		const FVector& Location,
		const FRotator& Rotation,
		TSubclassOf<ACentralSunActor> ActorClass,
		ESPECTRAL_CLASS InSpectralClass);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void SetStarColor(ESPECTRAL_CLASS Class);
	FLinearColor GetStarColor();
	void RefreshSceneCapture();
public:

	UFUNCTION()
	void EnsureRenderTarget();

	UPROPERTY(EditDefaultsOnly, Category = "Render")
	UTextureRenderTarget2D* SunRenderTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Render")
	USceneCaptureComponent2D* SceneCapture = nullptr;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SunMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BackgroundMesh;

	UFUNCTION(BlueprintCallable, Category = "Render")
	UTextureRenderTarget2D* GetRenderTarget() const { return SunRenderTarget; }
	
	UPROPERTY()
	USceneComponent* RootScene;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stellar")
	ESPECTRAL_CLASS SpectralClass = ESPECTRAL_CLASS::G;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar")
	UMaterialInterface* StarBaseMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* StarMaterialInstance;

	UPROPERTY(EditAnywhere, Category = "Sun")
	float RotationSpeed = 20.0f;

private:
	FRotator CurrentRotation;
	FLinearColor StarColor;
};