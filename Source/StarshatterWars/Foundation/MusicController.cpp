// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "MusicController.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

// Sets default values
AMusicController::AMusicController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->bAutoActivate = false;
    MusicComponent->bIsUISound = true;
   
    UIComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("UIComponent"));
    UIComponent->bAutoActivate = false;
    UIComponent->bIsUISound = true;
    SetActorHiddenInGame(true);
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
    if (!MusicComponent)
        return;
    if (!Music)
        return;
    
    MusicComponent->SetSound(Music);
    MusicComponent->Play();
}


void AMusicController::PlaySound(USoundBase* Sound)
{
    if (!UIComponent)
        return;
    if (!Sound)
        return;

    UIComponent->SetSound(Sound);
    UIComponent->Play();
}

void AMusicController::StopMusic()
{
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        MusicComponent->Stop();
    }
}

void AMusicController::StopSound()
{
    if (UIComponent && UIComponent->IsPlaying())
    {
        UIComponent->Stop();
    }
}

bool AMusicController::IsSoundPlaying() {
    return UIComponent->IsPlaying();
}
