// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Render")
	UTextureRenderTarget2D* SunRenderTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Render")
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SunMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BackgroundMesh;

	UFUNCTION(BlueprintCallable, Category = "Render")
	UTextureRenderTarget2D* GetRenderTarget() const { return SunRenderTarget; }

	UPROPERTY()
	USceneComponent* RootScene;

	UPROPERTY(EditAnywhere, Category = "Sun")
	float RotationSpeed = 20.0f;

private:
	FRotator CurrentRotation;
};