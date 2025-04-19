// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MusicController.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

// Sets default values
AMusicController::AMusicController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->bAutoActivate = false;
    AudioComponent->bIsUISound = true;
    RootComponent = AudioComponent;

    //SetActorHiddenInGame(true);
    SetCanBeDamaged(false);

    UE_LOG(LogTemp, Log, TEXT("Audio Component Initialized"));
}

// Called when the game starts or when spawned
void AMusicController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMusicController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMusicController::PlayMusic(USoundBase* Music)
{
    if (!AudioComponent)
        return;
    if (!Music)
        return;
    
    AudioComponent->SetSound(Music);
    AudioComponent->Play();
 
}

void AMusicController::StopMusic()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
}