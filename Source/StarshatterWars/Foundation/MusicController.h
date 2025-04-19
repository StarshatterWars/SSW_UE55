// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "MusicController.generated.h"

UCLASS()
class STARSHATTERWARS_API AMusicController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMusicController();

	UFUNCTION()
    void PlayMusic(USoundBase* Music);

	UFUNCTION()
    void PlaySound(USoundBase* Sound);

    UFUNCTION()
    void StopMusic();

	UFUNCTION()
    void StopSound();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
private:
    UPROPERTY()
    UAudioComponent* AudioComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
