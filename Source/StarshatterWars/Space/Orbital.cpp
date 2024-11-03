// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "Orbital.h"


// Sets default values
AOrbital::AOrbital()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOrbital::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOrbital::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Update();

}

void AOrbital::Update()
{
	if (system && primary && orbit > 0) {
		double grade = (retro) ? -1 : 1;

		// orbits are counter clockwise:
		phase = -2 * PI * grade * AStarSystem::GetStardate() / period;

		loc = primary->Location() + Point((double)(orbit * cos(phase)),
			(double)(orbit * sin(phase)),
			0);
	}

	ListIter<AOrbitalRegion> region = regions;
	//while (++region)
	//	region->Update();

	//if (rep)
	//	rep->MoveTo(loc.OtherHand());
}

Point AOrbital::PredictLocation(double delta_t)
{
	Point predicted_loc = Location();

	if (system && primary && orbit > 0) {
		predicted_loc = primary->PredictLocation(delta_t);

		double grade = (retro) ? -1 : 1;

		// orbits are(?) counter clockwise:
		double predicted_phase = (double)(-2 * PI * grade * (AStarSystem::GetStardate() + delta_t) / period);

		predicted_loc += Point((double)(orbit * cos(predicted_phase)),
			(double)(orbit * sin(predicted_phase)),
			0);
	}

	return predicted_loc;
}

