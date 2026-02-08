/*
    Project Starshatter Wars
    Fractal Dev Games
    Copyright (C) 2024-2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterGameDataSubsystem.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Implementation for the global game data loader and registry subsystem.

    This file is a direct port of the legacy GameDataLoader actor logic
    into a GameInstanceSubsystem. It performs no Tick() and has no world
    presence. Loading is triggered explicitly during EGameMode::INIT.
*/

#include "StarshatterGameDataSubsystem.h"

#include "SSWGameInstance.h"
#include "Game.h"
#include "Starsystem.h"
#include "Galaxy.h"
#include "CombatGroup.h"
#include "CombatRoster.h"
#include "CombatAction.h"
#include "CombatEvent.h"
#include "Combatant.h"
#include "Mission.h"
#include "Intel.h"
#include "Ship.h"
#include "RLoc.h"
#include "ShipDesign.h"
#include "PlayerData.h"
#include "SystemDesign.h"
#include "GameContent.h"
#include "GalaxyManager.h"

#include "Engine/DataTable.h"
#include "Engine/TimerHandle.h"
#include "TimerManager.h"
#include "FormattingUtils.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "SimComponent.h"
#include "PlayerSaveGame.h"

#include "Logging/LogMacros.h"

template<typename TEnum>
static bool FStringToEnum(const FString& InString, TEnum& OutEnum, bool bCaseSensitive = true)
{
    UEnum* Enum = StaticEnum<TEnum>();
    if (!Enum)
        return false;

    for (int32 i = 0; i < Enum->NumEnums(); ++i)
    {
        const FString Name = Enum->GetNameStringByIndex(i);
        if ((bCaseSensitive && Name == InString) ||
            (!bCaseSensitive && Name.Equals(InString, ESearchCase::IgnoreCase)))
        {
            OutEnum = static_cast<TEnum>(Enum->GetValueByIndex(i));
            return true;
        }
    }

    return false;
}

template <typename TEnum>
static FString EnumToDisplayNameString(TEnum EnumValue)
{
    static_assert(TIsEnum<TEnum>::Value, "EnumToDisplayNameString only works with UENUMs.");

    UEnum* EnumPtr = StaticEnum<TEnum>();
    if (!EnumPtr)
        return TEXT("Invalid");

    return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(EnumValue)).ToString();
}

static uint8 ToByteClamp(double v)
{
	// Legacy files sometimes store 0..255, sometimes 0..1.
	// Heuristic: if <= 1.0, treat as normalized.
	if (v <= 1.0)
	{
		v = v * 255.0;
	}
	v = FMath::Clamp(v, 0.0, 255.0);
	return (uint8)FMath::RoundToInt(v);
}

static FColor Vec3ToColor255(const Vec3& a)
{
	return FColor(ToByteClamp(a.X), ToByteClamp(a.Y), ToByteClamp(a.Z), 255);
}

static FColor ToFColor(const Color& c)
{
	// Assuming legacy Color uses 0..255 channels. Adjust if your Color is 0..1 floats.
	return FColor((uint8)c.Red(), (uint8)c.Green(), (uint8)c.Blue(), (uint8)c.Alpha());
}

static FIntRect ToFIntRect(const Rect& r)
{
	// Adjust if Rect is (x,y,w,h) instead of (l,t,r,b).
	return FIntRect((int32)r.x, (int32)r.y, (int32)(r.x + r.w), (int32)(r.y + r.h));
}

static FMargin ToFMargin(const Insets& in)
{
	return FMargin((float)in.left, (float)in.top, (float)in.right, (float)in.bottom);
}

// +--------------------------------------------------------------------+
void UStarshatterGameDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CampaignDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Game/DT_Campaign.DT_Campaign"));
	CampaignOOBDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Game/DT_CampaignOOB.DT_CampaignOOB"));
	CombatGroupDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Game/DT_CombatGroup.DT_CombatGroup"));
	GalaxyDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Game/DT_GalaxyMap.DT_GalaxyMap"));
	OrderOfBattleDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Game/DT_OrderOfBattle.DT_OrderOfBattle"));
	AwardsDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Game/DT_AwardInfo.DT_AwardInfo"));
	
	UE_LOG(LogTemp, Warning, TEXT("DT Campaign=%s Galaxy=%s"),
		*GetNameSafe(CampaignDataTable),
		*GetNameSafe(GalaxyDataTable));

	if (bClearTables)
	{
		if (CampaignDataTable)      CampaignDataTable->EmptyTable();
		if (CampaignOOBDataTable)   CampaignOOBDataTable->EmptyTable();
		if (CombatGroupDataTable)   CombatGroupDataTable->EmptyTable();
		if (OrderOfBattleDataTable) OrderOfBattleDataTable->EmptyTable();
		if (GalaxyDataTable)        GalaxyDataTable->EmptyTable();
	}
}

void UStarshatterGameDataSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UStarshatterGameDataSubsystem::SetProjectPath()
{
	ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/"));

	UE_LOG(LogTemp, Log, TEXT("Setting Game Data Directory %s"), *ProjectPath);
}

FString UStarshatterGameDataSubsystem::GetProjectPath()
{
	return ProjectPath;
}

void UStarshatterGameDataSubsystem::CreateOrderOfBattleTable()
{
	if (!OrderOfBattleDataTable)
		return;

	OrderOfBattleDataTable->EmptyTable();

	// Use LOCAL containers with unique names so we do not hide members:
	TArray<FS_OOBFleet> LocalFleets;
	TArray<FS_OOBCarrier> LocalCarriers;
	TArray<FS_OOBDestroyer> LocalDestroyers;
	TArray<FS_OOBBattle> LocalBattles;
	TArray<FS_OOBWing> LocalWings;

	TArray<FS_OOBFighter> LocalFighters;
	TArray<FS_OOBIntercept> LocalInterceptors;
	TArray<FS_OOBAttack> LocalAttacks;
	TArray<FS_OOBLanding> LocalLandings;

	TArray<FS_OOBBattalion> LocalBattalions;
	TArray<FS_OOBBattery> LocalBatteries;
	TArray<FS_OOBStation> LocalStations;
	TArray<FS_OOBStarbase> LocalStarbases;

	TArray<FS_OOBCivilian> LocalCivilians;
	TArray<FS_OOBMinefield> LocalMinefields;

	FS_OOBForce NewForce;
	FS_OOBForce* ForceRow = nullptr;

	int CurrentForceId = -1;
	EEMPIRE_NAME CurrentEmpire = EEMPIRE_NAME::Unknown;
	int CurrentIff = -1;
	FName CurrentForceRowName = NAME_None;

	// ---------------------------------------------
	// Pass 1: Walk roster and build flat collections
	// ---------------------------------------------
	for (const FS_CombatGroup& Item : CombatRosterData)
	{
		if (Item.Type == ECOMBATGROUP_TYPE::FORCE)
		{
			// reset per-force aggregates
			LocalFleets.Empty();
			LocalBattalions.Empty();
			LocalCivilians.Empty();

			// NOTE: These are "global" for the force and used later:
			CurrentForceId = Item.Id;
			CurrentEmpire = Item.EmpireId;
			CurrentIff = Item.Iff;

			NewForce = FS_OOBForce();
			NewForce.Id = Item.Id;
			NewForce.Name = Item.DisplayName;
			NewForce.Iff = Item.Iff;
			NewForce.Location = Item.Region;
			NewForce.Empire = Item.EmpireId;
			NewForce.Intel = Item.Intel;

			CurrentForceRowName = FName(*NewForce.Name);

			if (NewForce.Iff > -1)
			{
				OrderOfBattleDataTable->AddRow(CurrentForceRowName, NewForce);
			}

			continue;
		}

		// Everything below assumes we are currently under a valid FORCE:
		if (CurrentForceId < 0)
			continue;

		// ---------------------------------------------
		// Fleet under Force
		// ---------------------------------------------
		if (Item.Type == ECOMBATGROUP_TYPE::FLEET)
		{
			if (Item.ParentId == CurrentForceId && Item.EmpireId == CurrentEmpire)
			{
				FS_OOBFleet NewFleet;
				NewFleet.Id = Item.Id;
				NewFleet.ParentId = Item.ParentId;
				NewFleet.Name = Item.DisplayName;
				NewFleet.Iff = Item.Iff;
				NewFleet.Location = Item.Region;
				NewFleet.Empire = Item.EmpireId;
				NewFleet.Intel = Item.Intel;

				LocalFleets.Add(NewFleet);
			}

			continue;
		}

		// ---------------------------------------------
		// Battalion / Civilians under Force
		// ---------------------------------------------
		if (Item.Type == ECOMBATGROUP_TYPE::BATTALION)
		{
			if (Item.ParentId == CurrentForceId && Item.EmpireId == CurrentEmpire)
			{
				FS_OOBBattalion NewBattalion;
				NewBattalion.Id = Item.Id;
				NewBattalion.ParentId = Item.ParentId;
				NewBattalion.Name = Item.DisplayName;
				NewBattalion.Iff = Item.Iff;
				NewBattalion.Location = Item.Region;
				NewBattalion.Empire = Item.EmpireId;
				NewBattalion.Intel = Item.Intel;

				LocalBattalions.Add(NewBattalion);
			}

			continue;
		}

		if (Item.Type == ECOMBATGROUP_TYPE::CIVILIAN)
		{
			if (Item.ParentId == CurrentForceId && Item.EmpireId == CurrentEmpire)
			{
				FS_OOBCivilian NewCivilian;
				NewCivilian.Id = Item.Id;
				NewCivilian.ParentId = Item.ParentId;
				NewCivilian.Name = Item.DisplayName;
				NewCivilian.Iff = Item.Iff;
				NewCivilian.Location = Item.Region;
				NewCivilian.Empire = Item.EmpireId;
				NewCivilian.Intel = Item.Intel;

				LocalCivilians.Add(NewCivilian);
			}

			continue;
		}

		// ---------------------------------------------
		// Stations / Starbases / Batteries / Minefields
		// (stored globally then attached later)
		// ---------------------------------------------
		if (Item.Type == ECOMBATGROUP_TYPE::STATION)
		{
			FS_OOBStation NewStation;
			NewStation.Id = Item.Id;
			NewStation.ParentId = Item.ParentId;
			NewStation.Name = Item.DisplayName;
			NewStation.Iff = Item.Iff;
			NewStation.Location = Item.Region;
			NewStation.Empire = Item.EmpireId;
			NewStation.Intel = Item.Intel;

			NewStation.Unit.SetNum(Item.Unit.Num());

			int32 UnitIndex = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewStation.Unit.IsValidIndex(UnitIndex))
					break;

				NewStation.Unit[UnitIndex].Name = UnitItem.UnitName;
				NewStation.Unit[UnitIndex].Count = UnitItem.UnitCount;
				NewStation.Unit[UnitIndex].Location = Item.Region;
				NewStation.Unit[UnitIndex].ParentId = Item.ParentId;
				NewStation.Unit[UnitIndex].Empire = Item.EmpireId;
				NewStation.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::STATION;
				NewStation.Unit[UnitIndex].ParentType = ECOMBATGROUP_TYPE::STATION;
				NewStation.Unit[UnitIndex].Design = UnitItem.UnitDesign;

				++UnitIndex;
			}

			LocalStations.Add(NewStation);
			continue;
		}

		if (Item.Type == ECOMBATGROUP_TYPE::STARBASE)
		{
			FS_OOBStarbase NewStarbase;
			NewStarbase.Id = Item.Id;
			NewStarbase.ParentId = Item.ParentId;
			NewStarbase.Name = Item.DisplayName;
			NewStarbase.Iff = Item.Iff;
			NewStarbase.Location = Item.Region;
			NewStarbase.Empire = Item.EmpireId;
			NewStarbase.Intel = Item.Intel;

			NewStarbase.Unit.SetNum(Item.Unit.Num());

			int32 UnitIndex = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewStarbase.Unit.IsValidIndex(UnitIndex))
					break;

				NewStarbase.Unit[UnitIndex].Name = UnitItem.UnitName;
				NewStarbase.Unit[UnitIndex].Count = UnitItem.UnitCount;
				NewStarbase.Unit[UnitIndex].Location = Item.Region;
				NewStarbase.Unit[UnitIndex].ParentId = Item.ParentId;
				NewStarbase.Unit[UnitIndex].Empire = Item.EmpireId;
				NewStarbase.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::STARBASE;
				NewStarbase.Unit[UnitIndex].ParentType = ECOMBATGROUP_TYPE::STARBASE;
				NewStarbase.Unit[UnitIndex].Design = UnitItem.UnitDesign;

				++UnitIndex;
			}

			LocalStarbases.Add(NewStarbase);
			continue;
		}

		if (Item.Type == ECOMBATGROUP_TYPE::BATTERY)
		{
			FS_OOBBattery NewBattery;
			NewBattery.Id = Item.Id;
			NewBattery.ParentId = Item.ParentId;
			NewBattery.Name = Item.DisplayName;
			NewBattery.Iff = Item.Iff;
			NewBattery.Location = Item.Region;
			NewBattery.Empire = Item.EmpireId;
			NewBattery.Intel = Item.Intel;

			NewBattery.Unit.SetNum(Item.Unit.Num());

			int32 UnitIndex = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewBattery.Unit.IsValidIndex(UnitIndex))
					break;

				NewBattery.Unit[UnitIndex].Name = UnitItem.UnitName;
				NewBattery.Unit[UnitIndex].Count = UnitItem.UnitCount;
				NewBattery.Unit[UnitIndex].Location = Item.Region;
				NewBattery.Unit[UnitIndex].ParentId = Item.ParentId;
				NewBattery.Unit[UnitIndex].Empire = Item.EmpireId;
				NewBattery.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::BATTERY;
				NewBattery.Unit[UnitIndex].ParentType = ECOMBATGROUP_TYPE::BATTERY;
				NewBattery.Unit[UnitIndex].Design = UnitItem.UnitDesign;

				++UnitIndex;
			}

			LocalBatteries.Add(NewBattery);
			continue;
		}

		if (Item.Type == ECOMBATGROUP_TYPE::MINEFIELD)
		{
			FS_OOBMinefield NewMinefield;
			NewMinefield.Id = Item.Id;
			NewMinefield.ParentId = Item.ParentId;
			NewMinefield.Name = Item.DisplayName;
			NewMinefield.Iff = Item.Iff;
			NewMinefield.Location = Item.Region;
			NewMinefield.Empire = Item.EmpireId;
			NewMinefield.Intel = Item.Intel;

			NewMinefield.Unit.SetNum(Item.Unit.Num());

			int32 UnitIndex = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewMinefield.Unit.IsValidIndex(UnitIndex))
					break;

				NewMinefield.Unit[UnitIndex].Name = UnitItem.UnitName;
				NewMinefield.Unit[UnitIndex].Count = UnitItem.UnitCount;
				NewMinefield.Unit[UnitIndex].Location = Item.Region;
				NewMinefield.Unit[UnitIndex].ParentId = Item.ParentId;
				NewMinefield.Unit[UnitIndex].Empire = Item.EmpireId;
				NewMinefield.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::MINE;
				NewMinefield.Unit[UnitIndex].ParentType = ECOMBATGROUP_TYPE::MINEFIELD;
				NewMinefield.Unit[UnitIndex].Design = UnitItem.UnitDesign;

				++UnitIndex;
			}

			LocalMinefields.Add(NewMinefield);
			continue;
		}

		// ---------------------------------------------
		// Carrier / Destroyer / Battle groups and Wings
		// ---------------------------------------------
		if (Item.Type == ECOMBATGROUP_TYPE::CARRIER_GROUP)
		{
			FS_OOBCarrier NewCarrier;
			NewCarrier.Id = Item.Id;
			NewCarrier.ParentId = Item.ParentId;
			NewCarrier.Name = Item.DisplayName;
			NewCarrier.Iff = Item.Iff;
			NewCarrier.Location = Item.Region;
			NewCarrier.Empire = Item.EmpireId;
			NewCarrier.Intel = Item.Intel;

			NewCarrier.Unit.SetNum(Item.Unit.Num());

			int32 UnitIndex = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewCarrier.Unit.IsValidIndex(UnitIndex))
					break;

				NewCarrier.Unit[UnitIndex].Name = UnitItem.UnitName;
				NewCarrier.Unit[UnitIndex].Count = 1;
				NewCarrier.Unit[UnitIndex].ParentId = Item.ParentId;
				NewCarrier.Unit[UnitIndex].Regnum = UnitItem.UnitRegnum;
				NewCarrier.Unit[UnitIndex].Empire = Item.EmpireId;
				NewCarrier.Unit[UnitIndex].Location = Item.Region;
				NewCarrier.Unit[UnitIndex].ParentType = ECOMBATGROUP_TYPE::CARRIER_GROUP;
				NewCarrier.Unit[UnitIndex].Design = UnitItem.UnitDesign;

				if (UnitItem.UnitClass == "Cruiser")
					NewCarrier.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::CRUISER;
				else if (UnitItem.UnitClass == "Destroyer")
					NewCarrier.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::DESTROYER;
				else if (UnitItem.UnitClass == "Frigate")
					NewCarrier.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::FRIGATE;
				else if (UnitItem.UnitClass == "Carrier")
					NewCarrier.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::CARRIER;

				NewCarrier.Unit[UnitIndex].DisplayName =
					UFormattingUtils::GetUnitPrefixFromType(NewCarrier.Unit[UnitIndex].Type) +
					UnitItem.UnitRegnum + " " + UnitItem.UnitName;

				++UnitIndex;
			}

			LocalCarriers.Add(NewCarrier);
			continue;
		}

		if (Item.Type == ECOMBATGROUP_TYPE::DESTROYER_SQUADRON)
		{
			FS_OOBDestroyer NewDestroyer;
			NewDestroyer.Id = Item.Id;
			NewDestroyer.ParentId = Item.ParentId;
			NewDestroyer.Name = Item.DisplayName;
			NewDestroyer.Iff = Item.Iff;
			NewDestroyer.Location = Item.Region;
			NewDestroyer.Empire = Item.EmpireId;
			NewDestroyer.Intel = Item.Intel;

			NewDestroyer.Unit.SetNum(Item.Unit.Num());

			int32 UnitIndex = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewDestroyer.Unit.IsValidIndex(UnitIndex))
					break;

				NewDestroyer.Unit[UnitIndex].Name = UnitItem.UnitName;
				NewDestroyer.Unit[UnitIndex].Count = 1;
				NewDestroyer.Unit[UnitIndex].ParentId = Item.ParentId;
				NewDestroyer.Unit[UnitIndex].Empire = Item.EmpireId;
				NewDestroyer.Unit[UnitIndex].Regnum = UnitItem.UnitRegnum;
				NewDestroyer.Unit[UnitIndex].Location = Item.Region;
				NewDestroyer.Unit[UnitIndex].ParentType = ECOMBATGROUP_TYPE::DESTROYER_SQUADRON;
				NewDestroyer.Unit[UnitIndex].Design = UnitItem.UnitDesign;

				if (UnitItem.UnitClass == "Cruiser")
					NewDestroyer.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::CRUISER;
				else if (UnitItem.UnitClass == "Destroyer")
					NewDestroyer.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::DESTROYER;
				else if (UnitItem.UnitClass == "Frigate")
					NewDestroyer.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::FRIGATE;
				else if (UnitItem.UnitClass == "Carrier")
					NewDestroyer.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::CARRIER;

				NewDestroyer.Unit[UnitIndex].DisplayName =
					UFormattingUtils::GetUnitPrefixFromType(NewDestroyer.Unit[UnitIndex].Type) +
					UnitItem.UnitRegnum + " " + UnitItem.UnitName;

				++UnitIndex;
			}

			LocalDestroyers.Add(NewDestroyer);
			continue;
		}

		if (Item.Type == ECOMBATGROUP_TYPE::BATTLE_GROUP)
		{
			FS_OOBBattle NewBattle;
			NewBattle.Id = Item.Id;
			NewBattle.ParentId = Item.ParentId;
			NewBattle.Name = Item.DisplayName;
			NewBattle.Iff = Item.Iff;
			NewBattle.Location = Item.Region;
			NewBattle.Empire = Item.EmpireId;
			NewBattle.Intel = Item.Intel;

			NewBattle.Unit.SetNum(Item.Unit.Num());

			int32 UnitIndex = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewBattle.Unit.IsValidIndex(UnitIndex))
					break;

				NewBattle.Unit[UnitIndex].Name = UnitItem.UnitName;
				NewBattle.Unit[UnitIndex].Count = 1;
				NewBattle.Unit[UnitIndex].ParentId = Item.ParentId;
				NewBattle.Unit[UnitIndex].Regnum = UnitItem.UnitRegnum;
				NewBattle.Unit[UnitIndex].Empire = Item.EmpireId;
				NewBattle.Unit[UnitIndex].Location = Item.Region;
				NewBattle.Unit[UnitIndex].ParentType = ECOMBATGROUP_TYPE::BATTLE_GROUP;
				NewBattle.Unit[UnitIndex].Design = UnitItem.UnitDesign;

				if (UnitItem.UnitClass == "Cruiser")
					NewBattle.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::CRUISER;
				else if (UnitItem.UnitClass == "Destroyer")
					NewBattle.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::DESTROYER;
				else if (UnitItem.UnitClass == "Frigate")
					NewBattle.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::FRIGATE;
				else if (UnitItem.UnitClass == "Carrier")
					NewBattle.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::CARRIER;

				NewBattle.Unit[UnitIndex].DisplayName =
					UFormattingUtils::GetUnitPrefixFromType(NewBattle.Unit[UnitIndex].Type) +
					UnitItem.UnitRegnum + " " + UnitItem.UnitName;

				++UnitIndex;
			}

			LocalBattles.Add(NewBattle);
			continue;
		}

		if (Item.Type == ECOMBATGROUP_TYPE::WING)
		{
			FS_OOBWing NewWing;
			NewWing.Id = Item.Id;
			NewWing.ParentId = Item.ParentId;
			NewWing.Name = Item.DisplayName;
			NewWing.Iff = Item.Iff;
			NewWing.Location = Item.Region;
			NewWing.Empire = Item.EmpireId;
			NewWing.Intel = Item.Intel;

			LocalWings.Add(NewWing);
			continue;
		}

		// ---------------------------------------------
		// Squadrons (fighters/intercepts/attacks/landings)
		// ---------------------------------------------
		if (Item.Type == ECOMBATGROUP_TYPE::FIGHTER_SQUADRON)
		{
			FS_OOBFighter NewFighter;
			NewFighter.Id = Item.Id;
			NewFighter.ParentId = Item.ParentId;
			NewFighter.Name = Item.DisplayName;
			NewFighter.Iff = Item.Iff;
			NewFighter.Location = Item.Region;
			NewFighter.ParentType = Item.ParentType;
			NewFighter.Empire = Item.EmpireId;
			NewFighter.Intel = Item.Intel;

			NewFighter.Unit.SetNum(Item.Unit.Num());

			int32 UnitIndex = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewFighter.Unit.IsValidIndex(UnitIndex))
					break;

				NewFighter.Unit[UnitIndex].Name = UnitItem.UnitName;
				NewFighter.Unit[UnitIndex].Count = UnitItem.UnitCount;
				NewFighter.Unit[UnitIndex].Location = Item.Region;
				NewFighter.Unit[UnitIndex].ParentId = Item.ParentId;
				NewFighter.Unit[UnitIndex].Empire = Item.EmpireId;
				NewFighter.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::FIGHTER;
				NewFighter.Unit[UnitIndex].ParentType = ECOMBATGROUP_TYPE::FIGHTER_SQUADRON;
				NewFighter.Unit[UnitIndex].Design = UnitItem.UnitDesign;

				++UnitIndex;
			}

			LocalFighters.Add(NewFighter);
			continue;
		}

		if (Item.Type == ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON)
		{
			FS_OOBIntercept NewIntercept;
			NewIntercept.Id = Item.Id;
			NewIntercept.ParentId = Item.ParentId;
			NewIntercept.Name = Item.DisplayName;
			NewIntercept.Iff = Item.Iff;
			NewIntercept.Location = Item.Region;
			NewIntercept.ParentType = Item.ParentType;
			NewIntercept.Empire = Item.EmpireId;
			NewIntercept.Intel = Item.Intel;

			NewIntercept.Unit.SetNum(Item.Unit.Num());

			int32 UnitIndex = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewIntercept.Unit.IsValidIndex(UnitIndex))
					break;

				NewIntercept.Unit[UnitIndex].Name = UnitItem.UnitName;
				NewIntercept.Unit[UnitIndex].Count = UnitItem.UnitCount;
				NewIntercept.Unit[UnitIndex].Location = Item.Region;
				NewIntercept.Unit[UnitIndex].ParentId = Item.ParentId;
				NewIntercept.Unit[UnitIndex].Empire = Item.EmpireId;
				NewIntercept.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::INTERCEPT;
				NewIntercept.Unit[UnitIndex].ParentType = ECOMBATGROUP_TYPE::INTERCEPT_SQUADRON;
				NewIntercept.Unit[UnitIndex].Design = UnitItem.UnitDesign;

				++UnitIndex;
			}

			LocalInterceptors.Add(NewIntercept);
			continue;
		}

		if (Item.Type == ECOMBATGROUP_TYPE::ATTACK_SQUADRON)
		{
			FS_OOBAttack NewAttack;
			NewAttack.Id = Item.Id;
			NewAttack.ParentId = Item.ParentId;
			NewAttack.Name = Item.DisplayName;
			NewAttack.Iff = Item.Iff;
			NewAttack.Location = Item.Region;
			NewAttack.ParentType = Item.ParentType;
			NewAttack.Empire = Item.EmpireId;
			NewAttack.Intel = Item.Intel;

			NewAttack.Unit.SetNum(Item.Unit.Num());

			int32 UnitIndex = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewAttack.Unit.IsValidIndex(UnitIndex))
					break;

				NewAttack.Unit[UnitIndex].Name = UnitItem.UnitName;
				NewAttack.Unit[UnitIndex].Count = UnitItem.UnitCount;
				NewAttack.Unit[UnitIndex].Location = Item.Region;
				NewAttack.Unit[UnitIndex].ParentId = Item.ParentId;
				NewAttack.Unit[UnitIndex].Empire = Item.EmpireId;
				NewAttack.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::ATTACK;
				NewAttack.Unit[UnitIndex].ParentType = ECOMBATGROUP_TYPE::ATTACK_SQUADRON;
				NewAttack.Unit[UnitIndex].Design = UnitItem.UnitDesign;

				++UnitIndex;
			}

			LocalAttacks.Add(NewAttack);
			continue;
		}

		if (Item.Type == ECOMBATGROUP_TYPE::LCA_SQUADRON)
		{
			FS_OOBLanding NewLanding;
			NewLanding.Id = Item.Id;
			NewLanding.ParentId = Item.ParentId;
			NewLanding.Name = Item.DisplayName;
			NewLanding.Iff = Item.Iff;
			NewLanding.Location = Item.Region;
			NewLanding.ParentType = Item.ParentType;
			NewLanding.Empire = Item.EmpireId;
			NewLanding.Intel = Item.Intel;

			NewLanding.Unit.SetNum(Item.Unit.Num());

			int32 UnitIndex = 0;
			for (const auto& UnitItem : Item.Unit)
			{
				if (!NewLanding.Unit.IsValidIndex(UnitIndex))
					break;

				NewLanding.Unit[UnitIndex].Name = UnitItem.UnitName;
				NewLanding.Unit[UnitIndex].Count = UnitItem.UnitCount;
				NewLanding.Unit[UnitIndex].Location = Item.Region;
				NewLanding.Unit[UnitIndex].ParentId = Item.ParentId;
				NewLanding.Unit[UnitIndex].Empire = Item.EmpireId;
				NewLanding.Unit[UnitIndex].Type = ECOMBATUNIT_TYPE::LCA;
				NewLanding.Unit[UnitIndex].ParentType = ECOMBATGROUP_TYPE::LCA_SQUADRON;
				NewLanding.Unit[UnitIndex].Design = UnitItem.UnitDesign;

				++UnitIndex;
			}

			LocalLandings.Add(NewLanding);
			continue;
		}
	}

	// ---------------------------------------------
	// Pass 2: Attach batteries/stations/starbases to battalions
	// ---------------------------------------------
	for (FS_OOBBattalion& Battalion : LocalBattalions)
	{
		Battalion.Battery.Reset();
		Battalion.Station.Reset();
		Battalion.Starbase.Reset();

		for (const FS_OOBBattery& Battery : LocalBatteries)
		{
			if (Battery.ParentId == Battalion.Id && Battery.Empire == Battalion.Empire)
				Battalion.Battery.Add(Battery);
		}

		for (const FS_OOBStation& Station : LocalStations)
		{
			if (Station.ParentId == Battalion.Id && Station.Empire == Battalion.Empire)
				Battalion.Station.Add(Station);
		}

		for (const FS_OOBStarbase& Starbase : LocalStarbases)
		{
			if (Starbase.ParentId == Battalion.Id && Starbase.Empire == Battalion.Empire)
				Battalion.Starbase.Add(Starbase);
		}
	}

	// ---------------------------------------------
	// Pass 3: Attach fleets -> carriers/wings/squadrons + battles/destroyers/minefields
	// ---------------------------------------------
	for (FS_OOBFleet& Fleet : LocalFleets)
	{
		Fleet.Carrier.Reset();
		Fleet.Destroyer.Reset();
		Fleet.Battle.Reset();
		Fleet.Minefield.Reset();

		for (FS_OOBCarrier& Carrier : LocalCarriers)
		{
			if (Carrier.ParentId != Fleet.Id || Carrier.Empire != Fleet.Empire)
				continue;

			Carrier.Wing.Reset();
			Carrier.Fighter.Reset();
			Carrier.Attack.Reset();
			Carrier.Intercept.Reset();
			Carrier.Landing.Reset();

			for (const FS_OOBFighter& Fighter : LocalFighters)
			{
				if (Fighter.ParentId == Carrier.Id && Fighter.Empire == Carrier.Empire &&
					Fighter.ParentType == ECOMBATGROUP_TYPE::CARRIER_GROUP)
				{
					Carrier.Fighter.Add(Fighter);
				}
			}

			for (const FS_OOBAttack& Attack : LocalAttacks)
			{
				if (Attack.ParentId == Carrier.Id && Attack.Empire == Carrier.Empire &&
					Attack.ParentType == ECOMBATGROUP_TYPE::CARRIER_GROUP)
				{
					Carrier.Attack.Add(Attack);
				}
			}

			for (const FS_OOBIntercept& Intercept : LocalInterceptors)
			{
				if (Intercept.ParentId == Carrier.Id && Intercept.Empire == Carrier.Empire &&
					Intercept.ParentType == ECOMBATGROUP_TYPE::CARRIER_GROUP)
				{
					Carrier.Intercept.Add(Intercept);
				}
			}

			for (const FS_OOBLanding& Landing : LocalLandings)
			{
				if (Landing.ParentId == Carrier.Id && Landing.Empire == Carrier.Empire &&
					Landing.ParentType == ECOMBATGROUP_TYPE::CARRIER_GROUP)
				{
					Carrier.Landing.Add(Landing);
				}
			}

			for (FS_OOBWing& Wing : LocalWings)
			{
				if (Wing.ParentId != Carrier.Id || Wing.Empire != Carrier.Empire)
					continue;

				Wing.Fighter.Reset();
				Wing.Attack.Reset();
				Wing.Intercept.Reset();
				Wing.Landing.Reset();

				for (const FS_OOBFighter& Fighter : LocalFighters)
				{
					if (Fighter.ParentId == Wing.Id && Fighter.Empire == Wing.Empire &&
						Fighter.ParentType == ECOMBATGROUP_TYPE::WING)
					{
						Wing.Fighter.Add(Fighter);
					}
				}

				for (const FS_OOBAttack& Attack : LocalAttacks)
				{
					if (Attack.ParentId == Wing.Id && Attack.Empire == Wing.Empire)
						Wing.Attack.Add(Attack);
				}

				for (const FS_OOBIntercept& Intercept : LocalInterceptors)
				{
					if (Intercept.ParentId == Wing.Id && Intercept.Empire == Wing.Empire)
						Wing.Intercept.Add(Intercept);
				}

				for (const FS_OOBLanding& Landing : LocalLandings)
				{
					if (Landing.ParentId == Wing.Id && Landing.Empire == Wing.Empire)
						Wing.Landing.Add(Landing);
				}

				Carrier.Wing.Add(Wing);
			}

			Fleet.Carrier.Add(Carrier);
		}

		for (const FS_OOBBattle& Battle : LocalBattles)
		{
			if (Battle.ParentId == Fleet.Id && Battle.Empire == Fleet.Empire)
				Fleet.Battle.Add(Battle);
		}

		for (const FS_OOBDestroyer& Destroyer : LocalDestroyers)
		{
			if (Destroyer.ParentId == Fleet.Id && Destroyer.Empire == Fleet.Empire)
				Fleet.Destroyer.Add(Destroyer);
		}

		for (const FS_OOBMinefield& Minefield : LocalMinefields)
		{
			if (Minefield.ParentId == Fleet.Id && Minefield.Empire == Fleet.Empire)
				Fleet.Minefield.Add(Minefield);
		}
	}

	// ---------------------------------------------
	// Final: Assign aggregates into the force row
	// ---------------------------------------------
	if (CurrentForceRowName != NAME_None)
	{
		ForceRow = OrderOfBattleDataTable->FindRow<FS_OOBForce>(CurrentForceRowName, TEXT(""));
		if (ForceRow)
		{
			ForceRow->Fleet = LocalFleets;
			ForceRow->Battalion = LocalBattalions;
			ForceRow->Civilian = LocalCivilians;
		}
	}
}

void UStarshatterGameDataSubsystem::ExportDataToCSV(UDataTable* DataTable, const FString& FileName)
{
	if (!DataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Export failed: DataTable is null."));
		return;
	}

	// You can use none or pretty names — make sure the flag is in scope
	const FString CSVData = DataTable->GetTableAsCSV(EDataTableExportFlags::None);

	const FString SavePath = FPaths::ProjectDir() + FileName;

	if (FFileHelper::SaveStringToFile(CSVData, *SavePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully exported DataTable to: %s"), *SavePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to write DataTable CSV to file."));
	}
}

void UStarshatterGameDataSubsystem::GetSSWInstance()
{
	SSWInstance = (USSWGameInstance*)GetGameInstance();
}

void UStarshatterGameDataSubsystem::CacheSSWInstance()
{
	if (SSWInstance)
		return;

	UGameInstance* GI = GetGameInstance();
	if (!GI)
		return;

	SSWInstance = Cast<USSWGameInstance>(GI);
}

void UStarshatterGameDataSubsystem::LoadAll(bool bFull)
{
	if (bLoaded)
		return;

	bLoaded = true;

	CacheSSWInstance();
	if (!SSWInstance)
		return;
	
	LoadContentBundle();
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UStarshatterGameDataSubsystem* GD = GI->GetSubsystem<UStarshatterGameDataSubsystem>())
		{
			//GD->LoadForms();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GameData subsystem is NULL at LoadForms call site"));
		}
	}

	LoadGalaxyMap();
	LoadSystemDesigns();
	LoadShipDesigns();
	LoadStarsystems();
	LoadAwardTables();

	if (bClearTables)
	{
		InitializeCampaignData();
		InitializeCombatRoster();
	}

	//LoadSystemDesignsFromDT();

	// This can stay in GameInstance if it is truly "global timers"
	SSWInstance->StartGameTimers();

	// USystemDesign::Initialize(SystemDesignTable);
}

void UStarshatterGameDataSubsystem::InitializeCampaignData() {
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::InitializeCampaignData()"));

	ProjectPath = FPaths::ProjectContentDir();
	ProjectPath.Append(TEXT("GameData/Campaigns/"));
	FString FileName = ProjectPath;

	for (int i = 1; i < 6; i++) {
		FileName = ProjectPath;
		FileName.Append("0");
		FileName.Append(FString::FormatAsNumber(i));
		FileName.Append("/");
		CampaignPath = FileName;
		FileName.Append("campaign.def");
		LoadCampaignData(TCHAR_TO_ANSI(*FileName), true);
	}
}

void UStarshatterGameDataSubsystem::LoadCampaignData(const char* fs, bool full)
{
	const FString CampaignDPath = ANSI_TO_TCHAR(fs);

	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *CampaignDPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load: %s"), *CampaignDPath);
		return;
	}

	Bytes.Add(0); // null terminate for BlockReader

	Parser parser(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* term = parser.ParseTerm();

	if (!term) {
		UE_LOG(LogTemp, Log, TEXT("WARNING: could not parse '%s'"), ANSI_TO_TCHAR(fs));
		return;
	}
	else {
		const FString CampaignPathStr = ANSI_TO_TCHAR(fs);
		UE_LOG(LogTemp, Log, TEXT("Campaign file '%s'"), *CampaignPathStr);
	}

	FS_Campaign NewCampaignData;
	TArray<FString> OrdersArray;
	OrdersArray.Empty();

	CombatantArray.Empty();
	CampaignActionArray.Empty();
	MissionArray.Empty();
	TemplateMissionArray.Empty();
	ScriptedMissionArray.Empty();

	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {

				if (def->name()->value() == "name") {
					GetDefText(name, def, filename);
					NewCampaignData.Name = FString(name);
				}
				else if (def->name()->value() == "desc") {
					GetDefText(description, def, filename);
					NewCampaignData.Description = FString(description);
				}
				else if (def->name()->value() == "index") {
					GetDefNumber(Index, def, filename);
					NewCampaignData.Index = Index;
				}
				else if (def->name()->value() == "situation") {
					GetDefText(situation, def, filename);
					NewCampaignData.Situation = FString(situation);
				}
				else if (def->name()->value() == "system") {
					GetDefText(system, def, filename);
					NewCampaignData.System = FString(system);
				}
				else if (def->name()->value() == "region") {
					GetDefText(region, def, filename);
					NewCampaignData.Region = FString(region);
				}
				else if (def->name()->value() == "image") {
					GetDefText(MainImage, def, filename);
					NewCampaignData.MainImage = FString(MainImage);
				}
				else if (def->name()->value() == "date") {
					GetDefText(start, def, filename);
					NewCampaignData.Start = FString(start);
				}
				else if (def->name()->value() == "orders") {
					GetDefText(orders, def, filename);
					OrdersArray.Add(FString(orders));
					NewCampaignData.Orders = OrdersArray;
				}
				else if (def->name()->value() == "scripted") {
					if (def->term() && def->term()->isBool()) {
						scripted = def->term()->isBool()->value();
						NewCampaignData.bScripted = scripted;
					}
				}
				else if (def->name()->value() == "sequential") {
					if (def->term() && def->term()->isBool()) {
						sequential = def->term()->isBool()->value();
						NewCampaignData.bSequential = sequential;
					}
				}

				else if (def->name()->value() == "available") {
					if (def->term() && def->term()->isBool()) {
						available = def->term()->isBool()->value();
						NewCampaignData.bAvailable = available;
					}
				}

				else if (def->name()->value() == "combatant_groups") {
					GetDefNumber(CombatantSize, def, filename);
					NewCampaignData.CombatantSize = CombatantSize;
				}

				else if (def->name()->value() == "action_groups") {
					GetDefNumber(ActionSize, def, filename);
					NewCampaignData.ActionSize = ActionSize;
				}

				else if (def->name()->value() == "action") {

					TermStruct* ActionTerm = def->term()->isStruct();

					NewCampaignData.ActionSize = ActionTerm->elements()->size();

					if (NewCampaignData.ActionSize > 0)
					{
						ActionId = 0;
						ActionType = "";
						ActionSubtype = 0;
						OppType = -1;
						ActionTeam = 0;
						ActionSource = "";
						ActionLocation = FVector::Zero();

						ActionSystem = "";
						ActionRegion = "";
						ActionFile = "";

						ActionScene = "";
						ActionText = "";

						ActionCount = 1;
						StartBefore = Game::TIME_NEVER;
						StartAfter = 0;
						MinRank = 0;
						MaxRank = 100;
						Delay = 0;
						Probability = 100;

						AssetType = "";
						AssetId = 0;
						TargetType = "";
						TargetId = 0;
						TargetIff = 0;

						AssetKill = "";
						TargetKill = "";

						for (int ActionIdx = 0; ActionIdx < NewCampaignData.ActionSize; ActionIdx++)
						{
							ActionImage = "Empty";
							ActionAudio = "";

							TermDef* pdef = ActionTerm->elements()->at(ActionIdx)->isDef();

							if (pdef->name()->value() == "id") {
								GetDefNumber(ActionId, pdef, filename);
								NewCampaignAction.Id = ActionId;
								UE_LOG(LogTemp, Log, TEXT("action id: '%d'"), ActionId);
							}
							else if (pdef->name()->value() == "type") {
								GetDefText(ActionType, pdef, filename);
								//type = CombatAction::TypeFromName(txt);
								NewCampaignAction.Type = FString(ActionType);
								UE_LOG(LogTemp, Log, TEXT("action type: '%s'"), *FString(ActionType));
							}
							else if (pdef->name()->value() == "subtype") {
								if (pdef->term()->isNumber()) {
									GetDefNumber(ActionSubtype, pdef, filename);
									NewCampaignAction.Subtype = ActionSubtype;
								}
								else if (pdef->term()->isText()) {
									char txt[64];
									GetDefText(txt, pdef, filename);

									if (ActionType == ECOMBATACTION_TYPE::MISSION_TEMPLATE) {
										ActionSubtype = Mission::TypeFromName(txt);
									}
									else if (ActionType == ECOMBATACTION_TYPE::COMBAT_EVENT) {
										ActionSubtype = CombatEvent::TypeFromName(txt);
									}
									if (ActionType == ECOMBATACTION_TYPE::INTEL_EVENT) {
										ActionSubtype = Intel::IntelFromName(txt);
									}
									NewCampaignAction.Subtype = ActionSubtype;
								}

							}
							else if (pdef->name()->value() == "opp_type") {
								if (pdef->term()->isNumber()) {
									GetDefNumber(OppType, pdef, filename);
									NewCampaignAction.OppType = OppType;
								}

								else if (pdef->term()->isText()) {
									char txt[64];
									GetDefText(txt, pdef, filename);

									if (ActionType == ECOMBATACTION_TYPE::MISSION_TEMPLATE) {
										OppType = Mission::TypeFromName(txt);
									}
								}
							}
							else if (pdef->name()->value() == "source") {
								GetDefText(ActionSource, pdef, filename);
								//source = CombatEvent::SourceFromName(txt);
								NewCampaignAction.Source = FString(ActionSource);
							}
							else if (pdef->name()->value() == "team") {
								GetDefNumber(ActionTeam, pdef, filename);
								NewCampaignAction.Team = ActionTeam;
							}
							else if (pdef->name()->value() == "iff") {
								GetDefNumber(ActionTeam, pdef, filename);
								NewCampaignAction.Iff = ActionTeam;
							}
							else if (pdef->name()->value() == "count") {
								GetDefNumber(ActionCount, pdef, filename);
								NewCampaignAction.Count = ActionCount;
							}
							else if (pdef->name()->value().contains("before")) {
								if (pdef->term()->isNumber()) {
									GetDefNumber(StartBefore, pdef, filename);
									NewCampaignAction.StartBefore = StartBefore;
								}
								else {
									GetDefTime(StartBefore, pdef, filename);
									StartBefore -= Game::ONE_DAY;
									NewCampaignAction.StartBefore = StartBefore;
								}
							}
							else if (pdef->name()->value().contains("after")) {
								if (pdef->term()->isNumber()) {
									GetDefNumber(StartAfter, pdef, filename);
									NewCampaignAction.StartAfter = StartAfter;
								}
								else {
									GetDefTime(StartAfter, pdef, filename);
									StartAfter -= Game::ONE_DAY;
									NewCampaignAction.StartAfter = StartAfter;
								}
							}
							else if (pdef->name()->value() == "min_rank") {
								if (pdef->term()->isNumber()) {
									GetDefNumber(MinRank, pdef, filename);
									NewCampaignAction.MinRank = MinRank;
								}
								else {
									char rank_name[64];
									GetDefText(rank_name, pdef, filename);
									MinRank = PlayerData::RankFromName(rank_name);
									NewCampaignAction.MinRank = MinRank;
								}
							}
							else if (pdef->name()->value() == "max_rank") {
								if (pdef->term()->isNumber()) {
									GetDefNumber(MaxRank, pdef, filename);
									NewCampaignAction.MaxRank = MaxRank;
								}
								else {
									char rank_name[64];
									GetDefText(rank_name, pdef, filename);
									MaxRank = PlayerData::RankFromName(rank_name);
									NewCampaignAction.MaxRank = MaxRank;
								}
							}
							else if (pdef->name()->value() == "delay") {
								GetDefNumber(Delay, pdef, filename);
								NewCampaignAction.Delay = Delay;
							}
							else if (pdef->name()->value() == "probability") {
								GetDefNumber(Probability, pdef, filename);
								NewCampaignAction.Probability = Probability;
							}
							else if (pdef->name()->value() == "asset_type") {
								GetDefText(AssetType, pdef, filename);
								//asset_type = CombatGroup::TypeFromName(type_name);
								NewCampaignAction.AssetType = FString(AssetType);
							}
							else if (pdef->name()->value() == "target_type") {
								GetDefText(TargetType, pdef, filename);
								NewCampaignAction.TargetType = FString(TargetType);
								//target_type = CombatGroup::TypeFromName(type_name);
							}
							else if (pdef->name()->value() == "location" ||
								pdef->name()->value() == "loc") {
								GetDefVec(ActionLocation, pdef, filename);
								NewCampaignAction.Location.X = ActionLocation.X;
								NewCampaignAction.Location.Y = ActionLocation.Y;
								NewCampaignAction.Location.Z = ActionLocation.Z;

							}
							else if (pdef->name()->value() == "system" ||
								pdef->name()->value() == "sys") {
								GetDefText(ActionSystem, pdef, filename);
								NewCampaignAction.System = FString(ActionSystem);
							}
							else if (pdef->name()->value() == "region" ||
								pdef->name()->value() == "rgn" ||
								pdef->name()->value() == "zone") {
								GetDefText(ActionRegion, pdef, filename);
								NewCampaignAction.Region = FString(ActionRegion);
							}
							else if (pdef->name()->value() == "file") {
								GetDefText(ActionFile, pdef, filename);
								NewCampaignAction.Message = FString(ActionFile);
							}
							else if (pdef->name()->value() == "image") {
								GetDefText(ActionImage, pdef, filename);
								NewCampaignAction.Image = FString(ActionImage);
							}
							else if (pdef->name()->value() == "audio") {
								GetDefText(ActionAudio, pdef, filename);
								NewCampaignAction.Audio = FString(ActionAudio);
							}
							else if (pdef->name()->value() == "date") {
								GetDefText(ActionDate, pdef, filename);
								NewCampaignAction.Date = FString(ActionDate);
							}
							else if (pdef->name()->value() == "scene") {
								GetDefText(ActionScene, pdef, filename);
								NewCampaignAction.Scene = FString(ActionScene);
							}
							else if (pdef->name()->value() == "text") {
								GetDefText(ActionText, pdef, filename);
								NewCampaignAction.Title = FString(ActionText);
							}
							else if (pdef->name()->value() == "asset_id") {
								GetDefNumber(AssetId, pdef, filename);
								NewCampaignAction.AssetId = AssetId;
							}
							else if (pdef->name()->value() == "target_id") {
								GetDefNumber(TargetId, pdef, filename);
								NewCampaignAction.TargetId = TargetId;
							}

							else if (pdef->name()->value() == "target_iff") {
								GetDefNumber(TargetIff, pdef, filename);
								NewCampaignAction.TargetIff = TargetIff;
							}
							else if (pdef->name()->value() == "asset_kill") {
								GetDefText(AssetKill, pdef, filename);
								NewCampaignAction.AssetKill = FString(AssetKill);
							}

							else if (pdef->name()->value() == "target_kill") {
								GetDefText(TargetKill, pdef, filename);
								NewCampaignAction.TargetKill = FString(TargetKill);
							}
							else if (pdef->name()->value() == "req") {
								TermStruct* val2 = pdef->term()->isStruct();
								CampaignActionReqArray.Empty();
								Action = 0;
								ActionStatus = ECOMBATACTION_STATUS::COMPLETE;
								NotAction = false;

								Combatant1 = "";
								Combatant2 = "";

								comp = 0;
								score = 0;
								intel = 0;
								gtype = 0;
								gid = 0;

								FS_CampaignReq NewCampaignReq;
								ECOMBATACTION_STATUS AStatus = ECOMBATACTION_STATUS::UNKNOWN;
								for (int index = 0; index < val2->elements()->size(); index++) {
									TermDef* pdef2 = val2->elements()->at(index)->isDef();

									if (pdef2) {
										if (pdef2->name()->value() == "action") {
											GetDefNumber(Action, pdef2, filename);
											NewCampaignReq.Action = Action;
										}
										else if (pdef2->name()->value() == "status") {
											Text Buf;
											GetDefText(Buf, pdef2, filename);

											ECOMBATACTION_STATUS AcStatus = ECOMBATACTION_STATUS::UNKNOWN;
											if (FStringToEnum<ECOMBATACTION_STATUS>(FString(ANSI_TO_TCHAR(Buf)).ToUpper(), AcStatus, false))
											{
												UE_LOG(LogTemp, Log, TEXT("Converted to enum: %d"), (int32)AcStatus);
											}
											else
											{
												UE_LOG(LogTemp, Warning, TEXT("Invalid enum string: %s"), ANSI_TO_TCHAR(Buf));
											}
											NewCampaignReq.Status = AcStatus;
										}
										else if (pdef2->name()->value() == "not") {
											GetDefBool(NotAction, pdef2, filename);
											NewCampaignReq.NotAction = NotAction;
										}

										else if (pdef2->name()->value() == "c1") {
											GetDefText(Combatant1, pdef2, filename);
											NewCampaignReq.Combatant1 = FString(Combatant1);
											//c1 = GetCombatant(txt);
										}
										else if (pdef2->name()->value() == "c2") {
											GetDefText(Combatant2, pdef2, filename);
											NewCampaignReq.Combatant2 = FString(Combatant2);
											//c2 = GetCombatant(txt);
										}
										else if (pdef2->name()->value() == "comp") {
											char txt[64];
											GetDefText(txt, pdef2, filename);
											//comp = CombatActionReq::CompFromName(txt);
											NewCampaignReq.Comp = comp;

										}
										else if (pdef2->name()->value() == "score") {
											GetDefNumber(score, pdef2, filename);
											NewCampaignReq.Score = score;

										}
										else if (pdef2->name()->value() == "intel") {
											if (pdef2->term()->isNumber()) {
												GetDefNumber(intel, pdef2, filename);
												NewCampaignReq.Intel = intel;

											}
											else if (pdef2->term()->isText()) {
												char txt[64];
												GetDefText(txt, pdef2, filename);
												intel = Intel::IntelFromName(txt);
												NewCampaignReq.Intel = intel;

											}
										}
										else if (pdef2->name()->value() == "group_type") {
											char type_name[64];
											GetDefText(type_name, pdef2, filename);
											//gtype = CombatGroup::TypeFromName(type_name);
											NewCampaignReq.GroupType = gtype;

										}
										else if (pdef2->name()->value() == "group_id") {
											GetDefNumber(gid, pdef2, filename);
											NewCampaignReq.GroupId = gid;
										}
									}
								}
								CampaignActionReqArray.Add(NewCampaignReq);
							}
						}
						NewCampaignAction.Requirement = CampaignActionReqArray;
						CampaignActionReqArray.Empty();
						CampaignActionArray.Add(NewCampaignAction);
					}
					NewCampaignData.Action = CampaignActionArray;
				}

				else if (def->name()->value() == "combatant") {

					TermStruct* CombatantTerm = def->term()->isStruct();

					NewCampaignData.CombatantSize = CombatantTerm->elements()->size();

					if (NewCampaignData.CombatantSize > 0)
					{
						// Add Unit Stuff Here

						CombatantName = "";
						EEMPIRE_NAME EName = EEMPIRE_NAME::Unknown;
						CombatantSize = 0;
						NewCombatUnit.Group.Empty();

						for (int UnitIdx = 0; UnitIdx < NewCampaignData.CombatantSize; UnitIdx++)
						{
							def = CombatantTerm->elements()->at(UnitIdx)->isDef();

							if (def->name()->value() == "name") {
								GetDefText(CombatantName, def, filename);

								if (FStringToEnum<EEMPIRE_NAME>(FString(CombatantName).ToUpper(), EName, false))
								{
									UE_LOG(LogTemp, Log, TEXT("Converted to enum: %d"), static_cast<int32>(EName));
								}
								else
								{
									UE_LOG(LogTemp, Warning, TEXT("Invalid enum string"));
								}

								NewCombatUnit.Name = EName;
							}
							else if (def->name()->value() == "size") {
								GetDefNumber(CombatantSize, def, filename);
								NewCombatUnit.Size = CombatantSize;
							}
							else if (def->name()->value() == "group") {
								//ParseGroup(def->term()->isStruct(), filename);
								TermStruct* GroupTerm = def->term()->isStruct();

								CombatantType = "";
								ECOMBATGROUP_TYPE EType = ECOMBATGROUP_TYPE::NONE;

								CombatantId = 0;

								for (int i = 0; i < GroupTerm->elements()->size(); i++) {

									TermDef* pdef = GroupTerm->elements()->at(i)->isDef();

									if (pdef->name()->value() == ("type"))
									{
										GetDefText(CombatantType, pdef, filename);
										if (FStringToEnum<ECOMBATGROUP_TYPE>(FString(CombatantType).ToUpper(), EType, false))
										{
											UE_LOG(LogTemp, Log, TEXT("Converted to enum: %d"), static_cast<int32>(EType));
										}
										else
										{
											UE_LOG(LogTemp, Warning, TEXT("Invalid enum string"));
										}

										NewGroupUnit.Type = EType;
									}

									else if (pdef->name()->value() == "id") {
										GetDefNumber(CombatantId, pdef, filename);
										NewGroupUnit.Id = CombatantId;
										UE_LOG(LogTemp, Log, TEXT("%s: %d"), *FString(pdef->name()->value()), CombatantId);
									}
									else if (pdef->name()->value() == "req") {

										TermStruct* val2 = pdef->term()->isStruct();
									}
								}
								NewCombatUnit.Group.Add(NewGroupUnit);
							}
						}
						CombatantArray.Add(NewCombatUnit);
					}
					NewCampaignData.Combatant = CombatantArray;
				}
			}
		}

	} while (term);

	LoadZones(CampaignPath);
	LoadMissionList(CampaignPath);
	LoadTemplateList(CampaignPath);
	LoadMission(CampaignPath);
	LoadTemplateMission(CampaignPath);
	LoadScriptedMission(CampaignPath);

	NewCampaignData.Zone = ZoneArray;
	NewCampaignData.MissionList = MissionListArray;
	NewCampaignData.TemplateList = TemplateListArray;
	NewCampaignData.Missions = MissionArray;
	NewCampaignData.TemplateMissions = TemplateMissionArray;
	NewCampaignData.ScriptedMissions = ScriptedMissionArray;
	NewCampaignData.bAvailable = true;
	NewCampaignData.bCompleted = false;

	// define our data table struct
	FName RowName = FName(FString(name));
	SSWInstance->CampaignDataTable->AddRow(RowName, NewCampaignData);
	CampaignData = NewCampaignData;
}

void UStarshatterGameDataSubsystem::LoadZones(FString Path)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadZones()"));

	FString FileName = Path;
	FileName.Append(TEXT("zones.def"));

	UE_LOG(LogTemp, Log, TEXT("Loading Campaign Zone: %s"), *FileName);

	if (!FPaths::FileExists(FileName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Zones file not found: %s"), *FileName);
		return;
	}

	// Keep legacy char* for GetDef* helpers:
	const char* fn = TCHAR_TO_ANSI(*FileName);

	// ------------------------------------------------------------
	// REPLACEMENT FOR DataLoader::LoadBuffer
	// ------------------------------------------------------------
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *FileName))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read zones file: %s"), *FileName);
		return;
	}
	Bytes.Add(0); // null-terminate for BlockReader/Parser

	Parser parser(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* term = parser.ParseTerm();

	ZoneArray.Empty();

	if (!term)
	{
		return;
	}
	else
	{
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "ZONES")
		{
			UE_LOG(LogTemp, Log, TEXT("Invalid Zone File %s"), *FileName);
			delete term;
			return;
		}
	}

	do
	{
		delete term;
		term = parser.ParseTerm();

		if (term)
		{
			TermDef* def = term->isDef();
			if (def && def->name()->value() == "zone")
			{
				if (!def->term() || !def->term()->isStruct())
				{
					UE_LOG(LogTemp, Log, TEXT("WARNING: zone struct missing in '%s'"), *FileName);
				}
				else
				{
					TermStruct* val = def->term()->isStruct();

					FS_CampaignZone NewCampaignZone;

					for (int i = 0; i < val->elements()->size(); i++)
					{
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef)
						{
							if (pdef->name()->value() == "region")
							{
								GetDefText(ZoneRegion, pdef, fn);
								NewCampaignZone.Region = FString(ZoneRegion);
							}
							else if (pdef->name()->value() == "system")
							{
								GetDefText(ZoneSystem, pdef, fn);
								NewCampaignZone.System = FString(ZoneSystem);
							}
						}
					}

					ZoneArray.Add(NewCampaignZone);
				}
			}
		}

	} while (term);

	delete term; // defensive
}

// +--------------------------------------------------------------------+

void
UStarshatterGameDataSubsystem::LoadMissionList(FString Path)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadMissionList()"));
	MissionListArray.Empty();

	FString FileName = Path;
	FileName.Append(TEXT("Missions.def"));

	UE_LOG(LogTemp, Log, TEXT("Loading Mission List : %s"), *FileName);

	if (!FPaths::FileExists(FileName))
	{
		UE_LOG(LogTemp, Log, TEXT("Mission List does not exist"));
		return;
	}

	// Keep legacy fn for GetDef* helpers:
	const char* fn = TCHAR_TO_ANSI(*FileName);

	// ------------------------------------------------------------
	// REPLACEMENT FOR DataLoader::LoadBuffer
	// ------------------------------------------------------------
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *FileName))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read mission list file: %s"), *FileName);
		return;
	}
	Bytes.Add(0); // null terminate for BlockReader

	Parser parser(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* term = parser.ParseTerm();

	if (!term)
	{
		return;
	}
	else
	{
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "MISSIONLIST")
		{
			UE_LOG(LogTemp, Log, TEXT("WARNING: invalid mission list file '%s'"), *FileName);
			delete term;
			return;
		}
	}

	do
	{
		delete term; term = nullptr;
		term = parser.ParseTerm();

		if (term)
		{
			TermDef* def = term->isDef();
			if (def && def->name()->value() == "mission")
			{
				if (!def->term() || !def->term()->isStruct())
				{
					UE_LOG(LogTemp, Log, TEXT("WARNING: mission struct missing in '%s'"), *FileName);
				}
				else
				{
					TermStruct* val = def->term()->isStruct();

					FS_CampaignMissionList NewMissionList;

					int   MissionId = 0;
					Text  MLName;
					Text  Desc;
					Text  Script;
					Text  Sitrep;
					Text  Objective;
					Text  System = "Unknown";
					Text  Region = "Unknown";
					Text  TypeName = "";
					Text  MissionImage = "";
					Text  MissionAudio = "";
					Text  Start = "";
					int   Type = 0;

					for (int i = 0; i < val->elements()->size(); i++)
					{
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (!pdef) continue;

						if (pdef->name()->value() == "id")
						{
							GetDefNumber(MissionId, pdef, fn);
							NewMissionList.Id = MissionId;
						}
						else if (pdef->name()->value() == "name")
						{
							GetDefText(MLName, pdef, fn);
							NewMissionList.Name = FString(MLName);
						}
						else if (pdef->name()->value() == "desc")
						{
							GetDefText(Desc, pdef, fn);
							NewMissionList.Description = FString(Desc);
						}
						else if (pdef->name()->value() == "start")
						{
							GetDefText(Start, pdef, fn);
							NewMissionList.Start = FString(Start);
						}
						else if (pdef->name()->value() == "system")
						{
							GetDefText(System, pdef, fn);
							NewMissionList.System = FString(System);
						}
						else if (pdef->name()->value() == "region")
						{
							GetDefText(Region, pdef, fn);
							NewMissionList.Region = FString(Region);
						}
						else if (pdef->name()->value() == "objective")
						{
							GetDefText(Objective, pdef, fn);
							NewMissionList.Objective = FString(Objective);
						}
						else if (pdef->name()->value() == "image")
						{
							GetDefText(MissionImage, pdef, fn);
							NewMissionList.MissionImage = FString(MissionImage);
						}
						else if (pdef->name()->value() == "audio")
						{
							GetDefText(MissionAudio, pdef, fn);
							NewMissionList.MissionAudio = FString(MissionAudio);
						}
						else if (pdef->name()->value() == "sitrep")
						{
							GetDefText(Sitrep, pdef, fn);
							FString SitrepText = FString(Sitrep);
							SitrepText = SitrepText.Replace(TEXT("\n"), TEXT("\\n"));
							NewMissionList.Sitrep = SitrepText;
						}
						else if (pdef->name()->value() == "script")
						{
							GetDefText(Script, pdef, fn);
							NewMissionList.Script = FString(Script);
						}
						else if (pdef->name()->value() == "type")
						{
							GetDefText(TypeName, pdef, fn);
							//Type = Mission::TypeFromName(typestr);
							NewMissionList.TypeName = FString(TypeName);
						}

						// Preserve legacy defaults (set repeatedly in your original loop):
						NewMissionList.Available = true;
						NewMissionList.Complete = false;
						NewMissionList.Status = EMISSIONSTATUS::Available;
					}

					MissionListArray.Add(NewMissionList);
				}
			}
		}

	} while (term);

	delete term; // defensive
	// No Loader->ReleaseBuffer(block) needed (Bytes owns memory)
}

// +--------------------------------------------------------------------+

void
UStarshatterGameDataSubsystem::LoadTemplateList(FString Path)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadTemplateList()"));

	TemplateListArray.Empty();

	FString FileName = Path;
	FileName.Append(TEXT("Templates.def"));

	UE_LOG(LogTemp, Log, TEXT("Loading Template List : %s"), *FileName);

	if (!FPaths::FileExists(FileName))
	{
		UE_LOG(LogTemp, Log, TEXT("Template List does not exist"));
		return;
	}

	// Keep legacy fn for GetDef* helpers:
	const char* fn = TCHAR_TO_ANSI(*FileName);

	// ------------------------------------------------------------
	// REPLACEMENT FOR DataLoader::LoadBuffer
	// ------------------------------------------------------------
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *FileName))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read template list file: %s"), *FileName);
		return;
	}
	Bytes.Add(0); // null terminate for BlockReader

	Parser parser(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* term = parser.ParseTerm();

	if (!term)
	{
		return;
	}
	else
	{
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "TEMPLATELIST")
		{
			UE_LOG(LogTemp, Log, TEXT("WARNING: invalid template list file '%s'"), *FileName);
			delete term;
			return;
		}
	}

	do
	{
		delete term;
		term = parser.ParseTerm();

		if (term)
		{
			TermDef* def = term->isDef();
			if (def && def->name()->value() == "mission")
			{
				if (!def->term() || !def->term()->isStruct())
				{
					UE_LOG(LogTemp, Log, TEXT("WARNING: mission struct missing in '%s'"), *FileName);
				}
				else
				{
					TermStruct* val = def->term()->isStruct();

					FS_CampaignTemplateList NewTemplateList;

					Text  TLName = "";
					Text  Script = "";
					Text  Region = "";
					int   id = 0;
					int   msn_type = 0;
					int   grp_type = 0;

					int   min_rank = 0;
					int   max_rank = 0;
					int   action_id = 0;
					int   action_status = 0;
					int   exec_once = 0;
					int   start_before = Game::TIME_NEVER;
					int   start_after = 0;

					for (int i = 0; i < val->elements()->size(); i++)
					{
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (!pdef) continue;

						if (pdef->name()->value() == "id")
						{
							GetDefNumber(id, pdef, fn);
							NewTemplateList.Id = id;
						}
						else if (pdef->name()->value() == "name")
						{
							GetDefText(TLName, pdef, fn);
							NewTemplateList.Name = FString(TLName);
						}
						else if (pdef->name()->value() == "script")
						{
							GetDefText(Script, pdef, fn);
							NewTemplateList.Script = FString(Script);
						}
						else if (pdef->name()->value() == "rgn" || pdef->name()->value() == "region")
						{
							GetDefText(Region, pdef, fn);
							NewTemplateList.Region = FString(Region);
						}
						else if (pdef->name()->value() == "type")
						{
							char typestr[64];
							GetDefText(typestr, pdef, fn);
							msn_type = Mission::TypeFromName(typestr);
							NewTemplateList.MissionType = msn_type;
						}
						else if (pdef->name()->value() == "group")
						{
							char typestr[64];
							GetDefText(typestr, pdef, fn);
							//grp_type = CombatGroup::TypeFromName(typestr);
							NewTemplateList.GroupType = grp_type;
						}
						else if (pdef->name()->value() == "min_rank")
						{
							GetDefNumber(min_rank, pdef, fn);
							NewTemplateList.MinRank = min_rank;
						}
						else if (pdef->name()->value() == "max_rank")
						{
							GetDefNumber(max_rank, pdef, fn);
							NewTemplateList.MaxRank = max_rank;
						}
						else if (pdef->name()->value() == "action_id")
						{
							GetDefNumber(action_id, pdef, fn);
							NewTemplateList.ActionId = action_id;
						}
						else if (pdef->name()->value() == "action_status")
						{
							GetDefNumber(action_status, pdef, fn);
							NewTemplateList.ActionStatus = action_status;
						}
						else if (pdef->name()->value() == "exec_once")
						{
							GetDefNumber(exec_once, pdef, fn);
							NewTemplateList.ExecOnce = exec_once;
						}
						else if (pdef->name()->value().contains("before"))
						{
							if (pdef->term() && pdef->term()->isNumber())
							{
								GetDefNumber(start_before, pdef, fn);
								NewTemplateList.StartBefore = start_before;
							}
							else
							{
								GetDefTime(start_before, pdef, fn);
								start_before -= Game::ONE_DAY;
								NewTemplateList.StartBefore = start_before;
							}
						}
						else if (pdef->name()->value().contains("after"))
						{
							if (pdef->term() && pdef->term()->isNumber())
							{
								GetDefNumber(start_after, pdef, fn);
								NewTemplateList.StartAfter = start_after;
							}
							else
							{
								GetDefTime(start_after, pdef, fn);
								start_after -= Game::ONE_DAY;
								NewTemplateList.StartAfter = start_after;
							}
						}
					}

					TemplateListArray.Add(NewTemplateList);
				}
			}
		}

	} while (term);

	delete term; // defensive
	// No Loader->ReleaseBuffer(block) needed (Bytes owns memory)
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::LoadMission(FString Path)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadMission()"));

	// Use the passed-in Path (not CampaignPath), but keep legacy behavior if you intended CampaignPath:
	const FString ScenesDir = Path / TEXT("Scenes/");

	TArray<FString> Files;
	Files.Empty();

	const FString Pattern = ScenesDir / TEXT("*.def");
	FFileManagerGeneric::Get().FindFiles(Files, *Pattern, true, false);

	for (const FString& LeafName : Files)
	{
		const FString FullPath = ScenesDir / LeafName;

		// Keep legacy ParseMission signature:
		const char* fn = TCHAR_TO_ANSI(*FullPath);
		ParseMission(fn);
	}
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::LoadTemplateMission(FString Name)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadTemplateMission()"));

	// NOTE: Keeping legacy behavior: ignores Name and uses CampaignPath/Templates/.
	const FString TemplatesDir = CampaignPath / TEXT("Templates/");

	TArray<FString> Files;
	Files.Empty();

	const FString Pattern = TemplatesDir / TEXT("*.def");
	FFileManagerGeneric::Get().FindFiles(Files, *Pattern, true, false);

	for (const FString& LeafName : Files)
	{
		const FString FullPath = TemplatesDir / LeafName;

		// Keep legacy ParseMissionTemplate signature:
		const char* fn = TCHAR_TO_ANSI(*FullPath);
		ParseMissionTemplate(fn);
	}
}


// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::LoadScriptedMission(FString Name)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadScriptedMission()"));

	// Legacy behavior: ignore Name, use CampaignPath/Scripts/
	const FString ScriptsDir = CampaignPath / TEXT("Scripts/");

	TArray<FString> Files;
	Files.Empty();

	const FString Pattern = ScriptsDir / TEXT("*.def");
	FFileManagerGeneric::Get().FindFiles(Files, *Pattern, true, false);

	for (const FString& LeafName : Files)
	{
		const FString FullPath = ScriptsDir / LeafName;

		// Preserve legacy parser interface
		const char* fn = TCHAR_TO_ANSI(*FullPath);
		ParseScriptedTemplate(fn);
	}
}


// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseMission(const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseMission()"));

	if (!fn || !*fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseMission called with null/empty filename"));
		return;
	}

	// Use a local name that cannot collide with a class member:
	const FString MissionPath = ANSI_TO_TCHAR(fn);

	MissionElementArray.Empty();
	MissionEventArray.Empty();

	if (!FPaths::FileExists(MissionPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("MISSION file not found: %s"), *MissionPath);
		return;
	}

	// Read raw bytes (preserves legacy parser expectations)
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *MissionPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read MISSION file: %s"), *MissionPath);
		return;
	}

	// Null-terminate for BlockReader / legacy parsing
	Bytes.Add(0);

	Parser parser(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* term = parser.ParseTerm();

	if (!term)
	{
		UE_LOG(LogTemp, Warning, TEXT("WARNING: could not parse MISSION file '%s'"), *MissionPath);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MISSION file '%s'"), *MissionPath);

	FS_CampaignMission NewMission;

	int     id = 0;

	Text    Region = "";
	Text    Scene = "";
	Text    System = "";
	Text    Subtitles = "";
	Text    Name = "";
	Text    Desc = "";
	Text    TargetName = "";
	Text    WardName = "";
	Text    Objective = "";
	Text    Sitrep = "";

	int     Type = 0;
	int     Team = 0;
	Text    Start = ""; // time

	double  Stardate = 0.0;
	bool    Degrees = false;

	do
	{
		delete term;
		term = parser.ParseTerm();

		if (!term)
			break;

		TermDef* def = term->isDef();
		if (!def)
			continue;

		const Text& Key = def->name()->value();

		if (Key == "name")
		{
			GetDefText(Name, def, fn);
			NewMission.Name = FString(Name);
			UE_LOG(LogTemp, Log, TEXT("mission name '%s'"), *FString(Name));
		}
		else if (Key == "scene")
		{
			GetDefText(Scene, def, fn);
			NewMission.Scene = FString(Scene);
		}
		else if (Key == "desc")
		{
			GetDefText(Desc, def, fn);
			if (Desc.length() > 0 && Desc.length() < 32)
				NewMission.Desc = FString(Desc);
		}
		else if (Key == "type")
		{
			char typestr[64] = { 0 };
			GetDefText(typestr, def, fn);
			Type = Mission::TypeFromName(typestr);
			NewMission.Type = Type;
		}
		else if (Key == "system")
		{
			GetDefText(System, def, fn);
			NewMission.System = FString(System);
		}
		else if (Key == "region")
		{
			GetDefText(Region, def, fn);
			NewMission.Region = FString(Region);
		}
		else if (Key == "degrees")
		{
			GetDefBool(Degrees, def, fn);
			NewMission.Degrees = Degrees;
		}
		else if (Key == "objective")
		{
			GetDefText(Objective, def, fn);
			if (Objective.length() > 0 && Objective.length() < 32)
				NewMission.Objective = FString(Objective);
		}
		else if (Key == "sitrep")
		{
			GetDefText(Sitrep, def, fn);
			if (Sitrep.length() > 0 && Sitrep.length() < 32)
				NewMission.Sitrep = FString(Sitrep);
		}
		else if (Key == "subtitles")
		{
			GetDefText(Subtitles, def, fn);
			NewMission.Subtitles = FString(Subtitles);
		}
		else if (Key == "start")
		{
			GetDefText(Start, def, fn);
			NewMission.StartTime = FString(Start);
		}
		else if (Key == "stardate")
		{
			GetDefNumber(Stardate, def, fn);
			NewMission.Stardate = Stardate;
		}
		else if (Key == "team")
		{
			GetDefNumber(Team, def, fn);
			NewMission.Team = Team;
		}
		else if (Key == "target")
		{
			GetDefText(TargetName, def, fn);
			NewMission.TargetName = FString(TargetName);
		}
		else if (Key == "ward")
		{
			GetDefText(WardName, def, fn);
			NewMission.WardName = FString(WardName);
		}
		else if (Key == "event")
		{
			if (def->term() && def->term()->isStruct())
			{
				ParseEvent(def->term()->isStruct(), fn);
				NewMission.Event = MissionEventArray;
			}
		}
		else if (Key == "element" || Key == "ship" || Key == "station")
		{
			if (def->term() && def->term()->isStruct())
			{
				ParseElement(def->term()->isStruct(), fn);
				NewMission.Element = MissionElementArray;
			}
		}

	} while (term);

	if (term)
	{
		delete term;
		term = nullptr;
	}

	MissionArray.Add(NewMission);
}


// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseNavpoint(TermStruct* Val, const char* Fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseNavpoint()"));

	if (!Val || !Fn || !*Fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseNavpoint called with null args"));
		return;
	}

	// ---- legacy defaults ----
	int32 Formation = 0;
	int32 Speed = 0;
	int32 Priority = 1;
	int32 Farcast = 0;
	int32 Hold = 0;
	int32 EMCON = 0;

	FVector Location = FVector::ZeroVector;

	Text OrderName = "";
	Text StatusName = "";
	Text RegionName = "";
	Text TargetName = "";
	Text TargetDesc = "";

	FS_MissionInstruction NewInstr;

	// Legacy behavior: rlocs are per-navpoint
	MissionRLocArray.Empty();

	const int32 ElemCount = (int32)Val->elements()->size();
	for (int32 i = 0; i < ElemCount; ++i)
	{
		TermDef* PDef = Val->elements()->at(i)->isDef();
		if (!PDef)
			continue;

		const Text& Key = PDef->name()->value();

		if (Key == "cmd")
		{
			GetDefText(OrderName, PDef, Fn);
			NewInstr.OrderName = FString(OrderName);
		}
		else if (Key == "status")
		{
			GetDefText(StatusName, PDef, Fn);
			NewInstr.StatusName = FString(StatusName);
		}
		else if (Key == "loc")
		{
			// Use FVector only
			FVector V;
			GetDefVec(V, PDef, Fn);
			NewInstr.Location = V;
		}
		else if (Key == "rloc")
		{
			if (PDef->term() && PDef->term()->isStruct())
			{
				ParseRLoc(PDef->term()->isStruct(), Fn);
			}
		}
		else if (Key == "rgn")
		{
			GetDefText(RegionName, PDef, Fn);
			NewInstr.OrderRegionName = FString(RegionName);
		}
		else if (Key == "speed")
		{
			GetDefNumber(Speed, PDef, Fn);
			NewInstr.Speed = Speed;
		}
		else if (Key == "formation")
		{
			GetDefNumber(Formation, PDef, Fn);
			NewInstr.Formation = Formation;
		}
		else if (Key == "emcon")
		{
			GetDefNumber(EMCON, PDef, Fn);
			NewInstr.EMCON = EMCON;
		}
		else if (Key == "priority")
		{
			GetDefNumber(Priority, PDef, Fn);
			NewInstr.Priority = Priority;
		}
		else if (Key == "farcast")
		{
			// Legacy allows farcast as bool or number
			if (PDef->term() && PDef->term()->isBool())
			{
				bool b = false;
				GetDefBool(b, PDef, Fn);
				Farcast = b ? 1 : 0;
			}
			else
			{
				GetDefNumber(Farcast, PDef, Fn);
			}

			NewInstr.Farcast = Farcast;
		}
		else if (Key == "tgt")
		{
			GetDefText(TargetName, PDef, Fn);
			NewInstr.TargetName = FString(TargetName);
		}
		else if (Key == "tgt_desc")
		{
			GetDefText(TargetDesc, PDef, Fn);
			NewInstr.TargetDesc = FString(TargetDesc);
		}
		else
		{
			// legacy: hold, hold1, hold2, etc.
			const char* KeyC = Key; // Text ? const char* (Starshatter-style)
			if (KeyC && !strncmp(KeyC, "hold", 4))
			{
				GetDefNumber(Hold, PDef, Fn);
				NewInstr.Hold = Hold;
			}
		}
	}

	// Assign accumulated rlocs once
	NewInstr.RLoc = MissionRLocArray;

	MissionNavpointArray.Add(NewInstr);
}


// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseObjective(TermStruct* Val, const char* Fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseObjective()"));

	if (!Val || !Fn || !*Fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseObjective called with null args"));
		return;
	}

	// ---- legacy defaults ----
	int32 Formation = 0;
	int32 Speed = 0;
	int32 Priority = 1;
	int32 Farcast = 0;
	int32 Hold = 0;
	int32 EMCON = 0;

	Text OrderName = "";
	Text StatusName = "";
	Text RegionName = "";
	Text TargetName = "";
	Text TargetDesc = "";

	FS_MissionInstruction NewObj;

	// Legacy behavior: rlocs are per-objective; clear before parsing this objective.
	// NOTE: Assumes ParseRLoc appends into MissionRLocArray (member scratch).
	MissionRLocArray.Empty();

	const int32 ElemCount = (int32)Val->elements()->size();
	for (int32 i = 0; i < ElemCount; ++i)
	{
		TermDef* PDef = Val->elements()->at(i)->isDef();
		if (!PDef)
			continue;

		const Text& Key = PDef->name()->value();

		if (Key == "cmd")
		{
			GetDefText(OrderName, PDef, Fn);
			NewObj.OrderName = FString(OrderName);
		}
		else if (Key == "status")
		{
			GetDefText(StatusName, PDef, Fn);
			NewObj.StatusName = FString(StatusName);
		}
		else if (Key == "loc")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, PDef, Fn);
			NewObj.Location = V;
		}
		else if (Key == "rloc")
		{
			if (PDef->term() && PDef->term()->isStruct())
			{
				ParseRLoc(PDef->term()->isStruct(), Fn);
			}
		}
		else if (Key == "rgn")
		{
			GetDefText(RegionName, PDef, Fn);
			NewObj.OrderRegionName = FString(RegionName);
		}
		else if (Key == "speed")
		{
			GetDefNumber(Speed, PDef, Fn);
			NewObj.Speed = Speed;
		}
		else if (Key == "formation")
		{
			GetDefNumber(Formation, PDef, Fn);
			NewObj.Formation = Formation;
		}
		else if (Key == "emcon")
		{
			GetDefNumber(EMCON, PDef, Fn);
			NewObj.EMCON = EMCON;
		}
		else if (Key == "priority")
		{
			GetDefNumber(Priority, PDef, Fn);
			NewObj.Priority = Priority;
		}
		else if (Key == "farcast")
		{
			// Legacy allows farcast as bool or number
			if (PDef->term() && PDef->term()->isBool())
			{
				bool b = false;
				GetDefBool(b, PDef, Fn);
				Farcast = b ? 1 : 0;
			}
			else
			{
				GetDefNumber(Farcast, PDef, Fn);
			}

			NewObj.Farcast = Farcast;
		}
		else if (Key == "tgt")
		{
			GetDefText(TargetName, PDef, Fn);
			NewObj.TargetName = FString(TargetName);
		}
		else if (Key == "tgt_desc")
		{
			GetDefText(TargetDesc, PDef, Fn);
			NewObj.TargetDesc = FString(TargetDesc);
		}
		else
		{
			// legacy: hold, hold1, hold2, etc.
			const char* KeyC = Key; // Text -> const char* (Starshatter style)
			if (KeyC && !strncmp(KeyC, "hold", 4))
			{
				GetDefNumber(Hold, PDef, Fn);
				NewObj.Hold = Hold;
			}
		}
	}

	// Assign accumulated rlocs once
	NewObj.RLoc = MissionRLocArray;

	MissionObjectiveArray.Add(NewObj);
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseInstruction(TermStruct* Val, const char* Fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseInstruction()"));

	if (!Val || !Fn || !*Fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseInstruction called with null args"));
		return;
	}

	// ---- legacy defaults ----
	int32 Formation = 0;
	int32 Speed = 0;
	int32 Priority = 1;
	int32 Farcast = 0;
	int32 Hold = 0;
	int32 EMCON = 0;

	Text OrderName = "";
	Text StatusName = "";
	Text RegionName = "";
	Text TargetName = "";
	Text TargetDesc = "";

	FS_MissionInstruction NewInstr;

	// Legacy behavior: rlocs are per-instruction; clear before parsing this instruction.
	// NOTE: Assumes ParseRLoc appends into MissionRLocArray (member scratch).
	MissionRLocArray.Empty();

	const int32 ElemCount = (int32)Val->elements()->size();
	for (int32 i = 0; i < ElemCount; ++i)
	{
		TermDef* PDef = Val->elements()->at(i)->isDef();
		if (!PDef)
			continue;

		const Text& Key = PDef->name()->value();

		if (Key == "cmd")
		{
			GetDefText(OrderName, PDef, Fn);
			NewInstr.OrderName = FString(OrderName);
		}
		else if (Key == "status")
		{
			GetDefText(StatusName, PDef, Fn);
			NewInstr.StatusName = FString(StatusName);
		}
		else if (Key == "loc")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, PDef, Fn);
			NewInstr.Location = V;
		}
		else if (Key == "rloc")
		{
			if (PDef->term() && PDef->term()->isStruct())
			{
				ParseRLoc(PDef->term()->isStruct(), Fn);
			}
		}
		else if (Key == "rgn")
		{
			GetDefText(RegionName, PDef, Fn);
			NewInstr.OrderRegionName = FString(RegionName);
		}
		else if (Key == "speed")
		{
			GetDefNumber(Speed, PDef, Fn);
			NewInstr.Speed = Speed;
		}
		else if (Key == "formation")
		{
			GetDefNumber(Formation, PDef, Fn);
			NewInstr.Formation = Formation;
		}
		else if (Key == "emcon")
		{
			GetDefNumber(EMCON, PDef, Fn);
			NewInstr.EMCON = EMCON;
		}
		else if (Key == "priority")
		{
			GetDefNumber(Priority, PDef, Fn);
			NewInstr.Priority = Priority;
		}
		else if (Key == "farcast")
		{
			// Legacy allows farcast as bool or number
			if (PDef->term() && PDef->term()->isBool())
			{
				bool b = false;
				GetDefBool(b, PDef, Fn);
				Farcast = b ? 1 : 0;
			}
			else
			{
				GetDefNumber(Farcast, PDef, Fn);
			}

			NewInstr.Farcast = Farcast;
		}
		else if (Key == "tgt")
		{
			GetDefText(TargetName, PDef, Fn);
			NewInstr.TargetName = FString(TargetName);
		}
		else if (Key == "tgt_desc")
		{
			GetDefText(TargetDesc, PDef, Fn);
			NewInstr.TargetDesc = FString(TargetDesc);
		}
		else
		{
			// legacy: hold, hold1, hold2, etc.
			const char* KeyC = Key; // Text -> const char* (Starshatter style)
			if (KeyC && !strncmp(KeyC, "hold", 4))
			{
				GetDefNumber(Hold, PDef, Fn);
				NewInstr.Hold = Hold;
			}
		}
	}

	// Assign accumulated rlocs once
	NewInstr.RLoc = MissionRLocArray;

	MissionInstructionArray.Add(NewInstr);
}


// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseShip(TermStruct* Val, const char* Fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseShip()"));

	if (!Val || !Fn || !*Fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseShip called with null args"));
		return;
	}

	// --- strings ---
	Text ShipName = "";
	Text SkinName = "";
	Text RegNum = "";
	Text Region = "";

	// --- scalars ---
	int32  Respawns = -1;
	double Heading = -1.0e9;
	double Integrity = -1.0;

	// --- arrays (legacy) ---
	int32 Ammo[16];
	int32 Fuel[4];

	for (int32 i = 0; i < 16; ++i) Ammo[i] = -10;
	for (int32 i = 0; i < 4; ++i) Fuel[i] = -10;

	FS_MissionShip NewMissionShip;

	const int32 ElemCount = (int32)Val->elements()->size();
	for (int32 ElemIdx = 0; ElemIdx < ElemCount; ++ElemIdx)   // <-- renamed from Index
	{
		TermDef* PDef = Val->elements()->at(ElemIdx)->isDef();
		if (!PDef)
			continue;

		const Text& Key = PDef->name()->value();

		if (Key == "name")
		{
			GetDefText(ShipName, PDef, Fn);
			NewMissionShip.ShipName = FString(ShipName);
		}
		else if (Key == "skin")
		{
			GetDefText(SkinName, PDef, Fn);
			NewMissionShip.SkinName = FString(SkinName);
		}
		else if (Key == "regnum")
		{
			GetDefText(RegNum, PDef, Fn);
			NewMissionShip.RegNum = FString(RegNum);
		}
		else if (Key == "region")
		{
			GetDefText(Region, PDef, Fn);
			NewMissionShip.Region = FString(Region);
		}
		else if (Key == "loc")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, PDef, Fn);
			NewMissionShip.Location = V;
		}
		else if (Key == "velocity")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, PDef, Fn);
			NewMissionShip.Velocity = V;
		}
		else if (Key == "respawns")
		{
			GetDefNumber(Respawns, PDef, Fn);
			NewMissionShip.Respawns = Respawns;
		}
		else if (Key == "heading")
		{
			GetDefNumber(Heading, PDef, Fn);
			NewMissionShip.Heading = Heading;
		}
		else if (Key == "integrity")
		{
			GetDefNumber(Integrity, PDef, Fn);
			NewMissionShip.Integrity = Integrity;
		}
		else if (Key == "ammo")
		{
			GetDefArray(Ammo, 16, PDef, Fn);

			// If FS_MissionShip::Ammo is TArray<int32>
			NewMissionShip.Ammo.SetNum(16);
			for (int32 i = 0; i < 16; ++i)
			{
				NewMissionShip.Ammo[i] = Ammo[i];
			}
		}
		else if (Key == "fuel")
		{
			GetDefArray(Fuel, 4, PDef, Fn);

			// If FS_MissionShip::Fuel is TArray<int32>
			NewMissionShip.Fuel.SetNum(4);
			for (int32 i = 0; i < 4; ++i)
			{
				NewMissionShip.Fuel[i] = Fuel[i];
			}
		}
	}

	MissionShipArray.Add(NewMissionShip);
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseLoadout(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseLoadout()"));

	if (!val || !fn || !*fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseLoadout called with null args"));
		return;
	}

	int  ship = -1;
	int  stations[16];
	Text LoadoutName;

	for (int i = 0; i < 16; i++)
		stations[i] = -1;

	FS_MissionLoadout NewLoadout;

	for (int i = 0; i < val->elements()->size(); i++)
	{
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (!pdef)
			continue;

		const Text& Key = pdef->name()->value();

		if (Key == "ship")
		{
			GetDefNumber(ship, pdef, fn);
			NewLoadout.Ship = ship;
		}
		else if (Key == "name")
		{
			GetDefText(LoadoutName, pdef, fn);
			NewLoadout.LoadoutName = FString(LoadoutName);
		}
		else if (Key == "stations")
		{
			GetDefArray(stations, 16, pdef, fn);

			// If FS_MissionLoadout::Stations is a TArray<int32>:
			NewLoadout.Stations.SetNum(16);
			for (int index = 0; index < 16; index++)
			{
				NewLoadout.Stations[index] = stations[index];
			}

			// If FS_MissionLoadout::Stations is a fixed C array: int32 Stations[16];
			// for (int index = 0; index < 16; index++)
			// {
			//     NewLoadout.Stations[index] = stations[index];
			// }
		}
	}

	MissionLoadoutArray.Add(NewLoadout);
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseEvent(TermStruct* Val, const char* Fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseEvent()"));

	if (!Val || !Fn || !*Fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseEvent called with null args"));
		return;
	}

	// ---- strings ----
	Text EventType = "";
	Text TriggerName = "";
	Text EventShip = "";
	Text EventSource = "";
	Text EventTarget = "";
	Text EventMessage = "";
	Text EventSound = "";
	Text TriggerShip = "";
	Text TriggerTarget = "";

	// ---- scalars ----
	int32  EventId = 1;
	int32  EventChance = 0;
	int32  EventDelay = 0;
	double EventTime = 0.0;

	// ---- FVector only ----
	FVector EventPoint = FVector::ZeroVector;

	// ---- rect ----
	Rect EventRect{}; // legacy rect (x,y,w,h)

	// ---- params ----
	int32 EventParam[10];
	int32 TriggerParam[10];
	int32 EventNParams = 0;
	int32 TriggerNParams = 0;

	for (int32 k = 0; k < 10; ++k)
	{
		EventParam[k] = 0;
		TriggerParam[k] = 0;
	}

	FS_MissionEvent NewMissionEvent;

	const int32 ElemCount = (int32)Val->elements()->size();
	for (int32 i = 0; i < ElemCount; ++i)
	{
		TermDef* PDef = Val->elements()->at(i)->isDef();
		if (!PDef)
			continue;

		const Text& Key = PDef->name()->value();

		if (Key == "type")
		{
			GetDefText(EventType, PDef, Fn);
			NewMissionEvent.EventType = FString(EventType);
		}
		else if (Key == "trigger")
		{
			GetDefText(TriggerName, PDef, Fn);
			NewMissionEvent.TriggerName = FString(TriggerName);
		}
		else if (Key == "id")
		{
			GetDefNumber(EventId, PDef, Fn);
			NewMissionEvent.EventId = EventId;
		}
		else if (Key == "time")
		{
			GetDefNumber(EventTime, PDef, Fn);
			NewMissionEvent.EventTime = EventTime;
		}
		else if (Key == "delay")
		{
			GetDefNumber(EventDelay, PDef, Fn);
			NewMissionEvent.EventDelay = EventDelay;
		}
		else if (Key == "event_param" || Key == "param" || Key == "color")
		{
			if (PDef->term() && PDef->term()->isNumber())
			{
				GetDefNumber(EventParam[0], PDef, Fn);
				EventNParams = 1;

				NewMissionEvent.EventParam[0] = EventParam[0];
				NewMissionEvent.EventNParams = EventNParams;
			}
			else if (PDef->term() && PDef->term()->isArray())
			{
				std::vector<float> PList;
				GetDefArray(PList, PDef, Fn);

				EventNParams = 0;
				for (int32 idx = 0; idx < 10 && idx < (int32)PList.size(); ++idx)
				{
					EventParam[idx] = (int32)PList[idx];
					NewMissionEvent.EventParam[idx] = EventParam[idx];
					EventNParams = idx + 1;
				}

				NewMissionEvent.EventNParams = EventNParams;
			}
		}
		else if (Key == "trigger_param")
		{
			if (PDef->term() && PDef->term()->isNumber())
			{
				GetDefNumber(TriggerParam[0], PDef, Fn);
				TriggerNParams = 1;

				NewMissionEvent.TriggerParam[0] = TriggerParam[0];
				NewMissionEvent.TriggerNParams = TriggerNParams; // correct
			}
			else if (PDef->term() && PDef->term()->isArray())
			{
				std::vector<float> PList;
				GetDefArray(PList, PDef, Fn);

				TriggerNParams = 0;
				for (int32 ti = 0; ti < 10 && ti < (int32)PList.size(); ++ti)
				{
					TriggerParam[ti] = (int32)PList[ti];
					NewMissionEvent.TriggerParam[ti] = TriggerParam[ti];
					TriggerNParams = ti + 1;
				}

				NewMissionEvent.TriggerNParams = TriggerNParams;
			}
		}
		else if (Key == "event_ship" || Key == "ship")
		{
			GetDefText(EventShip, PDef, Fn);
			NewMissionEvent.EventShip = FString(EventShip);
		}
		else if (Key == "event_source" || Key == "source" || Key == "font")
		{
			GetDefText(EventSource, PDef, Fn);
			NewMissionEvent.EventSource = FString(EventSource);
		}
		else if (Key == "event_target" || Key == "target" || Key == "image")
		{
			GetDefText(EventTarget, PDef, Fn);
			NewMissionEvent.EventTarget = FString(EventTarget);
		}
		else if (Key == "event_message" || Key == "message")
		{
			GetDefText(EventMessage, PDef, Fn);
			NewMissionEvent.EventMessage = FString(EventMessage);
		}
		else if (Key == "event_chance" || Key == "chance")
		{
			GetDefNumber(EventChance, PDef, Fn);
			NewMissionEvent.EventChance = EventChance;
		}
		else if (Key == "event_sound" || Key == "sound")
		{
			GetDefText(EventSound, PDef, Fn);
			NewMissionEvent.EventSound = FString(EventSound);
		}
		else if (Key == "loc" || Key == "vec" || Key == "fade")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, PDef, Fn);
			NewMissionEvent.EventPoint = V;
		}
		else if (Key == "rect")
		{
			GetDefRect(EventRect, PDef, Fn);

			// Store rect as XYWH (x,y,w,h) in FVector4:
			NewMissionEvent.EventRect = FVector4(
				(float)EventRect.x,
				(float)EventRect.y,
				(float)EventRect.w,
				(float)EventRect.h
			);

			// Alternative if your FS_MissionEvent uses FIntRect:
			// NewMissionEvent.EventRect = FIntRect(EventRect.x, EventRect.y, EventRect.x + EventRect.w, EventRect.y + EventRect.h);
		}
		else if (Key == "trigger_ship")
		{
			GetDefText(TriggerShip, PDef, Fn);
			NewMissionEvent.TriggerShip = FString(TriggerShip);
		}
		else if (Key == "trigger_target")
		{
			GetDefText(TriggerTarget, PDef, Fn);
			NewMissionEvent.TriggerTarget = FString(TriggerTarget);
		}
	}

	MissionEventArray.Add(NewMissionEvent);
}


// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseElement(TermStruct* Eval, const char* Fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseElement()"));

	if (!Eval || !Fn || !*Fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseElement called with null args"));
		return;
	}

	// ---- strings ----
	Text ElementName = "";
	Text Carrier = "";
	Text Commander = "";
	Text Squadron = "";
	Text Path = "";
	Text Design = "";
	Text SkinName = "";
	Text RoleName = "";
	Text RegionName = "";
	Text Instr = "";
	Text ElementIntel = "";

	// ---- FVector only ----
	FVector ElementLoc = FVector::ZeroVector;

	// ---- ints/bools ----
	int32 Deck = 1;  // (not used below but preserved if you add it later)
	int32 IFFCode = 0;
	int32 Count = 1;
	int32 MaintCount = 0;
	int32 DeadCount = 0;
	int32 Player = 0;
	int32 CommandAI = 0;
	int32 Respawns = 0;
	int32 HoldTime = 0;
	int32 ZoneLock = 0;
	int32 Heading = 0;

	bool bAlert = false;
	bool bPlayable = false;
	bool bRogue = false;
	bool bInvulnerable = false;

	// Scratch arrays used by nested parsers:
	MissionLoadoutArray.Empty();
	MissionRLocArray.Empty();
	MissionInstructionArray.Empty();
	MissionNavpointArray.Empty();
	MissionShipArray.Empty();

	FS_MissionElement NewMissionElement;

	const int32 ElemCount = (int32)Eval->elements()->size();
	for (int32 i = 0; i < ElemCount; ++i)
	{
		TermDef* PDef = Eval->elements()->at(i)->isDef();
		if (!PDef)
			continue;

		const Text& Key = PDef->name()->value();

		if (Key == "name")
		{
			GetDefText(ElementName, PDef, Fn);
			NewMissionElement.Name = FString(ElementName);
		}
		else if (Key == "carrier")
		{
			GetDefText(Carrier, PDef, Fn);
			NewMissionElement.Carrier = FString(Carrier);
		}
		else if (Key == "commander")
		{
			GetDefText(Commander, PDef, Fn);
			NewMissionElement.Commander = FString(Commander);
		}
		else if (Key == "squadron")
		{
			GetDefText(Squadron, PDef, Fn);
			NewMissionElement.Squadron = FString(Squadron);
		}
		else if (Key == "path")
		{
			GetDefText(Path, PDef, Fn);
			NewMissionElement.Path = FString(Path);
		}
		else if (Key == "design")
		{
			GetDefText(Design, PDef, Fn);
			NewMissionElement.Design = FString(Design);
		}
		else if (Key == "skin")
		{
			GetDefText(SkinName, PDef, Fn);
			NewMissionElement.SkinName = FString(SkinName);
		}
		else if (Key == "mission")
		{
			GetDefText(RoleName, PDef, Fn);
			NewMissionElement.RoleName = FString(RoleName);
		}
		else if (Key == "intel")
		{
			// FIX: read into ElementIntel (not RoleName)
			GetDefText(ElementIntel, PDef, Fn);
			NewMissionElement.Intel = FString(ElementIntel);
		}
		else if (Key == "loc")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, PDef, Fn);
			NewMissionElement.Location = V;
		}
		else if (Key == "rloc")
		{
			// FIX: rloc belongs to ParseRLoc, not ParseLoadout
			if (PDef->term() && PDef->term()->isStruct())
			{
				ParseRLoc(PDef->term()->isStruct(), Fn);
			}
		}
		else if (Key == "head")
		{
			GetDefNumber(Heading, PDef, Fn);
			NewMissionElement.Heading = Heading;
		}
		else if (Key == "region" || Key == "rgn")
		{
			GetDefText(RegionName, PDef, Fn);
			NewMissionElement.RegionName = FString(RegionName);
		}
		else if (Key == "iff")
		{
			GetDefNumber(IFFCode, PDef, Fn);
			NewMissionElement.IFFCode = IFFCode;
		}
		else if (Key == "count")
		{
			GetDefNumber(Count, PDef, Fn);
			NewMissionElement.Count = Count;
		}
		else if (Key == "maint_count")
		{
			GetDefNumber(MaintCount, PDef, Fn);
			NewMissionElement.MaintCount = MaintCount;
		}
		else if (Key == "dead_count")
		{
			GetDefNumber(DeadCount, PDef, Fn);
			NewMissionElement.DeadCount = DeadCount;
		}
		else if (Key == "player")
		{
			GetDefNumber(Player, PDef, Fn);
			NewMissionElement.Player = Player;
		}
		else if (Key == "alert")
		{
			GetDefBool(bAlert, PDef, Fn);
			NewMissionElement.Alert = bAlert;
		}
		else if (Key == "playable")
		{
			GetDefBool(bPlayable, PDef, Fn);
			NewMissionElement.Playable = bPlayable;
		}
		else if (Key == "rogue")
		{
			GetDefBool(bRogue, PDef, Fn);
			NewMissionElement.Rogue = bRogue;
		}
		else if (Key == "invulnerable")
		{
			GetDefBool(bInvulnerable, PDef, Fn);
			NewMissionElement.Invulnerable = bInvulnerable;
		}
		else if (Key == "command_ai")
		{
			GetDefNumber(CommandAI, PDef, Fn);
			NewMissionElement.CommandAI = CommandAI;
		}
		else if (Key == "respawn")
		{
			GetDefNumber(Respawns, PDef, Fn);
			NewMissionElement.Respawns = Respawns;
		}
		else if (Key == "hold")
		{
			GetDefNumber(HoldTime, PDef, Fn);
			NewMissionElement.HoldTime = HoldTime;
		}
		else if (Key == "zone")
		{
			GetDefNumber(ZoneLock, PDef, Fn);
			NewMissionElement.ZoneLock = ZoneLock;
		}
		else if (Key == "objective")
		{
			if (!PDef->term() || !PDef->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("Mission error - No objective in '%s'"), *FString(Fn));
			}
			else
			{
				// Objective is an instruction-style struct in your pipeline:
				ParseInstruction(PDef->term()->isStruct(), Fn);
			}
		}
		else if (Key == "instr")
		{
			GetDefText(Instr, PDef, Fn);
			NewMissionElement.Instr = FString(Instr);
		}
		else if (Key == "ship")
		{
			if (!PDef->term() || !PDef->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("Mission error - no ship in '%s'"), *FString(Fn));
			}
			else
			{
				ParseShip(PDef->term()->isStruct(), Fn);
			}
		}
		else if (Key == "order" || Key == "navpt")
		{
			if (!PDef->term() || !PDef->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("Mission error - no navpt in '%s'"), *FString(Fn));
			}
			else
			{
				ParseNavpoint(PDef->term()->isStruct(), Fn);
			}
		}
		else if (Key == "loadout")
		{
			if (!PDef->term() || !PDef->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("Mission error - no loadout in '%s'"), *FString(Fn));
			}
			else
			{
				ParseLoadout(PDef->term()->isStruct(), Fn);
			}
		}
	}

	// Assign accumulated nested arrays once (supports multiple blocks cleanly)
	NewMissionElement.Loadout = MissionLoadoutArray;
	NewMissionElement.RLoc = MissionRLocArray;
	NewMissionElement.Instruction = MissionInstructionArray;
	NewMissionElement.Navpoint = MissionNavpointArray;
	NewMissionElement.Ship = MissionShipArray;

	MissionElementArray.Add(NewMissionElement);
}


// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseScriptedTemplate(const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseScriptedTemplate()"));

	if (!fn || !*fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseScriptedTemplate called with null/empty filename"));
		return;
	}

	// Don't name this FilePath (it collides with your class member):
	const FString ScriptPath = ANSI_TO_TCHAR(fn);

	if (!FPaths::FileExists(ScriptPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Scripted template file not found: %s"), *ScriptPath);
		return;
	}

	// ---- UE-native raw bytes load (preserves legacy parser expectations) ----
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *ScriptPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read scripted template file: %s"), *ScriptPath);
		return;
	}

	// Null-terminate for BlockReader/legacy char* parsing:
	Bytes.Add(0);

	Parser parser(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* term = parser.ParseTerm();

	// Reset scratch arrays for this template parse:
	MissionElementArray.Empty();
	MissionEventArray.Empty();
	MissionCallsignArray.Empty();
	MissionOptionalArray.Empty();
	MissionAliasArray.Empty();

	if (!term)
	{
		UE_LOG(LogTemp, Warning, TEXT("WARNING: could not parse '%s'"), *ScriptPath);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MISSIONTEMPLATE file '%s'"), *ScriptPath);

	// ----------------------------
	// Template-level locals
	// ----------------------------
	Text  TargetName = "";
	Text  WardName = "";
	Text  TemplateName = "";
	Text  TemplateSystem = "";
	Text  TemplateRegion = "";
	Text  TemplateObjective = "";
	Text  TemplateSitrep = "";
	Text  TemplateStart = "";

	int   TemplateType = 0;
	int   TemplateTeam = 0;
	bool  TemplateDegrees = false;

	FS_TemplateMission NewTemplateMission;

	do
	{
		delete term;
		term = parser.ParseTerm();

		if (!term)
			break;

		TermDef* def = term->isDef();
		if (!def)
			continue;

		const Text& Key = def->name()->value();

		if (Key == "name")
		{
			GetDefText(TemplateName, def, fn);
			NewTemplateMission.TemplateName = FString(TemplateName);
		}
		else if (Key == "type")
		{
			char typestr[64] = { 0 };
			GetDefText(typestr, def, fn);
			TemplateType = Mission::TypeFromName(typestr);
			NewTemplateMission.TemplateType = TemplateType;
		}
		else if (Key == "system")
		{
			GetDefText(TemplateSystem, def, fn);
			NewTemplateMission.TemplateSystem = FString(TemplateSystem);
		}
		else if (Key == "degrees")
		{
			GetDefBool(TemplateDegrees, def, fn);
			NewTemplateMission.TemplateDegrees = TemplateDegrees;
		}
		else if (Key == "region")
		{
			GetDefText(TemplateRegion, def, fn);
			NewTemplateMission.TemplateRegion = FString(TemplateRegion);
		}
		else if (Key == "objective")
		{
			GetDefText(TemplateObjective, def, fn);
			NewTemplateMission.TemplateObjective = FString(TemplateObjective);
		}
		else if (Key == "sitrep")
		{
			GetDefText(TemplateSitrep, def, fn);
			NewTemplateMission.TemplateSitrep = FString(TemplateSitrep);
		}
		else if (Key == "start")
		{
			GetDefText(TemplateStart, def, fn);
			NewTemplateMission.TemplateStart = FString(TemplateStart);
		}
		else if (Key == "team")
		{
			GetDefNumber(TemplateTeam, def, fn);
			NewTemplateMission.TemplateTeam = TemplateTeam;
		}
		else if (Key == "target")
		{
			GetDefText(TargetName, def, fn);
			NewTemplateMission.TargetName = FString(TargetName);
		}
		else if (Key == "ward")
		{
			GetDefText(WardName, def, fn);
			NewTemplateMission.WardName = FString(WardName);
		}
		else if (Key == "alias")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: alias struct missing in '%s'"), *ScriptPath);
			}
			else
			{
				ParseAlias(def->term()->isStruct(), fn);
				NewTemplateMission.Alias = MissionAliasArray;
			}
		}
		else if (Key == "callsign")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: callsign struct missing in '%s'"), *ScriptPath);
			}
			else
			{
				ParseCallsign(def->term()->isStruct(), fn);
				NewTemplateMission.Callsign = MissionCallsignArray;
			}
		}
		else if (Key == "optional")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: optional struct missing in '%s'"), *ScriptPath);
			}
			else
			{
				ParseOptional(def->term()->isStruct(), fn);
				NewTemplateMission.Optional = MissionOptionalArray;
			}
		}
		else if (Key == "element")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: element struct missing in '%s'"), *ScriptPath);
			}
			else
			{
				ParseElement(def->term()->isStruct(), fn);
				NewTemplateMission.Element = MissionElementArray;
			}
		}
		else if (Key == "event")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: event struct missing in '%s'"), *ScriptPath);
			}
			else
			{
				ParseEvent(def->term()->isStruct(), fn);
				NewTemplateMission.Event = MissionEventArray;
			}
		}

	} while (term);

	if (term)
	{
		delete term;
		term = nullptr;
	}

	ScriptedMissionArray.Add(NewTemplateMission);
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseMissionTemplate(const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseMissionTemplate()"));

	if (!fn || !*fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseMissionTemplate called with null/empty filename"));
		return;
	}

	// Don't name this FilePath (it collides with your class member):
	const FString TemplatePath = ANSI_TO_TCHAR(fn);

	if (!FPaths::FileExists(TemplatePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Mission template file not found: %s"), *TemplatePath);
		return;
	}

	// ---------------------------------------------------------
	// UE-native raw bytes load (preserves legacy parser behavior)
	// ---------------------------------------------------------
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *TemplatePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read mission template file: %s"), *TemplatePath);
		return;
	}

	// Null-terminate for BlockReader/legacy char* parser:
	Bytes.Add(0);

	Parser parser(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* term = parser.ParseTerm();

	// Reset scratch arrays for this template parse:
	MissionElementArray.Empty();
	MissionEventArray.Empty();
	MissionCallsignArray.Empty();
	MissionOptionalArray.Empty();
	MissionAliasArray.Empty();

	if (!term)
	{
		UE_LOG(LogTemp, Warning, TEXT("WARNING: could not parse '%s'"), *TemplatePath);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MISSIONTEMPLATE file '%s'"), *TemplatePath);

	Text TargetName = "";
	Text WardName = "";
	Text TemplateName = "";
	Text TemplateSystem = "";
	Text TemplateRegion = "";
	Text TemplateObjective = "";
	Text TemplateSitrep = "";
	Text TemplateStart = "";

	int  TemplateType = 0;
	int  TemplateTeam = 0;
	bool TemplateDegrees = false;

	FS_TemplateMission NewTemplateMission;

	do
	{
		delete term;
		term = parser.ParseTerm();

		if (!term)
			break;

		TermDef* def = term->isDef();
		if (!def)
			continue;

		const Text& Key = def->name()->value();

		if (Key == "name")
		{
			GetDefText(TemplateName, def, fn);
			NewTemplateMission.TemplateName = FString(TemplateName);
		}
		else if (Key == "type")
		{
			char typestr[64] = { 0 };
			GetDefText(typestr, def, fn);
			TemplateType = Mission::TypeFromName(typestr);
			NewTemplateMission.TemplateType = TemplateType;
		}
		else if (Key == "system")
		{
			GetDefText(TemplateSystem, def, fn);
			NewTemplateMission.TemplateSystem = FString(TemplateSystem);
		}
		else if (Key == "degrees")
		{
			GetDefBool(TemplateDegrees, def, fn);
			NewTemplateMission.TemplateDegrees = TemplateDegrees;
		}
		else if (Key == "region")
		{
			GetDefText(TemplateRegion, def, fn);
			NewTemplateMission.TemplateRegion = FString(TemplateRegion);
		}
		else if (Key == "objective")
		{
			GetDefText(TemplateObjective, def, fn);
			NewTemplateMission.TemplateObjective = FString(TemplateObjective);
		}
		else if (Key == "sitrep")
		{
			GetDefText(TemplateSitrep, def, fn);
			NewTemplateMission.TemplateSitrep = FString(TemplateSitrep);
		}
		else if (Key == "start")
		{
			GetDefText(TemplateStart, def, fn);
			NewTemplateMission.TemplateStart = FString(TemplateStart);
		}
		else if (Key == "team")
		{
			GetDefNumber(TemplateTeam, def, fn);
			NewTemplateMission.TemplateTeam = TemplateTeam;
		}
		else if (Key == "target")
		{
			GetDefText(TargetName, def, fn);
			NewTemplateMission.TargetName = FString(TargetName);
		}
		else if (Key == "ward")
		{
			GetDefText(WardName, def, fn);
			NewTemplateMission.WardName = FString(WardName);
		}
		else if (Key == "alias")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: alias struct missing in '%s'"), *TemplatePath);
			}
			else
			{
				ParseAlias(def->term()->isStruct(), fn);
				NewTemplateMission.Alias = MissionAliasArray;
			}
		}
		else if (Key == "callsign")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: callsign struct missing in '%s'"), *TemplatePath);
			}
			else
			{
				ParseCallsign(def->term()->isStruct(), fn);
				NewTemplateMission.Callsign = MissionCallsignArray;
			}
		}
		else if (Key == "optional")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: optional struct missing in '%s'"), *TemplatePath);
			}
			else
			{
				ParseOptional(def->term()->isStruct(), fn);
				NewTemplateMission.Optional = MissionOptionalArray;
			}
		}
		else if (Key == "element")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: element struct missing in '%s'"), *TemplatePath);
			}
			else
			{
				ParseElement(def->term()->isStruct(), fn);
				NewTemplateMission.Element = MissionElementArray;
			}
		}
		else if (Key == "event")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: event struct missing in '%s'"), *TemplatePath);
			}
			else
			{
				ParseEvent(def->term()->isStruct(), fn);
				NewTemplateMission.Event = MissionEventArray;
			}
		}

	} while (term);

	if (term)
	{
		delete term;
		term = nullptr;
	}

	TemplateMissionArray.Add(NewTemplateMission);
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseAlias(TermStruct* Val, const char* Fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseAlias()"));

	if (!Val || !Fn || !*Fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseAlias called with null args"));
		return;
	}

	// ---- strings ----
	Text AliasName = "";
	Text Design = "";
	Text Code = "";
	Text ElementName = "";
	Text Mission = "";

	// ---- scalars ----
	int32 Iff = -1;
	int32 Player = 0;

	// ---- FVector only ----
	bool    bUseLocation = false;
	FVector Location = FVector::ZeroVector;

	// Scratch arrays (nested parsers append into these):
	MissionRLocArray.Empty();
	MissionNavpointArray.Empty();
	MissionObjectiveArray.Empty();

	FS_MissionAlias NewMissionAlias;

	const int32 ElemCount = (int32)Val->elements()->size();
	for (int32 i = 0; i < ElemCount; ++i)
	{
		TermDef* PDef = Val->elements()->at(i)->isDef();
		if (!PDef)
			continue;

		const Text& Key = PDef->name()->value();

		if (Key == "name")
		{
			GetDefText(AliasName, PDef, Fn);
			NewMissionAlias.AliasName = FString(AliasName);
		}
		else if (Key == "elem")
		{
			GetDefText(ElementName, PDef, Fn);
			NewMissionAlias.ElementName = FString(ElementName);
		}
		else if (Key == "code")
		{
			GetDefText(Code, PDef, Fn);
			NewMissionAlias.Code = FString(Code);
		}
		else if (Key == "design")
		{
			GetDefText(Design, PDef, Fn);
			NewMissionAlias.Design = FString(Design);
		}
		else if (Key == "mission")
		{
			GetDefText(Mission, PDef, Fn);
			NewMissionAlias.Mission = FString(Mission);
		}
		else if (Key == "iff")
		{
			GetDefNumber(Iff, PDef, Fn);
			NewMissionAlias.Iff = Iff;
		}
		else if (Key == "loc")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, PDef, Fn);

			Location = V;
			bUseLocation = true;
		}
		else if (Key == "rloc")
		{
			if (PDef->term() && PDef->term()->isStruct())
			{
				ParseRLoc(PDef->term()->isStruct(), Fn);
			}
		}
		else if (Key == "player")
		{
			GetDefNumber(Player, PDef, Fn);

			// Always record Player
			NewMissionAlias.Player = Player;

			// Legacy behavior: if player is set and code is missing, force "player"
			if (Player && !Code.length())
			{
				Code = "player";
				NewMissionAlias.Code = FString(Code);
			}
		}
		else if (Key == "navpt")
		{
			if (PDef->term() && PDef->term()->isStruct())
			{
				ParseNavpoint(PDef->term()->isStruct(), Fn);
			}
		}
		else if (Key == "objective")
		{
			if (PDef->term() && PDef->term()->isStruct())
			{
				ParseObjective(PDef->term()->isStruct(), Fn);
			}
		}
	}

	// Assign accumulated nested arrays once
	NewMissionAlias.Location = Location;
	NewMissionAlias.UseLocation = bUseLocation;
	NewMissionAlias.RLoc = MissionRLocArray;
	NewMissionAlias.Navpoint = MissionNavpointArray;
	NewMissionAlias.Objective = MissionObjectiveArray;

	MissionAliasArray.Add(NewMissionAlias);
}


// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseRLoc(TermStruct* RVal, const char* Fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseRLoc()"));

	if (!RVal || !Fn || !*Fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseRLoc called with null args"));
		return;
	}

	// ---- FVector only ----
	FVector BaseLocation = FVector::ZeroVector;

	// ---- strings ----
	Text Reference = "";

	// ---- doubles ----
	double Dex = 0.0;
	double DexVar = 5e3;
	double Az = 0.0;
	double AzVar = PI;
	double El = 0.0;
	double ElVar = 0.1;

	FS_RLoc NewRLocElement;

	const int32 ElemCount = (int32)RVal->elements()->size();
	for (int32 ElemIdx = 0; ElemIdx < ElemCount; ++ElemIdx)   // <-- FIX
	{
		TermDef* RDef = RVal->elements()->at(ElemIdx)->isDef();
		if (!RDef)
			continue;

		const Text& Key = RDef->name()->value();

		if (Key == "dex")
		{
			GetDefNumber(Dex, RDef, Fn);
			NewRLocElement.Dex = Dex;
		}
		else if (Key == "dex_var")
		{
			GetDefNumber(DexVar, RDef, Fn);
			NewRLocElement.DexVar = DexVar;
		}
		else if (Key == "az")
		{
			GetDefNumber(Az, RDef, Fn);
			NewRLocElement.Azimuth = Az;
		}
		else if (Key == "az_var")
		{
			GetDefNumber(AzVar, RDef, Fn);
			NewRLocElement.AzimuthVar = AzVar;
		}
		else if (Key == "el")
		{
			GetDefNumber(El, RDef, Fn);
			NewRLocElement.Elevation = El;
		}
		else if (Key == "el_var")
		{
			GetDefNumber(ElVar, RDef, Fn);
			NewRLocElement.ElevationVar = ElVar;
		}
		else if (Key == "loc")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, RDef, Fn);

			BaseLocation = V;
			NewRLocElement.BaseLocation = BaseLocation;
		}
		else if (Key == "ref")
		{
			GetDefText(Reference, RDef, Fn);
			NewRLocElement.Reference = FString(Reference);
		}
	}

	MissionRLocArray.Add(NewRLocElement);
}


// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseCallsign(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseCallsign()"));

	if (!val || !fn || !*fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseCallsign called with null args"));
		return;
	}

	// Legacy-safe initialization
	Text CallsignName = "";
	int  CallsignIff = -1;

	FS_MissionCallsign NewMissionCallsign;

	for (int i = 0; i < val->elements()->size(); i++)
	{
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (!pdef)
			continue;

		const Text& Key = pdef->name()->value();

		if (Key == "name")
		{
			GetDefText(CallsignName, pdef, fn);
			NewMissionCallsign.Callsign = FString(CallsignName);
		}
		else if (Key == "iff")
		{
			GetDefNumber(CallsignIff, pdef, fn);
			NewMissionCallsign.Iff = CallsignIff;
		}
	}

	MissionCallsignArray.Add(NewMissionCallsign);
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseOptional(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseOptional()"));

	if (!val || !fn || !*fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseOptional called with invalid args"));
		return;
	}

	int Min = 0;
	int Max = 1000;

	// Reset scratch arrays used by nested parsing
	MissionElementArray.Empty();

	FS_MissionOptional NewOptional;
	NewOptional.Min = Min;
	NewOptional.Max = Max;

	for (int i = 0; i < val->elements()->size(); i++)
	{
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (!pdef)
			continue;

		const Text& Key = pdef->name()->value();

		if (Key == "min")
		{
			GetDefNumber(Min, pdef, fn);
			NewOptional.Min = Min;
		}
		else if (Key == "max")
		{
			GetDefNumber(Max, pdef, fn);
			NewOptional.Max = Max;
		}
		else if (Key == "element")
		{
			if (pdef->term() && pdef->term()->isStruct())
			{
				// Each call appends to MissionElementArray
				ParseElement(pdef->term()->isStruct(), fn);
			}
			else
			{
				UE_LOG(LogTemp, Warning,
					TEXT("Optional element missing struct in '%s'"),
					*FString(fn));
			}
		}
	}

	// Finalize
	NewOptional.Element = MissionElementArray;
	NewOptional.Total = MissionElementArray.Num();

	MissionOptionalArray.Add(NewOptional);
}



// +--------------------------------------------------------------------+

// ------------------------------------------------------------
// FIXED: LoadGalaxyMap
// - UE-native file load (no DataLoader / no ReleaseBuffer)
// - Correct per-system reset (system scratch arrays cleared once per system)
// - Removes undefined fn/filename usage; uses a single Fn derived from FileName
// - Uses your ParseStarMap -> ParsePlanetMap -> ParseMoonMap chain
// ------------------------------------------------------------

void UStarshatterGameDataSubsystem::LoadGalaxyMap()
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadGalaxyMap()"));

	const FString Dir = FPaths::ProjectContentDir() / TEXT("GameData/Galaxy/");
	const FString FileName = Dir / TEXT("Galaxy.def");

	UE_LOG(LogTemp, Log, TEXT("Loading Galaxy: %s"), *FileName);

	if (!FPaths::FileExists(FileName))
	{
		UE_LOG(LogTemp, Error, TEXT("Galaxy file not found: %s"), *FileName);
		return;
	}

	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *FileName))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read galaxy file: %s"), *FileName);
		return;
	}
	Bytes.Add(0); // null-terminate for BlockReader

	const char* Fn = TCHAR_TO_ANSI(*FileName);

	if (!SSWInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("SSWInstance is null in LoadGalaxyMap()"));
		return;
	}

	// Ensure we actually have a data table to fill:
	if (!GalaxyDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("GalaxyDataTable is null. Assign a DT asset with RowStruct=FS_Galaxy in defaults."));
		return;
	}

	// Ensure RowStruct is correct:
	if (GalaxyDataTable->GetRowStruct() == nullptr)
	{
		GalaxyDataTable->RowStruct = FS_Galaxy::StaticStruct();
	}
	else if (GalaxyDataTable->GetRowStruct() != FS_Galaxy::StaticStruct())
	{
		UE_LOG(LogTemp, Error,
			TEXT("GalaxyDataTable RowStruct mismatch. Expected %s, got %s"),
			*FS_Galaxy::StaticStruct()->GetName(),
			*GalaxyDataTable->GetRowStruct()->GetName());
		return;
	}

	// Clear output containers:
	GalaxyDataArray.Empty();
	GalaxyDataTable->EmptyTable();

	// Scratch arrays:
	StarMapArray.Empty();
	PlanetMapArray.Empty();
	MoonMapArray.Empty();
	RegionMapArray.Empty();

	Parser parser(new BlockReader((const char*)Bytes.GetData()));
	Term* term = parser.ParseTerm();

	if (!term)
	{
		UE_LOG(LogTemp, Warning, TEXT("WARNING: could not parse '%s'"), *FileName);
		return;
	}

	TermText* file_type = term->isText();
	if (!file_type || file_type->value() != "GALAXY")
	{
		UE_LOG(LogTemp, Warning, TEXT("WARNING: invalid galaxy file '%s' (missing GALAXY header)"), *FileName);
		delete term;
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Galaxy file OK: %s"), *FileName);

	double GalaxyRadius = 0.0;
	int32 SystemsParsed = 0;

	while (true)
	{
		delete term;
		term = parser.ParseTerm();
		if (!term)
			break;

		TermDef* def = term->isDef();
		if (!def)
			continue;

		const Text& DefName = def->name()->value();

		// global radius:
		if (DefName == "radius")
		{
			GetDefNumber(GalaxyRadius, def, Fn);
			continue;
		}

		// system:
		if (DefName == "system")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: system struct missing in '%s'"), *FileName);
				continue;
			}

			// Reset per system (scratch):
			StarMapArray.Empty();
			PlanetMapArray.Empty();
			MoonMapArray.Empty();
			RegionMapArray.Empty();

			FS_Galaxy NewGalaxyData;
			NewGalaxyData.Link.Empty();

			TermStruct* sys = def->term()->isStruct();

			Text  SystemName = "";
			Text  ClassName = "";
			Text  Link = "";
			Text  StarName = "";
			FVector SystemLocation{};
			int   SystemIff = 0;
			int   EmpireId = 0;

			ESPECTRAL_CLASS StarClass = ESPECTRAL_CLASS::G;

			for (int i = 0; i < sys->elements()->size(); i++)
			{
				TermDef* pdef = sys->elements()->at(i)->isDef();
				if (!pdef) continue;

				const Text& Key = pdef->name()->value();

				if (Key == "name")
				{
					GetDefText(SystemName, pdef, Fn);
					NewGalaxyData.Name = FString(SystemName);
				}
				else if (Key == "loc")
				{
					GetDefVec(SystemLocation, pdef, Fn);
					NewGalaxyData.Location = FVector(SystemLocation.X, SystemLocation.Y, SystemLocation.Z);
				}
				else if (Key == "iff")
				{
					GetDefNumber(SystemIff, pdef, Fn);
					NewGalaxyData.Iff = SystemIff;
				}
				else if (Key == "empire")
				{
					GetDefNumber(EmpireId, pdef, Fn);
					NewGalaxyData.Empire = UFormattingUtils::GetEmpireTypeFromIndex(EmpireId);
				}
				else if (Key == "link")
				{
					GetDefText(Link, pdef, Fn);
					NewGalaxyData.Link.Add(FString(Link));
				}
				else if (Key == "star")
				{
					GetDefText(StarName, pdef, Fn);
					NewGalaxyData.Star = FString(StarName);
				}
				else if (Key == "class")
				{
					GetDefText(ClassName, pdef, Fn);

					switch (ClassName[0])
					{
					case 'B': StarClass = ESPECTRAL_CLASS::B;           break;
					case 'A': StarClass = ESPECTRAL_CLASS::A;           break;
					case 'F': StarClass = ESPECTRAL_CLASS::F;           break;
					case 'G': StarClass = ESPECTRAL_CLASS::G;           break;
					case 'K': StarClass = ESPECTRAL_CLASS::K;           break;
					case 'M': StarClass = ESPECTRAL_CLASS::M;           break;
					case 'R': StarClass = ESPECTRAL_CLASS::RED_GIANT;   break;
					case 'W': StarClass = ESPECTRAL_CLASS::WHITE_DWARF; break;
					case 'Z': StarClass = ESPECTRAL_CLASS::BLACK_HOLE;  break;
					default:  StarClass = ESPECTRAL_CLASS::G;           break;
					}
					NewGalaxyData.Class = StarClass;
				}
				else if (Key == "stellar")
				{
					if (!pdef->term() || !pdef->term()->isStruct())
					{
						UE_LOG(LogTemp, Warning, TEXT("WARNING: stellar struct missing in '%s'"), *FileName);
					}
					else
					{
						ParseStarMap(pdef->term()->isStruct(), Fn);
						NewGalaxyData.Stellar = StarMapArray;
					}
				}
			}

			// HARD VALIDATION before adding:
			if (NewGalaxyData.Name.IsEmpty())
			{
				UE_LOG(LogTemp, Error, TEXT("Parsed a system with empty name. Skipping row."));
				continue;
			}

			// Ensure unique row names (duplicates overwrite in many workflows):
			FString BaseRow = NewGalaxyData.Name;
			FString RowStr = BaseRow;
			int32 Suffix = 1;

			while (GalaxyDataTable->FindRow<FS_Galaxy>(*RowStr, TEXT("LoadGalaxyMap"), false) != nullptr)
			{
				RowStr = FString::Printf(TEXT("%s_%d"), *BaseRow, Suffix++);
			}

			const FName RowName(*RowStr);

			GalaxyDataTable->AddRow(RowName, NewGalaxyData);
			GalaxyDataArray.Add(NewGalaxyData);

			SystemsParsed++;

			UE_LOG(LogTemp, Log, TEXT("Added system row: %s | Stars=%d"),
				*RowStr, NewGalaxyData.Stellar.Num());
		}
	}

	delete term; // defensive

	UE_LOG(LogTemp, Log, TEXT("Galaxy parse complete. SystemsParsed=%d  GalaxyData.Num=%d  DataTableRows=%d"),
		SystemsParsed,
		GalaxyDataArray.Num(),
		GalaxyDataTable->GetRowMap().Num());

	UGalaxyManager::Get(this)->LoadGalaxy(GalaxyDataArray);
}


// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseStar(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseStar()"));

	if (!val || !fn || !*fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseStar called with invalid args"));
		return;
	}

	Text   StarName = "";
	Text   ImgName = "";
	Text   MapName = "";

	double Light = 0.0;
	double Radius = 0.0;
	double Rot = 0.0;
	double Mass = 0.0;
	double Orbit = 0.0;
	double Tscale = 1.0;

	bool   Retro = false;

	FS_Star NewStarData;

	// IMPORTANT: Only clear this if it is truly a per-star scratch array.
	// If you support multiple stars per system and aggregate planets elsewhere,
	// keep this as-is. Otherwise remove the Empty().
	PlanetDataArray.Empty();

	for (int i = 0; i < val->elements()->size(); i++)
	{
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (!pdef)
			continue;

		const Text& Key = pdef->name()->value();

		if (Key == "name")
		{
			GetDefText(StarName, pdef, fn);
			NewStarData.Name = FString(StarName);
		}
		else if (Key == "map" || Key == "icon")
		{
			GetDefText(MapName, pdef, fn);
			NewStarData.Map = FString(MapName);
		}
		else if (Key == "image")
		{
			GetDefText(ImgName, pdef, fn);
			NewStarData.Image = FString(ImgName);
		}
		else if (Key == "mass")
		{
			GetDefNumber(Mass, pdef, fn);
			NewStarData.Mass = Mass;
		}
		else if (Key == "orbit")
		{
			GetDefNumber(Orbit, pdef, fn);
			NewStarData.Orbit = Orbit;
		}
		else if (Key == "radius")
		{
			GetDefNumber(Radius, pdef, fn);
			NewStarData.Radius = Radius;
		}
		else if (Key == "rotation")
		{
			GetDefNumber(Rot, pdef, fn);
			NewStarData.Rot = Rot;
		}
		else if (Key == "tscale")
		{
			GetDefNumber(Tscale, pdef, fn);
			NewStarData.Tscale = Tscale;
		}
		else if (Key == "light")
		{
			GetDefNumber(Light, pdef, fn);
			NewStarData.Light = Light;
		}
		else if (Key == "retro")
		{
			GetDefBool(Retro, pdef, fn);
			NewStarData.Retro = Retro;
		}
		else if (Key == "color")
		{
			Vec3 v;
			GetDefVec(v, pdef, fn);

			// v is legacy 0..255 most of the time. Clamp and cast safely:
			const uint8 R = (uint8)FMath::Clamp((int32)FMath::RoundToInt((float)v.X), 0, 255);
			const uint8 G = (uint8)FMath::Clamp((int32)FMath::RoundToInt((float)v.Y), 0, 255);
			const uint8 B = (uint8)FMath::Clamp((int32)FMath::RoundToInt((float)v.Z), 0, 255);

			NewStarData.Color = FColor(R, G, B, 255);
		}
		else if (Key == "back" || Key == "back_color")
		{
			Vec3 v;
			GetDefVec(v, pdef, fn);

			const uint8 R = (uint8)FMath::Clamp((int32)FMath::RoundToInt((float)v.X), 0, 255);
			const uint8 G = (uint8)FMath::Clamp((int32)FMath::RoundToInt((float)v.Y), 0, 255);
			const uint8 B = (uint8)FMath::Clamp((int32)FMath::RoundToInt((float)v.Z), 0, 255);

			NewStarData.Back = FColor(R, G, B, 255);
		}
		else if (Key == "planet")
		{
			if (!pdef->term() || !pdef->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: planet struct missing in '%s'"), *FString(fn));
			}
			else
			{
				ParsePlanet(pdef->term()->isStruct(), fn);

				// ParsePlanet appends to PlanetDataArray; snapshot it into this star:
				NewStarData.Planet = PlanetDataArray;
			}
		}
	}

	StarDataArray.Add(NewStarData);
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParsePlanet(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParsePlanet()"));

	if (!val || !fn || !*fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParsePlanet called with invalid args"));
		return;
	}

	Text   PlanetName = "";
	Text   ImgName = "";
	Text   MapName = "";
	Text   HiName = "";
	Text   ImgRing = "";
	Text   GloName = "";
	Text   GloHiName = "";
	Text   GlossName = "";

	double Radius = 0.0;
	double Mass = 0.0;
	double Orbit = 0.0;
	double Rot = 0.0;
	double Minrad = 0.0;
	double Maxrad = 0.0;
	double Tscale = 1.0;
	double Tilt = 0.0;

	bool   Retro = false;
	bool   Lumin = false;

	FS_Planet NewPlanetData;

	// per-planet scratch array
	MoonDataArray.Empty();

	for (int i = 0; i < val->elements()->size(); i++)
	{
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (!pdef)
			continue;

		const Text& Key = pdef->name()->value();

		if (Key == "name")
		{
			GetDefText(PlanetName, pdef, fn);
			NewPlanetData.Name = FString(PlanetName);
		}
		else if (Key == "map" || Key == "icon")
		{
			GetDefText(MapName, pdef, fn);
			NewPlanetData.Map = FString(MapName);
		}
		else if (Key == "image" || Key == "image_west" || Key == "image_east")
		{
			GetDefText(ImgName, pdef, fn);
			NewPlanetData.Image = FString(ImgName);
		}
		else if (Key == "glow")
		{
			GetDefText(GloName, pdef, fn);
			NewPlanetData.Glow = FString(GloName);
		}
		else if (Key == "gloss")
		{
			GetDefText(GlossName, pdef, fn);
			NewPlanetData.Gloss = FString(GlossName);
		}
		else if (Key == "high_res" || Key == "high_res_west" || Key == "high_res_east")
		{
			GetDefText(HiName, pdef, fn);
			NewPlanetData.High = FString(HiName);
		}
		else if (Key == "glow_high_res")
		{
			GetDefText(GloHiName, pdef, fn);
			NewPlanetData.GlowHigh = FString(GloHiName);
		}
		else if (Key == "mass")
		{
			GetDefNumber(Mass, pdef, fn);
			NewPlanetData.Mass = Mass;
		}
		else if (Key == "orbit")
		{
			GetDefNumber(Orbit, pdef, fn);
			NewPlanetData.Orbit = Orbit;
		}
		else if (Key == "retro")
		{
			GetDefBool(Retro, pdef, fn);
			NewPlanetData.Retro = Retro;
		}
		else if (Key == "luminous")
		{
			GetDefBool(Lumin, pdef, fn);
			NewPlanetData.Lumin = Lumin;
		}
		else if (Key == "rotation")
		{
			GetDefNumber(Rot, pdef, fn);
			NewPlanetData.Rot = Rot;
		}
		else if (Key == "radius")
		{
			GetDefNumber(Radius, pdef, fn);
			NewPlanetData.Radius = Radius;
		}
		else if (Key == "ring")
		{
			GetDefText(ImgRing, pdef, fn);
			NewPlanetData.Rings = FString(ImgRing);
		}
		else if (Key == "minrad")
		{
			GetDefNumber(Minrad, pdef, fn);
			NewPlanetData.Minrad = Minrad;
		}
		else if (Key == "maxrad")
		{
			GetDefNumber(Maxrad, pdef, fn);
			NewPlanetData.Maxrad = Maxrad;
		}
		else if (Key == "tscale")
		{
			GetDefNumber(Tscale, pdef, fn);
			NewPlanetData.Tscale = Tscale;
		}
		else if (Key == "tilt")
		{
			GetDefNumber(Tilt, pdef, fn);
			NewPlanetData.Tilt = Tilt;
		}
		else if (Key == "atmosphere")
		{
			Vec3 a;
			GetDefVec(a, pdef, fn);

			// Legacy Vec3 is typically 0..255. Clamp & cast to bytes.
			const uint8 R = (uint8)FMath::Clamp((int32)FMath::RoundToInt((float)a.X), 0, 255);
			const uint8 G = (uint8)FMath::Clamp((int32)FMath::RoundToInt((float)a.Y), 0, 255);
			const uint8 B = (uint8)FMath::Clamp((int32)FMath::RoundToInt((float)a.Z), 0, 255);

			NewPlanetData.Atmos = FColor(R, G, B, 255);
		}
		else if (Key == "moon")
		{
			if (!pdef->term() || !pdef->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: moon struct missing in '%s'"), *FString(fn));
			}
			else
			{
				ParseMoon(pdef->term()->isStruct(), fn);

				// ParseMoon appends into MoonDataArray; snapshot it:
				NewPlanetData.Moon = MoonDataArray;
			}
		}
	}

	PlanetDataArray.Add(NewPlanetData);
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseMoon(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseMoon()"));

	if (!val || !fn || !*fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseMoon called with invalid args"));
		return;
	}

	Text   MapName = "";
	Text   MoonName = "";
	Text   ImgName = "";
	Text   HiName = "";
	Text   GloName = "";
	Text   GloHiName = "";
	Text   GlossName = "";

	double Radius = 0.0;
	double Mass = 0.0;
	double Orbit = 0.0;
	double Rot = 0.0;
	double Tscale = 1.0;
	double Tilt = 0.0;
	bool   Retro = false;

	FS_Moon NewMoonData;

	for (int i = 0; i < val->elements()->size(); i++)
	{
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (!pdef)
			continue;

		const Text& Key = pdef->name()->value();

		if (Key == "name")
		{
			GetDefText(MoonName, pdef, fn);
			NewMoonData.Name = FString(MoonName);
		}
		else if (Key == "map" || Key == "icon")
		{
			GetDefText(MapName, pdef, fn);
			NewMoonData.Map = FString(MapName);
		}
		else if (Key == "image")
		{
			GetDefText(ImgName, pdef, fn);
			NewMoonData.Image = FString(ImgName);
		}
		else if (Key == "glow")
		{
			GetDefText(GloName, pdef, fn);
			NewMoonData.Glow = FString(GloName);
		}
		else if (Key == "high_res")
		{
			GetDefText(HiName, pdef, fn);
			NewMoonData.High = FString(HiName);
		}
		else if (Key == "glow_high_res")
		{
			GetDefText(GloHiName, pdef, fn);
			NewMoonData.GlowHigh = FString(GloHiName);
		}
		else if (Key == "gloss")
		{
			GetDefText(GlossName, pdef, fn);
			NewMoonData.Gloss = FString(GlossName);
		}
		else if (Key == "mass")
		{
			GetDefNumber(Mass, pdef, fn);
			NewMoonData.Mass = Mass;
		}
		else if (Key == "orbit")
		{
			GetDefNumber(Orbit, pdef, fn);
			NewMoonData.Orbit = Orbit;
		}
		else if (Key == "rotation")
		{
			GetDefNumber(Rot, pdef, fn);
			NewMoonData.Rot = Rot;
		}
		else if (Key == "retro")
		{
			GetDefBool(Retro, pdef, fn);
			NewMoonData.Retro = Retro;
		}
		else if (Key == "radius")
		{
			GetDefNumber(Radius, pdef, fn);
			NewMoonData.Radius = Radius;
		}
		else if (Key == "tscale")
		{
			GetDefNumber(Tscale, pdef, fn);
			NewMoonData.Tscale = Tscale;
		}
		else if (Key == "inclination")
		{
			GetDefNumber(Tilt, pdef, fn);
			NewMoonData.Tilt = Tilt;
		}
		else if (Key == "atmosphere")
		{
			Vec3 a;
			GetDefVec(a, pdef, fn);

			// Assume legacy Vec3 is 0..255; clamp + cast.
			const uint8 R = (uint8)FMath::Clamp(FMath::RoundToInt((float)a.X), 0, 255);
			const uint8 G = (uint8)FMath::Clamp(FMath::RoundToInt((float)a.Y), 0, 255);
			const uint8 B = (uint8)FMath::Clamp(FMath::RoundToInt((float)a.Z), 0, 255);

			NewMoonData.Atmos = FColor(R, G, B, 255);
		}
	}

	MoonDataArray.Add(NewMoonData);
}


void UStarshatterGameDataSubsystem::ParseRegion(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseRegion()"));

	if (!val || !fn || !*fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseRegion called with invalid args"));
		return;
	}

	Text  RegionName = "";
	Text  RegionParent = "";
	Text  LinkName = "";
	Text  ParentType = "";

	double Size = 1.0e6;
	double Orbit = 0.0;
	double Grid = 25000;
	double Inclination = 0.0;
	int    Asteroids = 0;

	EOrbitalType ParsedType = EOrbitalType::NOTHING;

	TArray<FString> LinksName;
	LinksName.Empty();

	// Maintain legacy List usage:
	List<Text> links;

	FS_RegionMap NewRegionData;

	for (int i = 0; i < val->elements()->size(); i++)
	{
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (!pdef)
			continue;

		const Text& Key = pdef->name()->value();

		if (Key == "name")
		{
			GetDefText(RegionName, pdef, fn);
			NewRegionData.Name = FString(ANSI_TO_TCHAR(RegionName.data()));
		}
		else if (Key == "parent")
		{
			GetDefText(RegionParent, pdef, fn);
			NewRegionData.Parent = FString(ANSI_TO_TCHAR(RegionParent.data()));
		}
		else if (Key == "type")
		{
			GetDefText(ParentType, pdef, fn);

			const FString RegionNameStr = FString(ANSI_TO_TCHAR(RegionName.data()));
			const FString RawTypeStr = FString(ANSI_TO_TCHAR(ParentType.data()));

			UE_LOG(LogTemp, Warning, TEXT("%s Region type raw: '%s'"), *RegionNameStr, *RawTypeStr);

			ParsedType = EOrbitalType::NOTHING;
			const bool bOk = UFormattingUtils::GetRegionTypeFromString(*RawTypeStr, ParsedType);
			if (bOk)
			{
				NewRegionData.Type = ParsedType;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s Region type parse failed. Raw='%s'"), *RegionNameStr, *RawTypeStr);
				NewRegionData.Type = EOrbitalType::NOTHING;
			}
		}
		else if (Key == "link")
		{
			GetDefText(LinkName, pdef, fn);

			if (LinkName.length() > 0)
			{
				// Keep legacy list:
				links.append(new Text(LinkName));

				LinksName.Add(FString(ANSI_TO_TCHAR(LinkName.data())));
				NewRegionData.Link = LinksName;
			}
		}
		else if (Key == "orbit")
		{
			GetDefNumber(Orbit, pdef, fn);
			NewRegionData.Orbit = Orbit;
		}
		else if (Key == "size" || Key == "radius")
		{
			GetDefNumber(Size, pdef, fn);
			NewRegionData.Size = Size;
		}
		else if (Key == "grid")
		{
			GetDefNumber(Grid, pdef, fn);
			NewRegionData.Grid = Grid;
		}
		else if (Key == "inclination")
		{
			GetDefNumber(Inclination, pdef, fn);
			NewRegionData.Inclination = Inclination;
		}
		else if (Key == "asteroids")
		{
			GetDefNumber(Asteroids, pdef, fn);
			NewRegionData.Asteroids = Asteroids;
		}
	}

	// Add row ONCE (after parsing):
	if (RegionName.length() > 0 && RegionsDataTable)
	{
		const FName RowName(*FString(ANSI_TO_TCHAR(RegionName.data())));

		// Avoid duplicate row asserts if file repeats names:
		if (RegionsDataTable->GetRowMap().Contains(RowName))
		{
			UE_LOG(LogTemp, Warning, TEXT("RegionsDataTable already has row '%s' - skipping duplicate"), *RowName.ToString());
		}
		else
		{
			RegionsDataTable->AddRow(RowName, NewRegionData);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseRegion: missing RegionName or RegionsDataTable is null"));
	}

	RegionMapArray.Add(NewRegionData);
}

void UStarshatterGameDataSubsystem::ParseMoonMap(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseMoonMap()"));

	Text   MoonIcon = "";
	Text   MoonName = "";
	Text   MoonTexture = "";

	double Radius = 0.0;
	double Mass = 0.0;
	double Orbit = 0.0;
	double Inclination = 0.0;
	double Rot = 0.0;
	double Tscale = 1.0;
	double Tilt = 0.0;
	bool   Retro = false;

	FS_MoonMap NewMoonMap;

	// Reset per-moon:
	RegionMapArray.Empty();

	for (int i = 0; i < val->elements()->size(); i++)
	{
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (!pdef) continue;

		const Text& Key = pdef->name()->value();

		if (Key == "name")
		{
			GetDefText(MoonName, pdef, fn);
			NewMoonMap.Name = FString(MoonName);
		}
		else if (Key == "icon")
		{
			GetDefText(MoonIcon, pdef, fn);
			NewMoonMap.Icon = FString(MoonIcon);
		}
		else if (Key == "texture")
		{
			GetDefText(MoonTexture, pdef, fn);
			NewMoonMap.Texture = FString(MoonTexture);
		}
		else if (Key == "mass")
		{
			GetDefNumber(Mass, pdef, fn);
			NewMoonMap.Mass = Mass;
		}
		else if (Key == "orbit")
		{
			GetDefNumber(Orbit, pdef, fn);
			NewMoonMap.Orbit = Orbit;
		}
		else if (Key == "inclination")
		{
			GetDefNumber(Inclination, pdef, fn);
			NewMoonMap.Inclination = Inclination;
		}
		else if (Key == "rotation")
		{
			GetDefNumber(Rot, pdef, fn);
			NewMoonMap.Rot = Rot;
		}
		else if (Key == "retro")
		{
			GetDefBool(Retro, pdef, fn);
			NewMoonMap.Retro = Retro;
		}
		else if (Key == "radius")
		{
			GetDefNumber(Radius, pdef, fn);
			NewMoonMap.Radius = Radius;
		}
		else if (Key == "tscale")
		{
			GetDefNumber(Tscale, pdef, fn);
			NewMoonMap.Tscale = Tscale;
		}
		else if (Key == "tilt") // FIXED (was mistakenly "inclination" again)
		{
			GetDefNumber(Tilt, pdef, fn);
			NewMoonMap.Tilt = Tilt;
		}
		else if (Key == "atmosphere")
		{
			Vec3 a;
			GetDefVec(a, pdef, fn);
			NewMoonMap.Atmos = Vec3ToColor255(a);
		}
		else if (Key == "region")
		{
			if (!pdef->term() || !pdef->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: region struct missing in '%s'"), *FString(fn));
			}
			else
			{
				ParseRegion(pdef->term()->isStruct(), fn);
				NewMoonMap.Region = RegionMapArray;
			}
		}
	}

	MoonMapArray.Add(NewMoonMap);
}

void UStarshatterGameDataSubsystem::ParseStarMap(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseStarMap()"));

	Text  StarName = "";
	Text  SystemName = "";
	Text  ImgName = "";
	Text  MapName = "";
	Text  ClassName = "";

	double Light = 0.0;
	double Radius = 0.0;
	double Rot = 0.0;
	double Mass = 0.0;
	double Orbit = 0.0;
	double Tscale = 1.0;
	bool   Retro = false;

	ESPECTRAL_CLASS StarClass = ESPECTRAL_CLASS::G;

	FS_StarMap NewStarMap;

	// Reset per-star:
	PlanetMapArray.Empty();
	RegionMapArray.Empty();

	for (int i = 0; i < val->elements()->size(); i++)
	{
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (!pdef) continue;

		const Text& Key = pdef->name()->value();

		if (Key == "name")
		{
			GetDefText(StarName, pdef, fn);
			NewStarMap.Name = FString(StarName);
		}
		else if (Key == "system")
		{
			GetDefText(SystemName, pdef, fn);
			NewStarMap.SystemName = FString(SystemName); // FIXED
		}
		else if (Key == "map")
		{
			GetDefText(MapName, pdef, fn);
			NewStarMap.Map = FString(MapName);
		}
		else if (Key == "image")
		{
			GetDefText(ImgName, pdef, fn);
			NewStarMap.Image = FString(ImgName);
		}
		else if (Key == "mass")
		{
			GetDefNumber(Mass, pdef, fn);
			NewStarMap.Mass = Mass;
		}
		else if (Key == "orbit")
		{
			GetDefNumber(Orbit, pdef, fn);
			NewStarMap.Orbit = Orbit;
		}
		else if (Key == "radius")
		{
			GetDefNumber(Radius, pdef, fn);
			NewStarMap.Radius = Radius;
		}
		else if (Key == "rotation")
		{
			GetDefNumber(Rot, pdef, fn);
			NewStarMap.Rot = Rot;
		}
		else if (Key == "tscale")
		{
			GetDefNumber(Tscale, pdef, fn);
			NewStarMap.Tscale = Tscale;
		}
		else if (Key == "light")
		{
			GetDefNumber(Light, pdef, fn);
			NewStarMap.Light = Light;
		}
		else if (Key == "retro")
		{
			GetDefBool(Retro, pdef, fn);
			NewStarMap.Retro = Retro;
		}
		else if (Key == "color")
		{
			Vec3 a;
			GetDefVec(a, pdef, fn);
			NewStarMap.Color = Vec3ToColor255(a);
		}
		else if (Key == "back" || Key == "back_color")
		{
			Vec3 a;
			GetDefVec(a, pdef, fn);
			NewStarMap.Back = Vec3ToColor255(a);
		}
		else if (Key == "class")
		{
			GetDefText(ClassName, pdef, fn);

			switch (ClassName[0])
			{
			case 'B': StarClass = ESPECTRAL_CLASS::B;           break;
			case 'A': StarClass = ESPECTRAL_CLASS::A;           break;
			case 'F': StarClass = ESPECTRAL_CLASS::F;           break;
			case 'G': StarClass = ESPECTRAL_CLASS::G;           break;
			case 'K': StarClass = ESPECTRAL_CLASS::K;           break;
			case 'M': StarClass = ESPECTRAL_CLASS::M;           break;
			case 'R': StarClass = ESPECTRAL_CLASS::RED_GIANT;   break;
			case 'W': StarClass = ESPECTRAL_CLASS::WHITE_DWARF; break;
			case 'Z': StarClass = ESPECTRAL_CLASS::BLACK_HOLE;  break;
			default:  StarClass = ESPECTRAL_CLASS::G;           break;
			}

			NewStarMap.Class = StarClass;
		}
		else if (Key == "region")
		{
			if (!pdef->term() || !pdef->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: region struct missing in '%s'"), *FString(fn));
			}
			else
			{
				ParseRegion(pdef->term()->isStruct(), fn);
				NewStarMap.Region = RegionMapArray;
			}
		}
		else if (Key == "planet")
		{
			if (!pdef->term() || !pdef->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: planet struct missing in '%s'"), *FString(fn));
			}
			else
			{
				// ParsePlanetMap appends to PlanetMapArray (and moons inside it)
				ParsePlanetMap(pdef->term()->isStruct(), fn);
				NewStarMap.Planet = PlanetMapArray;
			}
		}
	}

	StarMapArray.Add(NewStarMap);
}

void UStarshatterGameDataSubsystem::ParsePlanetMap(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParsePlanetMap()"));

	Text   PlanetName = "";
	Text   PlanetIcon = "";
	Text   PlanetRing = "";
	Text   PlanetTexture = "";
	Text   PlanetGloss = "";
	Text   PlanetLights = "";

	double Mass = 0.0;
	double Orbit = 0.0;
	double Inclination = 0.0;
	double Aphelion = 0.0;
	double Perihelion = 0.0;
	double Eccentricity = 0.0;
	double Radius = 0.0;
	double Rot = 0.0;
	double Minrad = 0.0;
	double Maxrad = 0.0;
	double Tscale = 1.0;
	double Tilt = 0.0;

	bool Retro = false;

	FS_PlanetMap NewPlanetMap;

	// Reset per-planet:
	MoonMapArray.Empty();
	RegionMapArray.Empty();

	for (int i = 0; i < val->elements()->size(); i++)
	{
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (!pdef) continue;

		const Text& Key = pdef->name()->value();

		if (Key == "name")
		{
			GetDefText(PlanetName, pdef, fn);
			NewPlanetMap.Name = FString(PlanetName);
		}
		else if (Key == "icon")
		{
			GetDefText(PlanetIcon, pdef, fn);
			NewPlanetMap.Icon = FString(PlanetIcon);
		}
		else if (Key == "texture")
		{
			GetDefText(PlanetTexture, pdef, fn);
			NewPlanetMap.Texture = FString(PlanetTexture);
		}
		else if (Key == "gloss")
		{
			GetDefText(PlanetGloss, pdef, fn);
			NewPlanetMap.Gloss = FString(PlanetGloss);
		}
		else if (Key == "lights")
		{
			GetDefText(PlanetLights, pdef, fn);
			NewPlanetMap.Lights = FString(PlanetLights);
		}
		else if (Key == "ring")
		{
			GetDefText(PlanetRing, pdef, fn);
			NewPlanetMap.Ring = FString(PlanetRing);
		}
		else if (Key == "mass")
		{
			GetDefNumber(Mass, pdef, fn);
			NewPlanetMap.Mass = Mass;
		}
		else if (Key == "orbit")
		{
			GetDefNumber(Orbit, pdef, fn);
			NewPlanetMap.Orbit = Orbit;
		}
		else if (Key == "inclination")
		{
			GetDefNumber(Inclination, pdef, fn);
			NewPlanetMap.Inclination = Inclination;
		}
		else if (Key == "aphelion")
		{
			GetDefNumber(Aphelion, pdef, fn);
			NewPlanetMap.Aphelion = Aphelion;
		}
		else if (Key == "perihelion")
		{
			GetDefNumber(Perihelion, pdef, fn);
			NewPlanetMap.Perihelion = Perihelion;
		}
		else if (Key == "eccentricity")
		{
			GetDefNumber(Eccentricity, pdef, fn);
			NewPlanetMap.Eccentricity = Eccentricity;
		}
		else if (Key == "retro")
		{
			GetDefBool(Retro, pdef, fn);
			NewPlanetMap.Retro = Retro;
		}
		else if (Key == "rotation")
		{
			GetDefNumber(Rot, pdef, fn);
			NewPlanetMap.Rot = Rot;
		}
		else if (Key == "radius")
		{
			GetDefNumber(Radius, pdef, fn);
			NewPlanetMap.Radius = Radius;
		}
		else if (Key == "minrad")
		{
			GetDefNumber(Minrad, pdef, fn);
			NewPlanetMap.Minrad = Minrad;
		}
		else if (Key == "maxrad")
		{
			GetDefNumber(Maxrad, pdef, fn);
			NewPlanetMap.Maxrad = Maxrad;
		}
		else if (Key == "tscale")
		{
			GetDefNumber(Tscale, pdef, fn);
			NewPlanetMap.Tscale = Tscale;
		}
		else if (Key == "tilt")
		{
			GetDefNumber(Tilt, pdef, fn);
			NewPlanetMap.Tilt = Tilt;
		}
		else if (Key == "atmosphere")
		{
			Vec3 a;
			GetDefVec(a, pdef, fn);
			NewPlanetMap.Atmos = Vec3ToColor255(a);
		}
		else if (Key == "moon")
		{
			if (!pdef->term() || !pdef->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: moon struct missing in '%s'"), *FString(fn));
			}
			else
			{
				// ParseMoonMap appends to MoonMapArray
				ParseMoonMap(pdef->term()->isStruct(), fn);
				NewPlanetMap.Moon = MoonMapArray;
			}
		}
		else if (Key == "region")
		{
			if (!pdef->term() || !pdef->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: region struct missing in '%s'"), *FString(fn));
			}
			else
			{
				ParseRegion(pdef->term()->isStruct(), fn);
				NewPlanetMap.Region = RegionMapArray;
			}
		}
	}

	PlanetMapArray.Add(NewPlanetMap);
}

void UStarshatterGameDataSubsystem::ParseTerrain(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseTerrain()"));

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();

		Text   RegionName = "";
		Text   PatchTexture = "";
		Text   NoiseTex0 = "";
		Text   NoiseTex1 = "";
		Text   ApronName = "";
		Text   ApronTexture = "";
		Text   WaterTexture = "";
		Text   EnvTexturePositive_x = "";
		Text   EnvTextureNegative_x = "";
		Text   EnvTexturePositive_y = "";
		Text   EnvTextureNegative_y = "";
		Text   EnvTexturePositive_z = "";
		Text   EnvTextureNegative_z = "";
		Text   HazeName = "";
		Text   SkyName = "";
		Text   CloudsHigh = "";
		Text   CloudsLow = "";
		Text   ShadesHigh = "";
		Text   ShadesLow = "";

		double size = 1.0e6;
		double grid = 25000;
		double inclination = 0.0;
		double scale = 10e3;
		double mtnscale = 1e3;
		double fog_density = 0;
		double fog_scale = 0;
		double haze_fade = 0;
		double clouds_alt_high = 0;
		double clouds_alt_low = 0;
		double w_period = 0;
		double w_chances[EWEATHER_STATE::NUM_STATES];

		if (pdef) {
			if (pdef->name()->value() == "name") {
				GetDefText(RegionName, pdef, fn);
			}
			else if (pdef->name()->value() == "patch" || pdef->name()->value() == "patch_texture") {
				GetDefText(PatchTexture, pdef, fn);
			}
			else if (pdef->name()->value() == "detail_texture_0") {
				GetDefText(NoiseTex0, pdef, fn);
			}
			else if (pdef->name()->value() == "detail_texture_1") {
				GetDefText(NoiseTex1, pdef, fn);
			}
			else if (pdef->name()->value() == "apron") {
				GetDefText(ApronName, pdef, fn);
			}
			else if (pdef->name()->value() == "apron_texture") {
				GetDefText(ApronTexture, pdef, fn);
			}
			else if (pdef->name()->value() == "water_texture") {
				GetDefText(WaterTexture, pdef, fn);
			}
			else if (pdef->name()->value() == "env_texture_positive_x") {
				GetDefText(EnvTexturePositive_x, pdef, fn);
			}
			else if (pdef->name()->value() == "env_texture_negative_x") {
				GetDefText(EnvTextureNegative_x, pdef, fn);
			}
			else if (pdef->name()->value() == "env_texture_positive_y") {
				GetDefText(EnvTexturePositive_y, pdef, fn);
			}
			else if (pdef->name()->value() == "env_texture_negative_y") {
				GetDefText(EnvTextureNegative_y, pdef, fn);
			}
			else if (pdef->name()->value() == "env_texture_positive_z") {
				GetDefText(EnvTexturePositive_z, pdef, fn);
			}
			else if (pdef->name()->value() == "env_texture_negative_z") {
				GetDefText(EnvTextureNegative_z, pdef, fn);
			}
			else if (pdef->name()->value() == "clouds_high") {
				GetDefText(CloudsHigh, pdef, fn);
			}
			else if (pdef->name()->value() == "shades_high") {
				GetDefText(ShadesHigh, pdef, fn);
			}
			else if (pdef->name()->value() == "clouds_low") {
				GetDefText(CloudsLow, pdef, fn);
			}
			else if (pdef->name()->value() == "shades_low") {
				GetDefText(ShadesLow, pdef, fn);
			}
			else if (pdef->name()->value() == "haze") {
				GetDefText(HazeName, pdef, fn);
			}
			else if (pdef->name()->value() == "sky_color") {
				GetDefText(SkyName, pdef, fn);
			}
			else if (pdef->name()->value() == "size" || pdef->name()->value() == "radius") {
				GetDefNumber(size, pdef, fn);
			}
			else if (pdef->name()->value() == "grid") {
				GetDefNumber(grid, pdef, fn);
			}
			else if (pdef->name()->value() == "inclination") {
				GetDefNumber(inclination, pdef, fn);
			}
			else if (pdef->name()->value() == "scale") {
				GetDefNumber(scale, pdef, fn);
			}
			else if (pdef->name()->value() == "mtnscale" || pdef->name()->value() == "mtn_scale") {
				GetDefNumber(mtnscale, pdef, fn);
			}
			else if (pdef->name()->value() == "fog_density") {
				GetDefNumber(fog_density, pdef, fn);
			}
			else if (pdef->name()->value() == "fog_scale") {
				GetDefNumber(fog_scale, pdef, fn);
			}
			else if (pdef->name()->value() == "haze_fade") {
				GetDefNumber(haze_fade, pdef, fn);
			}
			else if (pdef->name()->value() == "clouds_alt_high") {
				GetDefNumber(clouds_alt_high, pdef, fn);
			}
			else if (pdef->name()->value() == "clouds_alt_low") {
				GetDefNumber(clouds_alt_low, pdef, fn);
			}
			else if (pdef->name()->value() == "weather_period") {
				GetDefNumber(w_period, pdef, fn);
			}
			else if (pdef->name()->value() == "weather_clear") {
				GetDefNumber(w_chances[0], pdef, fn);
			}
			else if (pdef->name()->value() == "weather_high_clouds") {
				GetDefNumber(w_chances[1], pdef, fn);
			}
			else if (pdef->name()->value() == "weather_moderate_clouds") {
				GetDefNumber(w_chances[2], pdef, fn);
			}
			else if (pdef->name()->value() == "weather_overcast") {
				GetDefNumber(w_chances[3], pdef, fn);
			}
			else if (pdef->name()->value() == "weather_fog") {
				GetDefNumber(w_chances[4], pdef, fn);
			}
			else if (pdef->name()->value() == "weather_storm") {
				GetDefNumber(w_chances[5], pdef, fn);
			}

			else if (pdef->name()->value() == "layer") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					Print("WARNING: terrain layer struct missing in '%s'\n", fn);
				}
				else {

					//if (!region)
					//	region = new  TerrainRegion(this, rgn_name, size, primary);

					//TermStruct* val = pdef->term()->isStruct();
					//ParseLayer(region, val);
				}
			}
		}
	}
}
// +-------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::LoadStarsystems()
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadStarsystems()"));
	ProjectPath = FPaths::ProjectContentDir();
	ProjectPath.Append(TEXT("GameData/Galaxy/Systems/"));
	FString PathName = ProjectPath;

	TArray<FString> output;
	output.Empty();

	FString SysPath = PathName + "*.def";
	FFileManagerGeneric::Get().FindFiles(output, *SysPath, true, false);

	for (int i = 0; i < output.Num(); i++) {

		FString FileName = ProjectPath;
		FileName.Append(output[i]);

		char* fn = TCHAR_TO_ANSI(*FileName);
		UE_LOG(LogTemp, Log, TEXT("Found StarSystem: '%s'"), *FString(FileName));

		ParseStarSystem(fn);
	}
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::ParseStarSystem(const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParseStarSystem()"));

	if (!fn || !*fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParseStarSystem called with null/empty filename"));
		return;
	}

	const FString LocalPath = ANSI_TO_TCHAR(fn);

	if (!FPaths::FileExists(LocalPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Star system file not found: %s"), *LocalPath);
		return;
	}

	// UE-native raw bytes load (preserves legacy parser expectations)
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *LocalPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read star system file: %s"), *LocalPath);
		return;
	}
	Bytes.Add(0); // null terminate

	Parser parser(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* term = parser.ParseTerm();

	if (!term)
	{
		UE_LOG(LogTemp, Warning, TEXT("ERROR: could not parse '%s'"), *LocalPath);
		return;
	}

	// Header check:
	{
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "STARSYSTEM")
		{
			UE_LOG(LogTemp, Warning, TEXT("ERROR: invalid star system file '%s'"), *LocalPath);
			delete term;
			return;
		}
	}

	// Locals
	Text  SystemName = "";
	Text  SkyPolyStars = "";
	Text  SkyNebula = "";
	Text  SkyHaze = "";

	int   SkyStars = 0;
	int   SkyDust = 0;

	FColor AmbientColor = FColor::Black;

	FS_StarSystem NewStarSystem;

	// scratch arrays used by nested parsers:
	StarDataArray.Empty();
	PlanetDataArray.Empty();
	MoonDataArray.Empty();
	RegionMapArray.Empty();

	do
	{
		delete term;
		term = parser.ParseTerm();

		if (!term)
			break;

		TermDef* def = term->isDef();
		if (!def)
			continue;

		const Text& Key = def->name()->value();

		if (Key == "name")
		{
			GetDefText(SystemName, def, fn);
			NewStarSystem.SystemName = FString(SystemName);
		}
		else if (Key == "sky")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: sky struct missing in '%s'"), *LocalPath);
			}
			else
			{
				TermStruct* val = def->term()->isStruct();
				for (int i = 0; i < val->elements()->size(); i++)
				{
					TermDef* pdef = val->elements()->at(i)->isDef();
					if (!pdef) continue;

					const Text& SkyKey = pdef->name()->value();

					if (SkyKey == "poly_stars")
					{
						GetDefText(SkyPolyStars, pdef, fn);
						NewStarSystem.StarSky.SkyPolyStars = FString(SkyPolyStars);
					}
					else if (SkyKey == "nebula")
					{
						GetDefText(SkyNebula, pdef, fn);
						NewStarSystem.StarSky.SkyNebula = FString(SkyNebula);
					}
					else if (SkyKey == "haze")
					{
						GetDefText(SkyHaze, pdef, fn);
						NewStarSystem.StarSky.SkyHaze = FString(SkyHaze);
					}
				}
			}
		}
		else if (Key == "stars")
		{
			GetDefNumber(SkyStars, def, fn);
			NewStarSystem.SkyStars = SkyStars;
		}
		else if (Key == "ambient")
		{
			Vec3 a;
			GetDefVec(a, def, fn);
			AmbientColor = FColor((uint8)a.X, (uint8)a.Y, (uint8)a.Z, 255);
			NewStarSystem.AmbientColor = AmbientColor;
		}
		else if (Key == "dust")
		{
			GetDefNumber(SkyDust, def, fn);
			NewStarSystem.SkyDust = SkyDust;
		}
		else if (Key == "star")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: star struct missing in '%s'"), *LocalPath);
			}
			else
			{
				ParseStar(def->term()->isStruct(), fn);
				NewStarSystem.Star = StarDataArray; // <-- this must match FS_StarSystem::Star type
			}
		}
		else if (Key == "region")
		{
			if (!def->term() || !def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: region struct missing in '%s'"), *LocalPath);
			}
			else
			{
				ParseRegion(def->term()->isStruct(), fn);

				// IMPORTANT:
				// This only compiles if FS_StarSystem::Region is TArray<FS_RegionMap>.
				// If it's not, you must change FS_StarSystem OR change ParseRegion output.
				// NewStarSystem.Region = RegionMapArray;
			}
		}
		else if (Key == "terrain")
		{
			// TODO: ParseTerrain(def->term()->isStruct(), fn);
		}

	} while (term);

	// free last term if loop ended without deleting
	if (term)
	{
		delete term;
		term = nullptr;
	}

	// Add datatable row ONCE after parse:
	if (StarSystemDataTable)
	{
		const FName RowName(*FString(SystemName));
		StarSystemDataTable->AddRow(RowName, NewStarSystem);
	}
}


// +-------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::InitializeCombatRoster()
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::InitializeCombatRoster()"));

	// Content/GameData/Campaigns/
	ProjectPath = FPaths::ProjectContentDir();
	ProjectPath /= TEXT("GameData/Campaigns/");

	TArray<FString> Files;
	Files.Empty();

	// FindFiles wants the full wildcard path:
	const FString Wildcard = ProjectPath / TEXT("*.def");

	IFileManager::Get().FindFiles(Files, *Wildcard, /*Files=*/true, /*Directories=*/false);

	for (const FString& File : Files)
	{
		const FString FullPath = ProjectPath / File;

		// LoadOrderOfBattle expects const char* (legacy).
		// Convert for the call only (do NOT store the pointer).
		const FTCHARToUTF8 Utf8Path(*FullPath);
		LoadOrderOfBattle(Utf8Path.Get(), -1);
	}
}


void UStarshatterGameDataSubsystem::LoadShipDesigns()
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadShipDesigns()"));

	// Content/GameData/Ships/
	ProjectPath = FPaths::ProjectContentDir();
	ProjectPath /= TEXT("GameData/Ships/");

	TArray<FString> Files;
	const FString Wildcard = ProjectPath / TEXT("*.def");

	IFileManager::Get().FindFiles(Files, *Wildcard, /*Files=*/true, /*Directories=*/false);

	for (const FString& File : Files)
	{
		const FString FullPath = ProjectPath / File;

		// Legacy API wants const char*; convert for the call only (do NOT store the pointer).
		const FTCHARToUTF8 Utf8Path(*FullPath);
		LoadShipDesign(Utf8Path.Get());
	}
}

// In StarshatterGameDataSubsystem.h (member, NOT local):
// Keep as plain member (no need for UPROPERTY unless you want reflection/GC awareness)

void UStarshatterGameDataSubsystem::LoadSystemDesignsFromDT()
{
	if (!SystemDesignDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadSystemDesignsFromDT: SystemDesignDataTable is null"));
		return;
	}

	// Reset stable string storage each rebuild (or keep if you want incremental)
	SystemDesignStringStorage.Reset();

	const TArray<FName> RowNames = SystemDesignDataTable->GetRowNames();

	for (int32 RowIdx = 0; RowIdx < RowNames.Num(); ++RowIdx) // renamed from Index
	{
		const FName RowName = RowNames[RowIdx];

		FS_SystemDesign* Row = SystemDesignDataTable->FindRow<FS_SystemDesign>(RowName, TEXT("LoadSystemDesignsFromDT"));
		if (!Row)
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadSystemDesignsFromDT: missing row '%s'"), *RowName.ToString());
			continue;
		}

		SystemDesign* Design = new SystemDesign();

		// ---- Design->name (stable UTF-8 bytes) ----
		{
			const FString& UEName = Row->Name;

			TArray<uint8>& Bytes = SystemDesignStringStorage.AddDefaulted_GetRef();
			const FTCHARToUTF8 Utf8(*UEName);
			Bytes.Append(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
			Bytes.Add(0);

			Design->name = reinterpret_cast<const char*>(Bytes.GetData());

			UE_LOG(LogTemp, Log, TEXT("System Design: %s"), *UEName);
		}

		// Keep your UE-side table copy (note: you are storing FS_SystemDesign* pointers)
		SystemDesignTable.Add(Row);

		// ---- Components ----
		for (int32 CompRowIdx = 0; CompRowIdx < Row->Component.Num(); ++CompRowIdx)
		{
			const auto& CompRow = Row->Component[CompRowIdx];

			ComponentDesign* CompDesign = new ComponentDesign();

			// name
			{
				TArray<uint8>& Bytes = SystemDesignStringStorage.AddDefaulted_GetRef();
				const FTCHARToUTF8 Utf8(*CompRow.Name);
				Bytes.Append(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
				Bytes.Add(0);
				CompDesign->name = reinterpret_cast<const char*>(Bytes.GetData());
			}

			// abrv
			{
				TArray<uint8>& Bytes = SystemDesignStringStorage.AddDefaulted_GetRef();
				const FTCHARToUTF8 Utf8(*CompRow.Abrv);
				Bytes.Append(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
				Bytes.Add(0);
				CompDesign->abrv = reinterpret_cast<const char*>(Bytes.GetData());
			}

			CompDesign->repair_time = CompRow.RepairTime;
			CompDesign->replace_time = CompRow.ReplaceTime;
			CompDesign->spares = CompRow.Spares;
			CompDesign->affects = CompRow.Affects;

			UE_LOG(LogTemp, Log, TEXT("Component Design: %s"), *CompRow.Name);

			Design->components.append(CompDesign);
		}

		SystemDesign::catalog.append(Design);
	}
}

void UStarshatterGameDataSubsystem::LoadSystemDesigns()
{
	SystemDesignTable.Empty();

	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadSystemDesigns()"));

	const FString SysDefPath = FPaths::ProjectContentDir() / TEXT("GameData/Systems/sys.def");

	if (!FPaths::FileExists(SysDefPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadSystemDesigns: file not found: %s"), *SysDefPath);
		return;
	}

	// Load raw bytes (legacy parser expects a char* buffer)
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *SysDefPath))
	{
		UE_LOG(LogTemp, Error, TEXT("LoadSystemDesigns: failed to read: %s"), *SysDefPath);
		return;
	}

	// Null-terminate for BlockReader / legacy parsing
	Bytes.Add(0);

	// Stable UTF-8 filename for legacy GetDef* helpers / any internal logging expecting const char*
	const FTCHARToUTF8 Utf8Path(*SysDefPath);
	const char* fn = Utf8Path.Get();

	Parser parser(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* term = parser.ParseTerm();

	if (!term)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadSystemDesigns: could not parse '%s'"), *SysDefPath);
		return;
	}

	do
	{
		delete term;
		term = parser.ParseTerm();
		if (!term)
			break;

		TermDef* def = term->isDef();
		if (!def)
			continue;

		// Example:
		// if (def->name()->value() == "system_design") { ... }
		// Use GetDefText/GetDefNumber/etc with (.., def, fn)

	} while (term);

	if (term)
	{
		delete term;
		term = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("LoadSystemDesigns: loaded sys.def"));
}


void UStarshatterGameDataSubsystem::LoadOrderOfBattle(const char* InFilename, int32 Team)
{
	UE_LOG(LogTemp, Log, TEXT("LoadOrderOfBattle"));

	if (!InFilename || !*InFilename)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadOrderOfBattle: null/empty filename"));
		return;
	}

	const FString OobFilePath = ANSI_TO_TCHAR(InFilename);

	UE_LOG(LogTemp, Log, TEXT("Loading Order of Battle Data: %s"), *OobFilePath);

	if (!FPaths::FileExists(OobFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadOrderOfBattle: file not found: %s"), *OobFilePath);
		return;
	}

	// ------------------------------------------------------------
	// UE-native raw byte load (legacy parser compatible)
	// ------------------------------------------------------------
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *OobFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("LoadOrderOfBattle: failed to read: %s"), *OobFilePath);
		return;
	}

	// Null terminate for BlockReader
	Bytes.Add(0);

	// Stable UTF-8 filename for legacy GetDef* helpers
	const FTCHARToUTF8 Utf8Path(*OobFilePath);
	const char* fn = Utf8Path.Get();

	Parser ParserObj(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* TermPtr = ParserObj.ParseTerm();

	if (!TermPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadOrderOfBattle: could not parse: %s"), *OobFilePath);
		return;
	}

	// ------------------------------------------------------------
	// Header validation
	// ------------------------------------------------------------
	{
		TermText* FileType = TermPtr->isText();
		if (!FileType || FileType->value() != "ORDER_OF_BATTLE")
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Order of Battle File: %s"), *OobFilePath);
			delete TermPtr;
			return;
		}
	}

	// We’re done with the header term:
	delete TermPtr;
	TermPtr = nullptr;

	// ------------------------------------------------------------
	// Parse groups
	// ------------------------------------------------------------
	while ((TermPtr = ParserObj.ParseTerm()) != nullptr)
	{
		TermDef* Def = TermPtr->isDef();
		if (!Def || Def->name()->value() != "group")
		{
			delete TermPtr;
			TermPtr = nullptr;
			continue;
		}

		if (!Def->term() || !Def->term()->isStruct())
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: group struct missing in '%s'"), *OobFilePath);
			delete TermPtr;
			TermPtr = nullptr;
			continue;
		}

		TermStruct* GroupStruct = Def->term()->isStruct();

		FS_CombatGroup NewCombatGroup;
		NewCombatUnitArray.Empty();

		// ---- local group scratch ----
		Text LocalName = "";
		Text LocalType = "";
		Text LocalRegion = "";
		Text LocalSystem = "";
		Text LocalParentType = "";

		EINTEL_TYPE LocalIntelType = EINTEL_TYPE::KNOWN;
		ECOMBATGROUP_TYPE LocalGroupType = ECOMBATGROUP_TYPE::NONE;
		ECOMBATGROUP_TYPE LocalParentGroupType = ECOMBATGROUP_TYPE::NONE;

		int32 LocalParentId = 0;
		int32 LocalEmpireId = 0;
		int32 LocalIff = -1;
		int32 LocalUnitIndex = 0;

		Vec3 LocalLoc(1.0e9f, 0.0f, 0.0f);

		// --------------------------------------------------------
		// Group fields
		// --------------------------------------------------------
		const int32 GroupElemCount = (int32)GroupStruct->elements()->size();
		for (int32 FieldIdx = 0; FieldIdx < GroupElemCount; ++FieldIdx)
		{
			TermDef* PDef = GroupStruct->elements()->at(FieldIdx)->isDef();
			if (!PDef)
				continue;

			const Text& Key = PDef->name()->value();

			if (Key == "name")
			{
				GetDefText(LocalName, PDef, fn);
				NewCombatGroup.Name = FString(LocalName);
			}
			else if (Key == "intel")
			{
				Text Intel = "";
				GetDefText(Intel, PDef, fn);

				if (!FStringToEnum<EINTEL_TYPE>(FString(Intel).ToUpper(), LocalIntelType, false))
					LocalIntelType = EINTEL_TYPE::KNOWN;

				NewCombatGroup.Intel = LocalIntelType;
			}
			else if (Key == "region")
			{
				GetDefText(LocalRegion, PDef, fn);
				NewCombatGroup.Region = FString(LocalRegion);
			}
			else if (Key == "system")
			{
				GetDefText(LocalSystem, PDef, fn);
				NewCombatGroup.System = FString(LocalSystem);
			}
			else if (Key == "loc")
			{
				GetDefVec(LocalLoc, PDef, fn);
				NewCombatGroup.Location = FVector(LocalLoc.X, LocalLoc.Y, LocalLoc.Z);
			}
			else if (Key == "parent_type")
			{
				GetDefText(LocalParentType, PDef, fn);

				if (!FStringToEnum<ECOMBATGROUP_TYPE>(FString(LocalParentType).ToUpper(), LocalParentGroupType, false))
					LocalParentGroupType = ECOMBATGROUP_TYPE::NONE;

				NewCombatGroup.ParentType = LocalParentGroupType;
			}
			else if (Key == "parent_id")
			{
				GetDefNumber(LocalParentId, PDef, fn);
				NewCombatGroup.ParentId = LocalParentId;
			}
			else if (Key == "empire_id")
			{
				GetDefNumber(LocalEmpireId, PDef, fn);
				NewCombatGroup.EmpireId = UFormattingUtils::GetEmpireTypeFromIndex(LocalEmpireId);
			}
			else if (Key == "iff")
			{
				GetDefNumber(LocalIff, PDef, fn);
				NewCombatGroup.Iff = LocalIff;
			}
			else if (Key == "id")
			{
				int32 LocalId = 0;
				GetDefNumber(LocalId, PDef, fn);
				NewCombatGroup.Id = LocalId;
			}
			else if (Key == "unit_index")
			{
				GetDefNumber(LocalUnitIndex, PDef, fn);
				NewCombatGroup.UnitIndex = LocalUnitIndex;
			}
			else if (Key == "type")
			{
				GetDefText(LocalType, PDef, fn);

				if (!FStringToEnum<ECOMBATGROUP_TYPE>(FString(LocalType).ToUpper(), LocalGroupType, false))
					LocalGroupType = ECOMBATGROUP_TYPE::NONE;

				NewCombatGroup.Type = LocalGroupType;
			}
			else if (Key == "unit")
			{
				TermStruct* UnitStruct = (PDef->term() ? PDef->term()->isStruct() : nullptr);
				if (!UnitStruct)
					continue;

				FS_CombatGroupUnit NewUnit;

				// ---- local unit scratch ----
				Text LocalUnitName = "";
				Text LocalUnitRegnum = "";
				Text LocalUnitRegion = "";
				Text LocalUnitClass = "";
				Text LocalUnitDesign = "";
				Text LocalUnitSkin = "";

				int32 LocalUnitCount = 1;
				int32 LocalUnitDamage = 0;
				int32 LocalUnitDead = 0;
				int32 LocalUnitHeading = 0;
				Vec3  LocalUnitLoc(1.0e9f, 0.0f, 0.0f);

				const int32 UnitElemCount = (int32)UnitStruct->elements()->size();
				for (int32 UnitFieldIdx = 0; UnitFieldIdx < UnitElemCount; ++UnitFieldIdx)
				{
					TermDef* UDef = UnitStruct->elements()->at(UnitFieldIdx)->isDef();
					if (!UDef) continue;

					const Text& UKey = UDef->name()->value();

					if (UKey == "name") { GetDefText(LocalUnitName, UDef, fn); NewUnit.UnitName = FString(LocalUnitName); }
					else if (UKey == "regnum") { GetDefText(LocalUnitRegnum, UDef, fn); }
					else if (UKey == "region") { GetDefText(LocalUnitRegion, UDef, fn); }
					else if (UKey == "loc") { GetDefVec(LocalUnitLoc, UDef, fn); }
					else if (UKey == "type") { GetDefText(LocalUnitClass, UDef, fn); }
					else if (UKey == "design") { GetDefText(LocalUnitDesign, UDef, fn); }
					else if (UKey == "skin") { GetDefText(LocalUnitSkin, UDef, fn); }
					else if (UKey == "count") { GetDefNumber(LocalUnitCount, UDef, fn); }
					else if (UKey == "dead_count") { GetDefNumber(LocalUnitDead, UDef, fn); }
					else if (UKey == "damage") { GetDefNumber(LocalUnitDamage, UDef, fn); }
					else if (UKey == "heading") { GetDefNumber(LocalUnitHeading, UDef, fn); }
				}

				NewUnit.UnitRegnum = FString(LocalUnitRegnum);
				NewUnit.UnitRegion = FString(LocalUnitRegion);
				NewUnit.UnitLoc = FVector(LocalUnitLoc.X, LocalUnitLoc.Y, LocalUnitLoc.Z);
				NewUnit.UnitClass = FString(LocalUnitClass);
				NewUnit.UnitDesign = FString(LocalUnitDesign);
				NewUnit.UnitSkin = FString(LocalUnitSkin);
				NewUnit.UnitCount = LocalUnitCount;
				NewUnit.UnitDead = LocalUnitDead;
				NewUnit.UnitDamage = LocalUnitDamage;
				NewUnit.UnitHeading = LocalUnitHeading;

				NewCombatUnitArray.Add(NewUnit);
				NewCombatGroup.Unit = NewCombatUnitArray;
			}
		}

		// --------------------------------------------------------
		// Add row (team filtered)
		// --------------------------------------------------------
		const bool bPassTeam = (Team < 0) || (NewCombatGroup.Iff == Team);

		if (SSWInstance && SSWInstance->CombatGroupDataTable &&
			NewCombatGroup.Iff > -1 && bPassTeam)
		{
			const FName RowName(*(
				UFormattingUtils::GetOrdinal(NewCombatGroup.Id) + TEXT(" ") +
				FString(UFormattingUtils::GetGroupTypeDisplayName(NewCombatGroup.Type)) +
				TEXT(" [") + NewCombatGroup.Name + TEXT("]")
				));

			NewCombatGroup.DisplayName = RowName.ToString();
			SSWInstance->CombatGroupDataTable->AddRow(RowName, NewCombatGroup);
		}

		CombatGroupData = NewCombatGroup;

		delete TermPtr;
		TermPtr = nullptr;
	}
}


void UStarshatterGameDataSubsystem::LoadShipDesign(const char* InFilename)
{
	if (!InFilename || !*InFilename)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadShipDesign: null/empty filename"));
		return;
	}

	const FString ShipFilePath = ANSI_TO_TCHAR(InFilename);
	UE_LOG(LogTemp, Log, TEXT("Loading Ship Design Data: %s"), *ShipFilePath);

	if (!FPaths::FileExists(ShipFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadShipDesign: file not found: %s"), *ShipFilePath);
		return;
	}

	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *ShipFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("LoadShipDesign: failed to read: %s"), *ShipFilePath);
		return;
	}
	Bytes.Add(0);

	const FTCHARToUTF8 Utf8Path(*ShipFilePath);
	const char* fn = Utf8Path.Get();

	Parser ParserObj(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* TermPtr = ParserObj.ParseTerm();

	if (!TermPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadShipDesign: could not parse: %s"), *ShipFilePath);
		return;
	}

	// Header check:
	{
		TermText* FileType = TermPtr->isText();
		if (!FileType || FileType->value() != "SHIP")
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Ship File: %s"), *ShipFilePath);
			delete TermPtr;
			return;
		}
	}

	delete TermPtr;
	TermPtr = nullptr;

	// ------------------------------------------------------------
	// Locals
	// ------------------------------------------------------------
	Text LocalShipName = "";
	Text LocalDisplayName = "";
	Text LocalDescription = "";
	Text LocalAbrv = "";

	Text LocalDetailName0 = "";
	Text LocalDetailName1 = "";
	Text LocalDetailName2 = "";
	Text LocalDetailName3 = "";

	Text LocalShipClass = "";
	Text LocalCockpitName = "";
	Text LocalBeautyName = "";
	Text LocalHudIconName = "";

	int32 LocalPcs = 3;
	int32 LocalAcs = 1;
	float LocalDetec = 250.0e3f;
	float LocalScale = 1.0f;
	float LocalExplosionScale = 0.0f;
	float LocalMass = 0.0f;

	int32 LocalShipType = 0;

	float LocalVlimit = 8e3f;
	float LocalAgility = 2e2f;
	float LocalAirFactor = 0.1f;
	float LocalRollRate = 0.0f;
	float LocalPitchRate = 0.0f;
	float LocalYawRate = 0.0f;
	float LocalTransX = 0.0f;
	float LocalTransY = 0.0f;
	float LocalTransZ = 0.0f;
	float LocalTurnBank = (float)(PI / 8);

	float LocalCockpitScale = 1.0f;
	float LocalAutoRoll = 0.0f;

	float LocalCL = 0.0f;
	float LocalCD = 0.0f;
	float LocalStall = 0.0f;

	float LocalPrepTime = 30.0f;
	float LocalAvoidTime = 0.0f;
	float LocalAvoidFighter = 0.0f;
	float LocalAvoidStrike = 0.0f;
	float LocalAvoidTarget = 0.0f;
	float LocalCommitRange = 0.0f;

	float LocalSplashRadius = -1.0f;
	float LocalScuttle = 5e3f;
	float LocalRepairSpeed = 1.0f;

	int32 LocalRepairTeams = 2;

	float LocalFeatureSize[4] = { 0,0,0,0 };
	float LocalEFactor[3] = { 0.1f, 0.3f, 1.0f };

	bool bLocalRepairAuto = true;
	bool bLocalRepairScreen = true;
	bool bLocalWepScreen = true;
	bool bLocalDegrees = false;

	FVector LocalOffLoc = FVector::ZeroVector;
	FVector LocalSpin = FVector::ZeroVector;
	FVector LocalBeautyCam = FVector::ZeroVector;
	FVector LocalChaseVec = FVector(0.f, -100.f, 20.f);
	FVector LocalBridgeVec = FVector::ZeroVector;

	FS_ShipDesign NewShipDesign;

	// ------------------------------------------------------------
	// Parse terms
	// ------------------------------------------------------------
	while ((TermPtr = ParserObj.ParseTerm()) != nullptr)
	{
		TermDef* Def = TermPtr->isDef();
		if (!Def)
		{
			delete TermPtr;
			TermPtr = nullptr;
			continue;
		}

		const Text& Key = Def->name()->value();

		if (Key == "name")
		{
			GetDefText(LocalShipName, Def, fn);
			NewShipDesign.ShipName = FString(LocalShipName);
		}
		else if (Key == "display_name")
		{
			GetDefText(LocalDisplayName, Def, fn);
			NewShipDesign.DisplayName = FString(LocalDisplayName);
		}
		else if (Key == "class")
		{
			GetDefText(LocalShipClass, Def, fn);
			NewShipDesign.ShipClass = FString(LocalShipClass);

			LocalShipType = UFormattingUtils::GetDesignClassFromName(LocalShipClass);

			if (LocalShipType <= (int32)CLASSIFICATION::LCA)
			{
				bLocalRepairAuto = false;
				bLocalRepairScreen = false;
				bLocalWepScreen = false;
			}

			NewShipDesign.ShipType = LocalShipType;
			NewShipDesign.RepairAuto = bLocalRepairAuto;
			NewShipDesign.RepairScreen = bLocalRepairScreen;
			NewShipDesign.WepScreen = bLocalWepScreen;
		}
		else if (Key == "description")
		{
			GetDefText(LocalDescription, Def, fn);
			NewShipDesign.Description = FString(LocalDescription);
		}
		else if (Key == "abrv")
		{
			GetDefText(LocalAbrv, Def, fn);
			NewShipDesign.Abrv = FString(LocalAbrv);
		}
		else if (Key == "pcs")
		{
			GetDefNumber(LocalPcs, Def, fn);
			NewShipDesign.PCS = LocalPcs;
		}
		else if (Key == "acs")
		{
			GetDefNumber(LocalAcs, Def, fn);
			NewShipDesign.ACS = LocalAcs;
		}
		else if (Key == "detec")
		{
			GetDefNumber(LocalDetec, Def, fn);
			NewShipDesign.Detet = LocalDetec;
		}
		else if (Key == "scale")
		{
			GetDefNumber(LocalScale, Def, fn);
			NewShipDesign.Scale = LocalScale;
		}
		else if (Key == "explosion_scale")
		{
			GetDefNumber(LocalExplosionScale, Def, fn);
			NewShipDesign.ExplosionScale = LocalExplosionScale;
		}
		else if (Key == "mass")
		{
			GetDefNumber(LocalMass, Def, fn);
			NewShipDesign.Mass = LocalMass;
		}
		else if (Key == "vlimit")
		{
			GetDefNumber(LocalVlimit, Def, fn);
			NewShipDesign.Vlimit = LocalVlimit;
		}
		else if (Key == "agility")
		{
			GetDefNumber(LocalAgility, Def, fn);
			NewShipDesign.Agility = LocalAgility;
		}
		else if (Key == "air_factor")
		{
			GetDefNumber(LocalAirFactor, Def, fn);
			NewShipDesign.AirFactor = LocalAirFactor;
		}
		else if (Key == "roll_rate")
		{
			GetDefNumber(LocalRollRate, Def, fn);
			NewShipDesign.RollRate = LocalRollRate;
		}
		else if (Key == "pitch_rate")
		{
			GetDefNumber(LocalPitchRate, Def, fn);
			NewShipDesign.PitchRate = LocalPitchRate;
		}
		else if (Key == "yaw_rate")
		{
			GetDefNumber(LocalYawRate, Def, fn);
			NewShipDesign.YawRate = LocalYawRate;
		}
		else if (Key == "trans_x")
		{
			GetDefNumber(LocalTransX, Def, fn);
			NewShipDesign.Trans.X = LocalTransX;
		}
		else if (Key == "trans_y")
		{
			GetDefNumber(LocalTransY, Def, fn);
			NewShipDesign.Trans.Y = LocalTransY;
		}
		else if (Key == "trans_z")
		{
			GetDefNumber(LocalTransZ, Def, fn);
			NewShipDesign.Trans.Z = LocalTransZ;
		}
		else if (Key == "turn_bank")
		{
			GetDefNumber(LocalTurnBank, Def, fn);
			NewShipDesign.TurnBank = LocalTurnBank;
		}
		else if (Key == "cockpit_scale")
		{
			GetDefNumber(LocalCockpitScale, Def, fn);
			NewShipDesign.CockpitScale = LocalCockpitScale;
		}
		else if (Key == "auto_roll")
		{
			GetDefNumber(LocalAutoRoll, Def, fn);
			NewShipDesign.AutoRoll = LocalAutoRoll;
		}
		else if (Key == "CL")
		{
			GetDefNumber(LocalCL, Def, fn);
			NewShipDesign.CL = LocalCL;
		}
		else if (Key == "CD")
		{
			GetDefNumber(LocalCD, Def, fn);
			NewShipDesign.CD = LocalCD;
		}
		else if (Key == "stall")
		{
			GetDefNumber(LocalStall, Def, fn);
			NewShipDesign.Stall = LocalStall;
		}
		else if (Key == "prep_time")
		{
			GetDefNumber(LocalPrepTime, Def, fn);
			NewShipDesign.PrepTime = LocalPrepTime;
		}
		else if (Key == "avoid_time")
		{
			GetDefNumber(LocalAvoidTime, Def, fn);
			NewShipDesign.AvoidTime = LocalAvoidTime;
		}
		else if (Key == "avoid_fighter")
		{
			GetDefNumber(LocalAvoidFighter, Def, fn);
			NewShipDesign.AvoidFighter = LocalAvoidFighter;
		}
		else if (Key == "avoid_strike")
		{
			GetDefNumber(LocalAvoidStrike, Def, fn);
			NewShipDesign.AvoidStrike = LocalAvoidStrike;
		}
		else if (Key == "avoid_target")
		{
			GetDefNumber(LocalAvoidTarget, Def, fn);
			NewShipDesign.AvoidTarget = LocalAvoidTarget;
		}
		else if (Key == "commit_range")
		{
			GetDefNumber(LocalCommitRange, Def, fn);
			NewShipDesign.CommitRange = LocalCommitRange;
		}
		else if (Key == "splash_radius")
		{
			GetDefNumber(LocalSplashRadius, Def, fn);
			NewShipDesign.SplashRadius = LocalSplashRadius;
		}
		else if (Key == "scuttle")
		{
			GetDefNumber(LocalScuttle, Def, fn);
			NewShipDesign.Scuttle = LocalScuttle;
		}
		else if (Key == "repair_speed")
		{
			GetDefNumber(LocalRepairSpeed, Def, fn);
			NewShipDesign.RepairSpeed = LocalRepairSpeed;
		}
		else if (Key == "repair_teams")
		{
			GetDefNumber(LocalRepairTeams, Def, fn);
			NewShipDesign.RepairTeams = LocalRepairTeams;
		}
		else if (Key == "cockpit_model")
		{
			GetDefText(LocalCockpitName, Def, fn);
			NewShipDesign.CockpitName = FString(LocalCockpitName);
		}
		else if (Key == "model" || Key == "detail_0")
		{
			GetDefText(LocalDetailName0, Def, fn);
			NewShipDesign.DetailName0 = FString(LocalDetailName0);
		}
		else if (Key == "detail_1")
		{
			GetDefText(LocalDetailName1, Def, fn);
			NewShipDesign.DetailName1 = FString(LocalDetailName1);
		}
		else if (Key == "detail_2")
		{
			GetDefText(LocalDetailName2, Def, fn);
			NewShipDesign.DetailName2 = FString(LocalDetailName2);
		}
		else if (Key == "detail_3")
		{
			GetDefText(LocalDetailName3, Def, fn);
			NewShipDesign.DetailName3 = FString(LocalDetailName3);
		}
		else if (Key == "spin")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, Def, fn);
			LocalSpin = V;
			NewShipDesign.Spin = LocalSpin;
		}
		else if (Key == "offset_0")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, Def, fn);
			NewShipDesign.Offset[0] = V;
		}
		else if (Key == "offset_1")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, Def, fn);
			NewShipDesign.Offset[1] = V;
		}
		else if (Key == "offset_2")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, Def, fn);
			NewShipDesign.Offset[2] = V;
		}
		else if (Key == "offset_3")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, Def, fn);
			NewShipDesign.Offset[3] = V;
		}
		else if (Key == "beauty")
		{
			if (Def->term() && Def->term()->isArray())
			{
				FVector V = FVector::ZeroVector;
				GetDefVec(V, Def, fn);
				LocalBeautyCam = V;

				if (bLocalDegrees)
				{
					LocalBeautyCam.X *= (float)DEGREES;
					LocalBeautyCam.Y *= (float)DEGREES;
				}

				NewShipDesign.BeautyCam = LocalBeautyCam;
			}
			else
			{
				GetDefText(LocalBeautyName, Def, fn);
				NewShipDesign.BeautyName = FString(LocalBeautyName); // IMPORTANT
			}
		}
		else if (Key == "hud_icon")
		{
			GetDefText(LocalHudIconName, Def, fn);
			NewShipDesign.HudIconName = FString(LocalHudIconName);
		}
		else if (Key == "feature_0")
		{
			GetDefNumber(LocalFeatureSize[0], Def, fn);
			NewShipDesign.FeatureSize[0] = LocalFeatureSize[0];
		}
		else if (Key == "feature_1")
		{
			GetDefNumber(LocalFeatureSize[1], Def, fn);
			NewShipDesign.FeatureSize[1] = LocalFeatureSize[1];
		}
		else if (Key == "feature_2")
		{
			GetDefNumber(LocalFeatureSize[2], Def, fn);
			NewShipDesign.FeatureSize[2] = LocalFeatureSize[2];
		}
		else if (Key == "feature_3")
		{
			GetDefNumber(LocalFeatureSize[3], Def, fn);
			NewShipDesign.FeatureSize[3] = LocalFeatureSize[3];
		}
		else if (Key == "emcon_1")
		{
			GetDefNumber(LocalEFactor[0], Def, fn);
			NewShipDesign.EFactor[0] = LocalEFactor[0];
		}
		else if (Key == "emcon_2")
		{
			GetDefNumber(LocalEFactor[1], Def, fn);
			NewShipDesign.EFactor[1] = LocalEFactor[1];
		}
		else if (Key == "emcon_3")
		{
			GetDefNumber(LocalEFactor[2], Def, fn);
			NewShipDesign.EFactor[2] = LocalEFactor[2];
		}
		else if (Key == "chase")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, Def, fn);
			LocalChaseVec = V * LocalScale;
			NewShipDesign.ChaseVec = LocalChaseVec;
		}
		else if (Key == "bridge")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, Def, fn);
			LocalBridgeVec = V * LocalScale;
			NewShipDesign.BridgeVec = LocalBridgeVec;
		}
		else if (Key == "power")
		{
			if (!Def->term() || !Def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: power source struct missing in '%s'"), *ShipFilePath);
			}
			else
			{
				ParsePower(Def->term()->isStruct(), fn);
			}
		}
		else if (Key == "main_drive" || Key == "drive" ||
			Key == "quantum" || Key == "quantum_drive" ||
			Key == "sender" || Key == "farcaster" ||
			Key == "thruster" || Key == "navlight" ||
			Key == "flightdeck" || Key == "gear" ||
			Key == "weapon" || Key == "hardpoint" ||
			Key == "loadout" || Key == "decoy" ||
			Key == "probe" || Key == "sensor" ||
			Key == "nav" || Key == "computer" ||
			Key == "shield" || Key == "death_spiral" ||
			Key == "map" || Key == "squadron" ||
			Key == "skin")
		{
			if (!Def->term() || !Def->term()->isStruct())
			{
				UE_LOG(LogTemp, Warning, TEXT("WARNING: %s struct missing in '%s'"),
					*FString(ANSI_TO_TCHAR(Key.data())), *ShipFilePath);
			}
			else
			{
				// TermStruct* Val = Def->term()->isStruct();
				// ParseDrive(Val); etc...
			}
		}
		else
		{
			UE_LOG(LogTemp, Verbose, TEXT("WARNING: unknown ship parameter '%s' in '%s'"),
				*FString(ANSI_TO_TCHAR(Key.data())), *ShipFilePath);
		}

		delete TermPtr;
		TermPtr = nullptr;
	}

	// ------------------------------------------------------------
	// Add row ONCE after parse
	// ------------------------------------------------------------
	if (!ShipDesignDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("LoadShipDesign: ShipDesignDataTable is null"));
		return;
	}

	if (!LocalShipName.length())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadShipDesign: missing 'name' in '%s'"), *ShipFilePath);
		return;
	}

	const FName RowName(*FString(LocalShipName));

	// Optional overwrite:
	if (ShipDesignDataTable->FindRow<FS_ShipDesign>(RowName, TEXT("LoadShipDesign"), false))
	{
		ShipDesignDataTable->RemoveRow(RowName);
	}

	ShipDesignDataTable->AddRow(RowName, NewShipDesign);
}

// +--------------------------------------------------------------------+

static EPowerSource PowerTypeFromText(const Text& TypeName)
{
	if (!TypeName.length())
		return EPowerSource::NONE;

	const char c = TypeName[0];

	if (c == 'B' || c == 'b') return EPowerSource::BATTERY;
	if (c == 'A' || c == 'a') return EPowerSource::AUXILIARY;
	if (c == 'F' || c == 'f') return EPowerSource::FUSION;

	return EPowerSource::NONE;
}

void UStarshatterGameDataSubsystem::ParsePower(TermStruct* Val, const char* Fn)
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::ParsePower()"));

	if (!Val || !Fn || !*Fn)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParsePower called with null args"));
		return;
	}

	// If LoadShipDesign set this, we use it:
	const float Scale = (CurrentShipScale > 0.0f) ? CurrentShipScale : 1.0f;

	// Legacy defaults:
	float   Output = 1000.0f;
	float   FuelRange = 0.0f;
	FVector Loc = FVector::ZeroVector;
	float   Size = 0.0f;
	float   HullFactor = 0.5f;

	Text    DesignName = "";
	Text    PName = "";
	Text    PAbrv = "";
	Text    TypeName = "";

	int32   ExplosionType = 0;
	int32   Emcon1 = -1;
	int32   Emcon2 = -1;
	int32   Emcon3 = -1;

	FS_ShipPower NewShipPower;

	const int32 ElemCount = (int32)Val->elements()->size();
	for (int32 ElemIdx = 0; ElemIdx < ElemCount; ++ElemIdx)
	{
		TermDef* PDef = Val->elements()->at(ElemIdx)->isDef();
		if (!PDef)
			continue;

		const Text& Key = PDef->name()->value();

		if (Key == "type")
		{
			if (GetDefText(TypeName, PDef, Fn))
			{
				NewShipPower.Type = PowerTypeFromText(TypeName);

				if (NewShipPower.Type == EPowerSource::NONE)
				{
					UE_LOG(LogTemp, Warning,
						TEXT("ParsePower: unknown power type '%s' in '%s'"),
						*FString(ANSI_TO_TCHAR(TypeName.data())),
						*FString(ANSI_TO_TCHAR(Fn)));
				}
			}
		}
		else if (Key == "name")
		{
			GetDefText(PName, PDef, Fn);
			NewShipPower.PName = FString(PName);
		}
		else if (Key == "abrv")
		{
			GetDefText(PAbrv, PDef, Fn);
			NewShipPower.PAbrv = FString(PAbrv);
		}
		else if (Key == "design")
		{
			GetDefText(DesignName, PDef, Fn);
			NewShipPower.DesignName = FString(DesignName);
		}
		else if (Key == "max_output")
		{
			GetDefNumber(Output, PDef, Fn);
			NewShipPower.Output = Output;
		}
		else if (Key == "fuel_range")
		{
			GetDefNumber(FuelRange, PDef, Fn);
			NewShipPower.Fuel = FuelRange;
		}
		else if (Key == "loc")
		{
			FVector V = FVector::ZeroVector;
			GetDefVec(V, PDef, Fn);
			V *= Scale;

			Loc = V;
			NewShipPower.Loc = Loc;
		}
		else if (Key == "size")
		{
			GetDefNumber(Size, PDef, Fn);
			Size *= Scale;

			NewShipPower.Size = Size;
		}
		else if (Key == "hull_factor")
		{
			GetDefNumber(HullFactor, PDef, Fn);
			NewShipPower.Hull = HullFactor;
		}
		else if (Key == "explosion")
		{
			GetDefNumber(ExplosionType, PDef, Fn);
			NewShipPower.ExplosionType = ExplosionType;
		}
		else if (Key == "emcon_1")
		{
			GetDefNumber(Emcon1, PDef, Fn);
			NewShipPower.Emcon1 = Emcon1;
		}
		else if (Key == "emcon_2")
		{
			GetDefNumber(Emcon2, PDef, Fn);
			NewShipPower.Emcon2 = Emcon2;
		}
		else if (Key == "emcon_3")
		{
			GetDefNumber(Emcon3, PDef, Fn);
			NewShipPower.Emcon3 = Emcon3;
		}
	}

	// Store like your other parse helpers:
	NewShipPowerArray.Add(NewShipPower);
}

// +--------------------------------------------------------------------+

/*void
UStarshatterGameDataSubsystem::ParseDrive(TermStruct* val, const char* fn)
{
	Text  dname;
	Text  dabrv;
	int   dtype = 0;
	int   etype = 0;
	float dthrust = 1.0f;
	float daug = 0.0f;
	float dscale = 1.0f;
	Vec3  loc(0.0f, 0.0f, 0.0f);
	float size = 0.0f;
	float hull = 0.5f;
	Text  design_name;
	int   emcon_1 = -1;
	int   emcon_2 = -1;
	int   emcon_3 = -1;
	bool  trail = true;
	Drive* drive = 0;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "type") {
				TermText* tname = pdef->term()->isText();

				if (tname) {
					Text tval = tname->value();
					tval.setSensitive(false);

					if (tval == "Plasma")       dtype = Drive::PLASMA;
					else if (tval == "Fusion")  dtype = Drive::FUSION;
					else if (tval == "Alien")   dtype = Drive::GREEN;
					else if (tval == "Green")   dtype = Drive::GREEN;
					else if (tval == "Red")     dtype = Drive::RED;
					else if (tval == "Blue")    dtype = Drive::BLUE;
					else if (tval == "Yellow")  dtype = Drive::YELLOW;
					else if (tval == "Stealth") dtype = Drive::STEALTH;

					else Print("WARNING: unknown drive type '%s' in '%s'\n", tname->value().data(), filename);
				}
			}
			else if (defname == "name") {
				if (!GetDefText(dname, pdef, filename))
					Print("WARNING: invalid or missing name for drive in '%s'\n", filename);
			}

			else if (defname == "abrv") {
				if (!GetDefText(dabrv, pdef, filename))
					Print("WARNING: invalid or missing abrv for drive in '%s'\n", filename);
			}

			else if (defname == "design") {
				if (!GetDefText(design_name, pdef, filename))
					Print("WARNING: invalid or missing design for drive in '%s'\n", filename);
			}

			else if (defname == "thrust") {
				if (!GetDefNumber(dthrust, pdef, filename))
					Print("WARNING: invalid or missing thrust for drive in '%s'\n", filename);
			}

			else if (defname == "augmenter") {
				if (!GetDefNumber(daug, pdef, filename))
					Print("WARNING: invalid or missing augmenter for drive in '%s'\n", filename);
			}

			else if (defname == "scale") {
				if (!GetDefNumber(dscale, pdef, filename))
					Print("WARNING: invalid or missing scale for drive in '%s'\n", filename);
			}

			else if (defname == "port") {
				Vec3  port;
				float flare_scale = 0;

				if (pdef->term()->isArray()) {
					GetDefVec(port, pdef, filename);
					port *= scale;
					flare_scale = dscale;
				}

				else if (pdef->term()->isStruct()) {
					TermStruct* val = pdef->term()->isStruct();

					for (int i = 0; i < val->elements()->size(); i++) {
						TermDef* pdef2 = val->elements()->at(i)->isDef();
						if (pdef2) {
							if (pdef2->name()->value() == "loc") {
								GetDefVec(port, pdef2, filename);
								port *= scale;
							}

							else if (pdef2->name()->value() == "scale") {
								GetDefNumber(flare_scale, pdef2, filename);
							}
						}
					}

					if (flare_scale <= 0)
						flare_scale = dscale;
				}

				if (!drive)
					drive = new  Drive((Drive::SUBTYPE)dtype, dthrust, daug, trail);

				drive->AddPort(port, flare_scale);
			}

			else if (defname == "loc") {
				if (!GetDefVec(loc, pdef, filename))
					Print("WARNING: invalid or missing loc for drive in '%s'\n", filename);
				loc *= (float)scale;
			}

			else if (defname == "size") {
				if (!GetDefNumber(size, pdef, filename))
					Print("WARNING: invalid or missing size for drive in '%s'\n", filename);
				size *= (float)scale;
			}

			else if (defname == "hull_factor") {
				if (!GetDefNumber(hull, pdef, filename))
					Print("WARNING: invalid or missing hull_factor for drive in '%s'\n", filename);
			}

			else if (defname == "explosion") {
				if (!GetDefNumber(etype, pdef, filename))
					Print("WARNING: invalid or missing explosion for drive in '%s'\n", filename);
			}

			else if (defname == "emcon_1") {
				GetDefNumber(emcon_1, pdef, filename);
			}

			else if (defname == "emcon_2") {
				GetDefNumber(emcon_2, pdef, filename);
			}

			else if (defname == "emcon_3") {
				GetDefNumber(emcon_3, pdef, filename);
			}

			else if (defname == "trail" || defname == "show_trail") {
				GetDefBool(trail, pdef, filename);
			}
		}
	}
}*/

// +--------------------------------------------------------------------+

/*void
ShipDesign::ParseQuantumDrive(TermStruct* val)
{
	double   capacity = 250e3;
	double   consumption = 1e3;
	Vec3     loc(0.0f, 0.0f, 0.0f);
	float    size = 0.0f;
	float    hull = 0.5f;
	float    countdown = 5.0f;
	Text     design_name;
	Text     type_name;
	Text     abrv;
	int      subtype = QuantumDrive::QUANTUM;
	int      emcon_1 = -1;
	int      emcon_2 = -1;
	int      emcon_3 = -1;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "design") {
				GetDefText(design_name, pdef, filename);
			}
			else if (defname == "abrv") {
				GetDefText(abrv, pdef, filename);
			}
			else if (defname == "type") {
				GetDefText(type_name, pdef, filename);
				type_name.setSensitive(false);

				if (type_name.contains("hyper")) {
					subtype = QuantumDrive::HYPER;
				}
			}
			else if (defname == "capacity") {
				GetDefNumber(capacity, pdef, filename);
			}
			else if (defname == "consumption") {
				GetDefNumber(consumption, pdef, filename);
			}
			else if (defname == "loc") {
				GetDefVec(loc, pdef, filename);
				loc *= (float)scale;
			}
			else if (defname == "size") {
				GetDefNumber(size, pdef, filename);
				size *= (float)scale;
			}
			else if (defname == "hull_factor") {
				GetDefNumber(hull, pdef, filename);
			}
			else if (defname == "jump_time") {
				GetDefNumber(countdown, pdef, filename);
			}
			else if (defname == "countdown") {
				GetDefNumber(countdown, pdef, filename);
			}

			else if (defname == "emcon_1") {
				GetDefNumber(emcon_1, pdef, filename);
			}

			else if (defname == "emcon_2") {
				GetDefNumber(emcon_2, pdef, filename);
			}

			else if (defname == "emcon_3") {
				GetDefNumber(emcon_3, pdef, filename);
			}
		}
	}

	QuantumDrive* drive = new  QuantumDrive((QuantumDrive::SUBTYPE)subtype, capacity, consumption);
	drive->SetSourceIndex(reactors.size() - 1);
	drive->Mount(loc, size, hull);
	drive->SetCountdown(countdown);

	if (design_name.length()) {
		SystemDesign* sd = SystemDesign::Find(design_name);
		if (sd)
			drive->SetDesign(sd);
	}

	if (abrv.length())
		drive->SetAbbreviation(abrv);

	if (emcon_1 >= 0 && emcon_1 <= 100)
		drive->SetEMCONPower(1, emcon_1);

	if (emcon_2 >= 0 && emcon_2 <= 100)
		drive->SetEMCONPower(1, emcon_2);

	if (emcon_3 >= 0 && emcon_3 <= 100)
		drive->SetEMCONPower(1, emcon_3);

	quantum_drive = drive;
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseFarcaster(TermStruct* val)
{
	Text     design_name;
	double   capacity = 300e3;
	double   consumption = 15e3;  // twenty second recharge
	int      napproach = 0;
	Vec3     approach[Farcaster::NUM_APPROACH_PTS];
	Vec3     loc(0.0f, 0.0f, 0.0f);
	Vec3     start(0.0f, 0.0f, 0.0f);
	Vec3     end(0.0f, 0.0f, 0.0f);
	float    size = 0.0f;
	float    hull = 0.5f;
	int      emcon_1 = -1;
	int      emcon_2 = -1;
	int      emcon_3 = -1;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "design") {
				GetDefText(design_name, pdef, filename);
			}
			else if (defname == "capacity") {
				GetDefNumber(capacity, pdef, filename);
			}
			else if (defname == "consumption") {
				GetDefNumber(consumption, pdef, filename);
			}
			else if (defname == "loc") {
				GetDefVec(loc, pdef, filename);
				loc *= (float)scale;
			}
			else if (defname == "size") {
				GetDefNumber(size, pdef, filename);
				size *= (float)scale;
			}
			else if (defname == "hull_factor") {
				GetDefNumber(hull, pdef, filename);
			}

			else if (defname == "start") {
				GetDefVec(start, pdef, filename);
				start *= (float)scale;
			}
			else if (defname == "end") {
				GetDefVec(end, pdef, filename);
				end *= (float)scale;
			}
			else if (defname == "approach") {
				if (napproach < Farcaster::NUM_APPROACH_PTS) {
					GetDefVec(approach[napproach], pdef, filename);
					approach[napproach++] *= (float)scale;
				}
				else {
					Print("WARNING: farcaster approach point ignored in '%s' (max=%d)\n",
						filename, Farcaster::NUM_APPROACH_PTS);
				}
			}

			else if (defname == "emcon_1") {
				GetDefNumber(emcon_1, pdef, filename);
			}

			else if (defname == "emcon_2") {
				GetDefNumber(emcon_2, pdef, filename);
			}

			else if (defname == "emcon_3") {
				GetDefNumber(emcon_3, pdef, filename);
			}
		}
	}

	Farcaster* caster = new  Farcaster(capacity, consumption);
	caster->SetSourceIndex(reactors.size() - 1);
	caster->Mount(loc, size, hull);

	if (design_name.length()) {
		SystemDesign* sd = SystemDesign::Find(design_name);
		if (sd)
			caster->SetDesign(sd);
	}

	caster->SetStartPoint(start);
	caster->SetEndPoint(end);

	for (int i = 0; i < napproach; i++)
		caster->SetApproachPoint(i, approach[i]);

	if (emcon_1 >= 0 && emcon_1 <= 100)
		caster->SetEMCONPower(1, emcon_1);

	if (emcon_2 >= 0 && emcon_2 <= 100)
		caster->SetEMCONPower(1, emcon_2);

	if (emcon_3 >= 0 && emcon_3 <= 100)
		caster->SetEMCONPower(1, emcon_3);

	farcaster = caster;
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseThruster(TermStruct* val)
{
	if (thruster) {
		Print("WARNING: additional thruster ignored in '%s'\n", filename);
		return;
	}

	double thrust = 100;

	Vec3  loc(0.0f, 0.0f, 0.0f);
	float size = 0.0f;
	float hull = 0.5f;
	Text  design_name;
	float tscale = 1.0f;
	int   emcon_1 = -1;
	int   emcon_2 = -1;
	int   emcon_3 = -1;
	int   dtype = 0;

	Thruster* drive = 0;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);


			if (defname == "type") {
				TermText* tname = pdef->term()->isText();

				if (tname) {
					Text tval = tname->value();
					tval.setSensitive(false);

					if (tval == "Plasma")       dtype = Drive::PLASMA;
					else if (tval == "Fusion")  dtype = Drive::FUSION;
					else if (tval == "Alien")   dtype = Drive::GREEN;
					else if (tval == "Green")   dtype = Drive::GREEN;
					else if (tval == "Red")     dtype = Drive::RED;
					else if (tval == "Blue")    dtype = Drive::BLUE;
					else if (tval == "Yellow")  dtype = Drive::YELLOW;
					else if (tval == "Stealth") dtype = Drive::STEALTH;

					else Print("WARNING: unknown thruster type '%s' in '%s'\n", tname->value().data(), filename);
				}
			}

			else if (defname == "thrust") {
				GetDefNumber(thrust, pdef, filename);
			}

			else if (defname == "design") {
				GetDefText(design_name, pdef, filename);
			}

			else if (defname == "loc") {
				GetDefVec(loc, pdef, filename);
				loc *= (float)scale;
			}
			else if (defname == "size") {
				GetDefNumber(size, pdef, filename);
				size *= (float)scale;
			}
			else if (defname == "hull_factor") {
				GetDefNumber(hull, pdef, filename);
			}
			else if (defname == "scale") {
				GetDefNumber(tscale, pdef, filename);
			}
			else if (defname.contains("port") && pdef->term()) {
				Vec3  port;
				float port_scale = 0;
				DWORD fire = 0;

				if (pdef->term()->isArray()) {
					GetDefVec(port, pdef, filename);
					port *= scale;
					port_scale = tscale;
				}

				else if (pdef->term()->isStruct()) {
					TermStruct* val = pdef->term()->isStruct();

					for (int i = 0; i < val->elements()->size(); i++) {
						TermDef* pdef2 = val->elements()->at(i)->isDef();
						if (pdef2) {
							if (pdef2->name()->value() == "loc") {
								GetDefVec(port, pdef2, filename);
								port *= scale;
							}

							else if (pdef2->name()->value() == "fire") {
								GetDefNumber(fire, pdef2, filename);
							}

							else if (pdef2->name()->value() == "scale") {
								GetDefNumber(port_scale, pdef2, filename);
							}
						}
					}

					if (port_scale <= 0)
						port_scale = tscale;
				}

				if (!drive)
					drive = new  Thruster(dtype, thrust, tscale);

				if (defname == "port" || defname == "port_bottom")
					drive->AddPort(Thruster::BOTTOM, port, fire, port_scale);

				else if (defname == "port_top")
					drive->AddPort(Thruster::TOP, port, fire, port_scale);

				else if (defname == "port_left")
					drive->AddPort(Thruster::LEFT, port, fire, port_scale);

				else if (defname == "port_right")
					drive->AddPort(Thruster::RIGHT, port, fire, port_scale);

				else if (defname == "port_fore")
					drive->AddPort(Thruster::FORE, port, fire, port_scale);

				else if (defname == "port_aft")
					drive->AddPort(Thruster::AFT, port, fire, port_scale);
			}

			else if (defname == "emcon_1") {
				GetDefNumber(emcon_1, pdef, filename);
			}

			else if (defname == "emcon_2") {
				GetDefNumber(emcon_2, pdef, filename);
			}

			else if (defname == "emcon_3") {
				GetDefNumber(emcon_3, pdef, filename);
			}
		}
	}

	if (!drive)
		drive = new  Thruster(dtype, thrust, tscale);
	drive->SetSourceIndex(reactors.size() - 1);
	drive->Mount(loc, size, hull);

	if (design_name.length()) {
		SystemDesign* sd = SystemDesign::Find(design_name);
		if (sd)
			drive->SetDesign(sd);
	}

	if (emcon_1 >= 0 && emcon_1 <= 100)
		drive->SetEMCONPower(1, emcon_1);

	if (emcon_2 >= 0 && emcon_2 <= 100)
		drive->SetEMCONPower(1, emcon_2);

	if (emcon_3 >= 0 && emcon_3 <= 100)
		drive->SetEMCONPower(1, emcon_3);

	thruster = drive;
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseNavlight(TermStruct* val)
{
	Text  dname;
	Text  dabrv;
	Text  design_name;
	int   nlights = 0;
	float dscale = 1.0f;
	float period = 10.0f;
	Vec3  bloc[NavLight::MAX_LIGHTS];
	int   btype[NavLight::MAX_LIGHTS];
	DWORD pattern[NavLight::MAX_LIGHTS];

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "name")
				GetDefText(dname, pdef, filename);
			else if (defname == "abrv")
				GetDefText(dabrv, pdef, filename);

			else if (defname == "design") {
				GetDefText(design_name, pdef, filename);
			}

			else if (defname == "scale") {
				GetDefNumber(dscale, pdef, filename);
			}
			else if (defname == "period") {
				GetDefNumber(period, pdef, filename);
			}
			else if (defname == "light") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					Print("WARNING: light struct missing for ship '%s' in '%s'\n", name, filename);
				}
				else {
					TermStruct* val = pdef->term()->isStruct();

					Vec3  loc;
					int   t = 0;
					DWORD ptn = 0;

					for (int i = 0; i < val->elements()->size(); i++) {
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef) {
							Text defname = pdef->name()->value();
							defname.setSensitive(false);

							if (defname == "type") {
								GetDefNumber(t, pdef, filename);
							}
							else if (defname == "loc") {
								GetDefVec(loc, pdef, filename);
							}
							else if (defname == "pattern") {
								GetDefNumber(ptn, pdef, filename);
							}
						}
					}

					if (t < 1 || t > 4)
						t = 1;

					if (nlights < NavLight::MAX_LIGHTS) {
						bloc[nlights] = loc * scale;
						btype[nlights] = t - 1;
						pattern[nlights] = ptn;
						nlights++;
					}
					else {
						Print("WARNING: Too many lights ship '%s' in '%s'\n", name, filename);
					}
				}
			}
		}
	}

	NavLight* nav = new  NavLight(period, dscale);
	if (dname.length()) nav->SetName(dname);
	if (dabrv.length()) nav->SetAbbreviation(dabrv);

	if (design_name.length()) {
		SystemDesign* sd = SystemDesign::Find(design_name);
		if (sd)
			nav->SetDesign(sd);
	}

	for (int i = 0; i < nlights; i++)
		nav->AddBeacon(bloc[i], pattern[i], btype[i]);

	navlights.append(nav);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseFlightDeck(TermStruct* val)
{
	Text  dname;
	Text  dabrv;
	Text  design_name;
	float dscale = 1.0f;
	float az = 0.0f;
	int   etype = 0;

	bool  launch = false;
	bool  recovery = false;
	int   nslots = 0;
	int   napproach = 0;
	int   nrunway = 0;
	DWORD filters[10];
	Vec3  spots[10];
	Vec3  approach[FlightDeck::NUM_APPROACH_PTS];
	Vec3  runway[2];
	Vec3  loc(0, 0, 0);
	Vec3  start(0, 0, 0);
	Vec3  end(0, 0, 0);
	Vec3  cam(0, 0, 0);
	Vec3  box(0, 0, 0);
	float cycle_time = 0.0f;
	float size = 0.0f;
	float hull = 0.5f;

	float light = 0.0f;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "name")
				GetDefText(dname, pdef, filename);
			else if (defname == "abrv")
				GetDefText(dabrv, pdef, filename);
			else if (defname == "design")
				GetDefText(design_name, pdef, filename);

			else if (defname == "start") {
				GetDefVec(start, pdef, filename);
				start *= (float)scale;
			}
			else if (defname == "end") {
				GetDefVec(end, pdef, filename);
				end *= (float)scale;
			}
			else if (defname == "cam") {
				GetDefVec(cam, pdef, filename);
				cam *= (float)scale;
			}
			else if (defname == "box" || defname == "bounding_box") {
				GetDefVec(box, pdef, filename);
				box *= (float)scale;
			}
			else if (defname == "approach") {
				if (napproach < FlightDeck::NUM_APPROACH_PTS) {
					GetDefVec(approach[napproach], pdef, filename);
					approach[napproach++] *= (float)scale;
				}
				else {
					Print("WARNING: flight deck approach point ignored in '%s' (max=%d)\n",
						filename, FlightDeck::NUM_APPROACH_PTS);
				}
			}
			else if (defname == "runway") {
				GetDefVec(runway[nrunway], pdef, filename);
				runway[nrunway++] *= (float)scale;
			}
			else if (defname == "spot") {
				if (pdef->term()->isStruct()) {
					TermStruct* s = pdef->term()->isStruct();
					for (int i = 0; i < s->elements()->size(); i++) {
						TermDef* d = s->elements()->at(i)->isDef();
						if (d) {
							if (d->name()->value() == "loc") {
								GetDefVec(spots[nslots], d, filename);
								spots[nslots] *= (float)scale;
							}
							else if (d->name()->value() == "filter") {
								GetDefNumber(filters[nslots], d, filename);
							}
						}
					}

					nslots++;
				}

				else if (pdef->term()->isArray()) {
					GetDefVec(spots[nslots], pdef, filename);
					spots[nslots] *= (float)scale;
					filters[nslots++] = 0xf;
				}
			}

			else if (defname == "light") {
				GetDefNumber(light, pdef, filename);
			}

			else if (defname == "cycle_time") {
				GetDefNumber(cycle_time, pdef, filename);
			}

			else if (defname == "launch") {
				GetDefBool(launch, pdef, filename);
			}

			else if (defname == "recovery") {
				GetDefBool(recovery, pdef, filename);
			}

			else if (defname == "azimuth") {
				GetDefNumber(az, pdef, filename);
				if (degrees) az *= (float)DEGREES;
			}

			else if (defname == "loc") {
				GetDefVec(loc, pdef, filename);
				loc *= (float)scale;
			}
			else if (defname == "size") {
				GetDefNumber(size, pdef, filename);
				size *= (float)scale;
			}
			else if (defname == "hull_factor") {
				GetDefNumber(hull, pdef, filename);
			}
			else if (defname == "explosion") {
				GetDefNumber(etype, pdef, filename);
			}
		}
	}

	FlightDeck* deck = new  FlightDeck();
	deck->Mount(loc, size, hull);
	if (dname.length()) deck->SetName(dname);
	if (dabrv.length()) deck->SetAbbreviation(dabrv);

	if (design_name.length()) {
		SystemDesign* sd = SystemDesign::Find(design_name);
		if (sd)
			deck->SetDesign(sd);
	}

	if (launch)
		deck->SetLaunchDeck();
	else if (recovery)
		deck->SetRecoveryDeck();

	deck->SetAzimuth(az);
	deck->SetBoundingBox(box);
	deck->SetStartPoint(start);
	deck->SetEndPoint(end);
	deck->SetCamLoc(cam);
	deck->SetExplosionType(etype);

	if (light > 0)
		deck->SetLight(light);

	for (int i = 0; i < napproach; i++)
		deck->SetApproachPoint(i, approach[i]);

	for (int i = 0; i < nrunway; i++)
		deck->SetRunwayPoint(i, runway[i]);

	for (int i = 0; i < nslots; i++)
		deck->AddSlot(spots[i], filters[i]);

	if (cycle_time > 0)
		deck->SetCycleTime(cycle_time);

	flight_decks.append(deck);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseLandingGear(TermStruct* val)
{
	Text  dname;
	Text  dabrv;
	Text  design_name;
	int   ngear = 0;
	Vec3  start[LandingGear::MAX_GEAR];
	Vec3  end[LandingGear::MAX_GEAR];
	SimModel* model[LandingGear::MAX_GEAR];

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "name")
				GetDefText(dname, pdef, filename);
			else if (defname == "abrv")
				GetDefText(dabrv, pdef, filename);

			else if (defname == "design") {
				GetDefText(design_name, pdef, filename);
			}

			else if (defname == "gear") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					Print("WARNING: gear struct missing for ship '%s' in '%s'\n", name, filename);
				}
				else {
					TermStruct* val = pdef->term()->isStruct();

					Vec3  v1, v2;
					char  mod_name[256];

					ZeroMemory(mod_name, sizeof(mod_name));

					for (int i = 0; i < val->elements()->size(); i++) {
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef) {
							defname = pdef->name()->value();
							defname.setSensitive(false);

							if (defname == "model") {
								GetDefText(mod_name, pdef, filename);
							}
							else if (defname == "start") {
								GetDefVec(v1, pdef, filename);
							}
							else if (defname == "end") {
								GetDefVec(v2, pdef, filename);
							}
						}
					}

					if (ngear < LandingGear::MAX_GEAR) {
						SimModel* m = new  Model;
						if (!m->Load(mod_name, scale)) {
							Print("WARNING: Could notxload landing gear model '%s'\n", mod_name);
							delete m;
							m = 0;
						}
						else {
							model[ngear] = m;
							start[ngear] = v1 * scale;
							end[ngear] = v2 * scale;
							ngear++;
						}
					}
					else {
						Print("WARNING: Too many landing gear ship '%s' in '%s'\n", name, filename);
					}
				}
			}
		}
	}

	gear = new  LandingGear();
	if (dname.length()) gear->SetName(dname);
	if (dabrv.length()) gear->SetAbbreviation(dabrv);

	if (design_name.length()) {
		SystemDesign* sd = SystemDesign::Find(design_name);
		if (sd)
			gear->SetDesign(sd);
	}

	for (int i = 0; i < ngear; i++)
		gear->AddGear(model[i], start[i], end[i]);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseWeapon(TermStruct* val)
{
	Text  wtype;
	Text  wname;
	Text  wabrv;
	Text  design_name;
	Text  group_name;
	int   nmuz = 0;
	Vec3  muzzles[Weapon::MAX_BARRELS];
	Vec3  loc(0.0f, 0.0f, 0.0f);
	float size = 0.0f;
	float hull = 0.5f;
	float az = 0.0f;
	float el = 0.0f;
	float az_max = 1e6f;
	float az_min = 1e6f;
	float el_max = 1e6f;
	float el_min = 1e6f;
	float az_rest = 1e6f;
	float el_rest = 1e6f;
	int   etype = 0;
	int   emcon_1 = -1;
	int   emcon_2 = -1;
	int   emcon_3 = -1;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "type")
				GetDefText(wtype, pdef, filename);
			else if (defname == "name")
				GetDefText(wname, pdef, filename);
			else if (defname == "abrv")
				GetDefText(wabrv, pdef, filename);
			else if (defname == "design")
				GetDefText(design_name, pdef, filename);
			else if (defname == "group")
				GetDefText(group_name, pdef, filename);

			else if (defname == "muzzle") {
				if (nmuz < Weapon::MAX_BARRELS) {
					GetDefVec(muzzles[nmuz], pdef, filename);
					nmuz++;
				}
				else {
					Print("WARNING: too many muzzles (max=%d) for weapon in '%s'\n", filename, Weapon::MAX_BARRELS);
				}
			}
			else if (defname == "loc") {
				GetDefVec(loc, pdef, filename);
				loc *= (float)scale;
			}
			else if (defname == "size") {
				GetDefNumber(size, pdef, filename);
				size *= (float)scale;
			}
			else if (defname == "hull_factor") {
				GetDefNumber(hull, pdef, filename);
			}
			else if (defname == "azimuth") {
				GetDefNumber(az, pdef, filename);
				if (degrees) az *= (float)DEGREES;
			}
			else if (defname == "elevation") {
				GetDefNumber(el, pdef, filename);
				if (degrees) el *= (float)DEGREES;
			}

			else if (defname == ("aim_az_max")) {
				GetDefNumber(az_max, pdef, filename);
				if (degrees) az_max *= (float)DEGREES;
				az_min = 0.0f - az_max;
			}

			else if (defname == ("aim_el_max")) {
				GetDefNumber(el_max, pdef, filename);
				if (degrees) el_max *= (float)DEGREES;
				el_min = 0.0f - el_max;
			}

			else if (defname == ("aim_az_min")) {
				GetDefNumber(az_min, pdef, filename);
				if (degrees) az_min *= (float)DEGREES;
			}

			else if (defname == ("aim_el_min")) {
				GetDefNumber(el_min, pdef, filename);
				if (degrees) el_min *= (float)DEGREES;
			}

			else if (defname == ("aim_az_rest")) {
				GetDefNumber(az_rest, pdef, filename);
				if (degrees) az_rest *= (float)DEGREES;
			}

			else if (defname == ("aim_el_rest")) {
				GetDefNumber(el_rest, pdef, filename);
				if (degrees) el_rest *= (float)DEGREES;
			}

			else if (defname == "rest_azimuth") {
				GetDefNumber(az_rest, pdef, filename);
				if (degrees) az_rest *= (float)DEGREES;
			}
			else if (defname == "rest_elevation") {
				GetDefNumber(el_rest, pdef, filename);
				if (degrees) el_rest *= (float)DEGREES;
			}
			else if (defname == "explosion") {
				GetDefNumber(etype, pdef, filename);
			}

			else if (defname == "emcon_1") {
				GetDefNumber(emcon_1, pdef, filename);
			}

			else if (defname == "emcon_2") {
				GetDefNumber(emcon_2, pdef, filename);
			}

			else if (defname == "emcon_3") {
				GetDefNumber(emcon_3, pdef, filename);
			}
			else {
				Print("WARNING: unknown weapon parameter '%s' in '%s'\n",
					defname.data(), filename);
			}
		}
	}

	WeaponDesign* meta = WeaponDesign::Find(wtype);
	if (!meta) {
		Print("WARNING: unusual weapon name '%s' in '%s'\n", (const char*)wtype, filename);
	}
	else {
		// non-turret weapon muzzles are relative to ship scale:
		if (meta->turret_model == 0) {
			for (int i = 0; i < nmuz; i++)
				muzzles[i] *= (float)scale;
		}

		// turret weapon muzzles are relative to weapon scale:
		else {
			for (int i = 0; i < nmuz; i++)
				muzzles[i] *= (float)meta->scale;
		}

		Weapon* gun = new  Weapon(meta, nmuz, muzzles, az, el);
		gun->SetSourceIndex(reactors.size() - 1);
		gun->Mount(loc, size, hull);

		if (az_max < 1e6)    gun->SetAzimuthMax(az_max);
		if (az_min < 1e6)    gun->SetAzimuthMin(az_min);
		if (az_rest < 1e6)   gun->SetRestAzimuth(az_rest);

		if (el_max < 1e6)    gun->SetElevationMax(el_max);
		if (el_min < 1e6)    gun->SetElevationMin(el_min);
		if (el_rest < 1e6)   gun->SetRestElevation(el_rest);

		if (emcon_1 >= 0 && emcon_1 <= 100)
			gun->SetEMCONPower(1, emcon_1);

		if (emcon_2 >= 0 && emcon_2 <= 100)
			gun->SetEMCONPower(1, emcon_2);

		if (emcon_3 >= 0 && emcon_3 <= 100)
			gun->SetEMCONPower(1, emcon_3);

		if (wname.length()) gun->SetName(wname);
		if (wabrv.length()) gun->SetAbbreviation(wabrv);

		if (design_name.length()) {
			SystemDesign* sd = SystemDesign::Find(design_name);
			if (sd)
				gun->SetDesign(sd);
		}

		if (group_name.length()) {
			gun->SetGroup(group_name);
		}

		gun->SetExplosionType(etype);

		if (meta->decoy_type && !decoy)
			decoy = gun;
		else if (meta->probe && !probe)
			probe = gun;
		else
			weapons.append(gun);
	}

	DataLoader* loader = DataLoader::GetLoader();
	loader->SetDataPath(path_name);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseHardPoint(TermStruct* val)
{
	Text  wtypes[8];
	Text  wname;
	Text  wabrv;
	Text  design;
	Vec3  muzzle;
	Vec3  loc(0.0f, 0.0f, 0.0f);
	float size = 0.0f;
	float hull = 0.5f;
	float az = 0.0f;
	float el = 0.0f;
	int   ntypes = 0;
	int   emcon_1 = -1;
	int   emcon_2 = -1;
	int   emcon_3 = -1;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "type")
				GetDefText(wtypes[ntypes++], pdef, filename);
			else if (defname == "name")
				GetDefText(wname, pdef, filename);
			else if (defname == "abrv")
				GetDefText(wabrv, pdef, filename);
			else if (defname == "design")
				GetDefText(design, pdef, filename);

			else if (defname == "muzzle") {
				GetDefVec(muzzle, pdef, filename);
				muzzle *= (float)scale;
			}
			else if (defname == "loc") {
				GetDefVec(loc, pdef, filename);
				loc *= (float)scale;
			}
			else if (defname == "size") {
				GetDefNumber(size, pdef, filename);
				size *= (float)scale;
			}
			else if (defname == "hull_factor") {
				GetDefNumber(hull, pdef, filename);
			}
			else if (defname == "azimuth") {
				GetDefNumber(az, pdef, filename);
				if (degrees) az *= (float)DEGREES;
			}
			else if (defname == "elevation") {
				GetDefNumber(el, pdef, filename);
				if (degrees) el *= (float)DEGREES;
			}

			else if (defname == "emcon_1") {
				GetDefNumber(emcon_1, pdef, filename);
			}

			else if (defname == "emcon_2") {
				GetDefNumber(emcon_2, pdef, filename);
			}

			else if (defname == "emcon_3") {
				GetDefNumber(emcon_3, pdef, filename);
			}
			else {
				Print("WARNING: unknown weapon parameter '%s' in '%s'\n",
					defname.data(), filename);
			}
		}
	}

	HardPoint* hp = new  HardPoint(muzzle, az, el);
	if (hp) {
		for (int i = 0; i < ntypes; i++) {
			WeaponDesign* meta = WeaponDesign::Find(wtypes[i]);
			if (!meta) {
				Print("WARNING: unusual weapon name '%s' in '%s'\n", (const char*)wtypes[i], filename);
			}
			else {
				hp->AddDesign(meta);
			}
		}

		hp->Mount(loc, size, hull);
		if (wname.length())  hp->SetName(wname);
		if (wabrv.length())  hp->SetAbbreviation(wabrv);
		if (design.length()) hp->SetDesign(design);

		hard_points.append(hp);
	}

	DataLoader* loader = DataLoader::GetLoader();
	loader->SetDataPath(path_name);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseLoadout(TermStruct* val)
{
	ShipLoad* load = new  ShipLoad;

	if (!load) return;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "name")
				GetDefText(load->name, pdef, filename);

			else if (defname == "stations")
				GetDefArray(load->load, 16, pdef, filename);

			else
				Print("WARNING: unknown loadout parameter '%s' in '%s'\n",
					defname.data(), filename);
		}
	}

	loadouts.append(load);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseSensor(TermStruct* val)
{
	Text  design_name;
	Vec3  loc(0.0f, 0.0f, 0.0f);
	float size = 0.0f;
	float hull = 0.5f;
	int   nranges = 0;
	float ranges[8];
	int   emcon_1 = -1;
	int   emcon_2 = -1;
	int   emcon_3 = -1;

	ZeroMemory(ranges, sizeof(ranges));

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "range") {
				GetDefNumber(ranges[nranges++], pdef, filename);
			}
			else if (defname == "loc") {
				GetDefVec(loc, pdef, filename);
				loc *= (float)scale;
			}
			else if (defname == "size") {
				size *= (float)scale;
				GetDefNumber(size, pdef, filename);
			}
			else if (defname == "hull_factor") {
				GetDefNumber(hull, pdef, filename);
			}
			else if (defname == "design") {
				GetDefText(design_name, pdef, filename);
			}
			else if (defname == "emcon_1") {
				GetDefNumber(emcon_1, pdef, filename);
			}

			else if (defname == "emcon_2") {
				GetDefNumber(emcon_2, pdef, filename);
			}

			else if (defname == "emcon_3") {
				GetDefNumber(emcon_3, pdef, filename);
			}
		}
	}

	if (!sensor) {
		sensor = new  Sensor();

		if (design_name.length()) {
			SystemDesign* sd = SystemDesign::Find(design_name);
			if (sd)
				sensor->SetDesign(sd);
		}

		for (int i = 0; i < nranges; i++)
			sensor->AddRange(ranges[i]);

		if (emcon_1 >= 0 && emcon_1 <= 100)
			sensor->SetEMCONPower(1, emcon_1);

		if (emcon_2 >= 0 && emcon_2 <= 100)
			sensor->SetEMCONPower(1, emcon_2);

		if (emcon_3 >= 0 && emcon_3 <= 100)
			sensor->SetEMCONPower(1, emcon_3);

		sensor->Mount(loc, size, hull);
		sensor->SetSourceIndex(reactors.size() - 1);
	}
	else {
		Print("WARNING: additional sensor ignored in '%s'\n", filename);
	}
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseNavsys(TermStruct* val)
{
	Text  design_name;
	Vec3  loc(0.0f, 0.0f, 0.0f);
	float size = 0.0f;
	float hull = 0.5f;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "loc") {
				GetDefVec(loc, pdef, filename);
				loc *= (float)scale;
			}
			else if (defname == "size") {
				size *= (float)scale;
				GetDefNumber(size, pdef, filename);
			}
			else if (defname == "hull_factor") {
				GetDefNumber(hull, pdef, filename);
			}
			else if (defname == "design")
				GetDefText(design_name, pdef, filename);
		}
	}

	if (!navsys) {
		navsys = new  NavSystem;

		if (design_name.length()) {
			SystemDesign* sd = SystemDesign::Find(design_name);
			if (sd)
				navsys->SetDesign(sd);
		}

		navsys->Mount(loc, size, hull);
		navsys->SetSourceIndex(reactors.size() - 1);
	}
	else {
		Print("WARNING: additional nav system ignored in '%s'\n", filename);
	}
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseComputer(TermStruct* val)
{
	Text  comp_name("Computer");
	Text  comp_abrv("Comp");
	Text  design_name;
	int   comp_type = 1;
	Vec3  loc(0.0f, 0.0f, 0.0f);
	float size = 0.0f;
	float hull = 0.5f;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "name") {
				GetDefText(comp_name, pdef, filename);
			}
			else if (defname == "abrv") {
				GetDefText(comp_abrv, pdef, filename);
			}
			else if (defname == "design") {
				GetDefText(design_name, pdef, filename);
			}
			else if (defname == "type") {
				GetDefNumber(comp_type, pdef, filename);
			}
			else if (defname == "loc") {
				GetDefVec(loc, pdef, filename);
				loc *= (float)scale;
			}
			else if (defname == "size") {
				size *= (float)scale;
				GetDefNumber(size, pdef, filename);
			}
			else if (defname == "hull_factor") {
				GetDefNumber(hull, pdef, filename);
			}
		}
	}

	Computer* comp = new  Computer(comp_type, comp_name);
	comp->Mount(loc, size, hull);
	comp->SetAbbreviation(comp_abrv);
	comp->SetSourceIndex(reactors.size() - 1);

	if (design_name.length()) {
		SystemDesign* sd = SystemDesign::Find(design_name);
		if (sd)
			comp->SetDesign(sd);
	}

	computers.append(comp);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseShield(TermStruct* val)
{
	Text     dname;
	Text     dabrv;
	Text     design_name;
	Text     model_name;
	double   factor = 0;
	double   capacity = 0;
	double   consumption = 0;
	double   cutoff = 0;
	double   curve = 0;
	double   def_cost = 1;
	int      shield_type = 0;
	Vec3     loc(0.0f, 0.0f, 0.0f);
	float    size = 0.0f;
	float    hull = 0.5f;
	int      etype = 0;
	bool     shield_capacitor = false;
	bool     shield_bubble = false;
	int   emcon_1 = -1;
	int   emcon_2 = -1;
	int   emcon_3 = -1;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "type") {
				GetDefNumber(shield_type, pdef, filename);
			}
			else if (defname == "name")
				GetDefText(dname, pdef, filename);
			else if (defname == "abrv")
				GetDefText(dabrv, pdef, filename);
			else if (defname == "design")
				GetDefText(design_name, pdef, filename);
			else if (defname == "model")
				GetDefText(model_name, pdef, filename);

			else if (defname == "loc") {
				GetDefVec(loc, pdef, filename);
				loc *= (float)scale;
			}
			else if (defname == "size") {
				GetDefNumber(size, pdef, filename);
				size *= (float)scale;
			}
			else if (defname == "hull_factor")
				GetDefNumber(hull, pdef, filename);

			else if (defname.contains("factor"))
				GetDefNumber(factor, pdef, filename);
			else if (defname.contains("cutoff"))
				GetDefNumber(cutoff, pdef, filename);
			else if (defname.contains("curve"))
				GetDefNumber(curve, pdef, filename);
			else if (defname.contains("capacitor"))
				GetDefBool(shield_capacitor, pdef, filename);
			else if (defname.contains("bubble"))
				GetDefBool(shield_bubble, pdef, filename);
			else if (defname == "capacity")
				GetDefNumber(capacity, pdef, filename);
			else if (defname == "consumption")
				GetDefNumber(consumption, pdef, filename);
			else if (defname == "deflection_cost")
				GetDefNumber(def_cost, pdef, filename);
			else if (defname == "explosion")
				GetDefNumber(etype, pdef, filename);

			else if (defname == "emcon_1") {
				GetDefNumber(emcon_1, pdef, filename);
			}

			else if (defname == "emcon_2") {
				GetDefNumber(emcon_2, pdef, filename);
			}

			else if (defname == "emcon_3") {
				GetDefNumber(emcon_3, pdef, filename);
			}

			else if (defname == "bolt_hit_sound") {
				GetDefText(bolt_hit_sound, pdef, filename);
			}

			else if (defname == "beam_hit_sound") {
				GetDefText(beam_hit_sound, pdef, filename);
			}
		}
	}

	if (!shield) {
		if (shield_type) {
			shield = new  Shield((Shield::SUBTYPE)shield_type);
			shield->SetSourceIndex(reactors.size() - 1);
			shield->Mount(loc, size, hull);
			if (dname.length()) shield->SetName(dname);
			if (dabrv.length()) shield->SetAbbreviation(dabrv);

			if (design_name.length()) {
				SystemDesign* sd = SystemDesign::Find(design_name);
				if (sd)
					shield->SetDesign(sd);
			}

			shield->SetExplosionType(etype);
			shield->SetShieldCapacitor(shield_capacitor);
			shield->SetShieldBubble(shield_bubble);

			if (factor > 0) shield->SetShieldFactor(factor);
			if (capacity > 0) shield->SetCapacity(capacity);
			if (cutoff > 0) shield->SetShieldCutoff(cutoff);
			if (consumption > 0) shield->SetConsumption(consumption);
			if (def_cost > 0) shield->SetDeflectionCost(def_cost);
			if (curve > 0) shield->SetShieldCurve(curve);

			if (emcon_1 >= 0 && emcon_1 <= 100)
				shield->SetEMCONPower(1, emcon_1);

			if (emcon_2 >= 0 && emcon_2 <= 100)
				shield->SetEMCONPower(1, emcon_2);

			if (emcon_3 >= 0 && emcon_3 <= 100)
				shield->SetEMCONPower(1, emcon_3);

			if (model_name.length()) {
				shield_model = new  Model;
				if (!shield_model->Load(model_name, scale)) {
					Print("ERROR: Could notxload shield model '%s'\n", model_name.data());
					delete shield_model;
					shield_model = 0;
					valid = false;
				}
				else {
					shield_model->SetDynamic(true);
					shield_model->SetLuminous(true);
				}
			}

			DataLoader* loader = DataLoader::GetLoader();
			DWORD       SOUND_FLAGS = USound::LOCALIZED | USound::LOC_3D;

			if (bolt_hit_sound.length()) {
				if (!loader->LoadSound(bolt_hit_sound, bolt_hit_sound_resource, SOUND_FLAGS, true)) {
					loader->SetDataPath("Sounds/");
					loader->LoadSound(bolt_hit_sound, bolt_hit_sound_resource, SOUND_FLAGS);
					loader->SetDataPath(path_name);
				}
			}

			if (beam_hit_sound.length()) {
				if (!loader->LoadSound(beam_hit_sound, beam_hit_sound_resource, SOUND_FLAGS, true)) {
					loader->SetDataPath("Sounds/");
					loader->LoadSound(beam_hit_sound, beam_hit_sound_resource, SOUND_FLAGS);
					loader->SetDataPath(path_name);
				}
			}
		}
		else {
			Print("WARNING: invalid shield type in '%s'\n", filename);
		}
	}
	else {
		Print("WARNING: additional shield ignored in '%s'\n", filename);
	}
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseDeathSpiral(TermStruct* val)
{
	int   exp_index = -1;
	int   debris_index = -1;
	int   fire_index = -1;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* def = val->elements()->at(i)->isDef();
		if (def) {
			Text defname = def->name()->value();
			defname.setSensitive(false);

			if (defname == "time") {
				GetDefNumber(death_spiral_time, def, filename);
			}

			else if (defname == "explosion") {
				if (!def->term() || !def->term()->isStruct()) {
					Print("WARNING: explosion struct missing in '%s'\n", filename);
				}
				else {
					TermStruct* val = def->term()->isStruct();
					ParseExplosion(val, ++exp_index);
				}
			}

			// BACKWARD COMPATIBILITY:
			else if (defname == "explosion_type") {
				GetDefNumber(explosion[++exp_index].type, def, filename);
			}

			else if (defname == "explosion_time") {
				GetDefNumber(explosion[exp_index].time, def, filename);
			}

			else if (defname == "explosion_loc") {
				GetDefVec(explosion[exp_index].loc, def, filename);
				explosion[exp_index].loc *= (float)scale;
			}

			else if (defname == "final_type") {
				GetDefNumber(explosion[++exp_index].type, def, filename);
				explosion[exp_index].final = true;
			}

			else if (defname == "final_loc") {
				GetDefVec(explosion[exp_index].loc, def, filename);
				explosion[exp_index].loc *= (float)scale;
			}


			else if (defname == "debris") {
				if (def->term() && def->term()->isText()) {
					Text model_name;
					GetDefText(model_name, def, filename);
					SimModel* model = new  Model;
					if (!model->Load(model_name, scale)) {
						Print("Could notxload debris model '%s'\n", model_name.data());
						delete model;
						return;
					}

					PrepareModel(*model);
					debris[++debris_index].model = model;
					fire_index = -1;
				}
				else if (!def->term() || !def->term()->isStruct()) {
					Print("WARNING: debris struct missing in '%s'\n", filename);
				}
				else {
					TermStruct* val = def->term()->isStruct();
					ParseDebris(val, ++debris_index);
				}
			}

			else if (defname == "debris_mass") {
				GetDefNumber(debris[debris_index].mass, def, filename);
			}

			else if (defname == "debris_speed") {
				GetDefNumber(debris[debris_index].speed, def, filename);
			}

			else if (defname == "debris_drag") {
				GetDefNumber(debris[debris_index].drag, def, filename);
			}

			else if (defname == "debris_loc") {
				GetDefVec(debris[debris_index].loc, def, filename);
				debris[debris_index].loc *= (float)scale;
			}

			else if (defname == "debris_count") {
				GetDefNumber(debris[debris_index].count, def, filename);
			}

			else if (defname == "debris_life") {
				GetDefNumber(debris[debris_index].life, def, filename);
			}

			else if (defname == "debris_fire") {
				if (++fire_index < 5) {
					GetDefVec(debris[debris_index].fire_loc[fire_index], def, filename);
					debris[debris_index].fire_loc[fire_index] *= (float)scale;
				}
			}

			else if (defname == "debris_fire_type") {
				GetDefNumber(debris[debris_index].fire_type, def, filename);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseExplosion(TermStruct* val, int index)
{
	ShipExplosion* exp = &explosion[index];

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* def = val->elements()->at(i)->isDef();
		if (def) {
			Text defname = def->name()->value();
			defname.setSensitive(false);

			if (defname == "time") {
				GetDefNumber(exp->time, def, filename);
			}

			else if (defname == "type") {
				GetDefNumber(exp->type, def, filename);
			}

			else if (defname == "loc") {
				GetDefVec(exp->loc, def, filename);
				exp->loc *= (float)scale;
			}

			else if (defname == "final") {
				GetDefBool(exp->final, def, filename);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseDebris(TermStruct* val, int index)
{
	char        model_name[NAMELEN];
	int         fire_index = 0;
	ShipDebris* deb = &debris[index];

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* def = val->elements()->at(i)->isDef();
		if (def) {
			Text defname = def->name()->value();

			if (defname == "model") {
				GetDefText(model_name, def, filename);
				SimModel* model = new  Model;
				if (!model->Load(model_name, scale)) {
					Print("Could notxload debris model '%s'\n", model_name);
					delete model;
					return;
				}

				PrepareModel(*model);
				deb->model = model;
			}

			else if (defname == "mass") {
				GetDefNumber(deb->mass, def, filename);
			}

			else if (defname == "speed") {
				GetDefNumber(deb->speed, def, filename);
			}

			else if (defname == "drag") {
				GetDefNumber(deb->drag, def, filename);
			}

			else if (defname == "loc") {
				GetDefVec(deb->loc, def, filename);
				deb->loc *= (float)scale;
			}

			else if (defname == "count") {
				GetDefNumber(deb->count, def, filename);
			}

			else if (defname == "life") {
				GetDefNumber(deb->life, def, filename);
			}

			else if (defname == "fire") {
				if (fire_index < 5) {
					GetDefVec(deb->fire_loc[fire_index], def, filename);
					deb->fire_loc[fire_index] *= (float)scale;
					fire_index++;
				}
			}

			else if (defname == "fire_type") {
				GetDefNumber(deb->fire_type, def, filename);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseMap(TermStruct* val)
{
	char  sprite_name[NAMELEN];

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "sprite") {
				GetDefText(sprite_name, pdef, filename);

				Bitmap* sprite = new  Bitmap();
				DataLoader* loader = DataLoader::GetLoader();
				loader->LoadBitmap(sprite_name, *sprite, Bitmap::BMP_TRANSLUCENT);

				map_sprites.append(sprite);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseSquadron(TermStruct* val)
{
	char  name[NAMELEN];
	char  design[NAMELEN];
	int   count = 4;
	int   avail = 4;

	name[0] = 0;
	design[0] = 0;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "name") {
				GetDefText(name, pdef, filename);
			}
			else if (defname == "design") {
				GetDefText(design, pdef, filename);
			}
			else if (defname == "count") {
				GetDefNumber(count, pdef, filename);
			}
			else if (defname == "avail") {
				GetDefNumber(avail, pdef, filename);
			}
		}
	}

	ShipSquadron* s = new  ShipSquadron;
	strcpy_s(s->name, name);

	s->design = Get(design);
	s->count = count;
	s->avail = avail;

	squadrons.append(s);
}

// +--------------------------------------------------------------------+

Skin*
ShipDesign::ParseSkin(TermStruct* val)
{
	Skin* skin = 0;
	char  name[NAMELEN];

	name[0] = 0;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* def = val->elements()->at(i)->isDef();
		if (def) {
			Text defname = def->name()->value();
			defname.setSensitive(false);

			if (defname == "name") {
				GetDefText(name, def, filename);

				skin = new  Skin(name);
			}
			else if (defname == "material" || defname == "mtl") {
				if (!def->term() || !def->term()->isStruct()) {
					Print("WARNING: skin struct missing in '%s'\n", filename);
				}
				else {
					TermStruct* val = def->term()->isStruct();
					ParseSkinMtl(val, skin);
				}
			}
		}
	}

	if (skin && skin->NumCells()) {
		skins.append(skin);
	}

	else if (skin) {
		delete skin;
		skin = 0;
	}

	return skin;
}

void
ShipDesign::ParseSkinMtl(TermStruct* val, Skin* skin)
{
	Material* mtl = new  Material;
	if (mtl == nullptr)
		return;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* def = val->elements()->at(i)->isDef();
		if (def) {
			Text defname = def->name()->value();
			defname.setSensitive(false);

			if (defname == "name") {
				GetDefText(mtl->name, def, filename);
			}
			else if (defname == "Ka") {
				GetDefColor(mtl->Ka, def, filename);
			}
			else if (defname == "Kd") {
				GetDefColor(mtl->Kd, def, filename);
			}
			else if (defname == "Ks") {
				GetDefColor(mtl->Ks, def, filename);
			}
			else if (defname == "Ke") {
				GetDefColor(mtl->Ke, def, filename);
			}
			else if (defname == "Ns" || defname == "power") {
				GetDefNumber(mtl->power, def, filename);
			}
			else if (defname == "bump") {
				GetDefNumber(mtl->bump, def, filename);
			}
			else if (defname == "luminous") {
				GetDefBool(mtl->luminous, def, filename);
			}

			else if (defname == "blend") {
				if (def->term() && def->term()->isNumber())
					GetDefNumber(mtl->blend, def, filename);

				else if (def->term() && def->term()->isText()) {
					Text val;
					GetDefText(val, def, filename);
					val.setSensitive(false);

					if (val == "alpha" || val == "translucent")
						mtl->blend = Material::MTL_TRANSLUCENT;

					else if (val == "additive")
						mtl->blend = Material::MTL_ADDITIVE;

					else
						mtl->blend = Material::MTL_SOLID;
				}
			}

			else if (defname.indexOf("tex_d") == 0) {
				char tex_name[64];
				if (!GetDefText(tex_name, def, filename))
					Print("WARNING: invalid or missing tex_diffuse in '%s'\n", filename);

				DataLoader* loader = DataLoader::GetLoader();
				loader->LoadTexture(tex_name, mtl->tex_diffuse);
			}

			else if (defname.indexOf("tex_s") == 0) {
				char tex_name[64];
				if (!GetDefText(tex_name, def, filename))
					Print("WARNING: invalid or missing tex_specular in '%s'\n", filename);

				DataLoader* loader = DataLoader::GetLoader();
				loader->LoadTexture(tex_name, mtl->tex_specular);
			}

			else if (defname.indexOf("tex_b") == 0) {
				char tex_name[64];
				if (!GetDefText(tex_name, def, filename))
					Print("WARNING: invalid or missing tex_bumpmap in '%s'\n", filename);

				DataLoader* loader = DataLoader::GetLoader();
				loader->LoadTexture(tex_name, mtl->tex_bumpmap);
			}

			else if (defname.indexOf("tex_e") == 0) {
				char tex_name[64];
				if (!GetDefText(tex_name, def, filename))
					Print("WARNING: invalid or missing tex_emissive in '%s'\n", filename);

				DataLoader* loader = DataLoader::GetLoader();

				loader->LoadTexture(tex_name, mtl->tex_emissive);
			}
		}
	}

	if (skin && mtl)
		skin->AddMaterial(mtl);
}
*/
void
UStarshatterGameDataSubsystem::LoadSystemDesign(const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("Loading System Design Data: %s"), *FString(fn));

	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(fn);

	BYTE* block = 0;
	SSWInstance->loader->LoadBuffer(fn, block, true);

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	if (!term) {
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "SYSTEM")
		{
			UE_LOG(LogTemp, Log, TEXT("Invalid  SYSTEM File: %s"), *FString(fn));
			return;
		}
	}

	int type = 1;
	Text SystemName = "";
	Text ComponentName = "";
	Text ComponentAbrv = "";

	float ComponentRepairTime = 0;
	float ComponentReplaceTime = 0;

	int ComponentSpares = 1;
	int ComponentAffects = 0;

	FS_SystemDesign NewSystemDesign;

	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "system") {
					TermStruct* val = def->term()->isStruct();
					NewComponentArray.Empty();
					for (int i = 0; i < val->elements()->size(); i++) {
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef) {
							if (pdef->name()->value() == "name") {
								GetDefText(SystemName, pdef, fn);
								NewSystemDesign.Name = FString(SystemName);
							}

							else if (pdef->name()->value() == ("component")) {

								FS_ComponentDesign NewComponentDesign;
								TermStruct* val2 = pdef->term()->isStruct();
								for (int idx = 0; idx < val2->elements()->size(); idx++) {
									TermDef* pdef2 = val2->elements()->at(idx)->isDef();
									if (pdef2) {
										if (pdef2->name()->value() == "name") {
											GetDefText(ComponentName, pdef2, fn);
											NewComponentDesign.Name = FString(ComponentName);
										}
										else if (pdef2->name()->value() == "abrv") {
											GetDefText(ComponentAbrv, pdef2, fn);
											NewComponentDesign.Abrv = FString(ComponentAbrv);
										}
										else if (pdef2->name()->value() == "repair_time") {
											GetDefNumber(ComponentRepairTime, pdef2, fn);
											NewComponentDesign.RepairTime = ComponentRepairTime;
										}
										else if (pdef2->name()->value() == "replace_time") {
											GetDefNumber(ComponentReplaceTime, pdef2, fn);
											NewComponentDesign.ReplaceTime = ComponentReplaceTime;
										}
										else if (pdef2->name()->value() == "spares") {
											GetDefNumber(ComponentSpares, pdef2, fn);
											NewComponentDesign.Spares = ComponentSpares;
										}
										else if (pdef2->name()->value() == "affects") {
											GetDefNumber(ComponentAffects, pdef2, fn);
											NewComponentDesign.Affects = ComponentAffects;
										}
									}
								}
								NewComponentArray.Add(NewComponentDesign);
							}
							NewSystemDesign.Component = NewComponentArray;
						}
					}
				}
				FName RowName = FName(FString(SystemName));

				// call AddRow to insert the record
				SystemDesignDataTable->AddRow(RowName, NewSystemDesign);
				//SystemDesignTable.Add(NewSystemDesign);
			}
		}
	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
}

// +--------------------------------------------------------------------+

CombatGroup*
UStarshatterGameDataSubsystem::CloneOver(CombatGroup* force, CombatGroup* clone, CombatGroup* group)
{
	/*CombatGroup* orig_parent = group->GetParent();

	if (orig_parent) {
		CombatGroup* clone_parent = clone->FindGroup(orig_parent->Type(), orig_parent->GetID());

		if (!clone_parent)
			clone_parent = CloneOver(force, clone, orig_parent);

		CombatGroup* new_clone = clone->FindGroup(group->Type(), group->GetID());

		if (!new_clone) {
			new_clone = group->Clone(false);
			clone_parent->AddComponent(new_clone);
		}

		return new_clone;
	}
	else {
		return clone;
	}*/
	return 0;
}

// +--------------------------------------------------------------------+

void
UStarshatterGameDataSubsystem::SetCampaignStatus(ECampaignStatus s)
{
	CampaignStatus = s;

	// record the win in player profile:
	if (CampaignStatus == ECampaignStatus::SUCCESS) {
		PlayerData* player = PlayerData::GetCurrentPlayer();

		if (player)
			player->SetCampaignComplete(campaign_id);
	}

	if (CampaignStatus > ECampaignStatus::ACTIVE) {
		UE_LOG(LogTemp, Log, TEXT("Campaign::SetStatus() destroying mission list at campaign end"));
		missions.destroy();
	}
}

double
UStarshatterGameDataSubsystem::Stardate()
{
	return StarSystem::Stardate();
}

Combatant*
UStarshatterGameDataSubsystem::GetCombatant(const char* cname)
{
	/*ListIter<Combatant> iter = combatants;
	while (++iter) {
		Combatant* c = iter.value();
		if (!strcmp(c->Name(), cname))
			return c;
	}
	*/
	return 0;
}

// +--------------------------------------------------------------------+

Text
UStarshatterGameDataSubsystem::GetContentBundleText(const char* key) const
{
	return ContentValues.Find(key, Text(key));
}

// +--------------------------------------------------------------------+

void UStarshatterGameDataSubsystem::LoadContentBundle()
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadContentBundle()"));

	const FString ContentFilePath =
		FPaths::ProjectContentDir() / TEXT("GameData/Content/content.txt");

	UE_LOG(LogTemp, Log, TEXT("Loading Content Bundle: %s"), *ContentFilePath);

	if (!FPaths::FileExists(ContentFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Content bundle not found: %s"), *ContentFilePath);
		return;
	}

	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *ContentFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read content bundle: %s"), *ContentFilePath);
		return;
	}

	Bytes.Add(0); // null terminate

	const char* buffer = reinterpret_cast<const char*>(Bytes.GetData());
	if (!buffer || !*buffer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Content bundle empty: %s"), *ContentFilePath);
		return;
	}

	// Clear previous content (Dictionary<Text> does NOT have Destroy() in your build)
	// Use the legacy Dictionary API:
	ContentValues.Clear();      // <-- if your Dictionary uses destroy(), change to: ContentValues.destroy();

	Text key;
	Text val;

	const char* p = buffer;
	int s = 0; // 0=key, 1=after '=', waiting for first value char, 2=value, -1=comment
	key = "";
	val = "";

	int32 InsertedCount = 0;

	auto FlushPair = [&]()
		{
			if (key != "" && val != "")
			{
				Text k = Text(key).trim();
				Text v = Text(val).trim();

				ContentValues.Insert(k, v);
				++InsertedCount;

				const FString KStr(ANSI_TO_TCHAR(k.data()));
				const FString VStr(ANSI_TO_TCHAR(v.data()));
				UE_LOG(LogTemp, Log, TEXT("Inserted- %s: %s"), *KStr, *VStr);
			}

			key = "";
			val = "";
			s = 0;
		};

	while (*p)
	{
		if (*p == '\n' || *p == '\r')
		{
			// newline ends a line (and ends comment mode)
			if (s != -1)
			{
				FlushPair();
			}
			else
			{
				// comment line ended
				s = 0;
				key = "";
				val = "";
			}
		}
		else if (s == -1)
		{
			// comment: ignore until newline
		}
		else if (*p == '=')
		{
			s = 1;
		}
		else if (s == 0)
		{
			// key parse (legacy behavior)
			if (!key[0])
			{
				if (*p == '#')
				{
					s = -1; // comment
				}
				else if (!isspace((unsigned char)*p))
				{
					key.append(*p);
				}
			}
			else
			{
				key.append(*p);
			}
		}
		else if (s == 1)
		{
			// first non-space begins value
			if (!isspace((unsigned char)*p))
			{
				s = 2;
				val.append(*p);
			}
		}
		else if (s == 2)
		{
			val.append(*p);
		}

		++p;
	}

	// Handle EOF without trailing newline
	if (s != -1)
	{
		FlushPair();
	}

	UE_LOG(LogTemp, Log, TEXT("Content bundle loaded. Entries=%d"), InsertedCount);
}

// +--------------------------------------------------------------------+


void UStarshatterGameDataSubsystem::LoadAwardTables()
{
	UE_LOG(LogTemp, Log, TEXT("UStarshatterGameDataSubsystem::LoadAwardTables()"));

	// ------------------------------------------------------------
	// Resolve file path
	// ------------------------------------------------------------
	FString AwardsPath = FPaths::ProjectContentDir();
	AwardsPath /= TEXT("GameData/Awards/awards.def");

	UE_LOG(LogTemp, Log, TEXT("Loading Award Info Data: %s"), *AwardsPath);

	if (!FPaths::FileExists(AwardsPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadAwardTables: file not found: %s"), *AwardsPath);
		return;
	}

	// ------------------------------------------------------------
	// UE-native raw byte load
	// ------------------------------------------------------------
	TArray<uint8> Bytes;
	if (!FFileHelper::LoadFileToArray(Bytes, *AwardsPath))
	{
		UE_LOG(LogTemp, Error, TEXT("LoadAwardTables: failed to read: %s"), *AwardsPath);
		return;
	}

	Bytes.Add(0); // null terminate

	// IMPORTANT: do NOT name this "filename"
	const FTCHARToUTF8 Utf8Path(*AwardsPath);
	const char* FileNameAnsi = Utf8Path.Get();

	Parser ParserObj(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
	Term* TermPtr = ParserObj.ParseTerm();

	if (!TermPtr)
		return;

	// ------------------------------------------------------------
	// Header validation
	// ------------------------------------------------------------
	TermText* FileType = TermPtr->isText();
	if (!FileType || FileType->value() != "AWARDS")
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Awards file: %s"), *AwardsPath);
		delete TermPtr;
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Loading Ranks and Medals"));

	// ------------------------------------------------------------
	// Parse awards
	// ------------------------------------------------------------
	do
	{
		delete TermPtr;
		TermPtr = ParserObj.ParseTerm();
		if (!TermPtr)
			break;

		TermDef* Def = TermPtr->isDef();
		if (!Def || Def->name()->value() != "award")
			continue;

		if (!Def->term() || !Def->term()->isStruct())
		{
			UE_LOG(LogTemp, Warning, TEXT("WARNING: award structure missing in '%s'"), *AwardsPath);
			continue;
		}

		TermStruct* Val = Def->term()->isStruct();
		FS_AwardInfo NewAwardData;

		// --------------------------------------------------------
		// Local scratch (no shadowing)
		// --------------------------------------------------------
		int32  LocalAwardId = 0;
		Text   LocalAwardType = "";
		Text   LocalAwardName = "";
		Text   LocalAwardAbrv = "";
		Text   LocalAwardDesc = "";
		Text   LocalAwardText = "";

		Text   LocalDescSound = "";
		Text   LocalGrantSound = "";
		Text   LocalLargeImage = "";
		Text   LocalSmallImage = "";

		int32  LocalMinRank = 0;
		int32  LocalMaxRank = 0;
		int32  LocalMinShipClass = 0;
		int32  LocalMaxShipClass = 0;

		// --------------------------------------------------------
		// Field parsing
		// --------------------------------------------------------
		for (int32 i = 0; i < (int32)Val->elements()->size(); ++i)
		{
			TermDef* PDef = Val->elements()->at(i)->isDef();
			if (!PDef)
				continue;

			const Text& Key = PDef->name()->value();

			if (Key == "name")
			{
				GetDefText(LocalAwardName, PDef, FileNameAnsi);
				NewAwardData.AwardName = FString(LocalAwardName);
			}
			else if (Key == "desc")
				GetDefText(LocalAwardDesc, PDef, FileNameAnsi);
			else if (Key == "award")
				GetDefText(LocalAwardText, PDef, FileNameAnsi);
			else if (Key == "desc_sound")
				GetDefText(LocalDescSound, PDef, FileNameAnsi);
			else if (Key == "award_sound")
				GetDefText(LocalGrantSound, PDef, FileNameAnsi);
			else if (Key == "type")
				GetDefText(LocalAwardType, PDef, FileNameAnsi);
			else if (Key == "abrv")
				GetDefText(LocalAwardAbrv, PDef, FileNameAnsi);
			else if (Key == "id")
				GetDefNumber(LocalAwardId, PDef, FileNameAnsi);
			else if (Key == "min_rank")
				GetDefNumber(LocalMinRank, PDef, FileNameAnsi);
			else if (Key == "max_rank")
				GetDefNumber(LocalMaxRank, PDef, FileNameAnsi);
			else if (Key == "min_ship_class")
			{
				Text classname;
				GetDefText(classname, PDef, FileNameAnsi);
				LocalMinShipClass = Ship::ClassForName(classname);
			}
			else if (Key == "max_ship_class")
			{
				Text classname;
				GetDefText(classname, PDef, FileNameAnsi);
				LocalMaxShipClass = Ship::ClassForName(classname);
			}
		}

		// --------------------------------------------------------
		// Finalize struct
		// --------------------------------------------------------
		NewAwardData.AwardId = LocalAwardId;
		NewAwardData.AwardType = FString(LocalAwardType);
		NewAwardData.AwardAbrv = FString(LocalAwardAbrv);
		NewAwardData.AwardDesc = FString(LocalAwardDesc);
		NewAwardData.AwardText = FString(LocalAwardText);
		NewAwardData.DescSound = FString(LocalDescSound);
		NewAwardData.GrantSound = FString(LocalGrantSound);
		NewAwardData.LargeImage = FString(LocalLargeImage);
		NewAwardData.SmallImage = FString(LocalSmallImage);
		NewAwardData.MinRank = LocalMinRank;
		NewAwardData.MaxRank = LocalMaxRank;
		NewAwardData.MinShipClass = LocalMinShipClass;
		NewAwardData.MaxShipClass = LocalMaxShipClass;

		const FName RowName(*FString(LocalAwardName));
		AwardsDataTable->AddRow(RowName, NewAwardData);

		AwardData = NewAwardData;

	} while (TermPtr);

	if (TermPtr)
	{
		delete TermPtr;
		TermPtr = nullptr;
	}
}
