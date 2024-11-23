/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Space
	FILE:         OrbitalBody.cpp
	AUTHOR:       Carlos Bott
*/


#include "OrbitalBody.h"

// Sets default values
AOrbitalBody::AOrbitalBody()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	static ConstructorHelpers::FObjectFinder<UDataTable> StarSystemDataTableObject(TEXT("DataTable'/Game/Game/DT_StarSystem.DT_StarSystem'"));

	if (StarSystemDataTableObject.Succeeded())
	{
		StarSystemDataTable = StarSystemDataTableObject.Object;
		//StarsDataTable->EmptyTable();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to get Starsystem Data Table"));
	}

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Orbital Body"));
	RootComponent = Root;
}

void AOrbitalBody::InitializeStar(AStarSystem* sys, FString n, double m, double rad, double o, double rot, AOrbital* prime)
{
	rotation = rot;
}

void AOrbitalBody::InitializeStar(AStarSystem* sys, FS_Star StarData)
{
	UE_LOG(LogTemp, Log, TEXT("AOrbitalBody::InitializeStar()"));
	UE_LOG(LogTemp, Log, TEXT("Initialized Star Data '%s'"), *StarData.Name);

	name = TCHAR_TO_ANSI(*StarData.Name);
	img_name = TCHAR_TO_ANSI(*StarData.Image);
	map_name = TCHAR_TO_ANSI(*StarData.Map);

	light = StarData.Light;
	radius = StarData.Radius;
	rotation = StarData.Rot;
	mass = StarData.Mass;
	orbit = StarData.Orbit;
	tscale = StarData.Tscale;	
	retro = StarData.Retro;
	
	color = Color(StarData.Color.R, StarData.Color.G, StarData.Color.B);
	back = Color(StarData.Back.R, StarData.Back.G, StarData.Back.B);
}

void AOrbitalBody::InitializePlanet(AStarSystem* sys, FString n, double m, double rad, double o, double rot, AOrbital* prime)
{
}

void AOrbitalBody::InitializeMoon(AStarSystem* sys, FString n, double m, double rad, double o, double rot, AOrbital* prime)
{
}

// Called when the game starts or when spawned
void AOrbitalBody::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOrbitalBody::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOrbitalBody::Update()
{
	Super::Update();

	theta = 0;

	if (rotation > 0)
		theta = -2 * PI * AStarSystem::GetStardate() / rotation;

	ListIter<AOrbitalBody> body = satellites;
	while (++body)
		body->Update();

	/*if (rep && theta != 0) {
		Matrix m;
		m.Pitch(tilt);
		m.Roll(tilt / 2);
		m.Yaw(theta);
		rep->SetOrientation(m);
	}

	if (light_rep) {
		Point bodyloc = loc;
		bodyloc = bodyloc.OtherHand();
		light_rep->MoveTo(bodyloc);
	}*/
}

