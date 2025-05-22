// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/


#include "PlanetOrbitUtils.h"

FVector PlanetOrbitUtils::GetPlanetOrbitPositionWithAngle(const FS_PlanetMap& PlanetData, const FVector& StarLocation, float OrbitAngleDegrees)
{
	const float OrbitRadiusCM = PlanetData.Orbit * 100.0f;
	const float OrbitAngleRad = FMath::DegreesToRadians(OrbitAngleDegrees);
	const float InclinationDegrees = PlanetData.Inclination;

	FVector FlatOrbit = FVector(
		FMath::Cos(OrbitAngleRad) * OrbitRadiusCM,
		FMath::Sin(OrbitAngleRad) * OrbitRadiusCM,
		0.f
	);

	FVector TiltedOrbit = FlatOrbit.RotateAngleAxis(InclinationDegrees, FVector::RightVector);
	return StarLocation + TiltedOrbit;
}

FVector PlanetOrbitUtils::GetPlanetOrbitPositionFlat(const FS_PlanetMap& PlanetData, const FVector& StarLocation, float OrbitAngleDegrees)
{
	const float OrbitRadiusCM = PlanetData.Orbit * 100.0f; // km to cm
	const float OrbitAngleRad = FMath::DegreesToRadians(OrbitAngleDegrees);

	// Flat orbit in X-Y plane
	FVector OrbitPosition = FVector(
		FMath::Cos(OrbitAngleRad) * OrbitRadiusCM,
		FMath::Sin(OrbitAngleRad) * OrbitRadiusCM,
		0.f // flat
	);

	return StarLocation + OrbitPosition;
}

FVector2D PlanetOrbitUtils::Get2DOrbitPosition(float Radius, float OrbitAngleDegrees, float YTilt)
{
	const float AngleRad = FMath::DegreesToRadians(OrbitAngleDegrees);

	return FVector2D(
		FMath::Cos(AngleRad) * Radius,
		FMath::Sin(AngleRad) * Radius * YTilt
	);
}

