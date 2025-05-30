// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Game/GameStructs.h"
#include "Engine/TextureRenderTarget2D.h" 
#include "../System/SSWGameInstance.h"
#include "MoonPanelActor.generated.h"


class UStaticMeshComponent;
class USceneCaptureComponent2D;
class UMaterialInterface;
class APlanetPanelActor;

UCLASS()
class STARSHATTERWARS_API AMoonPanelActor : public AActor
{
	GENERATED_BODY()
	
public:	
	
	// Sets default values for this actor's properties
	AMoonPanelActor();

	static AMoonPanelActor* SpawnWithMoonData(
		UWorld* World,
		const FVector& Location,
		const FRotator& Rotation,
		TSubclassOf<AMoonPanelActor> ActorClass,
		FS_MoonMap MoonInfo
	);

	UFUNCTION()
	void AssignScreenCapture();
	
	UFUNCTION(BlueprintCallable, Category = "Moon")
	void InitializeMoon();

	void SetParentPlanet(APlanetPanelActor* InParent);
	UFUNCTION(BlueprintCallable, Category = "Render")
	UTextureRenderTarget2D* GetRenderTarget() const { return MoonRenderTarget; }

	UFUNCTION(BlueprintCallable, Category = "Render")
	UMaterialInstanceDynamic* GetMaterialInstance() const { return MoonMaterialInstance; }

	UPROPERTY(EditDefaultsOnly, Category = "Moon")
	UMaterialInterface* MoonBaseMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Moon")
	APlanetPanelActor* ParentPlanet = nullptr;

	UPROPERTY()
	bool isSceneDelay = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void EnsureRenderTarget();
	
	UFUNCTION()
	void InitMoon();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Render")
	UStaticMeshComponent* MoonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Render")
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY()
	USceneComponent* RootScene;

	UPROPERTY()
	UMaterialInstanceDynamic* MoonMaterialInstance;

	UPROPERTY()
	UTexture2D* MoonTexture;

	UPROPERTY()
	FRotator CurrentRotation;

	UPROPERTY(EditAnywhere, Category = "Moon")
	float RotationSpeed = 20.f;

	UPROPERTY()
	float Radius = 1.6e9f;

	UPROPERTY()
	FS_MoonMap MoonData;

private:
	UPROPERTY(BlueprintReadOnly, Category="Render", meta=(AllowPrivateAccess=true))
	UTextureRenderTarget2D* MoonRenderTarget = nullptr;
};
