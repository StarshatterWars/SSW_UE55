/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         Camera.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Camera Class - Position and Point of View
*/

#pragma once

#include "CoreMinimal.h"
#include "Types.h"
#include "Geometry.h"
#include "UObject/NoExportTypes.h"
#include "Camera.generated.h"

/**
 * 
 */
UCLASS()
class STARSHATTERWARS_API UCamera : public UObject
{
	GENERATED_BODY()

public:	
	static const char* TYPENAME() { return "Camera"; }

	
	UCamera();

	void     Aim(double roll, double pitch, double yaw) { orientation.Rotate(roll, pitch, yaw); }
	void     Roll(double roll) { orientation.Roll(roll); }
	void     Pitch(double pitch) { orientation.Pitch(pitch); }
	void     Yaw(double yaw) { orientation.Yaw(yaw); }

	void     MoveTo(double x, double y, double z);
	void     MoveTo(const Point& p);
	void     MoveBy(double dx, double dy, double dz);
	void     MoveBy(const Point& p);

	void     Clone(const UCamera* cam);
	void     LookAt(const Point& target);
	void     LookAt(const Point& target, const Point& eye, const Point& up);
	bool     Padlock(const Point& target, double alimit = -1, double e_lo = -1, double e_hi = -1);

	Point    Pos() const { return pos; }
	Point    vrt() const { return Point(orientation(0, 0), orientation(0, 1), orientation(0, 2)); }
	Point    vup() const { return Point(orientation(1, 0), orientation(1, 1), orientation(1, 2)); }
	Point    vpn() const { return Point(orientation(2, 0), orientation(2, 1), orientation(2, 2)); }

	const Matrix& Orientation() const { return orientation; }

	void Initialize(double x = 0.0, double y = 0.0, double z = 0.0);

protected:
	Point    pos;
	Matrix   orientation;
	
	
};
