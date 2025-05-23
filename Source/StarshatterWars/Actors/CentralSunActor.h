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
		ESPECTRAL_CLASS InSpectralClass,
		float InRadius 
	);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void RefreshSceneCapture();

public:
	UFUNCTION()
	void EnsureRenderTarget();
	UFUNCTION()
	void ApplyStarVisuals(ESPECTRAL_CLASS Class);
	UPROPERTY(EditDefaultsOnly, Category = "Render")
	UTextureRenderTarget2D* SunRenderTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Render")
	USceneCaptureComponent2D* SceneCapture = nullptr;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SunMesh;

	UFUNCTION(BlueprintCallable, Category = "Render")
	UTextureRenderTarget2D* GetRenderTarget() const { return SunRenderTarget; }
	
	UPROPERTY()
	USceneComponent* RootScene;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stellar")
	ESPECTRAL_CLASS SpectralClass = ESPECTRAL_CLASS::G;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar")
	UMaterialInterface* StarBaseMaterial;

	// Star material texture (sunspot mask)
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UTexture2D* SunspotTexture;

	UPROPERTY()
	UMaterialInstanceDynamic* StarMaterialInstance;

	UPROPERTY(EditAnywhere, Category = "Sun")
	float RotationSpeed = 20.0f;
	
	UFUNCTION(BlueprintCallable, Category = "Stellar")
	float GetRadius() const { return Radius; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stellar")
	float Radius = 1.6e9f; // Default to sun size


private:
	FRotator CurrentRotation;
	FLinearColor StarColor;
};