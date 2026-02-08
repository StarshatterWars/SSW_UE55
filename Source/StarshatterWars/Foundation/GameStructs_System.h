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

