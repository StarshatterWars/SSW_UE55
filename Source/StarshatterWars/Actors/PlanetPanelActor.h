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
class UMaterialInterface;

UCLASS()
class STARSHATTERWARS_API APlanetPanelActor : public AActor
{
	GENERATED_BODY()

public:
	APlanetPanelActor();

	static APlanetPanelActor* SpawnWithPlanetData(
		UWorld* World,
		const FVector& Location,
		const FRotator& Rotation,
		TSubclassOf<APlanetPanelActor> ActorClass,
		FS_PlanetMap PlanetInfo
	);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void InitializePlanet();

	UFUNCTION(BlueprintCallable, Category = "Render")
	UTextureRenderTarget2D* GetRenderTarget() const { return PlanetRenderTarget; }

	UFUNCTION(BlueprintCallable, Category = "Render")
	UMaterialInstanceDynamic* GetMaterialInstance() const { return PlanetMaterialInstance; }

	UPROPERTY(EditDefaultsOnly, Category = "Planet")
	UMaterialInterface* PlanetBaseMaterial;

	UPROPERTY()
	bool isSceneDelay = false;

	UPROPERTY()
	bool bUseSystemOverviewOnly = true;

	UPROPERTY()
	FS_PlanetMap PlanetData;

protected:	
	UFUNCTION()
	void InitPlanet();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Render")
	UStaticMeshComponent* PlanetMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Render")
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY()
	USceneComponent* RootScene;

	UPROPERTY()
	UMaterialInstanceDynamic* PlanetMaterialInstance;

	UPROPERTY()
	UTexture2D* PlanetTexture;

	UPROPERTY()
	FRotator CurrentRotation;

	UPROPERTY(EditAnywhere, Category = "Planet")
	float RotationSpeed = 20.f;

	UPROPERTY()
	float Radius = 1.6e9f;

private:
	UPROPERTY(BlueprintReadOnly, Category="Render", meta=(AllowPrivateAccess=true))
	UTextureRenderTarget2D* PlanetRenderTarget = nullptr;
};
