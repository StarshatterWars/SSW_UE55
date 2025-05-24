// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Game/GameStructs.h"
#include "Engine/TextureRenderTarget2D.h" 
#include "../System/SSWGameInstance.h"
#include "PlanetPanelActor.generated.h"

class UStaticMeshComponent;
class USceneCaptureComponent2D;

UCLASS()
class STARSHATTERWARS_API APlanetPanelActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlanetPanelActor();

	static APlanetPanelActor* SpawnWithPlanetData(
		UWorld* World,
		const FVector& Location,
		const FRotator& Rotation,
		TSubclassOf<APlanetPanelActor> ActorClass, 
		FS_PlanetMap& PlanetInfo
	);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void RefreshSceneCapture();

public:
	UFUNCTION()
	void EnsureRenderTarget();

	UFUNCTION()
	void ApplyPlanetVisuals();
	UFUNCTION()
	void InitializePlanet(float InRadius, UMaterialInterface* InMaterial);

	UPROPERTY(EditDefaultsOnly, Category = "Render")
	UTextureRenderTarget2D* PlanetRenderTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Render")
	USceneCaptureComponent2D* SceneCapture = nullptr;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PlanetMesh;

	UFUNCTION(BlueprintCallable, Category = "Render")
	UTextureRenderTarget2D* GetRenderTarget() const { return PlanetRenderTarget; }
	
	UPROPERTY()
	USceneComponent* RootScene;

	UPROPERTY(EditDefaultsOnly, Category = "Planet")
	UMaterialInterface* PlanetBaseMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* PlanetMaterialInstance;

	UPROPERTY(EditAnywhere, Category = "Planet")
	float RotationSpeed = 20.0f;

	UFUNCTION()
	float GetRadius() const { return Radius; }

	FS_PlanetMap PlanetInfo;

protected:
	UPROPERTY()
	float Radius = 1.6e9f; // Default to planet size
	/** Sets the mesh, material, and radius for the panel planet */

private:
	FRotator CurrentRotation;
	FS_PlanetMap PlanetData;
};
	
