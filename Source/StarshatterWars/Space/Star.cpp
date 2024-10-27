/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         Star.cpp
	AUTHOR:       Carlos Bott
*/


#include "Star.h"


// Sets default values
AStar::AStar()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AStar::Initialize(const char* n, const Point& l, int s)
{
}

Color AStar::GetColor() const
{
	return Color();
}

int AStar::GetSize() const
{
	return 0;
}

// Called when the game starts or when spawned
void AStar::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

Color
AStar::GetColor(ESPECTRAL_CLASS s)
{
	switch (s) {
	case ESPECTRAL_CLASS::O:           return Color(128, 128, 255); break;
	case ESPECTRAL_CLASS::B:           return Color(192, 192, 255); break;
	case ESPECTRAL_CLASS::A:           return Color(220, 220, 255); break;
	case ESPECTRAL_CLASS::F:           return Color(255, 255, 255); break;
	case ESPECTRAL_CLASS::G:           return Color(255, 255, 128); break;
	case ESPECTRAL_CLASS::K:           return Color(255, 192, 100); break;
	case ESPECTRAL_CLASS::M:           return Color(255, 100, 100); break;

	case ESPECTRAL_CLASS::RED_GIANT:   return Color(255, 80, 80); break;
	case ESPECTRAL_CLASS::WHITE_DWARF: return Color(255, 255, 255); break;
	case ESPECTRAL_CLASS::BLACK_HOLE:  return Color(0, 0, 0); break;
	}

	return Color::White;
}

int
AStar::GetSize(ESPECTRAL_CLASS s)
{
	switch (s) {
	case ESPECTRAL_CLASS::O:           return 4; break;
	case ESPECTRAL_CLASS::B:           return 4; break;
	case ESPECTRAL_CLASS::A:           return 3; break;
	case ESPECTRAL_CLASS::F:           return 3; break;
	case ESPECTRAL_CLASS::G:           return 2; break;
	case ESPECTRAL_CLASS::K:           return 2; break;
	case ESPECTRAL_CLASS::M:           return 1; break;

	case ESPECTRAL_CLASS::RED_GIANT:   return 4; break;
	case ESPECTRAL_CLASS::WHITE_DWARF: return 1; break;
	case ESPECTRAL_CLASS::BLACK_HOLE:  return 3; break;
	}

	return 3;
}

