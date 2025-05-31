// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Game/GameStructs.h"
#include "Engine/TextureRenderTarget2D.h" 
#include "../System/SSWGameInstance.h"
#include "../Foundation/SystemMapUtils.h"
#include "SystemOverviewActor.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS()
class STARSHATTERWARS_API ASystemOverviewActor : public AActor
{
	GENERATED_BODY()
	
public:	
	
	static ASystemOverviewActor* SpawnWithSystemData(
		UWorld* World,
		const FVector& Location,
		const FRotator& Rotation,
		TSubclassOf<ASystemOverviewActor> ActorClass,
		UTextureRenderTarget2D* RT
	);

	// Sets default values for this actor's properties
	ASystemOverviewActor();

	FS_Galaxy SystemData;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UFUNCTION()
	UTextureRenderTarget2D* GetRenderTarget() const { return OverviewRT; }

	void InitSystem(UTextureRenderTarget2D* Target, const FVector& LookFrom, const FRotator& LookAt);

	UPROPERTY(VisibleAnywhere)
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY()
	UTextureRenderTarget2D* OverviewRT;
};

	
