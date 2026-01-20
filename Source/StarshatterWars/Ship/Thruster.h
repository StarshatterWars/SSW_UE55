/*  Game:        Starshatter Wars
    Project:     Starshatter 4.5 (Unreal Port)
    Studio:      Fractal Dev Studios
    Copyright:   2025-2026

    Original Author and Studio:
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         Thruster.h
    AUTHOR:       Carlos Bott


    OVERVIEW
    ========
    Conventional Thruster (system) class
*/

#pragma once

#include "Types.h"
#include "Math/Vector.h"

#include "SimSystem.h"   // Base class (was System)
#include "Geometry.h"    // For Physical (Starshatter core type)

// +--------------------------------------------------------------------+

class Ship;
class Physical;
class UTexture2D;

// +--------------------------------------------------------------------+

struct ThrusterPort
{
    static const char* TYPENAME() { return "ThrusterPort"; }

    ThrusterPort(int t, const FVector& l, DWORD f, float s);
    ~ThrusterPort();

    int         type;
    DWORD       fire;
    float       burn;
    float       scale;
    FVector     loc;

    UTexture2D* flare;
    UTexture2D* trail;
};

// +--------------------------------------------------------------------+

class Thruster : public SimSystem
{
public:
    static const char* TYPENAME() { return "Thruster"; }

    enum Constants {
        LEFT, RIGHT, FORE, AFT, TOP, BOTTOM,
        YAW_L, YAW_R, PITCH_D, PITCH_U, ROLL_L, ROLL_R
    };

    Thruster(int dtype, double thrust, float flare_scale = 0);
    Thruster(const Thruster& rhs);
    virtual ~Thruster();

    static void       Initialize();
    static void       Close();

    virtual void      ExecFrame(double seconds);
    virtual void      ExecTrans(double x, double y, double z);
    virtual void      SetShip(Ship* s);

    virtual double    TransXLimit();
    virtual double    TransYLimit();
    virtual double    TransZLimit();

    virtual void      AddPort(int type, const FVector& loc, DWORD fire, float flare_scale = 0);
    virtual void      CreatePort(int type, const FVector& loc, DWORD fire, float flare_scale);

    int               NumThrusters()       const;
    UTexture2D* Flare(int engine)    const;
    UTexture2D* Trail(int engine)    const;

    virtual void      Orient(const Physical* rep);
    virtual double    GetRequest(double seconds) const;

protected:
    void              IncBurn(int inc, int dec);
    void              DecBurn(int a, int b);

    Ship* ship;
    float             thrust;
    float             scale;
    float             burn[12];

    float             avail_x, avail_y, avail_z;
    float             trans_x, trans_y, trans_z;
    float             roll_rate, pitch_rate, yaw_rate;
    float             roll_drag, pitch_drag, yaw_drag;

    List<ThrusterPort> ports;
};
