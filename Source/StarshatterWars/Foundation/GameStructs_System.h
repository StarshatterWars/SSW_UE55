/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.
    All Rights Reserved.

    FILE:         GameStructs_System.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Unreal-native enums + structs for "ship systems" (components).
    Intended as the shared data model for:
      - DEF ingestion (components.def, ship systems defs)
      - DataTables (static component definitions)
      - Runtime instances (damage, power, cooldowns, ammo, etc.)
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameStructs_System.generated.h"

UENUM(BlueprintType)
enum class EPowerSource : uint8
{
	NONE        UMETA(DisplayName = "None"),

	BATTERY     UMETA(DisplayName = "Battery"),
	AUXILIARY   UMETA(DisplayName = "Auxiliary"),
	FUSION      UMETA(DisplayName = "Fusion"),

	MAX         UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDriveType : uint8
{
	PLASMA      UMETA(DisplayName = "Plasma"),
	FUSION      UMETA(DisplayName = "Fusion"),
	GREEN       UMETA(DisplayName = "Green/Alien"),
	RED         UMETA(DisplayName = "Red"),
	BLUE        UMETA(DisplayName = "Blue"),
	YELLOW      UMETA(DisplayName = "Yellow"),
	STEALTH     UMETA(DisplayName = "Stealth"),
	UNKNOWN     UMETA(DisplayName = "Unknown"),
};

UENUM(BlueprintType)
enum class EQuantumDriveType : uint8
{
	QUANTUM UMETA(DisplayName = "Quantum"),
	HYPER   UMETA(DisplayName = "Hyper"),
};

UENUM(BlueprintType)
enum class EThrusterPortDir : uint8
{
	BOTTOM UMETA(DisplayName = "Bottom"),
	TOP    UMETA(DisplayName = "Top"),
	LEFT   UMETA(DisplayName = "Left"),
	RIGHT  UMETA(DisplayName = "Right"),
	FORE   UMETA(DisplayName = "Fore"),
	AFT    UMETA(DisplayName = "Aft"),
};

UENUM(BlueprintType)
enum class ENavLightType : uint8
{
	TYPE_1 UMETA(DisplayName = "Type 1"),
	TYPE_2 UMETA(DisplayName = "Type 2"),
	TYPE_3 UMETA(DisplayName = "Type 3"),
	TYPE_4 UMETA(DisplayName = "Type 4"),
};

USTRUCT(BlueprintType)
struct FNavLightBeacon
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENavLightType Type = ENavLightType::TYPE_1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	// Legacy: DWORD pattern
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Pattern = 0;
};

USTRUCT(BlueprintType)
struct FShipNavLight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Abbrev;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignName;

	// Legacy defaults:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale = 1.0f;      // dscale (navlight visual scale, not ship scale)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Period = 10.0f;

	// Beacons (up to NavLight::MAX_LIGHTS in legacy)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FNavLightBeacon> Beacons;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FThrusterPort
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EThrusterPortDir Direction = EThrusterPortDir::AFT;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	// Legacy: DWORD fire
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Fire = 0;

	// Legacy: port_scale (defaults to tscale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PortScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FShipThruster
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDriveType Type = EDriveType::UNKNOWN;

	// Legacy defaults: thrust=100, tscale=1
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Thrust = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrusterScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Size = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HullFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FThrusterPort> Ports;

	// Legacy: SetSourceIndex(reactors.size()-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SourceIndex = INDEX_NONE;

	// Blueprint/DataTable friendly:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon1 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon2 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon3 = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipFarcaster
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignName;

	// Legacy defaults: capacity=300e3, consumption=15e3
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Capacity = 300000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Consumption = 15000.0;

	// Legacy: loc scaled by ship scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	// Legacy: size scaled by ship scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Size = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HullFactor = 0.5f;

	// Legacy: SetStartPoint/SetEndPoint (scaled)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndPoint = FVector::ZeroVector;

	// Legacy: up to Farcaster::NUM_APPROACH_PTS
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> ApproachPoints;

	// Legacy: caster->SetSourceIndex(reactors.size()-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SourceIndex = INDEX_NONE;

	// Blueprint/DataTable friendly:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon1 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon2 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon3 = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipQuantum
{
	GENERATED_BODY()

	// Legacy: design_name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignName;

	// Legacy: abrv
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Abbrev;

	// Legacy: type/subtype
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EQuantumDriveType Type = EQuantumDriveType::QUANTUM;

	// Legacy defaults: capacity=250e3, consumption=1e3
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Capacity = 250000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Consumption = 1000.0;

	// Legacy: loc scaled by ship scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	// Legacy: size scaled by ship scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Size = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HullFactor = 0.5f;

	// Legacy: countdown/jump_time default 5
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Countdown = 5.0f;

	// Legacy: SetSourceIndex(reactors.size()-1)
	// Store it so runtime wiring can resolve it later:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SourceIndex = INDEX_NONE;

	// Blueprint/DataTable friendly:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Emcon1 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Emcon2 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Emcon3 = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipPower : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString  DesignName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString PName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString PAbrv;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EPowerSource Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int   SType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int   EType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int   Emcon1;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int   Emcon2;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int   Emcon3;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int   ExplosionType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Output;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Fuel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Size;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Hull;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Loc;

	FShipPower() {
		DesignName = "";
		PName = "";
		PAbrv = "";
		Type = EPowerSource::NONE;
		SType = 0;
		EType = 0;
		Emcon1 = -1;
		Emcon2 = -1;
		Emcon3 = -1;
		ExplosionType = 0;


		Output = 1000.0f;
		Fuel = 0.0f;
		Size = 0.0f;
		Hull = 0.5f;
		Loc = FVector::ZeroVector;
	}
};

USTRUCT(BlueprintType)
struct FDrivePort
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlareScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FShipDrive
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Abbrev;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DesignName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDriveType Type = EDriveType::UNKNOWN;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Thrust = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Augmenter = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DriveScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Size = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HullFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ExplosionType = 0;

	// Blueprint/DataTable friendly:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon1 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon2 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon3 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShowTrail = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FDrivePort> Ports;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString SourceFile;
};

USTRUCT(BlueprintType)
struct FFlightDeckSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	// Legacy: DWORD filter (bitmask)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FilterMask = 0xF;
};

USTRUCT(BlueprintType)
struct FShipFlightDeck
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Abbrev;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignName;

	// Deck role flags (legacy mutually exclusive usage: launch else recovery)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLaunchDeck = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRecoveryDeck = false;

	// Legacy: azimuth (may be in degrees depending on legacy 'degrees' flag)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AzimuthRadians = 0.0f;

	// Legacy: deck->Mount(loc,size,hull)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Size = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HullFactor = 0.5f;

	// Legacy: start/end/cam/box (scaled)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CamLocation = FVector::ZeroVector;

	// Legacy: SetBoundingBox(box) where box is a Vec3
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoundingBox = FVector::ZeroVector;

	// Legacy: explosion type + optional light + cycle time
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExplosionType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Light = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CycleTime = 0.0f;

	// Legacy: approach points, runway points, slots
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> ApproachPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> RunwayPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FFlightDeckSlot> Slots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FLandingGearItem
{
	GENERATED_BODY()

	// Legacy: mod_name (Model::Load(mod_name, scale))
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ModelName;

	// Legacy: v1 * scale, v2 * scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Start = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector End = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FShipLandingGear
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Abbrev;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignName;

	// Legacy: up to LandingGear::MAX_GEAR
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLandingGearItem> GearItems;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FWeaponDesignMeta
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WeaponType;

	// Legacy: meta->turret_model != 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTurret = false;

	// Legacy: meta->scale (used when turret)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeaponScale = 1.0f;

	// Legacy flags:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDecoy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsProbe = false;
};

// -----------------------------------------------------------------------------
// Aim Limits (GameStructs_System.h)
// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FWeaponAimLimits
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHasAzMax = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AzMax = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHasAzMin = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AzMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHasElMax = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ElMax = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHasElMin = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ElMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHasAzRest = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AzRest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHasElRest = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ElRest = 0.0f;
};

// -----------------------------------------------------------------------------
// Weapon Record (GameStructs_System.h)
// -----------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FShipWeapon
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString WeaponType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Abbrev;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DesignName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString GroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float   Size = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float   HullFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AzimuthRadians = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ElevationRadians = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FWeaponAimLimits Aim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ExplosionType = 0;

	// Stored raw (unscaled) by ParseWeapon, then scaled in ResolveWeaponsForCurrentShip:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FVector> Muzzles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SourceIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon1 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon2 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon3 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMuzzlesWereAuthoredInShipSpace = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMuzzlesResolved = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMuzzlesAreInShipSpace = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipHardPoint
{
	GENERATED_BODY()

	// Allowed weapon types for this hardpoint (legacy: wtypes[8])
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedWeaponTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Abbrev;

	// Legacy: hp->SetDesign(design) (string ref)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignName;

	// Mount + muzzle (both scaled by ship scale in legacy)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Muzzle = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Size = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HullFactor = 0.5f;

	// Orientation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AzimuthRadians = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElevationRadians = 0.0f;

	// EMCON
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon1 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon2 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon3 = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipLoadout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	// Legacy: GetDefArray(load->load, 16, ...)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> Stations;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipSensor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> Ranges; // max 8

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Size = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HullFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon1 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon2 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon3 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SourceIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipNavSystem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Size = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HullFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SourceIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipComputer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = TEXT("Computer");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Abbrev = TEXT("Comp");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignName;

	// Legacy: int comp_type = 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Type = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Size = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HullFactor = 0.5f;

	// Legacy: comp->SetSourceIndex(reactors.size()-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SourceIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipShield
{
	GENERATED_BODY()

	// Legacy identity
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ShieldType = 0; // must be >0 to be valid
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Abbrev;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DesignName;

	// Visual/sfx refs (parsing only, no loading)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString ModelName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString BoltHitSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString BeamHitSound;

	// Shield params (legacy doubles; float is fine for UE config-level data)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) double Factor = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) double Capacity = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) double Consumption = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) double Cutoff = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) double Curve = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) double DeflectionCost = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShieldCapacitor = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShieldBubble = false;

	// Mount
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Location = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float   Size = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float   HullFactor = 0.5f;

	// Explosion + EMCON
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ExplosionType = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon1 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon2 = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Emcon3 = -1;

	// Source (reactor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SourceIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipSquadron
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	// Legacy: s->design = Get(design)
	// Store the design key/string; resolve later.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DesignName;

	// Legacy defaults:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Avail = 4;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FDebris
{
	GENERATED_BODY()

	// Legacy: deb->model (loaded). Parsing only stores model name/key.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ModelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Mass = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Drag = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Life = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FireType = 0;

	// Legacy cap: 5
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> FireLocations;
};

USTRUCT(BlueprintType)
struct FExplosion
{
	GENERATED_BODY()

	// Legacy: exp->time
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0.0f;

	// Legacy: exp->type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Type = 0;

	// Legacy: exp->loc
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	// Legacy: exp->final
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFinal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipDeathSpiral
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0.0f; // death_spiral_time

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FExplosion> Explosions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDebris> Debris;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipMapSprite
{
	GENERATED_BODY()

	// Legacy: sprite_name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpriteName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FSkinMtlCell
{
	GENERATED_BODY()

	// These cover the typical legacy Skin::AddCell(...) patterns.
	// Keep them flexible until you paste ParseSkinMtl.

	// Optional: cell id / index / material slot:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CellIndex = INDEX_NONE;

	// Optional: material name or slot name:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MaterialName;

	// Optional: texture/bitmap key (legacy loads bitmaps; we store keys only):
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TextureName;

	// Optional: additional map (normal/spec/etc) if present in your .def:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AuxTextureName;
};

USTRUCT(BlueprintType)
struct FShipSkin
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSkinMtlCell> Cells;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SourceFile;
};

USTRUCT(BlueprintType)
struct FShipDesign : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString ShipName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DisplayName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Abrv;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Description;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	DetailName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	ShipClass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	CockpitName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	BeautyName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString HudIconName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DetailName0;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DetailName1;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DetailName2;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DetailName3;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int PCS;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ACS;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Detet;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int RepairTeams;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ShipType;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Scale;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float ExplosionScale;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Mass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Vlimit;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Agility;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AirFactor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float RollRate;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float PitchRate;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float YawRate;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float TurnBank;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float CockpitScale;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AutoRoll;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float CL;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float CD;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Stall;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Drag;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float ArcadeDrag;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float RollDrag;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float PitchDrag;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float YawDrag;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float PrepTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AvoidTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AvoidFighter;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AvoidStrike;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AvoidTarget;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float CommitRange;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float SplashRadius;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Scuttle;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float RepairSpeed;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Secret;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool RepairAuto;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool RepairScreen;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool WepScreen;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Degrees;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<float> FeatureSize;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<float> EFactor;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Trans;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FVector> Offset;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Spin;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector BeautyCam;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector	ChaseVec;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector	BridgeVec;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipPower> Power;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipDrive> Drive;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipQuantum> Quantum;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipFarcaster> Farcaster;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipThruster> Thruster;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipNavLight> Navlight;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipFlightDeck> FlightDeck;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipLandingGear> LandingGear;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipWeapon> Weapon;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipHardPoint> Hardpoint;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipLoadout> Loadout;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipSensor> Sensor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipNavSystem> NavSys;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipComputer> Computer;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipShield> Shield;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipSquadron> Squadron;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipDeathSpiral> DeathSpiral;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipMapSprite> Map;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FShipSkin> Skin;
	
	FShipDesign() {
		ShipName = "";
		DisplayName = "";
		Description = "";
		Abrv = "";

		DetailName0 = "";
		DetailName1 = "";
		DetailName2 = "";
		DetailName3 = "";

		ShipClass = "";
		CockpitName = "";
		BeautyName = "";
		HudIconName = "";

		PCS = 3.0f;
		ACS = 1.0f;
		Detet = 250.0e3f;
		RepairTeams = 2;
		ShipType = 0;


		Scale = 1.0f;
		ExplosionScale = 0.0f;
		Mass = 0;

		Vlimit = 8e3f;
		Agility = 2e2f;
		AirFactor = 0.1f;
		RollRate = 0.0f;
		PitchRate = 0.0f;
		YawRate = 0.0f;


		TurnBank = (float)(PI / 8);

		CockpitScale = 1.0f;
		AutoRoll = 0;

		CL = 0.0f;
		CD = 0.0f;
		Stall = 0.0f;
		Drag = 2.5e-5f;

		ArcadeDrag = 1.0f;
		RollDrag = 5.0f;
		PitchDrag = 5.0f;
		YawDrag = 5.0f;

		PrepTime = 30.0f;
		AvoidTime = 0.0f;
		AvoidFighter = 0.0f;
		AvoidStrike = 0.0f;
		AvoidTarget = 0.0f;
		CommitRange = 0.0f;

		SplashRadius = -1.0f;
		Scuttle = 5e3f;
		RepairSpeed = 1.0f;

		FeatureSize.SetNum(4);
		EFactor.SetNum(3);
		Offset.SetNum(4);

		EFactor[0] = 0.1f;
		EFactor[1] = 0.3f;
		EFactor[2] = 1.0f;

		Secret = false;
		RepairAuto = true;
		RepairScreen = true;
		WepScreen = true;
		Degrees = false;

		Trans = FVector::ZeroVector;

		Spin = FVector::ZeroVector;
		BeautyCam = FVector::ZeroVector;
		ChaseVec = FVector(0, -100, 20);
		BridgeVec = FVector::ZeroVector;
		for (int i = 0; i < 4; i++) {
			Offset[i] = FVector::ZeroVector;
		}
	}
};

