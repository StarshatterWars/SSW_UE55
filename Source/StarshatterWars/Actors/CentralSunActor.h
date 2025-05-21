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

	UFUNCTION(BlueprintCallable, Category = "Render")
	void SetMaterial(ESPECTRAL_CLASS Class);

	UPROPERTY()
	USceneComponent* RootScene;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stellar")
	ESPECTRAL_CLASS SpectralClass = ESPECTRAL_CLASS::G;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_O;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_B;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_A;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_F;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_G;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_K;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_M;
	
	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_R;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_N;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_S;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_BlackHole;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_WhiteDwarf;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_RedGiant;

	UPROPERTY(EditDefaultsOnly, Category = "Stellar Materials")
	UMaterialInterface* Mat_Unknown;

	UPROPERTY(EditAnywhere, Category = "Sun")
	float RotationSpeed = 20.0f;

private:
	FRotator CurrentRotation;
};