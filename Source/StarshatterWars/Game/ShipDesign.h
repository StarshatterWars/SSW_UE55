/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         ShipDesign.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Starship Design parameters class
*/

#pragma once

#include "CoreMinimal.h"
#include "Types.h"
//#include "Bitmap.h"
#include "Geometry.h"
#include "Term.h"
#include "List.h"

// +----------------------------------------------------------------------+

class ShipDesign;
class Model;
class Skin;
class PowerSource;
class Weapon;
class HardPoint;
class Computer;
class Drive;
class QuantumDrive;
class Farcaster;
class Thruster;
class Sensor;
class NavLight;
class NavSystem;
class Shield;
class FlightDeck;
class LandingGear;
class System;
class Sound;

// +====================================================================+
/**
 * 
 */
class STARSHATTERWARS_API ShipDesign
{
public:
	static const char* TYPENAME() { return "ShipDesign"; }

	enum CONSTANTS {
		MAX_DEBRIS = 10,
		MAX_EXPLOSIONS = 10
	};

	ShipDesign();
	ShipDesign(const char* name, const char* path, const char* filename, bool secret = false);
	~ShipDesign();

	// public interface:
	static void          Initialize();
	static void          Close();
	static bool          CheckName(const char* name);
	static ShipDesign* Get(const char* design_name, const char* design_path = 0);
	static ShipDesign* FindModDesign(const char* design_name, const char* design_path = 0);
	static void          ClearModCatalog();
	static int           GetDesignList(int type, List<Text>& designs); // never destroy the design list!

	static int           ClassForName(const char* name);
	static const char* ClassName(int type);

	static int           LoadCatalog(const char* path, const char* file, bool mod = false);
	static void          LoadSkins(const char* path, const char* archive = 0);
	static void          PreloadCatalog(int index = -1);
	static int           StandardCatalogSize();

	int operator == (const ShipDesign& s) const { return !strncmp(name, s.name, 31); }

	// Parser:
	void  ParseShip(TermDef* def);

	void  ParsePower(TermStruct* val);
	void  ParseDrive(TermStruct* val);
	void  ParseQuantumDrive(TermStruct* val);
	void  ParseFarcaster(TermStruct* val);
	void  ParseThruster(TermStruct* val);
	void  ParseNavlight(TermStruct* val);
	void  ParseFlightDeck(TermStruct* val);
	void  ParseLandingGear(TermStruct* val);
	void  ParseWeapon(TermStruct* val);
	void  ParseHardPoint(TermStruct* val);
	void  ParseSensor(TermStruct* val);
	void  ParseNavsys(TermStruct* val);
	void  ParseComputer(TermStruct* val);
	void  ParseShield(TermStruct* val);
	void  ParseDeathSpiral(TermStruct* val);
	void  ParseExplosion(TermStruct* val, int index);
	void  ParseDebris(TermStruct* val, int index);
	void  ParseLoadout(TermStruct* val);
	void  ParseMap(TermStruct* val);
	void  ParseSquadron(TermStruct* val);
	Skin* ParseSkin(TermStruct* val);
	void  ParseSkinMtl(TermStruct* val, Skin* skin);

	// general information:
	const char* DisplayName() const;

	char              filename[64];
	char              path_name[64];
	char              name[64];
	char              display_name[64];
	char              abrv[16];
	int               type;
	float             scale;
	int               auto_roll;
	bool              valid;
	bool              secret;        // don't display in editor
	Text              description;   // background info for tactical reference

	// LOD representation:
	int               lod_levels;
	//List<Model>       models[4];
	List<Point>       offsets[4];
	float             feature_size[4];
	List<Point>       spin_rates;

	// player selectable skins:
	//List<Skin>        skins;
	//const Skin* FindSkin(const char* skin_name) const;

	// virtual cockpit:
	Model* cockpit_model;
	float             cockpit_scale;

	// performance:
	float             vlimit;
	float             agility;
	float             air_factor;
	float             roll_rate;
	float             pitch_rate;
	float             yaw_rate;
	float             trans_x;
	float             trans_y;
	float             trans_z;
	float             turn_bank;
	Vec3              chase_vec;
	Vec3              bridge_vec;
	Vec3              beauty_cam;

	float             prep_time;

	// physical data:
	float             drag, roll_drag, pitch_drag, yaw_drag;
	float             arcade_drag;
	float             mass, integrity, radius;

	// aero data:
	float             CL, CD, stall;

	// weapons:
	int               primary;
	int               secondary;

	// drives:
	int               main_drive;

	// visibility:
	float             pcs;           // passive sensor cross section
	float             acs;           // active sensor cross section
	float             detet;         // maximum detection range
	float             e_factor[3];   // pcs scaling by emcon setting

	// ai settings:
	float             avoid_time;
	float             avoid_fighter;
	float             avoid_strike;
	float             avoid_target;
	float             commit_range;

	// death spriral sequence:
	float             death_spiral_time;
	float             explosion_scale;
	//ShipExplosion     explosion[MAX_EXPLOSIONS];
	//ShipDebris        debris[MAX_DEBRIS];

	List<PowerSource> reactors;
	List<Weapon>      weapons;
	List<HardPoint>   hard_points;
	List<Drive>       drives;
	List<Computer>    computers;
	List<FlightDeck>  flight_decks;
	List<NavLight>    navlights;
	QuantumDrive* quantum_drive;
	Farcaster* farcaster;
	Thruster* thruster;
	Sensor* sensor;
	NavSystem* navsys;
	Shield* shield;
	Model* shield_model;
	Weapon* decoy;
	Weapon* probe;
	LandingGear* gear;

	float             splash_radius;
	float             scuttle;
	float             repair_speed;
	int               repair_teams;
	bool              repair_auto;
	bool              repair_screen;
	bool              wep_screen;

	Text              bolt_hit_sound;
	Text              beam_hit_sound;

	//Sound* bolt_hit_sound_resource;
	//Sound* beam_hit_sound_resource;

	//List<ShipLoad>    loadouts;
	//List<Bitmap>      map_sprites;
	//List<ShipSquadron>      squadrons;

	//Bitmap            beauty;
	//Bitmap            hud_icon;
};
