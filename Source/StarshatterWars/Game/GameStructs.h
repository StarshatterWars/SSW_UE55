// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/
// /*  Project nGenEx	Fractal Dev Games	Copyright (C) 2024. All Rights Reserved.	SUBSYSTEM:    SSW	FILE:         Game.cpp	AUTHOR:       Carlos Bott*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Blueprint/UserWidget.h"
#include "GameStructs.generated.h"

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
enum class EMISSIONSTATUS : uint8 {
	Available UMETA(DisplayName = "Available"),
	Complete UMETA(DisplayName = "Complete"),
	Ready UMETA(DisplayName = "Ready"),
	Active UMETA(DisplayName = "Active"),
	Pending UMETA(DisplayName = "Pending"),
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
enum class EINTEL_TYPE : uint8 {
	UNKNOWN		UMETA(DisplayName = "Unknown"), // This status should not exist in game
	RESERVE		UMETA(DisplayName = "Reserve"), // out-system reserve: this group is not even here
	SECRET		UMETA(DisplayName = "Secret"),   // enemy is completely unaware of this group
	KNOWN		UMETA(DisplayName = "Known"),    // enemy knows this group is in the system
	LOCATED		UMETA(DisplayName = "Located"),  // enemy has located at least the lead ship
	TRACKED		UMETA(DisplayName = "Tracked"),  // enemy is tracking all elements
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
enum class ECOMBATGROUP_TYPE : uint8 
{
	NONE, 
	FORCE				UMETA(DisplayName = "Force"), // Commander In Chief
	WING				UMETA(DisplayName = "Tactical Fighter Wing"), // Air Force
	INTERCEPT_SQUADRON  UMETA(DisplayName = "Intercept Squadron"), // a2a fighter
	FIGHTER_SQUADRON	UMETA(DisplayName = "Fighter Squadron"),   // multi-role fighter
	ATTACK_SQUADRON		UMETA(DisplayName = "Attack Squadron"),    // strike / attack
	LCA_SQUADRON		UMETA(DisplayName = "LCA Squadron"),       // landing craft
	FLEET				UMETA(DisplayName = "Fleet"),		       // Navy
	DESTROYER_SQUADRON	UMETA(DisplayName = "DESRON"),			   // destroyer
	BATTLE_GROUP		UMETA(DisplayName = "BG"),                 // heavy cruiser(s)
	CARRIER_GROUP		UMETA(DisplayName = "CVBG"),       	       // fleet carrier
	BATTALION			UMETA(DisplayName = "Battalion"),		   // Army
	MINEFIELD			UMETA(DisplayName = "Minefield"),
	BATTERY				UMETA(DisplayName = "Battery"), 
	MISSILE				UMETA(DisplayName = "Missile"),
	STATION				UMETA(DisplayName = "Station"),            // orbital station
	STARBASE			UMETA(DisplayName = "Starbase"),           // planet-side base
	C3I					UMETA(DisplayName = "C3I"),                // Command, Control, Communications, Intelligence
	COMM_RELAY			UMETA(DisplayName = "Comm Relay"),
	EARLY_WARNING		UMETA(DisplayName = "Early Warning"), 
	FWD_CONTROL_CTR		UMETA(DisplayName = "Forward Control Center"), 
	ECM					UMETA(DisplayName = "ECM"),
	SUPPORT				UMETA(DisplayName = "Support"), 
	COURIER				UMETA(DisplayName = "Courier"), 
	MEDICAL				UMETA(DisplayName = "Medical"), 
	SUPPLY				UMETA(DisplayName = "Supply"), 
	REPAIR				UMETA(DisplayName = "Repair"), 
	CIVILIAN			UMETA(DisplayName = "Civilian"),     // root for civilian groups
	WAR_PRODUCTION		UMETA(DisplayName = "War Production"), 
	FACTORY				UMETA(DisplayName = "Factory"), 
	REFINERY			UMETA(DisplayName = "Refinery"),
	RESOURCE			UMETA(DisplayName = "Resource"), 
	INFRASTRUCTURE		UMETA(DisplayName = "Infrastructure"), 
	TRANSPORT			UMETA(DisplayName = "Transport"), 
	NETWORK				UMETA(DisplayName = "Network"), 
	HABITAT				UMETA(DisplayName = "Habitat"), 
	STORAGE				UMETA(DisplayName = "Storage"), 
	NON_COM				UMETA(DisplayName = "Non-Com"),       // other civilian traffic
	FREIGHT				UMETA(DisplayName = "Freight"), 
	PASSENGER			UMETA(DisplayName = "Passenger"), 
	PRIVATE				UMETA(DisplayName = "Private"),
	UNKNOWN				UMETA(DisplayName = "Unknown"),
};

UENUM(BlueprintType)
enum class ECOMBATUNIT_TYPE : uint8 
{
	NONE				UMETA(DisplayName = "None"),
	FIGHTER				UMETA(DisplayName = "Fighter"),
	ATTACK				UMETA(DisplayName = "Attack"),
	INTERCEPT			UMETA(DisplayName = "Interceptor"),
	LCA					UMETA(DisplayName = "Landing Craft"),
	CRUISER				UMETA(DisplayName = "Cruiser"),
	DESTROYER			UMETA(DisplayName = "Destroyer"),
	FRIGATE				UMETA(DisplayName = "Frigate"),
	CARRIER				UMETA(DisplayName = "Carrier"),
	MINE				UMETA(DisplayName = "Mine"),
	BATTERY				UMETA(DisplayName = "Battery"),
	STATION				UMETA(DisplayName = "Station"),
	STARBASE			UMETA(DisplayName = "Starbase"),
};

UENUM(BlueprintType)
enum class EGroupType : uint8 {
    Force,
    Fleet,
    CarrierGroup,
    BattleGroup,
    DestroyerSquadron,
    Wing,
    InterceptSquadron,
    FighterSquadron,
    AttackSquadron,
    LcaSquadron,
    Unknown
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
	Terellian	UMETA(DisplayName = "Terellian Alliance"),
	Marakan		UMETA(DisplayName = "Marakan Hegemony"),
	Independent UMETA(DisplayName = "Independent Systems"),
	Dantari		UMETA(DisplayName = "Dantari Separatists"),
	Zolon		UMETA(DisplayName = "Zolon Empire"),
	Other		UMETA(DisplayName = "Other"),
	Pirate		UMETA(DisplayName = "Brotherhood of Iron"),
	Neutral     UMETA(DisplayName = "Neutral"),
	Unknown     UMETA(DisplayName = "Unknown"),
	Silessian   UMETA(DisplayName = "Silessian Confederacy"),
	Solus		UMETA(DisplayName = "Independent System of Solus"),
	Haiche		UMETA(DisplayName = "Haiche Protectorate"),
	NONE		UMETA(DisplayName = "Show All"),
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

UENUM(BlueprintType)
enum EControlType : uint8
{ 
	WINDEF_LABEL,
	WINDEF_BUTTON,
	WINDEF_COMBO,
	WINDEF_EDIT,
	WINDEF_IMAGE,
	WINDEF_SLIDER,
	WINDEF_LIST,
	WINDEF_RICH,
};

/**
 * STRUCTS
 */

USTRUCT()
struct FJumpLink
{
    GENERATED_BODY()

    FVector2D Start;
    FVector2D End;
};

USTRUCT(BlueprintType)
struct FUnitData {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Type;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Design;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RegNum;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;
};

USTRUCT(BlueprintType)
struct FGroupData : public FTableRowBase {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Id;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGroupType Type;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ParentId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGroupType ParentType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Region;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FUnitData> Units;
	FGroupData()
	{
		Id = 0;
		Name = "";
		Type = EGroupType::Unknown;
		ParentId = 0;
		ParentType = EGroupType::Unknown;
		Region = "";
		Location = FVector::ZeroVector; 
	}
};

USTRUCT(BlueprintType)
struct FS_PlayerGameInfo : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id;
	UPROPERTY(BlueprintReadWrite)
	FString Name;
	UPROPERTY(BlueprintReadWrite)
	FString Nickname;
	UPROPERTY(BlueprintReadWrite)
	int32 Campaign;
	UPROPERTY(BlueprintReadWrite)
	int32 Avatar;
	UPROPERTY(BlueprintReadWrite)
	int32 Mission;
	UPROPERTY(BlueprintReadWrite)
	int32 Rank;
	UPROPERTY(BlueprintReadWrite)
	int32 Empire;
	UPROPERTY(BlueprintReadWrite)
	int32 ShipColor;
	UPROPERTY(BlueprintReadWrite)
	int32 HudMode;
	UPROPERTY(BlueprintReadWrite)
	int32 GunMode;
	UPROPERTY(BlueprintReadWrite)
	int32 HudColor;
	UPROPERTY(BlueprintReadWrite)
	int32 FlightModel;
	UPROPERTY(BlueprintReadWrite)
	int32 LandingMode;
	UPROPERTY(BlueprintReadWrite)
	bool FlyingStart;
	UPROPERTY(BlueprintReadWrite)
	bool GridMode;
	UPROPERTY(BlueprintReadWrite)
	bool TrainingMode;
	UPROPERTY(BlueprintReadWrite)
	bool GunSightMode;
	UPROPERTY(BlueprintReadWrite)
	int64 CreateTime;

	UPROPERTY(BlueprintReadWrite)
	int64 GameTime;

	UPROPERTY(BlueprintReadWrite)
	int64 CampaignTime;

	UPROPERTY(BlueprintReadWrite)
	int64 FlightTime;
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerKills;
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerWins;
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerLosses;
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerPoints;
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerLevel;
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerExperience;
	UPROPERTY(BlueprintReadWrite)
	FString PlayerStatus;
	UPROPERTY(BlueprintReadWrite)
	FString PlayerShip;
	UPROPERTY(BlueprintReadWrite)
	FString PlayerRegion;
	UPROPERTY(BlueprintReadWrite)
	FString PlayerSystem;
	UPROPERTY(BlueprintReadWrite)
	int PlayerSquadron;
	UPROPERTY(BlueprintReadWrite)
	int PlayerWing;
	UPROPERTY(BlueprintReadWrite)
	int PlayerDesronGroup;
	UPROPERTY(BlueprintReadWrite)
	int PlayerBattleGroup;
	UPROPERTY(BlueprintReadWrite)
	int PlayerCarrier;
	UPROPERTY(BlueprintReadWrite)
	int PlayerFleet;
	UPROPERTY(BlueprintReadWrite)
	int PlayerForce;
	UPROPERTY(BlueprintReadWrite)
	TArray<bool> CampaignComplete;

	FS_PlayerGameInfo()
	{
		Id = 0;
		Name = "";
		Nickname = "";
		Campaign = 0;
		Avatar = -1;
		Mission = -1;
		Rank = 0;
		Empire = 0;
		ShipColor = 0;
		HudMode = 0;
		GunMode = 0;
		HudColor = 0;
		FlightModel = 0;
		LandingMode = 0;
		FlyingStart = false;
		GridMode = false;
		TrainingMode = false;
		GunSightMode = false;
		CreateTime = 0;
		FlightTime = 0;
		PlayerKills = 0;
		PlayerWins = 0;
		PlayerLosses = 0;
		PlayerPoints = 0;
		PlayerLevel = 0;
		PlayerExperience = 0;
		PlayerStatus = "";
		PlayerShip = "";
		PlayerSystem = "";
		PlayerRegion = "";
		PlayerForce = 1;
		PlayerFleet = -1;
		PlayerWing = -1;
		PlayerCarrier = - 1;
		PlayerBattleGroup = -1;
		PlayerDesronGroup = -1;
		PlayerSquadron = -1;
		
		GameTime = 0;
		CampaignTime = 0;
		FlightTime = 0;

		CampaignComplete.SetNum(5);
		for (int i = 0; i < CampaignComplete.Num(); i++) {
			CampaignComplete[i] = false;
		}
	}
};
USTRUCT(BlueprintType)
struct FS_MoonMap : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Icon;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Radius;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Mass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Orbit;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Inclination;
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

	FS_MoonMap() {
		Name = "";
		Icon = "";

		Radius = 0.0;
		Mass = 0.0;
		Orbit = 0.0;
		Inclination = 0.0;
		Rot = 0.0;
		Tscale = 1.0;
		Tilt = 0.0;
		Retro = false;
		Atmos = FColor::Black;
	}
};

USTRUCT(BlueprintType)
struct FS_PlanetMap : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString  Name;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString  Icon;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString  Ring;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Radius;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Mass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Orbit;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Inclination;
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
	double Perihelion;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Aphelion;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Eccentricity;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool   Retro;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor  Atmos;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MoonMap> Moon;

	FS_PlanetMap() {
		Name = "";
		Icon = "";
		Ring = "";
		Radius = 0.0;
		Mass = 0.0;
		Orbit = 0.0;
		Inclination = 0.0;
		Rot = 0.0;
		Minrad = 0.0;
		Maxrad = 0.0;
		Tscale = 1.0;
		Tilt = 0.0;
		Perihelion = 0.0f;
		Aphelion = 0.0f;
		Eccentricity = 0.0f;
		Retro = false;
		Atmos = FColor::Black;
	}
};

USTRUCT(BlueprintType)
struct FS_StarMap : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString  Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString  SystemName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString  Map;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString  Image;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	ESPECTRAL_CLASS  Class;
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
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_PlanetMap> Planet;

	FS_StarMap() {
		Name = "";
		SystemName = "";
		Map = "";
		Image = "";
		Light = 0.0;
		Radius = 0.0;
		Rot = 0.0;
		Mass = 0.0;
		Orbit = 0.0;
		Tscale = 1.0;
		Retro = false;
		Color = FColor(0, 0, 0, 0);
		Back = FColor(0, 0, 0, 0);
		Class = ESPECTRAL_CLASS::G;
	}
};

USTRUCT(BlueprintType)
struct FS_Galaxy : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString  Name;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString  Level;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	ESPECTRAL_CLASS  Class;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector  Location;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Star;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FString> Link;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray <FS_StarMap> Stellar;

	FS_Galaxy() {
		Name = "";
		Level = "";
		Class = ESPECTRAL_CLASS::G;
		Location = FVector::ZeroVector;
		Iff = 0;
		Empire = EEMPIRE_NAME::Terellian;
		Star = "";
	}
};

USTRUCT(BlueprintType)
struct FS_StarSky : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString SkyPolyStars;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString SkyNebula;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString SkyHaze;

	FS_StarSky() {
		SkyPolyStars = "";
		SkyNebula = "";
		SkyHaze = "";
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
		Parent = "";
		Type = EOrbitalType::NOTHING;
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
		Glow = "";
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
	double Inclination;
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
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_Moon> Moon;

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
		Inclination = 0.0;
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
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_Planet> Planet;

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
struct FS_StarSystem : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString SystemName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int SkyStars;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int SkyDust;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor AmbientColor = FColor::Black;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FS_StarSky StarSky;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_Star> Star;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FString> Link;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_Region> Region;
	
	FS_StarSystem() {
		SystemName = "";
		SkyStars = 0;
		SkyDust = 0;
		AmbientColor = FColor::Black;
	}
};

USTRUCT(BlueprintType)
struct FS_CampaignZone : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString  System;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString  Region;

	FS_CampaignZone() {
		System = "";
		Region = "";
	}
};

USTRUCT(BlueprintType)
struct FS_CampaignReq : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Action;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Status;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool NotAction;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Combatant1;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Combatant2;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int GroupType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int GroupId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Comp;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Score;

	FS_CampaignReq() {
		Action = 0;
		Status = "";
		NotAction = false;

		Combatant1 = "";
		Combatant2 = "";
		GroupType = 0;
		GroupId = 0;
		Comp = 0;
		Intel = 0;
		Score = 0;
	}
};

USTRUCT(BlueprintType)
struct FS_CampaignAction : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Subtype;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Team;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int OppType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Count;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int StartBefore;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int StartAfter;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int MinRank;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int MaxRank;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Delay;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int AssetId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int TargetId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int TargetIff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Probability;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString System;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Region;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Title;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Source;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Message;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Image;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Audio;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Date;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Scene;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString AssetType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TargetType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString AssetKill;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TargetKill;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_CampaignReq> Requirement;

	FS_CampaignAction() {
		Id = 0;
		Type = "";
		Subtype = 0;
		Team = 0;
		OppType = 0;
		Iff = 0;
		Count = 0;
		StartBefore = 0;;
		StartAfter = 0;
		
		MinRank = 0;
		MaxRank = 0;

		Delay = 0;
		Probability = 0;

		AssetId = 0;
		TargetId = 0;
		TargetIff = 0;

		Region = "";
		System = "";

		Title = "";
		Source = "";
		Message = "";
		Image = "";
		Scene = "";
		Audio = "";
		Date = "";

		AssetType = "";
		TargetType = "";

		AssetKill = "";
		TargetKill = "";

		Location = FVector::ZeroVector;
	}
};

USTRUCT(BlueprintType)
struct FS_CombatantGroup {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;

	FS_CombatantGroup() {
		Type = ECOMBATGROUP_TYPE::NONE;
		Id = 0;
	}
};


USTRUCT(BlueprintType)
struct FS_Combatant : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Size;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_CombatantGroup> Group;
	
	FS_Combatant() {
		Name = EEMPIRE_NAME::Unknown;
		Size = 0;
	}
};

USTRUCT(BlueprintType)
struct FS_CampaignMissionList : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Description;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Sitrep;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString System;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Region;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Objective;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Start;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Script;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TypeName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString MissionImage;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString MissionAudio;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EMISSIONSTATUS Status;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Available;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Complete;

	FS_CampaignMissionList() {
		
		Id = 0;
		Name = "";
		Description = "";
		TypeName = "";
		Sitrep = "";
		Objective = "";
		MissionImage = "";
		MissionAudio = "";
	
		System = "Unknown";
		Region = "Unknown";

		Start = "";
		Script = "";
		Type = 0;

		Available = true;
		Complete = false;

		Status = EMISSIONSTATUS::Available;
	}
};

USTRUCT(BlueprintType)
struct FS_CampaignTemplateList : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Script;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Region;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int MissionType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int GroupType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int MinRank;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int MaxRank;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ActionId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ActionStatus;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ExecOnce;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int StartBefore;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int StartAfter;

	FS_CampaignTemplateList() {

		Id = 0;
		Name = "";
		Script = "";
		Region = "";

		MissionType = 0;
		GroupType = 0;
		MinRank = 0;
		MaxRank = 0;
		ActionId = 0;
		ActionStatus = 0;
		ExecOnce = 0;
		StartBefore  = 0;
		StartAfter = 0;
	}
};

USTRUCT(BlueprintType)
struct FS_RLoc : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Reference;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Dex;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float DexVar;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Azimuth;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float AzimuthVar;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Elevation;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float ElevationVar;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector BaseLocation;

	FS_RLoc() {
		Reference = "";
		Dex = 0;
		DexVar = 0;
		Azimuth = 0;
		AzimuthVar = 0;
		Elevation = 0;
		ElevationVar = 0;

		BaseLocation = FVector::ZeroVector;
	}
};

USTRUCT(BlueprintType)
struct FS_MissionInstruction : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Formation;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Speed;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Priority;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Farcast;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Hold;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int EMCON;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString OrderName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString StatusName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString OrderRegionName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TargetName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TargetDesc;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_RLoc> RLoc;

	FS_MissionInstruction() {
		Formation = 0;
		Speed = 0;
		Priority = 1;
		Farcast = 0;
		Hold = 0;
		EMCON = 0;
		Location = FVector::ZeroVector;
		OrderName ="";
		StatusName = "";
		OrderRegionName = "";
		TargetName = "";
		TargetDesc = "";
	}
};

USTRUCT(BlueprintType)
struct FS_MissionShip : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString ShipName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString SkinName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString RegNum;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Region;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Velocity;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Respawns;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Heading;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Integrity;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<int> Ammo;;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<int> Fuel;;

	FS_MissionShip() {
		ShipName = "";
		SkinName = "";
		RegNum = "";
		Region = "";

		Location = FVector(-1.0e9f, -1.0e9f, -1.0e9f);
		Velocity = FVector(-1.0e9f, -1.0e9f, -1.0e9f);

		Respawns = -1;
		Heading = -1e9;
		Integrity = -1;

		Ammo.SetNum(16);
		Fuel.SetNum(4);

		for (int AmmoIndex = 0; AmmoIndex < Ammo.Num(); AmmoIndex++) {
			Ammo[AmmoIndex] = 0;
		}

		for (int FuelIndex = 0; FuelIndex < Fuel.Num(); FuelIndex++) {
			Fuel[FuelIndex] = 0;
		}
	}
};

USTRUCT(BlueprintType)
struct FS_MissionCallsign : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Callsign;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	
	FS_MissionCallsign() {
		Callsign = "";
		Iff = 0;
	}
};


USTRUCT(BlueprintType)
struct FS_MissionLoadout : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Ship;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString LoadoutName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<int> Stations;
	FS_MissionLoadout() {
		Ship = -1;
		LoadoutName = "";
		Stations.SetNum(16);
		for (int i = 0; i < Stations.Num(); i++)
			Stations[i] = 0;
	}
};

USTRUCT(BlueprintType)
struct FS_MissionAlias : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString AliasName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Design;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Code;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString ElementName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Mission;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Player;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool UseLocation;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_RLoc> RLoc;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionInstruction> Navpoint;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionInstruction> Objective;

	FS_MissionAlias() {
		AliasName = "";
		Design = "";
		Code = "";
		ElementName = "";
		Mission = "";
		
		Iff = -1;
		Player = 0;

		UseLocation = false;
		Location = FVector::ZeroVector;
	}
};

USTRUCT(BlueprintType)
struct FS_MissionElement : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Carrier;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Commander;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Squadron;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Text;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Path;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Design;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString SkinName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString RoleName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString RegionName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Instr;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Deck;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int IFFCode;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Count;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int MaintCount;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int DeadCount;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Player;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int CommandAI;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Respawns;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int HoldTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ZoneLock;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Heading;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Alert;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Playable;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Rogue;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Invulnerable;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_RLoc> RLoc;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionLoadout> Loadout;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionInstruction> Instruction;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionInstruction> Navpoint;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionShip> Ship;

	FS_MissionElement() {
		Name = "";
		Carrier = "";
		Commander = "";
		Squadron = "";
		Path = "";
		Design = "";
		SkinName = "";
		RoleName = "";
		RegionName = "";
		Intel = "";
		Instr = "";

		Location = FVector::ZeroVector;

		Deck = 1;
		IFFCode = 0;
		Count = 1;
		MaintCount = 0;
		DeadCount = 0;
		Player = 0;
		CommandAI = 0;
		Respawns = 0;
		HoldTime = 0;
		ZoneLock = 0;
		Heading = 0;

		Alert = false;
		Playable = false;
		Rogue = false;
		Invulnerable = false;
	}
};

USTRUCT(BlueprintType)
struct FS_MissionOptional : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Min;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Max;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Total;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionElement> Element; 
	
	FS_MissionOptional() {
		Min = 0;
		Max = 1000;
		Total = 0;
	}
};

USTRUCT(BlueprintType)
struct FS_MissionEvent : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString EventType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	TriggerName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	EventShip;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	EventSource;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	EventTarget;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	EventMessage;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	EventSound;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString	TriggerShip;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TriggerTarget;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int	EventId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int	EventChance;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int	EventDelay;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double	EventTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector	EventPoint;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 EventRect;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<int> EventParam;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<int> TriggerParam;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int	EventNParams;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int	TriggerNParams;

	FS_MissionEvent() {
		EventType = "";
		TriggerName = "";
		EventShip = "";
		EventSource = "";
		EventTarget = "";
		EventMessage = "";
		EventSound = "";
		TriggerShip = "";
		TriggerTarget = "";

		EventId = 1;
		EventChance = 0;
		EventDelay = 0;

		EventTime = 0;

		EventPoint = FVector::ZeroVector;
		EventRect = FVector4::Zero();

		EventNParams = 0;
		TriggerNParams = 0;

		EventParam.SetNum(10);
		TriggerParam.SetNum(10);

		for (int EventIndex = 0; EventIndex < EventParam.Num(); EventIndex++) {
			EventParam[EventIndex] = 0;
		}

		for (int TriggerIndex = 0; TriggerIndex < TriggerParam.Num(); TriggerIndex++) {
			TriggerParam[TriggerIndex] = 0;
		}
	}
};

USTRUCT(BlueprintType)
struct FS_CampaignMission : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Region;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString System;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Scene;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Subtitles;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Desc;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TargetName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString WardName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Objective;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Sitrep;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString StartTime;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Team;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Stardate;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Degrees;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionElement> Element;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionEvent> Event;
	
	FS_CampaignMission() {

		Id = 0;

		Region = "";
		System = "";
		Scene = "";

		Subtitles = "";
		Name = "";
		Desc = "";
		TargetName = "";
		WardName = "";
		Objective = "";
		Sitrep = "";
		StartTime = ""; // time

		Type = 0;
		Team = 0;
		
		Stardate = 0;

		Degrees = false;
	}
};

USTRUCT(BlueprintType)
struct FS_TemplateMission : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TemplateName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TargetName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString WardName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TemplateSystem;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TemplateRegion;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TemplateObjective;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TemplateSitrep;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TemplateStart;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int TemplateType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int TemplateTeam;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool TemplateDegrees = false;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionElement> Element;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionEvent> Event;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionCallsign> Callsign;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionOptional> Optional;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MissionAlias> Alias;

	FS_TemplateMission() {
		TargetName = "";
		WardName = "";
		TemplateName = "";
		TemplateSystem = "";
		TemplateRegion = "";
		TemplateObjective = "";
		TemplateSitrep = "";
		TemplateStart = "";

		TemplateType = 0;
		TemplateTeam = 0;

	    TemplateDegrees = false;
	}
};

USTRUCT(BlueprintType)
struct FS_Campaign : public FTableRowBase {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Description;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Situation;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FString> Orders;	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bScripted;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bSequential;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bAvailable;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bCompleted;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString System;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Region;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString MainImage;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ActionSize;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int CombatantSize;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Index;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_Combatant> Combatant;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_CampaignAction> Action;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_CampaignZone> Zone;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_CampaignMissionList> MissionList;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_CampaignTemplateList> TemplateList;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_CampaignMission> Missions;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_TemplateMission> TemplateMissions;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_TemplateMission> ScriptedMissions;
	
	FS_Campaign() {
		Name = "";
		Description = "";
		Situation = "";

		System = "";
		Region = "";
		MainImage = "";

		bScripted = false;
		bSequential = false;

		bAvailable = false;
		bCompleted = false;

		ActionSize = 0;
		CombatantSize = 0;
		Index = 0;
	}
};

// Structure to represent a flat entry in the ListView
USTRUCT(BlueprintType)
struct FS_OOBFlatEntry
{
    GENERATED_BODY()

       UPROPERTY(BlueprintReadWrite)
    int32 Id;

    UPROPERTY(BlueprintReadWrite)
    int32 ParentId;

    UPROPERTY(BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite)
    int32 IndentLevel;

    UPROPERTY(BlueprintReadWrite)
    ECOMBATGROUP_TYPE GroupType;

	FS_OOBFlatEntry() {
		IndentLevel  = 0;
		ParentId = -1;
		Id = -1;
		DisplayName = "";
		GroupType = ECOMBATGROUP_TYPE::NONE;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBUnit : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DisplayName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Regnum;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Design;	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATUNIT_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Count;

	FS_OOBUnit() {
		Name = "";
		DisplayName = "";
		Regnum = "";
		Location = "";
		ParentId = 0;
		Count = -1;
		Type = ECOMBATUNIT_TYPE::NONE;
		ParentType = ECOMBATGROUP_TYPE::NONE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBBatteryUnit : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Design;	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATUNIT_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Count;

	FS_OOBBatteryUnit() {
		Name = "";
		Location = "";
		ParentId = 0;
		Count = -1;
		Type = ECOMBATUNIT_TYPE::BATTERY;
		ParentType = ECOMBATGROUP_TYPE::BATTERY;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBStationUnit : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Design;	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATUNIT_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Count;

	FS_OOBStationUnit() {
		Name = "";
		Location = "";
		ParentId = 0;
		Count = -1;
		Type = ECOMBATUNIT_TYPE::STATION;
		ParentType = ECOMBATGROUP_TYPE::STATION;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBStarbaseUnit : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Design;	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATUNIT_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Count;

	FS_OOBStarbaseUnit() {
		Name = "";
		Location = "";
		ParentId = 0;
		Count = -1;
		Type = ECOMBATUNIT_TYPE::STARBASE;
		ParentType = ECOMBATGROUP_TYPE::STARBASE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};
USTRUCT(BlueprintType)
struct FS_OOBMinefieldUnit : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Design;	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATUNIT_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Count;

	FS_OOBMinefieldUnit() {
		Name = "";
		Location = "";
		ParentId = 0;
		Count = -1;
		Type = ECOMBATUNIT_TYPE::MINE;
		ParentType = ECOMBATGROUP_TYPE::MINEFIELD;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBFighterUnit : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Design;	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATUNIT_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Count;

	FS_OOBFighterUnit() {
		Name = "";
		Location = "";
		ParentId = 0;
		Count = -1;
		Type = ECOMBATUNIT_TYPE::NONE;
		ParentType = ECOMBATGROUP_TYPE::NONE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBCivilian : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	
	FS_OOBCivilian() {
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::CIVILIAN;
		ParentType = ECOMBATGROUP_TYPE::FORCE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};
USTRUCT(BlueprintType)
struct FS_OOBLanding : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray <FS_OOBFighterUnit> Unit;
	
	FS_OOBLanding() {	
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::LCA_SQUADRON;
		ParentType = ECOMBATGROUP_TYPE::NONE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};


USTRUCT(BlueprintType)
struct FS_OOBFighter : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray <FS_OOBFighterUnit> Unit;
	
	FS_OOBFighter() {	
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
		ParentType = ECOMBATGROUP_TYPE::NONE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBAttack : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray <FS_OOBFighterUnit> Unit;
	
	FS_OOBAttack() {		
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
		ParentType = ECOMBATGROUP_TYPE::NONE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBIntercept : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray <FS_OOBFighterUnit> Unit;
	
	FS_OOBIntercept() {		
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON;
		ParentType = ECOMBATGROUP_TYPE::NONE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBStarbase : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBIntercept> Intercept;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBAttack> Attack;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBFighter> Fighter;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBLanding> Landing;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBStarbaseUnit> Unit;
	
	FS_OOBStarbase() {
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::STARBASE;
		ParentType = ECOMBATGROUP_TYPE::BATTALION;
		Empire = EEMPIRE_NAME::Unknown;
	}
};
USTRUCT(BlueprintType)
struct FS_OOBStation : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBIntercept> Intercept;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBAttack> Attack;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBFighter> Fighter;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBLanding> Landing;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBStationUnit> Unit;
	
	FS_OOBStation() {
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::STATION;
		ParentType = ECOMBATGROUP_TYPE::BATTALION;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBBattery : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBMinefieldUnit> Unit;
	
	FS_OOBBattery() {
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::BATTERY;
		ParentType = ECOMBATGROUP_TYPE::BATTALION;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBBattalion : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray <FS_OOBBattery> Battery;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray <FS_OOBStation> Station;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray <FS_OOBStarbase> Starbase;

	FS_OOBBattalion() {	
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::BATTALION;
		ParentType = ECOMBATGROUP_TYPE::FORCE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBMinefield : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray <FS_OOBMinefieldUnit> Unit;
	
	FS_OOBMinefield() {
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::MINEFIELD;
		ParentType = ECOMBATGROUP_TYPE::FORCE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBWing : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBIntercept> Intercept;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBAttack> Attack;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBFighter> Fighter;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBLanding> Landing;
	
	FS_OOBWing() {
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::WING;
		ParentType = ECOMBATGROUP_TYPE::CARRIER_GROUP;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBBattle : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray <FS_OOBUnit> Unit;
	
	FS_OOBBattle() {	
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::BATTLE_GROUP;
		ParentType = ECOMBATGROUP_TYPE::FLEET;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBDestroyer : public FTableRowBase {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray <FS_OOBUnit> Unit;
	
	FS_OOBDestroyer() {
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;
		ParentType = ECOMBATGROUP_TYPE::FLEET;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBCarrier : public FTableRowBase {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBWing> Wing;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBIntercept> Intercept;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBAttack> Attack;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBFighter> Fighter;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBLanding> Landing;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray <FS_OOBUnit> Unit;

	FS_OOBCarrier() {
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::CARRIER_GROUP;
		ParentType = ECOMBATGROUP_TYPE::FLEET;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBFleet : public FTableRowBase {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBCarrier> Carrier;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBDestroyer> Destroyer;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBBattle> Battle;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBMinefield> Minefield;
	
	FS_OOBFleet() {
		Id = 0;
		Iff = -1;
		ParentId = 0;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::FLEET;
		ParentType = ECOMBATGROUP_TYPE::FORCE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_OOBForce : public FTableRowBase {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBFleet> Fleet;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBBattalion> Battalion;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_OOBCivilian> Civilian;
	
	FS_OOBForce() {
		
		Id = 0; 
		Iff = -1;
		Name = "";
		Location = "";
		Intel = EINTEL_TYPE::KNOWN;
		Type = ECOMBATGROUP_TYPE::FORCE;
		Empire = EEMPIRE_NAME::Unknown;
	}
};

USTRUCT(BlueprintType)
struct FS_CombatGroupUnit : public FTableRowBase {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString UnitName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString UnitRegnum;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString UnitRegion;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString UnitClass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString UnitDesign;	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString UnitSkin;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector UnitLoc;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int UnitCount;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int UnitDamage;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int UnitDead;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int UnitHeading;

	FS_CombatGroupUnit() {
		UnitName = "";
		UnitRegnum = "";
		UnitRegion = "";
		UnitClass = "";
		UnitDesign = "";
		UnitSkin = "";

		UnitLoc = FVector::ZeroVector;
		UnitCount = 0;
		UnitDamage = 0;
		UnitDead = 0;
		UnitHeading = 0;
	}
};

USTRUCT(BlueprintType)
struct FS_DisplayElement : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bShowUnit;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATUNIT_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	
	FS_DisplayElement() {
		bShowUnit = false;
		Name = "";
		Empire = EEMPIRE_NAME::Unknown;
		Type = ECOMBATUNIT_TYPE::NONE;
		Location = ""; 
	}
};
USTRUCT(BlueprintType)
struct FS_DisplayUnit : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bShowUnit;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME Empire;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	UUserWidget* Widget;
	
	FS_DisplayUnit() {
		bShowUnit = false;
		Name = "";
		Empire = EEMPIRE_NAME::Unknown;
		Type = ECOMBATGROUP_TYPE::NONE;
		Location = ""; 
		Widget = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FS_CombatGroup : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString DisplayName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EINTEL_TYPE Intel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Iff;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EEMPIRE_NAME EmpireId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString System;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Region;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Location;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	ECOMBATGROUP_TYPE ParentType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ParentId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int UnitIndex;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_CombatGroupUnit> Unit;

	FS_CombatGroup() {
		Id =  0;

		Name = "";
		DisplayName = "";
		Iff = -1;
		System = "";
		Region = "";

		Type = ECOMBATGROUP_TYPE::NONE;
		EmpireId = EEMPIRE_NAME::Unknown;
		ParentType = ECOMBATGROUP_TYPE::NONE;
		Intel = EINTEL_TYPE::KNOWN;
		Location = FVector::ZeroVector;
		ParentId = 0;
		//EmpireId = 0;
		UnitIndex = 0;
	}

	// Determines if this group is a unit (no subgroups)
	bool IsUnit() const {
		return Unit.Num() > 0;
	}

	// Equality check for tree matching
	bool MatchesParent(int InParentId) const {
		return ParentId == InParentId;
	}
};

USTRUCT(BlueprintType)
struct FS_AwardInfo : public FTableRowBase {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int      AwardId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString   
	AwardType;	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     AwardName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     AwardAbrv;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     AwardDesc;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     AwardText;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     DescSound;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString     GrantSound;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString		LargeImage;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString		SmallImage;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		AwardGrant;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		RequiredAwards;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		Lottery;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		MinRank;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		MaxRank;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int     MinShipClass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int     MaxShipClass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		GrantedShipClasses;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		TotalPoints;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		MissionPoints;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		TotalMissions;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		Kills;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		Lost;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		Collision;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int		CampaignId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool	CampaignComplete;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool	DynamicCampaign;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool	Ceremony;

	FS_AwardInfo() {
		AwardId = 0;
		AwardType = "";
		AwardName = "";
		AwardAbrv = "";
		AwardDesc = "";
		AwardText = "";

		DescSound = "";
		GrantSound = "";
		LargeImage = "";
		SmallImage = "";

		AwardGrant = 0;
		RequiredAwards = 0;
		Lottery = 0;
		MinRank = 0;
		MaxRank = 0;
		MinShipClass = 0;
		MaxShipClass = 0;
		GrantedShipClasses = 0;
	
		TotalPoints = 0;
		MissionPoints = 0;
		TotalMissions = 0;
	
		Kills = 0;
		Lost = 0;
		Collision = 0;
		CampaignId = 0;

		CampaignComplete = false;
		DynamicCampaign = false;
		Ceremony = false;
	}
};

USTRUCT(BlueprintType)
struct FS_ShipPower : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString  DesignName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString PName;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString PAbrv;
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
	float Output;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Fuel;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Size;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	float Hull;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector Loc;

	FS_ShipPower() {
		DesignName = "";
		PName = "";
		PAbrv = "";

		SType = 0;
		EType = 0;
		Emcon1 = -1;
		Emcon2 = -1;
		Emcon3 = -1;

		Output = 1000.0f;
		Fuel = 0.0f;
		Size = 0.0f;
		Hull = 0.5f;
		Loc = FVector::ZeroVector;

	}
};

USTRUCT(BlueprintType)
struct FS_ShipDesign : public FTableRowBase {
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
	TArray<FS_ShipPower> Power;

	FS_ShipDesign() {
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
		for(int i = 0; i < 4; i++) {
			Offset[i] = FVector::ZeroVector;
		}
	}
};

USTRUCT(BlueprintType)
struct FS_ComponentDesign : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Abrv;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int RepairTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ReplaceTime;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Spares;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Affects;

	FS_ComponentDesign() {
		Name = "";
		Abrv = "";
		RepairTime = 0;
		ReplaceTime = 0,
			Spares = 1;
		Affects = 0;
	}
};

USTRUCT(BlueprintType)
struct FS_SystemDesign : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_ComponentDesign> Component;

	FS_SystemDesign() {
		Name = "";
	}
};

USTRUCT(BlueprintType)
struct FS_LayoutDef : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<int> XMin;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<int> YMin;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<float> XWeight;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<float> YWeight;
	
	FS_LayoutDef() {

	}
};

USTRUCT(BlueprintType)
struct FS_CtrlDef : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Text;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Alt;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int PId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TEnumAsByte<EControlType> Type;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 Rect;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Font;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor ActiveColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor BackColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor BaseColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor BorderColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor ForeColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Texture;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 Margins;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 TextInsets;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 CellInsets;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 Cells;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int FixedWidth;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int FixedHeight;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString StandardImage;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString ActiavtedImage;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString TransitionImage;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Picture;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Enabled;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Item;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Tab;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Orientation;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Leading;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int LineHeight;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Multiselect;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int DragDrop;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ScrollBar;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool SmoothScroll;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int PictureLoc;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int PictureType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Style;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Align;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool SingleLine;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int BevelWidth;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Active;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Animated;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Border;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool DropShadow;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool ShowHeadings;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Sticky;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Transparent;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool HidePartial;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int NumLeds;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int ItemStyle;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int SelectedStyle;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Password;
	
	FS_CtrlDef() {
		Text = "";
		Alt = "";
		Id = 0; 
		PId = 0;
		Type = EControlType::WINDEF_LABEL;
		Rect = FVector4::Zero();
		Font = "";
		ActiveColor = FColor::Black;
		BackColor = FColor::Black;
		BaseColor = FColor::Black;
		BorderColor = FColor::Black;
		ForeColor = FColor::Black;
		Texture = "";
		Margins = FVector4::Zero();
		TextInsets = FVector4::Zero();
		CellInsets = FVector4::Zero();
		Cells = FVector4::Zero();
		FixedWidth = 0;
		FixedHeight = 0;
		StandardImage = "";
		ActiavtedImage = "";
		TransitionImage = "";
		Picture = "";
		Enabled = false;
		Item = 0;
		Tab = 0;
		Orientation = 0;
		Leading = 0;
		LineHeight = 0;
		Multiselect = 0;
		DragDrop = 0;
		ScrollBar = 0;
		SmoothScroll = false;
		PictureLoc = 0;
		PictureType = 0;
		Style = 0;
		Align = 0;
		SingleLine = false;
		BevelWidth = 0;
		Active = false;
		Animated = false;
		Border = false;
		DropShadow = false;
		ShowHeadings = false;
		Sticky = false;
		Transparent = false;
		HidePartial = false;
		NumLeds = 0;
		ItemStyle = 0;
		SelectedStyle = 0;
		Password = "";
	}
};


USTRUCT(BlueprintType)
struct FS_FormDesign : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Caption;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Id;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int PId;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 Rect;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 Cells;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 Insets;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 TextInsets;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FVector4 CellInsets;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Font;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor BackColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor BaseColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor ForeColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Texture;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool Transparent;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Style;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Align;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_CtrlDef> CtrlDef;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FS_LayoutDef LayoutDef;

	FS_FormDesign() {
		Name = "";
		Caption = "";
		Id = 0;
		PId = 0;
		Rect = FVector4::Zero();
		Cells = FVector4::Zero();
		Insets = FVector4::Zero();
		TextInsets = FVector4::Zero();
		CellInsets = FVector4::Zero();
		Font = "";
		BackColor = FColor::Black;
		BaseColor = FColor::Black;
		ForeColor = FColor::Black;
		Texture = "";
		Transparent = false;
		Style = 0;
		Align = 0;
	}
};
