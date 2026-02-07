/*  Project Starshatter Wars
	Fractal Dev Studios
	Copyright (C) 2025-2026. All Rights Reserved.

	SUBSYSTEM:    GAME
	FILE:         GameStructs.h
	AUTHOR:       Carlos Bott
	ORIGINAL:     John DiCamillo / Destroyer Studios LLC

	OVERVIEW
	========
	Game Structs and Enums
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Blueprint/UserWidget.h"
#include "Text.h"
#include "InputCoreTypes.h"
#include "GameStructs.generated.h"

const double STARSHIP_TACTICAL_DROP_TIME = 15;
static const double GRAV = 6.673e-11;
static const int    NAMELEN = 64;

const int MAX_DETAIL = 4;
const int PATCH_SIZE = 17;
const int HALF_PATCH_SIZE = 8;
const int MAX_VERTS = PATCH_SIZE * PATCH_SIZE;
const int NUM_INDICES_TRI = 3;

/************************************************************************/
/* ENUMS                                                                */
/************************************************************************/

UENUM(BlueprintType)
enum class EBodyUISizeClass : uint8 {
	Moon,
	Planet,
	GasGiant,
	Star, 
	Jumpgate, 
	None
};

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
enum class EGameMode : uint8
{
	BOOT    UMETA(DisplayName = "Booting up"),
	INIT    UMETA(DisplayName = "Game Initialization"),

	MENU    UMETA(DisplayName = "Main Menu"),
	CLOD    UMETA(DisplayName = "Loading Campaign"),
	CMPN    UMETA(DisplayName = "Operational Campaign"),
	PREP    UMETA(DisplayName = "Loading Mission Info"),
	PLAN    UMETA(DisplayName = "Loading Briefing"),
	LOAD    UMETA(DisplayName = "Loading Mission"),
	PLAY    UMETA(DisplayName = "Active Simulation"),
	EXIT    UMETA(DisplayName = "Shutting Down"),
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

UENUM()
enum class EMissionPrimaryDomain : uint8
{
	Fighter,
	Starship
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

UENUM()
enum class ECampaignType : uint32 {
	UNKNOWN						UMETA(DisplayName = "Unknowm"),
	TRAINING_CAMPAIGN			UMETA(DisplayName = "Training Campaign"),
	DYNAMIC_CAMPAIGN			UMETA(DisplayName = "Dynamic Campaign"),
	MOD_CAMPAIGN				UMETA(DisplayName = "Mod campaign"),
	SINGLE_MISSION				UMETA(DisplayName = "Single Mission"),
	MULTIPLAYER_MISSION			UMETA(DisplayName = "Multplayer Mission"),
	CUSTOM_MISSION				UMETA(DisplayName = "Custom Mission"),
};

UENUM()
enum class ECampaignStatus {
	INIT				UMETA(DisplayName = "Campaign - Init"),
	ACTIVE				UMETA(DisplayName = "Campaign - Init"),
	SUCCESS				UMETA(DisplayName = "Campaign - Success"),
	FAILED				UMETA(DisplayName = "Campaign - Failed")
};

enum COMPARISON_OPERATOR {
	LT, LE, GT, GE, EQ,    // absolute score comparison
	RLT, RLE, RGT, RGE, REQ    // delta score comparison
};

UENUM()
enum class ECombatActionReqComp : uint8
{
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

UENUM()
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

UENUM()
enum ECOMBATACTION_STATUS : uint8
{
	UNKNOWN UMETA(DisplayName = "Unknown"),
	PENDING UMETA(DisplayName = "Pending"), 
	ACTIVE UMETA(DisplayName = "Active"), 
	SKIPPED UMETA(DisplayName = "Skipped"),
	FAILED UMETA(DisplayName = "Failed"), 
	COMPLETE UMETA(DisplayName = "Complete"),
	INCOMPLETE UMETA(DisplayName = "Incomplete"),
};

UENUM()
enum class EINTEL_TYPE : uint8 {
	UNKNOWN		UMETA(DisplayName = "Unknown"), // This status should notxexist in game
	RESERVE		UMETA(DisplayName = "Reserve"), // out-system reserve: this group is notxeven here
	SECRET		UMETA(DisplayName = "Secret"),   // enemy is completely unaware of this group
	KNOWN		UMETA(DisplayName = "Known"),    // enemy knows this group is in the system
	LOCATED		UMETA(DisplayName = "Located"),  // enemy has located at least the lead ship
	TRACKED		UMETA(DisplayName = "Tracked"),  // enemy is tracking all elements
	ACTIVE		UMETA(DisplayName = "Active"),   // enemy is Intel Tracking is Active
};

UENUM()
enum class ECOMBATEVENT_TYPE : uint8 
{
	ATTACK			UMETA(DisplayName = "Attack"),
	DEFEND			UMETA(DisplayName = "Defend"), 
	MOVE_TO			UMETA(DisplayName = "Move to"), 
	CAPTURE			UMETA(DisplayName = "Capture"), 
	STRATEGY		UMETA(DisplayName = "Strategy"), 
	CAMPAIGN_START	UMETA(DisplayName = "Campaign Start"), 
	STORY			UMETA(DisplayName = "Story"), 
	CAMPAIGN_END	UMETA(DisplayName = "Campaign End"),
	CAMPAIGN_FAIL	UMETA(DisplayName = "Campaign Fail"),
};

UENUM()
enum class ECOMBATEVENT_SOURCE : uint8
{
	FORCOM			UMETA(DisplayName = "FORCOM"), 
	TACNET			UMETA(DisplayName = "TACNET"),
	INTEL			UMETA(DisplayName = "Intel"),
	MAIL			UMETA(DisplayName = "Mail"),
	NEWS			UMETA(DisplayName = "News"),
};

UENUM()
enum class MISSIONEVENT_TYPE : uint8
{
	MESSAGE			UMETA(DisplayName = "Message"),
	OBJECTIVE		UMETA(DisplayName = "Objective"),
	INSTRUCTION		UMETA(DisplayName = "Instruction"),
	IFF				UMETA(DisplayName = "IFF"),
	DAMAGE			UMETA(DisplayName = "Damage"), 
	JUMP			UMETA(DisplayName = "Jump"),
	HOLD			UMETA(DisplayName = "Hold"),
	SKIP			UMETA(DisplayName = "Skip"),
	END_MISSION		UMETA(DisplayName = "End Mission"),
	BEGIN_SCENE		UMETA(DisplayName = "Begin Scene"),
	CAMERA			UMETA(DisplayName = "Camera"),
	VOLUME			UMETA(DisplayName = "Volume"),
	DISPLAY			UMETA(DisplayName = "Display"),
	FIRE_WEAPON		UMETA(DisplayName = "Fire Weapon"),
	END_SCENE		UMETA(DisplayName = "End Scene"),
	NUM_EVENTS
};

UENUM()
enum class MISSIONEVENT_STATUS : uint8
{
	PENDING			UMETA(DisplayName = "Pending"), 
	ACTIVE			UMETA(DisplayName = "Active"),
	COMPLETE		UMETA(DisplayName = "Complete"),
	SKIPPED			UMETA(DisplayName = "Skipped"),
};

UENUM()
enum class MISSIONEVENT_TRIGGER : uint8
{
	TRIGGER_TIME		UMETA(DisplayName = "Trigger Time"),
	TRIGGER_DAMAGE		UMETA(DisplayName = "Trigger Damage"),
	TRIGGER_DESTROYED	UMETA(DisplayName = "Trigger Destroyed"),
	TRIGGER_JUMP		UMETA(DisplayName = "Trigger Jump"),
	TRIGGER_LAUNCH		UMETA(DisplayName = "Trigger Launch"),
	TRIGGER_DOCK		UMETA(DisplayName = "Trigger Dock"),
	TRIGGER_NAVPT		UMETA(DisplayName = "Trigger Navpoint"),
	TRIGGER_EVENT		UMETA(DisplayName = "Trigger Event"),
	TRIGGER_SKIPPED		UMETA(DisplayName = "Trigger Skipped"),
	TRIGGER_TARGET		UMETA(DisplayName = "Trigger Target"),
	TRIGGER_SHIPS_LEFT	UMETA(DisplayName = "Trigger Ships Left"),
	TRIGGER_DETECT		UMETA(DisplayName = "Trigger Detect"),
	TRIGGER_RANGE		UMETA(DisplayName = "Trigger Range"),
	TRIGGER_EVENT_ALL	UMETA(DisplayName = "Trigger Event All"),
	TRIGGER_EVENT_ANY	UMETA(DisplayName = "Trigger Event Any"),
	NUM_TRIGGERS
};

UENUM()
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
	NOTHING			UMETA(DisplayName = "None"),
	STAR			UMETA(DisplayName = "Star"),
	PLANET			UMETA(DisplayName = "Planet"),
	MOON			UMETA(DisplayName = "Moon"),
	JUMPGATE		UMETA(DisplayName = "JumpGate"),
	REGION			UMETA(DisplayName = "Region"),
	STATION			UMETA(DisplayName = "Station"),
	TERRAIN			UMETA(DisplayName = "Terrain")
};

UENUM()
enum class CLASSIFICATION : uint32
{
	EMPTY					UMETA(DisplayName = "None"),

	DRONE = 0x0001			UMETA(DisplayName = "Drone"),
	FIGHTER = 0x0002		UMETA(DisplayName = "Fighter"),
	ATTACK = 0x0004			UMETA(DisplayName = "Attack"),
	LCA = 0x0008			UMETA(DisplayName = "Landing Craft"),

	COURIER = 0x0010		UMETA(DisplayName = "Courier"),
	CARGO = 0x0020			UMETA(DisplayName = "Cargo"),

	CORVETTE = 0x0040      UMETA(DisplayName = "Corvette"),
	FREIGHTER = 0x0080      UMETA(DisplayName = "Freighter"),
	FRIGATE = 0x0100      UMETA(DisplayName = "Frigate"),
	DESTROYER = 0x0200      UMETA(DisplayName = "Destroyer"),
	CRUISER = 0x0400      UMETA(DisplayName = "Cruiser"),
	BATTLESHIP = 0x0800      UMETA(DisplayName = "Battleship"),
	CARRIER = 0x1000      UMETA(DisplayName = "Carrier"),
	DREADNAUGHT = 0x2000      UMETA(DisplayName = "Dreadnaught"),

	STATION = 0x4000      UMETA(DisplayName = "Station"),
	FARCASTER = 0x8000      UMETA(DisplayName = "Farcaster"),

	MINE = 0x00010000  UMETA(DisplayName = "Mine"),
	COMSAT = 0x00020000  UMETA(DisplayName = "Comsat"),
	DEFSAT = 0x00040000  UMETA(DisplayName = "Defense Satellite"),
	SWACS = 0x00080000  UMETA(DisplayName = "SWACS"),

	BUILDING = 0x00100000  UMETA(DisplayName = "Building"),
	FACTORY = 0x00200000  UMETA(DisplayName = "Factory"),
	SAM = 0x00400000  UMETA(DisplayName = "SAM Site"),
	EWR = 0x00800000  UMETA(DisplayName = "Early Warning Radar"),
	C3I = 0x01000000  UMETA(DisplayName = "C3I"),
	STARBASE = 0x02000000  UMETA(DisplayName = "Starbase"),

	DROPSHIPS = 0x0000000f  UMETA(Hidden),
	STARSHIPS = 0x0000fff0  UMETA(Hidden),
	SPACE_UNITS = 0x000f0000  UMETA(Hidden),
	GROUND_UNITS = 0xfff00000  UMETA(Hidden)
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

UENUM(BlueprintType)
enum class EMISSIONTYPE : uint8
{
	PATROL,
	SWEEP,
	INTERCEPT,
	AIR_PATROL,
	AIR_SWEEP,
	AIR_INTERCEPT,
	STRIKE,     // ground attack
	ASSAULT,    // starship attack
	DEFEND,
	ESCORT,
	ESCORT_FREIGHT,
	ESCORT_SHUTTLE,
	ESCORT_STRIKE,
	INTEL,
	SCOUT,
	RECON,
	BLOCKADE,
	FLEET,
	BOMBARDMENT,
	FLIGHT_OPS,
	TRANSPORT,
	CARGO,
	TRAINING,
	OTHER, 
	UNKNOWN,
};

UENUM()
enum class SYSTEM_CATEGORY : uint8 {
	MISC_SYSTEM		UMETA(DisplayName = "Miscellaneous"), 
	DRIVE			UMETA(DisplayName = "Drive"), 
	WEAPON			UMETA(DisplayName = "Weapon"),
	SHIELD			UMETA(DisplayName = "Shield"),
	SENSOR			UMETA(DisplayName = "Sensor"),
	COMPUTER		UMETA(DisplayName = "Computer"),
	POWER_SOURCE	UMETA(DisplayName = "Power Source"),
	FLIGHT_DECK		UMETA(DisplayName = "Flight Deck"),
	FARCASTER		UMETA(DisplayName = "Farcaster"),
};

UENUM()
enum class SYSTEM_STATUS : uint8 {
	DESTROYED		UMETA(DisplayName = "Destroyed"),
	CRITICAL		UMETA(DisplayName = "Critical"),
	DEGRADED		UMETA(DisplayName = "Degraded"),
	NOMINAL			UMETA(DisplayName = "Nominal"),
	REPAIR			UMETA(DisplayName = "Repair"),
	REPLACE			UMETA(DisplayName = "Replace"),
	MAINT			UMETA(DisplayName = "Maintanence"),
	UNKNOWN			UMETA(DisplayName = "Unknown"),
};

UENUM()
enum class SYSTEM_POWER_FLAGS : uint8 {
	POWER_OFF		UMETA(DisplayName = "Power off"),
	POWER_WATTS		UMETA(DisplayName = "Power watts"),
	POWER_CRITICAL	UMETA(DisplayName = "Power critical"),
};

UENUM()
enum class EFadeState : uint8
{
	StateStart,
	State2,
	StateIn,
	StateHold,
	StateOut,
	StateDone
};

UENUM(BlueprintType)
enum class EMFDMode : uint8
{
	OFF			UMETA(DisplayName = "MFD Mode Off"),
	GAME		UMETA(DisplayName = "HUD Mode Game"),
	SHIP		UMETA(DisplayName = "HUD Mode Ship"),
	FOV			UMETA(DisplayName = "HUD Mode FOV"),
	HSD			UMETA(DisplayName = "HUD Mode HSD"),
	RADAR3D		UMETA(DisplayName = "HUD Mode Radar"),
};

UENUM()
enum class EHUDMode : uint8
{
	Off			UMETA(DisplayName = "HUD Mode Off"),
	Tactical	UMETA(DisplayName = "HUD Mode Tactical"),
	Navigation	UMETA(DisplayName = "HUD Mode Navigation"),
	ILS			UMETA(DisplayName = "HUD Mode ILS"),
};

UENUM()
enum class LIGHTTYPE : uint32 
{
	NONE,
	POINT,
	SPOT,
	DIRECTIONAL,
	LIGHT_FORCE_DWORD = 0x7fffffff
};

UENUM()
enum class INSTRUCTION_ACTION : uint8
{
	NONE		UMETA(DisplayName = "None"), 
	VECTOR		UMETA(DisplayName = "Vector"),
	LAUNCH		UMETA(DisplayName = "Launch"),
	DOCK		UMETA(DisplayName = "Dock"),
	RTB			UMETA(DisplayName = "Return to Base"),

	DEFEND		UMETA(DisplayName = "Defend"),
	ESCORT		UMETA(DisplayName = "Escort"),
	PATROL		UMETA(DisplayName = "Patrol"),
	SWEEP		UMETA(DisplayName = "Sweep"),
	INTERCEPT	UMETA(DisplayName = "Intercept"),
	STRIKE		UMETA(DisplayName = "Strike"),     // ground attack
	ASSAULT		UMETA(DisplayName = "Assault"),    // starship attack
	RECON		UMETA(DisplayName = "Recon"),

	RECALL		UMETA(DisplayName = "Recall"),
	DEPLOY		UMETA(DisplayName = "Deploy"),

	NUM_ACTIONS
};

UENUM()
enum class INSTRUCTION_STATUS : uint8
{
	NONE			UMETA(DisplayName = "None"),
	PENDING			UMETA(DisplayName = "Pending"),
	ACTIVE			UMETA(DisplayName = "Active"),
	SKIPPED			UMETA(DisplayName = "Skipped"),
	ABORTED			UMETA(DisplayName = "Aborted"),
	FAILED			UMETA(DisplayName = "Failed"),
	COMPLETE		UMETA(DisplayName = "Complete"),

	NUM_STATUS
};

UENUM()
enum class INSTRUCTION_FORMATION : uint8
{
	NONE			UMETA(DisplayName = "None"),
	DIAMOND			UMETA(DisplayName = "Diamond"),
	SPREAD			UMETA(DisplayName = "Spread"),
	BOX				UMETA(DisplayName = "Box"),
	TRAIL			UMETA(DisplayName = "Trail"),

	NUM_FORMATIONS
};

UENUM()
enum class INSTRUCTION_PRIORITY : uint8
{
	NONE,
	PRIMARY,
	SECONDARY,
	BONUS
};

UENUM()
enum class WeaponsOrders : uint8 
{
	MANUAL				UMETA(DisplayName = "Manual"),
	AUTO				UMETA(DisplayName = "Automatic"), 
	POINT_DEFENSE		UMETA(DisplayName = "Point Defense"),
};

UENUM()
enum class WeaponsControl : uint8 
{
	SINGLE_FIRE			UMETA(DisplayName = "Single Fire"),
	RIPPLE_FIRE			UMETA(DisplayName = "Ripple Fire"),
	SALVO_FIRE			UMETA(DisplayName = "Salvo Fire"),
};

UENUM()
enum class WeaponsSweep : uint8 
{
	SWEEP_NONE			UMETA(DisplayName = "Sweep None"),
	SWEEP_TIGHT			UMETA(DisplayName = "Sweep Tight"),
	SWEEP_WIDE			UMETA(DisplayName = "Sweep Wide"),
};

UENUM()
enum class RadioMessageAction : uint8 
{
	NONE = 0,

	DOCK_WITH = INSTRUCTION_ACTION::DOCK,
	RTB = INSTRUCTION_ACTION::RTB,
	QUANTUM_TO = INSTRUCTION_ACTION::NUM_ACTIONS,
	FARCAST_TO,

	// protocol:
	ACK,
	NACK,

	// target mgt:
	ATTACK,
	ESCORT,
	BRACKET,
	IDENTIFY,

	// combat mgt:
	COVER_ME,
	WEP_FREE,
	WEP_HOLD,
	FORM_UP,       // alias for wep_hold
	SAY_POSITION,

	// sensor mgt:
	LAUNCH_PROBE,
	GO_EMCON1,
	GO_EMCON2,
	GO_EMCON3,

	// formation mgt:
	GO_DIAMOND,
	GO_SPREAD,
	GO_BOX,
	GO_TRAIL,

	// mission mgt:
	MOVE_PATROL,
	SKIP_NAVPOINT,
	RESUME_MISSION,

	// misc announcements:
	CALL_ENGAGING,
	FOX_1,
	FOX_2,
	FOX_3,
	SPLASH_1,
	SPLASH_2,
	SPLASH_3,
	SPLASH_4,
	SPLASH_5,   // target destroyed
	SPLASH_6,   // enemy destroyed
	SPLASH_7,   // confirmed kill
	DISTRESS,
	BREAK_ORBIT,
	MAKE_ORBIT,
	QUANTUM_JUMP,

	// friendly fire:
	WARN_ACCIDENT,
	WARN_TARGETED,
	DECLARE_ROGUE,

	// support:
	PICTURE,
	REQUEST_PICTURE,
	REQUEST_SUPPORT,

	// traffic control:
	CALL_INBOUND,
	CALL_APPROACH,
	CALL_CLEARANCE,
	CALL_FINALS,
	CALL_WAVE_OFF,

	NUM_ACTIONS
};

UENUM()
enum class TacticalViewMenu : uint32 {
	FORWARD = 1000,
	CHASE,
	PADLOCK,
	ORBIT,
	NAV,
	WEP,
	ENG,
	FLT,
	INS,
	CMD, 
	QUANTUM = 2000,
	FARCAST = 2001
};

/*static enum ETXT : int32
{
	MAX_CONTACT = 50,

	TXT_CAUTION_TXT = 0,
	TXT_LAST_CAUTION = 23,
	TXT_CAM_ANGLE,
	TXT_CAM_MODE,
	TXT_PAUSED,
	TXT_GEAR_DOWN,

	TXT_HUD_MODE,
	TXT_PRIMARY_WEP,
	TXT_SECONDARY_WEP,
	TXT_DECOY,
	TXT_SHIELD,
	TXT_AUTO,
	TXT_SHOOT,
	TXT_NAV_INDEX,
	TXT_NAV_ACTION,
	TXT_NAV_FORMATION,
	TXT_NAV_SPEED,
	TXT_NAV_ETR,
	TXT_NAV_HOLD,

	TXT_SPEED,
	TXT_RANGE,
	TXT_CLOSING_SPEED,
	TXT_THREAT_WARN,
	TXT_COMPASS,
	TXT_HEADING,
	TXT_PITCH,
	TXT_ALTITUDE,
	TXT_GFORCE,
	TXT_MISSILE_T1,
	TXT_MISSILE_T2,
	TXT_ICON_SHIP_TYPE,
	TXT_ICON_TARGET_TYPE,
	TXT_TARGET_NAME,
	TXT_TARGET_DESIGN,
	TXT_TARGET_SHIELD,
	TXT_TARGET_HULL,
	TXT_TARGET_SUB,
	TXT_TARGET_ETA,

	TXT_MSG_1,
	TXT_MSG_2,
	TXT_MSG_3,
	TXT_MSG_4,
	TXT_MSG_5,
	TXT_MSG_6,

	TXT_NAV_PT,
	TXT_SELF,
	TXT_SELF_NAME,
	TXT_CONTACT_NAME,
	TXT_CONTACT_INFO = TXT_CONTACT_NAME + MAX_CONTACT,
	TXT_LAST = TXT_CONTACT_INFO + MAX_CONTACT,

	TXT_LAST_ACTIVE = TXT_NAV_HOLD,
	TXT_INSTR_PAGE = TXT_CAUTION_TXT + 6,
} */

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CampaignRowName;
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
		CampaignRowName = NAME_None;

		CampaignComplete.SetNum(5);
		for (int i = 0; i < CampaignComplete.Num(); i++) {
			CampaignComplete[i] = false;
		}
	}
};

USTRUCT(BlueprintType)
struct FS_TerrainRegion : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString PatchTexture;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString DetailTexture0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString DetailTexture1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString ApronName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString ApronTexture;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString WaterTexture;

	// These are FILE NAMES in the legacy data, so keep them as strings:
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString EnvPosX;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString EnvNegX;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString EnvPosY;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString EnvNegY;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString EnvPosZ;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString EnvNegZ;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString HazeName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString SkyName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString CloudsHigh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString CloudsLow;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString ShadesHigh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString ShadesLow;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) double Size = 1.0e6;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) double Grid = 25000.0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) double Inclination = 0.0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) double Scale = 1.0e4;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) double MountainScale = 1.0e3;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) double FogDensity = 0.0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) double FogScale = 0.0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) double HazeFade = 0.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) double CloudsAltHigh = 0.0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) double CloudsAltLow = 0.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) double WeatherPeriod = 0.0;

	// Chances are numeric weights (double), indexed by EWEATHER_STATE:
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TArray<double> WeatherChances;

	FS_TerrainRegion()
	{
		WeatherChances.SetNumZeroed((int32)EWEATHER_STATE::NUM_STATES);
	}
};

USTRUCT(BlueprintType)
struct FS_RegionMap : public FTableRowBase {
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

	FS_RegionMap() {
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
struct FS_MoonMap : public FTableRowBase {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Parent;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Icon;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FString  Texture;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Radius;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Mass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Orbit;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double OrbitAngle = 0.0;
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
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EBodyUISizeClass  BodyType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_RegionMap> Region; 

	FS_MoonMap() {
		Name = "";
		Parent = "";
		Icon = "";
		Texture = "";
		Radius = 0.0;
		Mass = 0.0;
		Orbit = 0.0;
		OrbitAngle = 0.0;
		Inclination = 0.0;
		Rot = 0.0;
		Tscale = 1.0;
		Tilt = 0.0;
		Retro = false;
		Atmos = FColor::Black;
		BodyType = EBodyUISizeClass::Moon;
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
	FString  Texture;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString  Gloss;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString  Lights;
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString  Ring;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Radius;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Mass;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Orbit;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double OrbitAngle = 0.0;
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
	EBodyUISizeClass  BodyType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_RegionMap> Region;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_MoonMap> Moon;

	FS_PlanetMap() {
		Name = "";
		Icon = "";
		Ring = "";
		Radius = 0.0;
		Mass = 0.0;
		Orbit = 0.0;
		OrbitAngle = 0.0;
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
		BodyType = EBodyUISizeClass::Planet;
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
	double OrbitAngle = 0.0;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	double Tscale = 1.0;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool   Retro = false;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor  Color;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FColor  Back;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EBodyUISizeClass  BodyType;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TArray<FS_RegionMap> Region;
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
		OrbitAngle = 0.0;
		Tscale = 1.0;
		Retro = false;
		Color = FColor(0, 0, 0, 0);
		Back = FColor(0, 0, 0, 0);
		Class = ESPECTRAL_CLASS::G;
		BodyType = EBodyUISizeClass::Star;
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
	TEnumAsByte<ECOMBATACTION_STATUS> Status;
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
		Status = ECOMBATACTION_STATUS::UNKNOWN;
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
	FString Start;
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

// ------------------------------------------------------------
// Control Type (frm "type")
// ------------------------------------------------------------
UENUM(BlueprintType)
enum class EUIControlType : uint8
{
	Label      UMETA(DisplayName = "Label"),
	Button     UMETA(DisplayName = "Button"),
	Combo      UMETA(DisplayName = "Combo"),
	Edit       UMETA(DisplayName = "Edit"),
	Image      UMETA(DisplayName = "Image"),
	Slider     UMETA(DisplayName = "Slider"),
	List       UMETA(DisplayName = "List"),
	RichText   UMETA(DisplayName = "RichText"),
};

// ------------------------------------------------------------
// Layout Def (frm "layout")
// x_mins / cols, y_mins / rows, x_weights / col_wts, y_weights / row_wts
// ------------------------------------------------------------
USTRUCT(BlueprintType)
struct FS_LayoutDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<int32> XMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<int32> YMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> XWeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<float> YWeight;
};

// ------------------------------------------------------------
// Single UI Control Def (frm "defctrl" and "ctrl")
// Notes:
// - Rect is pixel-space in legacy; keep as FIntRect for now.
// - Insets/margins are UE-native FMargin.
// - Colors are UE-native FColor.
// - Style/Align are stored as legacy numeric flags for later mapping.
// ------------------------------------------------------------
USTRUCT(BlueprintType)
struct FS_UIControlDef
{
	GENERATED_BODY()

	// Identity
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ParentId = 0;

	// Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EUIControlType Type = EUIControlType::Label;

	// Textual
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Text;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Caption;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Alt;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Font;

	// Geometry
	// IMPORTANT: Decide your Rect convention globally (XYWH vs LTRB).
	// Recommended for legacy FRM: treat as XYWH (X,Y,Width,Height) during parsing,
	// then store as: Min=(X,Y) Max=(X+W, Y+H) in FIntRect.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FIntRect Rect = FIntRect(0, 0, 0, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FMargin Margins = FMargin(0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FMargin TextInsets = FMargin(0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FMargin CellInsets = FMargin(0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FIntRect Cells = FIntRect(0, 0, 0, 0);

	// Fixed sizing constraints (if present)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FixedWidth = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FixedHeight = 0;

	// Colors
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColor ActiveColor = FColor::Transparent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColor BackColor = FColor::Transparent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColor BaseColor = FColor::Transparent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColor BorderColor = FColor::Transparent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColor ForeColor = FColor::White;

	// Assets / images
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Texture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString StandardImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString ActivatedImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString TransitionImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Picture;

	// List / combo items
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Item;

	// Password
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Password;

	// Legacy flags / options
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSmoothScroll = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSingleLine = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bActive = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAnimated = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bBorder = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDropShadow = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShowHeadings = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSticky = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bTransparent = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHidePartial = false;

	// Misc numeric
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Tab = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Orientation = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Leading = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 LineHeight = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MultiSelect = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 DragDrop = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ScrollBar = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 PictureLoc = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 PictureType = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 NumLeds = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ItemStyle = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SelectedStyle = 0;

	// Raw legacy style bits (frm "style")
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Style = 0;

	// Raw legacy bevel width (frm "bevel_width")
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BevelWidth = 0;

	// Raw legacy alignment (frm "align" or "text_align")
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Align = 0; // DT_LEFT/DT_RIGHT/DT_CENTER, etc.
};

// ------------------------------------------------------------
// Form Design (frm "FORM" / "form")
// ------------------------------------------------------------
USTRUCT(BlueprintType)
struct FS_FormDesign : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Caption;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 PId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FIntRect Rect = FIntRect(0, 0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColor BackColor = FColor::Black;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColor BaseColor = FColor::Black;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FColor ForeColor = FColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FMargin Insets = FMargin(0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FMargin TextInsets = FMargin(0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FMargin CellInsets = FMargin(0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FIntRect Cells = FIntRect(0, 0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bTransparent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Style = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Align = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FS_LayoutDef LayoutDef;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FS_UIControlDef DefaultCtrl;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FS_UIControlDef> Controls;
};

USTRUCT(BlueprintType)
struct FSystemState
{
	GENERATED_BODY()

	UPROPERTY() FName SystemName;      // key
	UPROPERTY() int32 OwnerIff = 0;    // or EFactionId
	UPROPERTY() float Influence = 1.f; // optional
	UPROPERTY() bool bContested = false;
};

USTRUCT()
struct FCampaignMissionReq
{
	GENERATED_BODY()

	UPROPERTY() int32 MissionType = 0;              // map to your mission enum/int
	UPROPERTY() uint64 StartUniverseSeconds = 0;    // absolute universe time
	UPROPERTY() int32 OwningIff = -1;

	// “Primary group” in SS terms:
	UPROPERTY() int32 PrimaryGroupId = -1;
	UPROPERTY() ECOMBATGROUP_TYPE PrimaryGroupType = ECOMBATGROUP_TYPE::UNKNOWN;

	// Optional objective group:
	UPROPERTY() int32 ObjectiveGroupId = -1;
	UPROPERTY() ECOMBATGROUP_TYPE ObjectiveGroupType = ECOMBATGROUP_TYPE::UNKNOWN;

	// Optional: for scripted/template missions
	UPROPERTY() FName ScriptRowName = NAME_None;
	UPROPERTY() FString ScriptText;

	UPROPERTY() FString ObjectiveText;

	UPROPERTY() EMissionPrimaryDomain Domain = EMissionPrimaryDomain::Fighter;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// AUDIO SETTINGS
////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FStarshatterAudioConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EffectsVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VoiceVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Audio", meta = (ClampMin = "0", ClampMax = "3"))
	int32 SoundQuality = 1;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// VIDEO SETTINGS
////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FStarshatterVideoConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	int32 Width = 1920;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	int32 Height = 1080;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	bool bFullscreen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	int32 MaxTextureSize = 2048;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video", meta = (ClampMin = "32", ClampMax = "224"))
	int32 GammaLevel = 128;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	bool bShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	bool bSpecularMaps = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	bool bBumpMaps = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	bool bLensFlare = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	bool bCorona = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	bool bNebula = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	int32 DustLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	int32 TerrainDetailIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Video")
	bool bTerrainTextures = true;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// CONTROL SETTINGS (NON-KEY BINDINGS)
////////////////////////////////////////////////////////////////////////////////////////////////

UENUM(BlueprintType)
enum class EStarshatterControlModel : uint8
{
	Arcade     UMETA(DisplayName = "Arcade"),
	FlightSim  UMETA(DisplayName = "Flight Simulator"),
	Hybrid     UMETA(DisplayName = "Hybrid")
};

USTRUCT(BlueprintType)
struct FStarshatterControlsConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Controls")
	EStarshatterControlModel ControlModel = EStarshatterControlModel::FlightSim;

	// Joystick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Controls|Joystick")
	int32 JoystickIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Controls|Joystick")
	int32 ThrottleAxis = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Controls|Joystick")
	int32 RudderAxis = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Controls|Joystick", meta = (ClampMin = "0", ClampMax = "10"))
	int32 JoystickSensitivity = 5;

	// Mouse
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Controls|Mouse", meta = (ClampMin = "0", ClampMax = "50"))
	int32 MouseSensitivity = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Controls|Mouse")
	bool bMouseInvert = false;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT ACTIONS (ENHANCED INPUT)
////////////////////////////////////////////////////////////////////////////////////////////////

UENUM(BlueprintType)
enum class EStarshatterInputAction : uint8
{
	// Core
	ExitGame            UMETA(DisplayName = "Exit Game"),
	Pause               UMETA(DisplayName = "Pause"),

	// Time
	TimeCompress        UMETA(DisplayName = "Time Compress"),
	TimeExpand          UMETA(DisplayName = "Time Expand"),
	TimeSkip            UMETA(DisplayName = "Time Skip"),

	// Flight
	ThrottleUp          UMETA(DisplayName = "Throttle Up"),
	ThrottleDown        UMETA(DisplayName = "Throttle Down"),
	ThrottleZero        UMETA(DisplayName = "Throttle Zero"),
	ThrottleFull        UMETA(DisplayName = "Throttle Full"),

	// Weapons
	CyclePrimary        UMETA(DisplayName = "Cycle Primary Weapon"),
	CycleSecondary      UMETA(DisplayName = "Cycle Secondary Weapon"),
	FirePrimary         UMETA(DisplayName = "Fire Primary"),
	FireSecondary       UMETA(DisplayName = "Fire Secondary"),

	// Targeting
	LockTarget          UMETA(DisplayName = "Lock Target"),
	LockThreat          UMETA(DisplayName = "Lock Threat"),
	TargetNext          UMETA(DisplayName = "Target Next"),
	TargetPrevious      UMETA(DisplayName = "Target Previous"),

	// Camera
	CameraNextView      UMETA(DisplayName = "Next Camera View"),
	CameraChase         UMETA(DisplayName = "Chase Camera"),
	CameraExternal      UMETA(DisplayName = "External Camera"),
	CameraZoomIn        UMETA(DisplayName = "Zoom In"),
	CameraZoomOut       UMETA(DisplayName = "Zoom Out"),

	// UI
	NavDialog           UMETA(DisplayName = "Navigation Dialog"),
	WeaponDialog        UMETA(DisplayName = "Weapon Dialog"),
	FlightDialog        UMETA(DisplayName = "Flight Dialog"),
	EngineDialog        UMETA(DisplayName = "Engine Dialog"),

	// Comms
	RadioMenu           UMETA(DisplayName = "Radio Menu"),
	CommandMode         UMETA(DisplayName = "Command Mode"),

	// Debug
	IncStardate         UMETA(DisplayName = "Increase Stardate"),
	DecStardate         UMETA(DisplayName = "Decrease Stardate")
};

USTRUCT(BlueprintType)
struct FStarshatterKeyboardConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Keyboard")
	bool bKeyboardEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Keyboard", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float KeyRepeatDelaySeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Keyboard", meta = (ClampMin = "0.01", ClampMax = "0.50"))
	float KeyRepeatRateSeconds = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Keyboard")
	TMap<EStarshatterInputAction, FKey> RemappedKeys;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// INPUT BINDINGS (ENHANCED INPUT–FRIENDLY)
////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FStarshatterInputBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Input")
	EStarshatterInputAction Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Input")
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Input")
	bool bShift = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Input")
	bool bCtrl = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Input")
	bool bAlt = false;
};

USTRUCT(BlueprintType)
struct FStarshatterKeyMap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starshatter|Input")
	TArray<FStarshatterInputBinding> Bindings;
};