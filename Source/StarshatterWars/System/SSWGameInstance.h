// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectGlobals.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"

#include "Kismet/DataTableFunctionLibrary.h"
#include "SSWGameInstance.generated.h"

/**
 * 
 */

 class UUniverse;
 class USim;
 class AGalaxy;
 class AGameDataLoader;
 class DataLoader;

/************************************************************************/
/* ENUMS                                                                */
/************************************************************************/
UENUM(BlueprintType)
enum class EGraphicsRHI : uint8 {
	RHI_DX11 UMETA(DisplayName = "DirectX 11"),
	RHI_DX12 UMETA(DisplayName = "DirectX 12"),
	RHI_VULKAN UMETA(DisplayName = "Vulkan")
};

UENUM(BlueprintType)
enum class EMODE : uint8 {
	MENU_MODE UMETA(DisplayName = "Main Menu"),// main menu
	CLOD_MODE UMETA(DisplayName = "Loading Campaign"), // loading campaign
	CMPN_MODE UMETA(DisplayName = "Operational Campaign"), // operational command for dynamic campaign
	PREP_MODE UMETA(DisplayName = "Loading Mission Info"), // loading mission info for planning
	PLAN_MODE UMETA(DisplayName = "Loading Briefing"),// mission briefing
	LOAD_MODE UMETA(DisplayName = "Loading Mission"), // loading mission into simulator
	PLAY_MODE UMETA(DisplayName = "Active Simulation"),// active simulation
	EXIT_MODE  UMETA(DisplayName = "Shutting Down"), // shutting down
};

UENUM(BlueprintType)
enum EGAMESTATUS : uint8 
{
	OK UMETA(DisplayName = "OK"),
	RUN UMETA(DisplayName = "Run"),
	EXIT UMETA(DisplayName = "Exit"),
	PANIC UMETA(DisplayName = "Panic"),
	INIT_FAILED UMETA(DisplayName = "Init Failed"),
	TOO_MANY UMETA(DisplayName = "Too Many Instances")
};

UENUM(BlueprintType)
enum DEBUGLEVEL : uint8 
{
	OFF UMETA(DisplayName = "OFF"),
	LEAKS UMETA(DisplayName = "Leaks"),
	PERIODIC UMETA(DisplayName = "Periodic"),
	MAXIMAL UMETA(DisplayName = "Maximal")
};

UENUM(BlueprintType)
enum ECAM_MODE : uint8 
{
	MODE_NONE UMETA(DisplayName = "Camera - None"),
	MODE_COCKPIT UMETA(DisplayName = "Camera - Cockpit"),
	MODE_CHASE UMETA(DisplayName = "Camera - Chase"),
	MODE_TARGET UMETA(DisplayName = "Camera - Target"),
	MODE_THREAT UMETA(DisplayName = "Camera - Threat"),
	MODE_ORBIT UMETA(DisplayName = "Camera - Orbit"),
	MODE_VIRTUAL UMETA(DisplayName = "Camera - Virtual"),
	MODE_TRANSLATE UMETA(DisplayName = "Camera - Translate"),
	MODE_ZOOM UMETA(DisplayName = "Camera - Zoom"),
	MODE_DOCKING UMETA(DisplayName = "Camera - Docking"),
	MODE_DROP UMETA(DisplayName = "Camera - Drop"),

	MODE_LAST UMETA(DisplayName = "Camera - Last")
};

enum CAMPAIGN_CONSTANTS {
	TRAINING_CAMPAIGN = 1,
	DYNAMIC_CAMPAIGN,
	MOD_CAMPAIGN = 100,
	SINGLE_MISSIONS = 1000,
	MULTIPLAYER_MISSIONS,
	CUSTOM_MISSIONS,

	NUM_IMAGES = 6
};

enum COMPARISON_OPERATOR {
	LT, LE, GT, GE, EQ,    // absolute score comparison
	RLT, RLE, RGT, RGE, REQ    // delta score comparison
};

UENUM(BlueprintType)
enum EDAMAGE_STATUS : uint8 
{
	DS_DESTROYED UMETA(DisplayName = "Damage - Destroyed"),
	DS_CRITICAL UMETA(DisplayName = "Damage - Critical"),
	DS_DEGRADED UMETA(DisplayName = "Damage - Degraded"),
	DS_NOMINAL UMETA(DisplayName = "Damage - Nominal"),
	DS_REPLACE UMETA(DisplayName = "Damage - Replace"),
	DS_REPAIR UMETA(DisplayName = "Damage - Repair"),
};

UENUM(BlueprintType)
enum EDAMAGE_TYPE : uint8 
{
	DAMAGE_NONE, 
	DAMAGE_EFFICIENCY = 0x01,
	DAMAGE_SAFETY = 0x02,
	DAMAGE_STABILITY = 0x04
};

UENUM(BlueprintType)
enum ECAMPAIGN_STATUS : uint8 {
	CAMPAIGN_INIT UMETA(DisplayName = "Campaign - Init"),
	CAMPAIGN_ACTIVE UMETA(DisplayName = "Campaign - Init"),
	CAMPAIGN_SUCCESS UMETA(DisplayName = "Campaign - Success"),
	CAMPAIGN_FAILED UMETA(DisplayName = "Campaign - Failed")
};

UENUM(BlueprintType)
enum ECOMMAND_MODE : uint8 {
	MODE_ORDERS UMETA(DisplayName = "Command - Orders"),
	MODE_THEATER UMETA(DisplayName = "Command - Theater"),
	MODE_FORCES UMETA(DisplayName = "Command - Forces"),
	MODE_INTEL UMETA(DisplayName = "Command - Intel"),
	MODE_MISSIONS UMETA(DisplayName = "Command - Missions"),
	NUM_MODES 
};

UENUM(BlueprintType)
enum ECOMBATACTION_TYPE : uint8
{
	NO_ACTION UMETA(DisplayName = "No Action"),
	STRATEGIC_DIRECTIVE UMETA(DisplayName = "Strategic Directive"),
	ZONE_ASSIGNMENT UMETA(DisplayName = "Zone Assignment"),
	SYSTEM_ASSIGNMENT UMETA(DisplayName = "System Assignment"),
	MISSION_TEMPLATE UMETA(DisplayName = "Mission Template"),
	COMBAT_EVENT UMETA(DisplayName = "Combat Event0"), 
	INTEL_EVENT UMETA(DisplayName = "Intel Event"), 
	CAMPAIGN_SITUATION UMETA(DisplayName = "Campaign Situation"), 
	CAMPAIGN_ORDERS UMETA(DisplayName = "Campaign Orders"),
};

UENUM(BlueprintType)
enum  ECOMBATACTION_STATUS : uint8
{
	PENDING UMETA(DisplayName = "Pending"),
	ACTIVE UMETA(DisplayName = "Active"),
	SKIPPED UMETA(DisplayName = "Skipped"),
	FAILED UMETA(DisplayName = "Failed"),
	COMPLETE UMETA(DisplayName = "Complete")
};

UENUM(BlueprintType)
enum ECOMBATEVENT_TYPE : uint8 
{
	ATTACK,
	DEFEND,
	MOVE_TO,
	CAPTURE,
	STRATEGY,
	CAMPAIGN_START,
	STORY,
	CAMPAIGN_END,
	CAMPAIGN_FAIL
};

UENUM(BlueprintType)
enum ECOMBATEVENT_SOURCE : uint8 
{
	FORCOM,
	TACNET,
	INTEL,
	MAIL,
	NEWS
};

UENUM(BlueprintType)
enum ECOMBATGROUP_TYPE : uint8 
{
	NONE, 
	FORCE,           // Commander In Chief
	WING,                // Air Force
	INTERCEPT_SQUADRON,  // a2a fighter
	FIGHTER_SQUADRON,    // multi-role fighter
	ATTACK_SQUADRON,     // strike / attack
	LCA_SQUADRON,        // landing craft
	FLEET,               // Navy
	DESTROYER_SQUADRON,  // destroyer
	BATTLE_GROUP,        // heavy cruiser(s)
	CARRIER_GROUP,       // fleet carrier
	BATTALION,           // Army
	MINEFIELD,
	BATTERY,
	MISSILE,
	STATION,             // orbital station
	STARBASE,            // planet-side base
	C3I,                 // Command, Control, Communications, Intelligence
	COMM_RELAY,
	EARLY_WARNING,
	FWD_CONTROL_CTR,
	ECM,
	SUPPORT,
	COURIER,
	MEDICAL,
	SUPPLY,
	REPAIR,
	CIVILIAN,            // root for civilian groups
	WAR_PRODUCTION,
	FACTORY,
	REFINERY,
	RESOURCE,
	INFRASTRUCTURE,
	TRANSPORT,
	NETWORK,
	HABITAT,
	STORAGE,
	NON_COM,             // other civilian traffic
	FREIGHT,
	PASSENGER,
	PRIVATE
};

UENUM(BlueprintType)
enum EHUDModes : uint8
{
	HUD_MODE_OFF UMETA(DisplayName = "HUDMode Off"),
	HUD_MODE_TAC UMETA(DisplayName = "HUDMode Tactical"),
	HUD_MODE_NAV UMETA(DisplayName = "HUDMode Navigation"), 
	HUD_MODE_ILS UMETA(DisplayName = "HUDMode ILS")
};

UENUM(BlueprintType)
enum class ESPECTRAL_CLASS : uint8 
{
	O UMETA(DisplayName = "O Class Star"),
	B UMETA(DisplayName = "B Class Star"),
	A UMETA(DisplayName = "A Class Star"),
	F UMETA(DisplayName = "F Class Star"),
	G UMETA(DisplayName = "G Class Star"),
	K UMETA(DisplayName = "K Class Star"),
	M UMETA(DisplayName = "M Class Star"),
	R UMETA(DisplayName = "R Class Star"),
	N UMETA(DisplayName = "N Class Star"),
	S UMETA(DisplayName = "S Class Star"),
	BLACK_HOLE UMETA(DisplayName = "Black Hole"),
	WHITE_DWARF UMETA(DisplayName = "White Dwarf"),
	RED_GIANT UMETA(DisplayName = "Red Giant"),
	UNKNOWN UMETA(DisplayName = "Unknown"),
};

UENUM(BlueprintType)
enum class ESTAR_SIZE : uint8
{
	Ia UMETA(DisplayName = "Ia"),
	Ib UMETA(DisplayName = "Ib"),
	II UMETA(DisplayName = "II"),
	III UMETA(DisplayName = "III"),
	IV UMETA(DisplayName = "IV"),
	V UMETA(DisplayName = "V"),
	VI UMETA(DisplayName = "VI"),
	VII UMETA(DisplayName = "VII"),
};

UENUM(BlueprintType)
enum class EEMPIRE_NAME : uint8
{
	Terellian_Alliance UMETA(DisplayName = "Terellian Alliance"),
	Marakan_Hegemony UMETA(DisplayName = "Marakan Hegemony"),
	INDEPENDENT_SYSTEMS UMETA(DisplayName = "Independent Systems"),
	Dantari_Separatists UMETA(DisplayName = "Dantari_Separatists"),
	Other UMETA(DisplayName = "Other"),
	Pirate UMETA(DisplayName = "Pirate"),
};

UENUM(BlueprintType)
enum EWEATHER_STATE : uint8 
{
	CLEAR UMETA(DisplayName = "Clear"), 
	HIGH_CLOUDS UMETA(DisplayName = "High Clouds"), 
	MODERATE_CLOUDS UMETA(DisplayName = "Moderate Clouds"), 
	OVERCAST UMETA(DisplayName = "Overcast"), 
	FOG UMETA(DisplayName = "Fog"), 
	STORM UMETA(DisplayName = "Storm"), 
	NUM_STATES
};

UENUM(BlueprintType)
enum EOrbitalType : uint8 
{ 
	NOTHING UMETA(DisplayName = "Nothing"),
	STAR UMETA(DisplayName = "Star"),
	PLANET UMETA(DisplayName = "Planet"),
	MOON UMETA(DisplayName = "Moon"),
	REGION UMETA(DisplayName = "Region"),
	TERRAIN UMETA(DisplayName = "Terrain")
};

UENUM()
enum class CLASSIFICATION  : uint32
{
	EMPTY, 
	DRONE = 0x0001,
	FIGHTER = 0x0002,
	ATTACK = 0x0004,
	LCA = 0x0008,
	COURIER = 0x0010,
	CARGO = 0x0020,
	CORVETTE = 0x0040,
	FREIGHTER = 0x0080,
	FRIGATE = 0x0100,
	DESTROYER = 0x0200,
	CRUISER = 0x0400,
	BATTLESHIP = 0x0800,
	CARRIER = 0x1000,
	DREADNAUGHT = 0x2000,

	STATION = 0x4000,
	FARCASTER = 0x8000,

	MINE = 0x00010000,
	COMSAT = 0x00020000,
	DEFSAT = 0x00040000,
	SWACS = 0x00080000,

	BUILDING = 0x00100000,
	FACTORY = 0x00200000,
	SAM = 0x00400000,
	EWR = 0x00800000,
	C3I = 0x01000000,
	STARBASE = 0x02000000,

	DROPSHIPS = 0x0000000f,
	STARSHIPS = 0x0000fff0,
	SPACE_UNITS = 0x000f0000,
	GROUND_UNITS = 0xfff00000
};

enum INTEL_TYPE {
	NOINTEL,
	RESERVE,       // out-system reserve: this group is not even here
	SECRET,        // enemy is completely unaware of this group
	KNOWN,         // enemy knows this group is in the system
	LOCATED,       // enemy has located at least the lead ship
	TRACKED        // enemy is tracking all elements
};

USTRUCT(BlueprintType)
struct FS_Galaxy : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString  Name;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int  Class;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector  Location;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Iff;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EEMPIRE_NAME Empire;

	FS_Galaxy() {
		Name = "";
		Class = (int) ESPECTRAL_CLASS::G;
		Location = FVector::ZeroVector;
		Iff = 0;
		Empire = EEMPIRE_NAME::Terellian_Alliance;
	}
};

USTRUCT(BlueprintType)
struct FS_StarSystem : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString System;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ESPECTRAL_CLASS Class;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ESTAR_SIZE Size;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;

	FS_StarSystem() {
	
	}
};

USTRUCT(BlueprintType)
struct FS_Star : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString  Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString  Image;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString  Map;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Light = 0.0;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Radius = 0.0;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Rot = 0.0;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Mass = 0.0;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Orbit = 0.0;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Tscale = 1.0;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool   Retro = false;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor  Color;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor  Back;

	FS_Star() {
		Name = "";
		Image = "";
		Map = "";
		Light = 0.0;
		Radius = 0.0;
		Rot = 0.0;
		Mass = 0.0;
		Orbit = 0.0;
		Tscale = 1.0;
		Retro = false;
		Color = FColor(0, 0, 0, 0);
		Back = FColor(0, 0, 0, 0);
	}
};

USTRUCT(BlueprintType)
struct FS_Planet : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Image;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Map;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString High;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Rings;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Glow;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString GlowHigh;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Gloss;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Radius;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Mass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Orbit;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Rot;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Minrad;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Maxrad;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Tscale;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Tilt;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool   Retro;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool   Lumin;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor  Atmos;
	FS_Planet() {
		Name = "";
		Image = "";
		Map = "";
		High = "";
		Rings = "";
		Glow = "";
		GlowHigh = "";
		Gloss = "";

		Radius = 0.0;
		Mass = 0.0;
		Orbit = 0.0;
		Rot = 0.0;
		Minrad = 0.0;
		Maxrad = 0.0;
		Tscale = 1.0;
		Tilt = 0.0;
		Retro = false;
		Lumin = false;
		Atmos = FColor::Black;
	}
};

USTRUCT(BlueprintType)
struct FS_Moon : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Image;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Map;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString High;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString GlowHigh;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Gloss;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Radius;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Mass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Orbit;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Rot;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Tscale;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Tilt;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool   Retro;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor  Atmos;
	
	FS_Moon() {
		Name = "";
		Image = "";
		Map = "";
		High = "";
		GlowHigh = "";
		Gloss = "";

		Radius = 0.0;
		Mass = 0.0;
		Orbit = 0.0;
		Rot = 0.0;
		Tscale = 1.0;
		Tilt = 0.0;
		Retro = false;
		Atmos = FColor::Black;
	}
};

USTRUCT(BlueprintType)
struct FS_Region : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FString> Link;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Size;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Orbit;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Grid;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Inclination;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Asteroids;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Parent;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TEnumAsByte<EOrbitalType> Type;
	
	FS_Region() {
		Name = "";
		Size = 1.0e6;
		Orbit = 0.0;
		Grid = 25000;
		Inclination = 0;
		Asteroids = 0;
		Parent = "",
		Type = EOrbitalType::NOTHING;
	}
};




UCLASS()
class STARSHATTERWARS_API USSWGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USSWGameInstance(
		const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Variables")
	FString ProjectPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Variables")
	FString FilePath;

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void SetProjectPath();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	FString GetProjectPath();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void Print(FString Msg, FString File);

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void SpawnGalaxy();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void GetGameData();

	UFUNCTION(BlueprintCallable, Category = "Game Variables")
	void StartGame();

	AGalaxy* GameGalaxy;
	AGameDataLoader* GameData;

	UPROPERTY()
	double StarDate;

	DataLoader* loader;
	FS_Galaxy* Galaxy;

	class UDataTable* StarsDataTable;
	class UDataTable* PlanetsDataTable;
	class UDataTable* MoonsDataTable;

	FString GetEmpireNameFromType(EEMPIRE_NAME emp);

protected:
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual bool InitContent();
	virtual bool InitGame();

	UPROPERTY()
	FString AppName;
	UPROPERTY()
	FString TitleText;
	UPROPERTY()
	FString PaletteName;

	// Internal variables for the state of the app
	UPROPERTY()
	bool              bIsWindowed;
	UPROPERTY()
	bool              bIsActive;
	UPROPERTY()
	bool              bIsDeviceLost;
	UPROPERTY()
	bool              bIsMinimized;
	UPROPERTY()
	bool              bIsMaximized;
	UPROPERTY()
	bool              bIgnoreSizeChange;
	UPROPERTY()
	bool              bIsDeviceInitialized;
	UPROPERTY()
	bool              bIsDeviceRestored;

	EGAMESTATUS Status;

	private:
		bool bUniverseLoaded;

};
