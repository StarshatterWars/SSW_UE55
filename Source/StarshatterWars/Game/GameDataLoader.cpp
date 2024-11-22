/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         GameDataLoader.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of Master Game Data Tables
	Will not be used after Dable Table is Generated.
*/



#include "GameDataLoader.h"
#include "../System/SSWGameInstance.h"
#include "../System/Game.h"
#include "../Space/Starsystem.h"
#include "../Space/Galaxy.h"
#include "CombatGroup.h"
#include "CombatRoster.h"
#include "CombatAction.h"
#include "CombatActionReq.h"
#include "CombatEvent.h"
#include "Combatant.h"
#include "Mission.h"
#include "Intel.h"
#include "PlayerData.h"


// Sets default values
AGameDataLoader::AGameDataLoader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::AGameDataLoader()"));

	static ConstructorHelpers::FObjectFinder<UDataTable> CampaignDataTableObject(TEXT("DataTable'/Game/Game/DT_Campaign.DT_Campaign'"));

	if (CampaignDataTableObject.Succeeded())
	{
		CampaignDataTable = CampaignDataTableObject.Object;
		
		CampaignDataTable->EmptyTable();
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> GalaxyDataTableObject(TEXT("DataTable'/Game/Game/DT_GalaxyMap.DT_GalaxyMap'"));

	if (GalaxyDataTableObject.Succeeded())
	{
		GalaxyDataTable = GalaxyDataTableObject.Object;
		//GalaxyDataTable->EmptyTable();
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> StarSystemDataTableObject(TEXT("DataTable'/Game/Game/DT_StarSystem.DT_StarSystem'"));

	if (StarSystemDataTableObject.Succeeded())
	{
		StarSystemDataTable = StarSystemDataTableObject.Object;
		//GalaxyDataTable->EmptyTable();
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> StarsDataTableObject(TEXT("DataTable'/Game/Game/DT_Stars.DT_Stars'"));

	if (StarsDataTableObject.Succeeded())
	{
		StarsDataTable = StarsDataTableObject.Object;
		//StarsDataTable->EmptyTable();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to get Stars Data Table"));
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> PlanetsDataTableObject(TEXT("DataTable'/Game/Game/DT_Planets.DT_Planets'"));

	if (PlanetsDataTableObject.Succeeded())
	{
		PlanetsDataTable = PlanetsDataTableObject.Object;
		//StarsDataTable->EmptyTable();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to get Planets Data Table"));
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> MoonsDataTableObject(TEXT("DataTable'/Game/Game/DT_Moons.DT_Moons'"));

	if (MoonsDataTableObject.Succeeded())
	{
		MoonsDataTable = MoonsDataTableObject.Object;
		//StarsDataTable->EmptyTable();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to get Moons Data Table"));
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> RegionsDataTableObject(TEXT("DataTable'/Game/Game/DT_Regions.DT_Regions'"));

	if (RegionsDataTableObject.Succeeded())
	{
		RegionsDataTable = RegionsDataTableObject.Object;
		//StarsDataTable->EmptyTable();
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to get Regions Data Table"));
	}
}

// Called when the game starts or when spawned
void AGameDataLoader::BeginPlay()
{
	Super::BeginPlay();
	GetSSWInstance();
	LoadGalaxyMap();
	LoadStarsystems();
	InitializeCampaignData();
}

// Called every frame
void AGameDataLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGameDataLoader::LoadGalaxyData()
{

}

void AGameDataLoader::GetSSWInstance()
{
	SSWInstance = (USSWGameInstance*)GetGameInstance();
}

void AGameDataLoader::InitializeCampaignData() {
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::InitializeCampaignData()"));

	FString ProjectPath = FPaths::ProjectDir();
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

void AGameDataLoader::LoadCampaignData(const char* FileName, bool full)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadCampaignData"));
	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(FileName);

	FString fs = FString(ANSI_TO_TCHAR(FileName));
	FString FileString;
	BYTE* block = 0;

	SSWInstance->loader->LoadBuffer(FileName, block, true);

	UE_LOG(LogTemp, Log, TEXT("Loading Campaign Data: %s"), *fs);

	if (FFileHelper::LoadFileToString(FileString, *fs, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);

		const char* result = TCHAR_TO_ANSI(*FileString);
	}

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	if (!term) {
		UE_LOG(LogTemp, Log, TEXT("WARNING: could not parse '%s'"), *fs);
		return;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Campaign file '%s'"), *fs);
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
				else if (def->name()->value() == "situation") {
						GetDefText(situation, def, filename);
						NewCampaignData.Situation = FString(situation);
				}
				else if (def->name()->value() == "orders") {
						GetDefText(orders, def, filename);
						OrdersArray.Add(FString(orders));
						NewCampaignData.Orders = OrdersArray;
				}
				else if (def->name()->value() == "scripted") {
					if (def->term() && def->term()->isBool()) {
						scripted = def->term()->isBool()->value();
						NewCampaignData.Scripted = scripted;
					}
				}
				else if (def->name()->value() == "sequential") {
					if (def->term() && def->term()->isBool()) {
						sequential = def->term()->isBool()->value();
						NewCampaignData.Sequential = sequential;
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
						ActionLocation = Vec3(0.0f, 0.0f, 0.0f);
						
						ActionSystem = "";
						ActionRegion = "";
						ActionFile = "";
						ActionImage = "";
						ActionScene = "";
						ActionText = "";

						ActionCount = 1;
						StartBefore = Game::TIME_NEVER;
						StartAfter = 0;
						MinRank = 0;
						MaxRank = 100;
						Delay = 0 ;
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

									//if (type == CombatAction::MISSION_TEMPLATE) {
									//	ActionSubtype = Mission::TypeFromName(txt);
									//}
									//else if (type == CombatAction::COMBAT_EVENT) {
									//	ActionSubtype = CombatEvent::TypeFromName(txt);
									//}
									if (ActionType == CombatAction::INTEL_EVENT) {
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

								/*else if (pdef->term()->isText()) {
									GetDefText(txt, pdef, filename);

									if (type == CombatAction::MISSION_TEMPLATE) {
										opp_type = Mission::TypeFromName(txt);
									}
								}*/
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
								NewCampaignAction.Location.X = ActionLocation.x;
								NewCampaignAction.Location.Y = ActionLocation.y;
								NewCampaignAction.Location.Z = ActionLocation.z;

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
								NewCampaignAction.File = FString(ActionFile);
							}
							else if (pdef->name()->value() == "image") {
								GetDefText(ActionImage, pdef, filename);
								NewCampaignAction.Image = FString(ActionImage);
							}
							else if (pdef->name()->value() == "scene") {
								GetDefText(ActionScene, pdef, filename);
								NewCampaignAction.Scene = FString(ActionScene);
							}
							else if (pdef->name()->value() == "text") {
								GetDefText(ActionText, pdef, filename);
								NewCampaignAction.Text = FString(ActionText);
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
								ActionStatus = CombatAction::COMPLETE;
								NotAction = false;

								Combatant1 ="";
								Combatant2 = "";

								comp = 0;
								score = 0;
								intel = 0;
								gtype = 0;
								gid = 0;

								FS_CampaignReq NewCampaignReq;

								for (int index = 0; index < val2->elements()->size(); index++) {
									TermDef* pdef2 = val2->elements()->at(index)->isDef();
									
									if (pdef2) {
										if (pdef2->name()->value() == "action") {
											GetDefNumber(Action, pdef2, filename);
											NewCampaignReq.Action = Action;
										}
										else if (pdef2->name()->value() == "status") {
											char txt[64];
											GetDefText(txt, pdef2, filename);
											ActionStatus = CombatAction::StatusFromName(txt);
											NewCampaignReq.Status = FString(ActionStatus);

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
											comp = CombatActionReq::CompFromName(txt);
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
											gtype = CombatGroup::TypeFromName(type_name);
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
						CombatantSize = 0;
						NewCombatUnit.Group.Empty();

						for (int UnitIdx = 0; UnitIdx < NewCampaignData.CombatantSize; UnitIdx++)
						{	
							def = CombatantTerm->elements()->at(UnitIdx)->isDef();

							if (def->name()->value() == "name") {
								GetDefText(CombatantName, def, filename);
								NewCombatUnit.Name = FString(CombatantName);
							} else if (def->name()->value() == "size") {
								GetDefNumber(CombatantSize, def, filename);
								NewCombatUnit.Size = CombatantSize;
							} else if (def->name()->value() == "group") {
								//ParseGroup(def->term()->isStruct(), filename);
								TermStruct* GroupTerm = def->term()->isStruct();

								CombatantType = "";
								CombatantId = 0;
								
								for (int i = 0; i < GroupTerm->elements()->size(); i++) {

									TermDef* pdef = GroupTerm->elements()->at(i)->isDef();
									if (pdef->name()->value() == "type") {
										GetDefText(CombatantType, pdef, filename);
										NewGroupUnit.Type = FString(CombatantType);
										UE_LOG(LogTemp, Log, TEXT("%s:  %s"), *FString(pdef->name()->value()), *FString(CombatantType));
										//type = CombatGroup::TypeFromName(type_name);
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

	// define our data table struct
	FName RowName = FName(FString(name));
	CampaignDataTable->AddRow(RowName, NewCampaignData);
	CampaignData = NewCampaignData;

	SSWInstance->loader->ReleaseBuffer(block);
}

void AGameDataLoader::LoadZones(FString Path)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadZones()"));
	
	FString FileName = Path;
	FileName.Append("zones.def");

	UE_LOG(LogTemp, Log, TEXT("Loading Campaign Zone: %s"), *FileName);

	const char* fn = TCHAR_TO_ANSI(*FileName);

	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(fn);

	BYTE* block = 0;

	SSWInstance->loader->LoadBuffer(fn, block, true);

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	ZoneArray.Empty();

	if (!term) {
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "ZONES") {
			UE_LOG(LogTemp, Log, TEXT("Invalid Zone File %s"), *FString(fn));
			return;
		}
	}

	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def->name()->value() == "zone") {
				if (!def->term() || !def->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: zone struct missing in '%s'"), *FString(filename));
				}
				else {
					TermStruct* val = def->term()->isStruct();

					FS_CampaignZone NewCampaignZone;

					for (int i = 0; i < val->elements()->size(); i++) {
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef) {
							if (pdef->name()->value() == "region") {
								GetDefText(ZoneRegion, pdef, fn);
								NewCampaignZone.Region = FString(ZoneRegion);
								//zone->AddRegion(rgn);
							}
							else if (pdef->name()->value() == "system") {
								GetDefText(ZoneSystem, pdef, fn);
								NewCampaignZone.System = FString(ZoneSystem);
								//zone->system = rgn;
							}
						}	
					}
					ZoneArray.Add(NewCampaignZone);
				}
			}
		}

	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
}

void
AGameDataLoader::LoadMissionList(FString Path)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadMissionList()"));
	MissionListArray.Empty();

	FString FileName = Path;
	FileName.Append("Missions.def");

	UE_LOG(LogTemp, Log, TEXT("Loading Mission List : %s"), *FileName);

	const char* fn = TCHAR_TO_ANSI(*FileName);

	if(FFileManagerGeneric::Get().FileExists(*FileName) == false)
	{ 
		UE_LOG(LogTemp, Log, TEXT("Mission List does not exist"));
		return;
	}

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
		if (!file_type || file_type->value() != "MISSIONLIST") {
			UE_LOG(LogTemp, Log, TEXT("WARNING: invalid mission list file '%s'"), *FString(fn));
			return;
		}
	}

	do {
		delete term; term = 0;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def->name()->value() == "mission") {
				if (!def->term() || !def->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: zone struct missing in '%s'"), *FString(fn));
				}
				else {
					TermStruct* val = def->term()->isStruct();

					FS_CampaignMissionList NewMissionList;

					int   Id = 0;
					Text  Name;
					Text  Desc;
					Text  Script;
					Text  System = "Unknown";
					Text  Region = "Unknown";
					int   Start = 0;
					int   Type = 0;

					for (int i = 0; i < val->elements()->size(); i++) {
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef->name()->value() == "id") {
							GetDefNumber(Id, pdef, fn);
							NewMissionList.Id = Id;
						}
						else if (pdef->name()->value() == "name") {
							GetDefText(Name, pdef, fn);
							NewMissionList.Name = FString(Name);
						}
						else if (pdef->name()->value() == "desc") {
							GetDefText(Desc, pdef, fn);
							NewMissionList.Description = FString(Desc);
						}
						else if (pdef->name()->value() == "start") {
							GetDefTime(Start, pdef, fn);
							NewMissionList.Description = FString(Desc);
						}
						else if (pdef->name()->value() == "system") {
							GetDefText(System, pdef, fn);
							NewMissionList.System = FString(System);
						}
						else if (pdef->name()->value() == "region") {
							GetDefText(Region, pdef, fn);
							NewMissionList.Region = FString(Region);
						}
						else if (pdef->name()->value() == "script") {
							GetDefText(Script, pdef, fn);
							NewMissionList.Script = FString(Script);
						}
						else if (pdef->name()->value() == "type") {
							char typestr[64];
							GetDefText(typestr, pdef, fn);
							Type = Mission::TypeFromName(typestr);
							NewMissionList.Type = Type;
						}
					}
					MissionListArray.Add(NewMissionList);
				}
			}
		}
	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
}

void
AGameDataLoader::LoadTemplateList(FString Path)
{
	
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadTemplateList()"));

	FString FileName = Path;
	FileName.Append("Templates.def");

	UE_LOG(LogTemp, Log, TEXT("Loading Template List : %s"), *FileName);

	const char* fn = TCHAR_TO_ANSI(*FileName);

	if (FFileManagerGeneric::Get().FileExists(*FileName) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("Template List does not exist"));
		return;
	}

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
		if (!file_type || file_type->value() != "TEMPLATELIST") {
			UE_LOG(LogTemp, Log, TEXT("WARNING: invalid template list file '%s'"), *FString(fn));
			return;
		}
	}

	do {
		delete term; 
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def->name()->value() == "mission") {
				if (!def->term() || !def->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: mission struct missing in '%s'"), *FString(fn));
				}
				else {
					TermStruct* val = def->term()->isStruct();
					
					FS_CampaignTemplateList NewTemplateList;

					Text  Name = "";
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

					for (int i = 0; i < val->elements()->size(); i++) {
					TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef->name()->value() == "id") {
							GetDefNumber(id, pdef, fn);
							NewTemplateList.Id = id;
						}
						else if (pdef->name()->value() == "name") {
							GetDefText(Name, pdef, fn);
							NewTemplateList.Name = FString(Name);
						}
						else if (pdef->name()->value() == "script") {
							GetDefText(Script, pdef, fn);
							NewTemplateList.Script = FString(Script);
						}
						else if (pdef->name()->value() == "rgn" || pdef->name()->value() == "region") {
							GetDefText(Region, pdef, fn);
							NewTemplateList.Region = FString(Region);
						}
						else if (pdef->name()->value() == "type") {
							char typestr[64];
							GetDefText(typestr, pdef, fn);
							msn_type = Mission::TypeFromName(typestr);
							NewTemplateList.MissionType = msn_type;
						}

						else if (pdef->name()->value() == "group") {
							char typestr[64];
							GetDefText(typestr, pdef, fn);
							grp_type = CombatGroup::TypeFromName(typestr);
							NewTemplateList.GroupType = grp_type;
						}

						else if (pdef->name()->value() == "min_rank") {
							GetDefNumber(min_rank, pdef, fn);
							NewTemplateList.MinRank = min_rank;
						}
						else if (pdef->name()->value() == "max_rank") {
							GetDefNumber(max_rank, pdef, fn);
							NewTemplateList.MaxRank = max_rank;
						}
						else if (pdef->name()->value() == "action_id") {
							GetDefNumber(action_id, pdef, fn);
							NewTemplateList.ActionId = action_id;
						}
						else if (pdef->name()->value() == "action_status") {
							GetDefNumber(action_status, pdef, fn);
							NewTemplateList.ActionStatus = action_status;
						}
						else if (pdef->name()->value() == "exec_once") {
							GetDefNumber(exec_once, pdef, fn);
							NewTemplateList.ExecOnce = exec_once;
						}
						else if (pdef->name()->value().contains("before")) {
							if (pdef->term()->isNumber()) {
								GetDefNumber(start_before, pdef, fn);
								NewTemplateList.StartBefore = start_before;
							}
							else {
								GetDefTime(start_before, pdef, fn);
								start_before -= Game::ONE_DAY;
								NewTemplateList.StartBefore = start_before;
							}
						}
						else if (pdef->name()->value().contains("after")) {
							if (pdef->term()->isNumber()) {
								GetDefNumber(start_after, pdef, fn);
								NewTemplateList.StartAfter = start_after;
							}
							else {
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

	SSWInstance->loader->ReleaseBuffer(block);
}

void AGameDataLoader::LoadMission(FString Path)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadMission()"));
	FString PathName = CampaignPath;
	PathName.Append("Scenes/");
	
	TArray<FString> output;
	output.Empty();

	FString file = PathName + "*.def";
	FFileManagerGeneric::Get().FindFiles(output, *file, true, false);

	for (int i = 0; i < output.Num(); i++) {

		FString FileName = PathName;
		FileName.Append(output[i]);

		char* fn = TCHAR_TO_ANSI(*FileName);

		ParseMission(fn);
	}
}

void AGameDataLoader::LoadTemplateMission(FString Name)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadTemplateMission()"));
	FString PathName = CampaignPath;
	PathName.Append("Templates/");

	TArray<FString> output;
	output.Empty();

	FString file = PathName + "*.def";
	FFileManagerGeneric::Get().FindFiles(output, *file, true, false);

	for (int i = 0; i < output.Num(); i++) {

		FString FileName = PathName;
		FileName.Append(output[i]);

		char* fn = TCHAR_TO_ANSI(*FileName);

		ParseMissionTemplate(fn);
	}
}

void AGameDataLoader::LoadScriptedMission(FString Name)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadScriptedMission()"));
	FString PathName = CampaignPath;
	PathName.Append("Scripts/");

	TArray<FString> output;
	output.Empty();

	FString file = PathName + "*.def";
	FFileManagerGeneric::Get().FindFiles(output, *file, true, false);

	for (int i = 0; i < output.Num(); i++) {

		FString FileName = PathName;
		FileName.Append(output[i]);

		char* fn = TCHAR_TO_ANSI(*FileName);

		ParseScriptedTemplate(fn);
	}
}

void
AGameDataLoader::ParseMission(const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseMission()"));

	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(fn);

	BYTE* block = 0;
	SSWInstance->loader->LoadBuffer(fn, block, true);

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();
	
	FString fs = FString(ANSI_TO_TCHAR(fn));
	FString FileString;

	MissionElementArray.Empty();
	MissionEventArray.Empty();

	if (FFileHelper::LoadFileToString(FileString, *fs, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);

		const char* result = TCHAR_TO_ANSI(*FileString);
	}
	if (!term) {
		UE_LOG(LogTemp, Log, TEXT("WARNING: could not parse '%s'"), *FString(fn));
		return;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("MISSION file '%s'"), *FString(fn));
	}

	FS_CampaignMission NewMission;

	int					id = 0;

	Text				Region = "";
	Text				Scene = "";
	Text                System = "";
	Text                Subtitles = "";
	Text                Name = "";
	Text                Desc = "";
	Text				TargetName = "";
	Text				WardName = "";
	Text                Objective = "";
	Text                Sitrep = "";

	int                 Type = 0;
	int                 Team = 0;
	Text                Start = ""; // time

	double              Stardate = 0;

	bool                Degrees = false;

	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "name") {
					GetDefText(Name, def, fn);
					NewMission.Name = FString(Name);
					UE_LOG(LogTemp, Log, TEXT("mission name '%s'"), *FString(Name));
				}
		
				else if (def->name()->value() == "scene") {
					GetDefText(Scene, def, fn);
					NewMission.Scene = FString(Scene);

				}
				else if (def->name()->value() == "desc") {
					GetDefText(Desc, def, fn);
					if (Desc.length() > 0 && Desc.length() < 32) {
						NewMission.Desc = FString(Desc);
					}
				}
				else if (def->name()->value() == "type") {
					char typestr[64];
					GetDefText(typestr, def, fn);
					Type = Mission::TypeFromName(typestr);
					NewMission.Type = Type;
				}
				else if (def->name()->value() == "system") {
					GetDefText(System, def, fn);
					NewMission.System = FString(System);
				}
				else if (def->name()->value() == "region") {
					GetDefText(Region, def, fn);
					NewMission.Region = FString(Region);
				}
				else if (def->name()->value() == "degrees") {
					GetDefBool(Degrees, def, fn);
					NewMission.Degrees = Degrees;
				}
				else if (def->name()->value() == "objective") {
					GetDefText(Objective, def, fn);
					if (Objective.length() > 0 && Objective.length() < 32) {
						NewMission.Objective = FString(Objective);
					}
				}
				else if (def->name()->value() == "sitrep") {
					GetDefText(Sitrep, def, fn);
					if (Sitrep.length() > 0 && Sitrep.length() < 32) {
						NewMission.Sitrep = FString(Sitrep);
					}
				}
				else if (def->name()->value() == "subtitles") {
					GetDefText(Subtitles, def, fn);
					NewMission.Subtitles = FString(Subtitles);
				
				}
				else if (def->name()->value() == "start") {
					GetDefText(Start, def, fn);
					NewMission.StartTime = FString(Start);
					//GetDefTime(start, def, fn);

				}
				else if (def->name()->value() == "stardate") {
					GetDefNumber(Stardate, def, fn);
					NewMission.Stardate = Stardate;
				}
				else if (def->name()->value() == "team") {
					GetDefNumber(Team, def, fn);
					NewMission.Team = Team;
				}
				else if (def->name()->value() == "target") {
					GetDefText(TargetName, def, fn);
					NewMission.TargetName = FString(TargetName);
				}
				else if (def->name()->value() == "ward") {
					GetDefText(WardName, def, filename);
					NewMission.WardName = FString(WardName);
				}
				else if (def->name()->value() == "event") {
					ParseEvent(def->term()->isStruct(), fn);
					NewMission.Event = MissionEventArray;
				}

				else if ((def->name()->value() == "element") ||
					(def->name()->value() == "ship") ||
					(def->name()->value() == "station")) {
					 
					 ParseElement(def->term()->isStruct(), fn);
					 NewMission.Element = MissionElementArray;
				}	
			}
		}        // term
	} while (term); 

	SSWInstance->loader->ReleaseBuffer(block);
	MissionArray.Add(NewMission);
}

void
AGameDataLoader::ParseNavpoint(TermStruct* val, const char* fn)
{
	
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseNavpoint()"));

	int   formation = 0;
	int   speed = 0;
	int   priority = 1;
	int   farcast = 0;
	int   hold = 0;
	int   emcon = 0;
	Vec3  loc(0, 0, 0);
	Text  order_name;
	Text  status_name;
	Text  order_rgn_name;
	Text  tgt_name;
	Text  tgt_desc;

	MissionRLocArray.Empty();

	FS_MissionInstruction NewMissionInstruction;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "cmd") {
				GetDefText(order_name, pdef, fn);
				NewMissionInstruction.OrderName = FString(order_name);
			}

			else if (pdef->name()->value() == "status") {
				GetDefText(status_name, pdef, fn);
				NewMissionInstruction.StatusName = FString(status_name);
			}

			else if (pdef->name()->value() == "loc") {
				GetDefVec(loc, pdef, fn);
				NewMissionInstruction.Location.X = loc.x;
				NewMissionInstruction.Location.Y = loc.y;
				NewMissionInstruction.Location.Z = loc.z;
			}

			else if (pdef->name()->value() == "rloc") {
				if (pdef->term()->isStruct()) {
					ParseRLoc(pdef->term()->isStruct(), fn);
					NewMissionInstruction.RLoc = MissionRLocArray;
				}
			}

			else if (pdef->name()->value() == "rgn") {
				GetDefText(order_rgn_name, pdef, fn);
				NewMissionInstruction.OrderRegionName = FString(order_rgn_name);
			}
			else if (pdef->name()->value() == "speed") {
				GetDefNumber(speed, pdef, fn);
				NewMissionInstruction.Speed = speed;
			}
			else if (pdef->name()->value() == "formation") {
				GetDefNumber(formation, pdef, fn);
				NewMissionInstruction.Formation = formation;
			}
			else if (pdef->name()->value() == "emcon") {
				GetDefNumber(emcon, pdef, fn);
				NewMissionInstruction.EMCON = emcon;
			}
			else if (pdef->name()->value() == "priority") {
				GetDefNumber(priority, pdef, fn);
				NewMissionInstruction.Priority = priority;
			}
			else if (pdef->name()->value() == "farcast") {
				if (pdef->term()->isBool()) {
					bool f = false;
					GetDefBool(f, pdef, fn);
					if (f)
						farcast = 1;
					else
						farcast = 0;
					NewMissionInstruction.Farcast = farcast;
				}
				else {
					GetDefNumber(farcast, pdef, fn);
					NewMissionInstruction.Farcast = farcast;
				}
			}
			else if (pdef->name()->value() == "tgt") {
				GetDefText(tgt_name, pdef, fn);
				NewMissionInstruction.TargetName = FString(tgt_name);
			}
			else if (pdef->name()->value() == "tgt_desc") {
				GetDefText(tgt_desc, pdef, fn);
				NewMissionInstruction.TargetDesc = FString(tgt_desc);
			}
			else if (pdef->name()->value().indexOf("hold") == 0) {
				GetDefNumber(hold, pdef, fn);
				NewMissionInstruction.Hold = hold;
			}
		}
	}
	MissionNavpointArray.Add(NewMissionInstruction);
}

void
AGameDataLoader::ParseObjective(TermStruct* val, const char* fn)
{

	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseInstruction()"));

	int   formation = 0;
	int   speed = 0;
	int   priority = 1;
	int   farcast = 0;
	int   hold = 0;
	int   emcon = 0;
	Vec3  loc(0, 0, 0);
	Text  order_name;
	Text  status_name;
	Text  order_rgn_name;
	Text  tgt_name;
	Text  tgt_desc;

	MissionRLocArray.Empty();

	FS_MissionInstruction NewMissionObjective;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "cmd") {
				GetDefText(order_name, pdef, fn);
				NewMissionObjective.OrderName = FString(order_name);
			}

			else if (pdef->name()->value() == "status") {
				GetDefText(status_name, pdef, fn);
				NewMissionObjective.StatusName = FString(status_name);
			}

			else if (pdef->name()->value() == "loc") {
				GetDefVec(loc, pdef, fn);
				NewMissionObjective.Location.X = loc.x;
				NewMissionObjective.Location.Y = loc.y;
				NewMissionObjective.Location.Z = loc.z;
			}

			else if (pdef->name()->value() == "rloc") {
				if (pdef->term()->isStruct()) {
					ParseRLoc(pdef->term()->isStruct(), fn);
					NewMissionObjective.RLoc = MissionRLocArray;
				}
			}

			else if (pdef->name()->value() == "rgn") {
				GetDefText(order_rgn_name, pdef, fn);
				NewMissionObjective.OrderRegionName = FString(order_rgn_name);
			}
			else if (pdef->name()->value() == "speed") {
				GetDefNumber(speed, pdef, fn);
				NewMissionObjective.Speed = speed;
			}
			else if (pdef->name()->value() == "formation") {
				GetDefNumber(formation, pdef, fn);
				NewMissionObjective.Formation = formation;
			}
			else if (pdef->name()->value() == "emcon") {
				GetDefNumber(emcon, pdef, fn);
				NewMissionObjective.EMCON = emcon;
			}
			else if (pdef->name()->value() == "priority") {
				GetDefNumber(priority, pdef, fn);
				NewMissionObjective.Priority = priority;
			}
			else if (pdef->name()->value() == "farcast") {
				if (pdef->term()->isBool()) {
					bool f = false;
					GetDefBool(f, pdef, fn);
					if (f)
						farcast = 1;
					else
						farcast = 0;
					NewMissionObjective.Farcast = farcast;
				}
				else {
					GetDefNumber(farcast, pdef, fn);
					NewMissionObjective.Farcast = farcast;
				}
			}
			else if (pdef->name()->value() == "tgt") {
				GetDefText(tgt_name, pdef, fn);
				NewMissionObjective.TargetName = FString(tgt_name);
			}
			else if (pdef->name()->value() == "tgt_desc") {
				GetDefText(tgt_desc, pdef, fn);
				NewMissionObjective.TargetDesc = FString(tgt_desc);
			}
			else if (pdef->name()->value().indexOf("hold") == 0) {
				GetDefNumber(hold, pdef, fn);
				NewMissionObjective.Hold = hold;
			}
		}
	}
	MissionObjectiveArray.Add(NewMissionObjective);
}

void
AGameDataLoader::ParseInstruction(TermStruct* val, const char* fn)
{
	
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseInstruction()"));

	int   formation = 0;
	int   speed = 0;
	int   priority = 1;
	int   farcast = 0;
	int   hold = 0;
	int   emcon = 0;
	Vec3  loc(0, 0, 0);
	Text  order_name;
	Text  status_name;
	Text  order_rgn_name;
	Text  tgt_name;
	Text  tgt_desc;

	MissionRLocArray.Empty();

	FS_MissionInstruction NewMissionInstruction;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "cmd") {
				GetDefText(order_name, pdef, fn);
				NewMissionInstruction.OrderName = FString(order_name);
			}

			else if (pdef->name()->value() == "status") {
				GetDefText(status_name, pdef, fn);
				NewMissionInstruction.StatusName = FString(status_name);
			}

			else if (pdef->name()->value() == "loc") {
				GetDefVec(loc, pdef, fn);
				NewMissionInstruction.Location.X = loc.x;
				NewMissionInstruction.Location.Y = loc.y;
				NewMissionInstruction.Location.Z = loc.z;
			}

			else if (pdef->name()->value() == "rloc") {
				if (pdef->term()->isStruct()) {
					ParseRLoc(pdef->term()->isStruct(), fn);
					NewMissionInstruction.RLoc = MissionRLocArray;
				}
			}

			else if (pdef->name()->value() == "rgn") {
				GetDefText(order_rgn_name, pdef, fn);
				NewMissionInstruction.OrderRegionName = FString(order_rgn_name);
			}
			else if (pdef->name()->value() == "speed") {
				GetDefNumber(speed, pdef, fn);
				NewMissionInstruction.Speed = speed;
			}
			else if (pdef->name()->value() == "formation") {
				GetDefNumber(formation, pdef, fn);
				NewMissionInstruction.Formation = formation;
			}
			else if (pdef->name()->value() == "emcon") {
				GetDefNumber(emcon, pdef, fn);
				NewMissionInstruction.EMCON = emcon;
			}
			else if (pdef->name()->value() == "priority") {
				GetDefNumber(priority, pdef, fn);
				NewMissionInstruction.Priority = priority;
			}
			else if (pdef->name()->value() == "farcast") {
				if (pdef->term()->isBool()) {
					bool f = false;
					GetDefBool(f, pdef, fn);
					if(f) 
						farcast = 1;
					else 
						farcast = 0;
					NewMissionInstruction.Farcast = farcast;
				}
				else {
					GetDefNumber(farcast, pdef, fn);
					NewMissionInstruction.Farcast = farcast;
				}
			}
			else if (pdef->name()->value() == "tgt") {
				GetDefText(tgt_name, pdef, fn);
				NewMissionInstruction.TargetName = FString(tgt_name);
			}
			else if (pdef->name()->value() == "tgt_desc") {
				GetDefText(tgt_desc, pdef, fn);
				NewMissionInstruction.TargetDesc = FString(tgt_desc);
			}
			else if (pdef->name()->value().indexOf("hold") == 0) {
				GetDefNumber(hold, pdef, fn);
				NewMissionInstruction.Hold = hold;
			}
		}
	}
	MissionInstructionArray.Add(NewMissionInstruction);
}

void
AGameDataLoader::ParseShip(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseShip()"));

	Text     ShipName;
	Text     SkinName;
	Text     RegNum;
	Text     Region;

	Vec3     Location(-1.0e9f, -1.0e9f, -1.0e9f);
	Vec3     Velocity(-1.0e9f, -1.0e9f, -1.0e9f);

	int      Respawns = -1;
	double   Heading = -1e9;
	double   Integrity = -1;
	int      Ammo[16];
	int      Fuel[4];
	int      index;

	FS_MissionShip NewMissionShip;

	for (index = 0; index < 16; index++)
		Ammo[index] = -10;

	for (index = 0; index < 4; index++)
		Fuel[index] = -10;

	for (index = 0; index < val->elements()->size(); index++) {
		TermDef* pdef = val->elements()->at(index)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name") {
				GetDefText(ShipName, pdef, fn);
				NewMissionShip.ShipName = FString(ShipName);
			}
			else if (pdef->name()->value() == "skin") {
				GetDefText(SkinName, pdef, fn);
				NewMissionShip.SkinName = FString(SkinName);
			}

			else if (pdef->name()->value() == "regnum") {
				GetDefText(RegNum, pdef, fn);
				NewMissionShip.RegNum = FString(RegNum);
			}
			else if (pdef->name()->value() == "region") {
				GetDefText(Region, pdef, fn);
				NewMissionShip.Region = FString(Region);
			}
			else if (pdef->name()->value() == "loc") {
				GetDefVec(Location, pdef, fn);
				NewMissionShip.Location.X = Location.x;
				NewMissionShip.Location.Y = Location.y;
				NewMissionShip.Location.Z = Location.z;
			
			}
			else if (pdef->name()->value() == "velocity") {
				GetDefVec(Velocity, pdef, fn);
				NewMissionShip.Velocity.X = Velocity.x;
				NewMissionShip.Velocity.Y = Velocity.y;
				NewMissionShip.Velocity.Z = Velocity.z;
			}

			else if (pdef->name()->value() == "respawns") {
				GetDefNumber(Respawns, pdef, fn);
				NewMissionShip.Respawns = Respawns;
			
			}
			else if (pdef->name()->value() == "heading") {
					GetDefNumber(Heading, pdef, fn);
					NewMissionShip.Heading = Heading;
			}

			else if (pdef->name()->value() == "integrity") {
				GetDefNumber(Integrity, pdef, fn);
				NewMissionShip.Integrity = Integrity;
			}

			else if (pdef->name()->value() == "ammo") {
				GetDefArray(Ammo, 16, pdef, fn);

				for (int AmmoIndex = 0; AmmoIndex < 16; AmmoIndex++) {
					NewMissionShip.Ammo[AmmoIndex] = Ammo[AmmoIndex];
				}
			}

			else if (pdef->name()->value() == "fuel") {
				GetDefArray(Fuel, 4, pdef, fn);
			
				for (int FuelIndex = 0; FuelIndex < 16; FuelIndex++) {
					NewMissionShip.Fuel[FuelIndex] = Fuel[FuelIndex];
				}
			}
		}
	}
	MissionShipArray.Add(NewMissionShip);
}

void
AGameDataLoader::ParseLoadout(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseLoadout()"));

	int  ship = -1;
	int  stations[16];
	Text LoadoutName;

	FS_MissionLoadout NewLoadout;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "ship") {
				GetDefNumber(ship, pdef, fn);
				NewLoadout.Ship = ship;
			}
			else if (pdef->name()->value() == "name") {
				GetDefText(LoadoutName, pdef, fn);
				NewLoadout.LoadoutName = FString(LoadoutName);
			}
			else if (pdef->name()->value() == "stations") {
				GetDefArray(stations, 16, pdef, fn);

				for(int index = 0; index < 16; index++) {
					NewLoadout.Stations[index] = stations[index];
				}
			}
		}
	}
	MissionLoadoutArray.Add(NewLoadout);
}

void
AGameDataLoader::ParseEvent(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseEvent()"));

	Text			EventType = "";
	Text			TriggerName = "";
	Text			EventShip = "";
	Text			EventSource = "";
	Text			EventTarget = "";
	Text			EventMessage = "";
	Text			EventSound = "";
	Text			TriggerShip = "";
	Text			TriggerTarget = "";

	int				EventId = 1;
	int				EventChance = 0;
	int				EventDelay = 0;

	double			EventTime = 0;

	Vec3			EventPoint;
	Rect			EventRect;
	int             EventParam[10];
	int				EventNParams = 0;
	int             TriggerParam[10];
	int				TriggerNParams = 0;

	FS_MissionEvent NewMissionEvent;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "type") {
				GetDefText(EventType, pdef, fn);
				NewMissionEvent.EventType = FString(EventType);
			}

			else if (pdef->name()->value() == "trigger") {
				GetDefText(TriggerName, pdef, fn);
				NewMissionEvent.TriggerName = FString(TriggerName);
			}

			else if (pdef->name()->value() == "id") {
				GetDefNumber(EventId, pdef, fn);
				NewMissionEvent.EventId = EventId;
			
			}

			else if (pdef->name()->value() == "time") {
				GetDefNumber(EventTime, pdef, fn);
				NewMissionEvent.EventTime = EventTime;
			}

			else if (pdef->name()->value() == "delay") {
				GetDefNumber(EventDelay, pdef, fn);
				NewMissionEvent.EventDelay = EventDelay;
			}
			
			else if (pdef->name()->value() == "event_param" || pdef->name()->value() == "param" || pdef->name()->value() == "color") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(EventParam[0], pdef, fn);
					EventNParams = 1;
					NewMissionEvent.EventParam[0] = EventParam[0];
					NewMissionEvent.EventNParams = EventNParams;
				}

				else if (pdef->term()->isArray()) {
					std::vector<float> plist;
					GetDefArray(plist, pdef, fn);
					EventNParams = 0;

					for (int idx = 0; i < 10 && idx < (int)plist.size(); idx++) {
						float f = plist[idx];
						EventParam[idx] = (int)f;
						EventNParams = idx + 1;
						NewMissionEvent.EventParam[idx] = EventParam[idx];
					}
					NewMissionEvent.EventNParams = EventNParams;
				}
			}

			else if (pdef->name()->value() == "trigger_param") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(TriggerParam[0], pdef, fn);
					TriggerNParams = 1;
					NewMissionEvent.TriggerParam[0] = TriggerParam[0];
					NewMissionEvent.TriggerNParams = EventNParams;
				}

				else if (pdef->term()->isArray()) {
					std::vector<float> plist;
					GetDefArray(plist, pdef, fn);

					for (int ti = 0; ti < 10 && i < (int)plist.size(); ti++) {
						float f = plist[ti];
						TriggerParam[ti] = (int)f;
						TriggerNParams = ti + 1;
						NewMissionEvent.TriggerParam[ti] = TriggerParam[ti];
					}
					NewMissionEvent.TriggerNParams = TriggerNParams;
				}
			}

			else if (pdef->name()->value() == "event_ship" || pdef->name()->value() == "ship") {
				GetDefText(EventShip, pdef, fn);
				NewMissionEvent.EventShip = FString(EventShip);
			}

			else if (pdef->name()->value() == "event_source" || pdef->name()->value() == "source" || pdef->name()->value() == "font") {
				GetDefText(EventSource, pdef, fn);
				NewMissionEvent.EventSource = FString(EventSource);
			}
			else if (pdef->name()->value() == "event_target" || pdef->name()->value() == "target" || pdef->name()->value() == "image") {
				GetDefText(EventTarget, pdef, fn);
				NewMissionEvent.EventTarget = FString(EventTarget);
			}

			else if (pdef->name()->value() == "event_message" || pdef->name()->value() == "message") {
				GetDefText(EventMessage, pdef, fn);
				NewMissionEvent.EventMessage = FString(EventMessage);
			}

			else if (pdef->name()->value() == "event_chance" || pdef->name()->value() == "chance") {
				GetDefNumber(EventChance, pdef, fn);
				NewMissionEvent.EventChance = EventChance;
			}
			else if (pdef->name()->value() == "event_sound" || pdef->name()->value() == "sound") {
				GetDefText(EventSound, pdef, fn);
				NewMissionEvent.EventSound = FString(EventSound);
			}

			else if (pdef->name()->value() == "loc" || pdef->name()->value() == "vec" || pdef->name()->value() == "fade") {
				GetDefVec(EventPoint, pdef, fn);
				NewMissionEvent.EventPoint.X = EventPoint.x;
				NewMissionEvent.EventPoint.Y = EventPoint.y;
				NewMissionEvent.EventPoint.Z = EventPoint.z;
			}

			else if (pdef->name()->value() == "rect") {
				GetDefRect(EventRect, pdef, fn);
				NewMissionEvent.EventRect.X = EventRect.x;
				NewMissionEvent.EventRect.Y = EventRect.y;
				NewMissionEvent.EventRect.Z = EventRect.h;
				NewMissionEvent.EventRect.W = EventRect.w;
			}
			else if (pdef->name()->value() == "trigger_ship") {
				GetDefText(TriggerShip, pdef, fn);
				NewMissionEvent.TriggerShip = FString(TriggerShip);
			}

			else if (pdef->name()->value() == "trigger_target") {
				GetDefText(TriggerTarget, pdef, fn);
				NewMissionEvent.TriggerTarget = FString(TriggerTarget);
			}
		}
	}
	MissionEventArray.Add(NewMissionEvent);
}

void
AGameDataLoader::ParseElement(TermStruct* eval, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseElement()"));

	Text  ElementName = "";
	Text  Carrier = "";
	Text  Commander = "";
	Text  Squadron = "";
	Text  Path = "";
	Text  Design = "";
	Text  SkinName = "";
	Text  RoleName = "";
	Text  RegionName = "";
	Text  Instr = "";
	Text  Intel = "";

	Vec3  Loc(0.0f, 0.0f, 0.0f);

	int   Deck = 1;
	int   IFFCode = 0;
	int   Count = 1;
	int   MaintCount = 0;
	int   DeadCount = 0;
	int   Player = 0;
	int   CommandAI = 0;
	int   Respawns = 0;
	int   HoldTime = 0;
	int   ZoneLock = 0;
	int   Heading = 0;

	bool Alert = false;
	bool Playable = false;
	bool Rogue = false;
	bool Invulnerable = false;

	MissionLoadoutArray.Empty();
	MissionRLocArray.Empty();
	MissionInstructionArray.Empty();
	MissionNavpointArray.Empty();
	MissionShipArray.Empty();

	FS_MissionElement NewMissionElement;

	for (int i = 0; i < eval->elements()->size(); i++) {
		TermDef* pdef = eval->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name") {
				GetDefText(ElementName, pdef, fn);
				NewMissionElement.Name = FString(ElementName);
			}
			else if (pdef->name()->value() == "carrier") {
				GetDefText(Carrier, pdef, fn);
				NewMissionElement.Carrier = FString(Carrier);
			}
			else if (pdef->name()->value() == "commander") {
				GetDefText(Commander, pdef, fn);
				NewMissionElement.Commander = FString(Commander);
			}
			else if (pdef->name()->value() == "squadron") {
				GetDefText(Squadron, pdef, fn);
				NewMissionElement.Squadron = FString(Squadron);
			}
			else if (pdef->name()->value() == "path") {
				GetDefText(Path, pdef, fn);
				NewMissionElement.Path = FString(Path);
			}
			else if (pdef->name()->value() == "design") {
				GetDefText(Design, pdef, fn);
				NewMissionElement.Design = FString(Design);
			}
			else if (pdef->name()->value() == "skin") {
				GetDefText(SkinName, pdef, fn);
				NewMissionElement.SkinName = FString(SkinName);
			}
			else if (pdef->name()->value() == "mission") {
				GetDefText(RoleName, pdef, fn);
				NewMissionElement.RoleName = FString(RoleName);
			}
			else if (pdef->name()->value() == "intel") {
				GetDefText(RoleName, pdef, fn);
				NewMissionElement.Intel = FString(Intel);
			}

			else if (pdef->name()->value() == "loc") {
				GetDefVec(Loc, pdef, fn);
				NewMissionElement.Location.X = Loc.x;
				NewMissionElement.Location.Y = Loc.y;
				NewMissionElement.Location.Z = Loc.z;
			}

			else if (pdef->name()->value() == "rloc") {
				if (pdef->term()->isStruct()) {
					ParseLoadout(pdef->term()->isStruct(), fn);
					NewMissionElement.RLoc = MissionRLocArray;
				}
			}

			else if (pdef->name()->value() == "head") {
				GetDefNumber(Heading, pdef, fn);
				NewMissionElement.Heading = Heading;
			}

			else if (pdef->name()->value() == "region" || pdef->name()->value() == "rgn") {
				GetDefText(RegionName, pdef, fn);
				NewMissionElement.RegionName = FString(RegionName);
			}

			else if (pdef->name()->value() == "iff") {
				GetDefNumber(IFFCode, pdef, fn);
				NewMissionElement.IFFCode = IFFCode;

			}
			else if (pdef->name()->value() == "count") {
				GetDefNumber(Count, pdef, fn);
				NewMissionElement.Count = Count;

			}
			else if (pdef->name()->value() == "maint_count") {
				GetDefNumber(MaintCount, pdef, fn);
				NewMissionElement.MaintCount = MaintCount;
			}
			else if (pdef->name()->value() == "dead_count") {
				GetDefNumber(DeadCount, pdef, fn);
				NewMissionElement.DeadCount = DeadCount;
			}
			else if (pdef->name()->value() == "player") {
				GetDefNumber(Player, pdef, fn);
				NewMissionElement.Player = Player;
			}
			else if (pdef->name()->value() == "alert") {
				GetDefBool(Alert, pdef, fn);
				NewMissionElement.Alert = Alert;
			}
			else if (pdef->name()->value() == "playable") {
				GetDefBool(Playable, pdef, fn);
				NewMissionElement.Playable = Playable;
			}
			else if (pdef->name()->value() == "rogue") {
				GetDefBool(Rogue, pdef, fn);
				NewMissionElement.Rogue = Rogue;
			}
			else if (pdef->name()->value() == "invulnerable") {
				GetDefBool(Invulnerable, pdef, fn);
				NewMissionElement.Invulnerable = Invulnerable;
			}
			else if (pdef->name()->value() == "command_ai") {
				GetDefNumber(CommandAI, pdef, fn);
				NewMissionElement.CommandAI = CommandAI;
			}
			else if (pdef->name()->value() == "respawn") {
				GetDefNumber(Respawns, pdef, fn);
				NewMissionElement.Respawns = Respawns;
			}
			else if (pdef->name()->value() == "hold") {
				GetDefNumber(HoldTime, pdef, fn);
				NewMissionElement.HoldTime = HoldTime;
			}
			else if (pdef->name()->value() == "zone") {
				GetDefNumber(ZoneLock, pdef, fn);
				NewMissionElement.ZoneLock = ZoneLock;
			}
			else if (pdef->name()->value() == "objective") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("Mission error - No objective"));
				}
				else {
					ParseInstruction(pdef->term()->isStruct(), fn);
					NewMissionElement.Instruction = MissionInstructionArray;
				}
			}
			else if (pdef->name()->value() == "instr") {
				GetDefText(Instr, pdef, fn);
				NewMissionElement.Instr = FString(Instr);
			}

			else if (pdef->name()->value() == "ship") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("Mission error - no ship"));
				}
				else {
					ParseShip(pdef->term()->isStruct(), fn);
					NewMissionElement.Ship = MissionShipArray;
				}
			}

			else if (pdef->name()->value() == "order" || pdef->name()->value() == "navpt") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("Mission error - no navpt"));

				}
				else {
					ParseNavpoint(pdef->term()->isStruct(), fn);
					NewMissionElement.Navpoint = MissionNavpointArray;
				}
			}

			else if (pdef->name()->value() == "loadout") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("Mission error - no loadout"));

				}
				else {
					ParseLoadout(pdef->term()->isStruct(), fn);
					NewMissionElement.Loadout = MissionLoadoutArray;
				}
			}
		}
	}
	MissionElementArray.Add(NewMissionElement);
}


// +--------------------------------------------------------------------+

void
AGameDataLoader::ParseScriptedTemplate(const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseMissionTemplate()"));

	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(fn);

	BYTE* block = 0;
	SSWInstance->loader->LoadBuffer(fn, block, true);

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	FString fs = FString(ANSI_TO_TCHAR(fn));
	FString FileString;

	MissionElementArray.Empty();
	MissionEventArray.Empty();
	MissionCallsignArray.Empty();
	MissionOptionalArray.Empty();
	MissionAliasArray.Empty();

	if (FFileHelper::LoadFileToString(FileString, *fs, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);

		const char* result = TCHAR_TO_ANSI(*FileString);
	}
	if (!term) {
		UE_LOG(LogTemp, Log, TEXT("WARNING: could not parse '%s'"), *FString(fn));
		return;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("MISSIONTEMPLATE file '%s'"), *FString(fn));
	}

	Text  TargetName = "";
	Text  WardName = "";
	Text  TemplateName = "";
	Text  TemplateSystem = "";
	Text  TemplateRegion = "";
	Text  TemplateObjective = "";
	Text  TemplateSitrep = "";
	Text  TemplateStart = "";

	int TemplateType = 0;
	int TemplateTeam;

	bool TemplateDegrees = false;

	FS_TemplateMission NewTemplateMission;

	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				Text defname = def->name()->value();

				if (def->name()->value() == "name") {
					GetDefText(TemplateName, def, fn);
					NewTemplateMission.TemplateName = FString(TemplateName);
				}
				else if (def->name()->value() == "type") {
					char typestr[64];
					GetDefText(typestr, def, fn);
					TemplateType = Mission::TypeFromName(typestr);
					NewTemplateMission.TemplateType = TemplateType;
				}

				else if (def->name()->value() == "system") {
					GetDefText(TemplateSystem, def, fn);
					NewTemplateMission.TemplateSystem = FString(TemplateSystem);
				}

				else if (def->name()->value() == "degrees") {
					GetDefBool(TemplateDegrees, def, fn);
					NewTemplateMission.TemplateDegrees = TemplateDegrees;
				}
				else if (def->name()->value() == "region") {
					GetDefText(TemplateRegion, def, fn);
					NewTemplateMission.TemplateRegion = FString(TemplateRegion);
				}

				else if (def->name()->value() == "objective") {
					GetDefText(TemplateObjective, def, fn);
					NewTemplateMission.TemplateObjective = FString(TemplateObjective);
				}

				else if (def->name()->value() == "sitrep") {
					GetDefText(TemplateSitrep, def, fn);
					NewTemplateMission.TemplateSitrep = FString(TemplateSitrep);
				}

				else if (def->name()->value() == "start") {
					//GetDefTime(start, def, fn);
					GetDefText(TemplateStart, def, fn);
					NewTemplateMission.TemplateStart = FString(TemplateStart);
				}
				else if (def->name()->value() == "team") {
					GetDefNumber(TemplateTeam, def, fn);
					NewTemplateMission.TemplateTeam = TemplateTeam;
				}

				else if (def->name()->value() == "target") {
					GetDefText(TargetName, def, fn);
					NewTemplateMission.TargetName = FString(TargetName);
				}

				else if (def->name()->value() == "ward") {
					GetDefText(WardName, def, fn);
					NewTemplateMission.WardName = FString(WardName);
				}

				else if ((def->name()->value() == "alias")) {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: alias struct missing in '%s'"), *FString(fn));
					}
					else {
						ParseAlias(def->term()->isStruct(), fn);
						NewTemplateMission.Alias = MissionAliasArray;
					}
				}

				else if ((def->name()->value() == "callsign")) {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: callsign struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseCallsign(val, fn);
						NewTemplateMission.Callsign = MissionCallsignArray;
					}
				}

				else if (def->name()->value() == "optional") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: optional group struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseOptional(val, fn);
						NewTemplateMission.Optional = MissionOptionalArray;
					}
				}

				else if (def->name()->value() == "element") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: element struct missing in '%s'\n", fn);
					}
					else {
						ParseElement(def->term()->isStruct(), fn);
						NewTemplateMission.Element = MissionElementArray;
					}
				}

				else if (def->name()->value() == "event") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: event struct missing in '%s'"), *FString(fn));
					}
					else {
						ParseEvent(def->term()->isStruct(), fn);
						NewTemplateMission.Event = MissionEventArray;;
					}
				}
			}     // def
		}        // term
	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
	ScriptedMissionArray.Add(NewTemplateMission);
}
// +--------------------------------------------------------------------+

void
AGameDataLoader::ParseMissionTemplate(const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseMissionTemplate()"));

	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(fn);

	BYTE* block = 0;
	SSWInstance->loader->LoadBuffer(fn, block, true);

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	FString fs = FString(ANSI_TO_TCHAR(fn));
	FString FileString;

	MissionElementArray.Empty();
	MissionEventArray.Empty();
	MissionCallsignArray.Empty();
	MissionOptionalArray.Empty();
	MissionAliasArray.Empty();

	if (FFileHelper::LoadFileToString(FileString, *fs, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);

		const char* result = TCHAR_TO_ANSI(*FileString);
	}
	if (!term) {
		UE_LOG(LogTemp, Log, TEXT("WARNING: could not parse '%s'"), *FString(fn));
		return;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("MISSIONTEMPLATE file '%s'"), *FString(fn));
	}

	Text  TargetName = "";
	Text  WardName = "";
	Text  TemplateName = "";
	Text  TemplateSystem = "";
	Text  TemplateRegion = "";
	Text  TemplateObjective = "";
	Text  TemplateSitrep = "";
	Text  TemplateStart = "";

	int TemplateType = 0;
	int TemplateTeam;

	bool TemplateDegrees = false;

	FS_TemplateMission NewTemplateMission;

	do {
		delete term; 
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				Text defname = def->name()->value();

				if (def->name()->value() == "name") {
					GetDefText(TemplateName, def, fn);
					NewTemplateMission.TemplateName = FString(TemplateName);
				}
				else if (def->name()->value() == "type") {
					char typestr[64];
					GetDefText(typestr, def, fn);
					TemplateType = Mission::TypeFromName(typestr);
					NewTemplateMission.TemplateType = TemplateType;
				}

				else if (def->name()->value() == "system") {
					GetDefText(TemplateSystem, def, fn);
					NewTemplateMission.TemplateSystem = FString(TemplateSystem);
				}

				else if (def->name()->value() == "degrees") {
					GetDefBool(TemplateDegrees, def, fn);
					NewTemplateMission.TemplateDegrees = TemplateDegrees;
				}
				else if (def->name()->value() == "region") {
					GetDefText(TemplateRegion, def, fn);
					NewTemplateMission.TemplateRegion = FString(TemplateRegion);
				}

				else if (def->name()->value() == "objective") {
					GetDefText(TemplateObjective, def, fn);
					NewTemplateMission.TemplateObjective = FString(TemplateObjective);
				}

				else if (def->name()->value() == "sitrep") {
					GetDefText(TemplateSitrep, def, fn);
					NewTemplateMission.TemplateSitrep = FString(TemplateSitrep);
				}

				else if (def->name()->value() == "start") {
					//GetDefTime(start, def, fn);
					GetDefText(TemplateStart, def, fn);
					NewTemplateMission.TemplateStart = FString(TemplateStart);
				}
				else if (def->name()->value() == "team") {
					GetDefNumber(TemplateTeam, def, fn);
					NewTemplateMission.TemplateTeam = TemplateTeam;
				}

				else if (def->name()->value() == "target") {
					GetDefText(TargetName, def, fn);
					NewTemplateMission.TargetName = FString(TargetName);
				}

				else if (def->name()->value() == "ward") {
					GetDefText(WardName, def, fn);
					NewTemplateMission.WardName = FString(WardName);
				}

				else if ((def->name()->value() == "alias")) {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: alias struct missing in '%s'"), *FString(fn));
					}
					else {
						ParseAlias(def->term()->isStruct(), fn);
						NewTemplateMission.Alias = MissionAliasArray;
					}
				}

				else if ((def->name()->value() == "callsign")) {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: callsign struct missing in '%s'"), *FString(fn));
					}
					else {
						ParseCallsign(def->term()->isStruct(), fn);
						NewTemplateMission.Callsign = MissionCallsignArray;
					}
				}

				else if (def->name()->value() == "optional") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: optional group struct missing in '%s'"), *FString(fn));
					}
					else {
						ParseOptional(def->term()->isStruct(), fn);
						NewTemplateMission.Optional = MissionOptionalArray;
					}
				}

				else if (def->name()->value() == "element") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: element struct missing in '%s'\n", fn);
					}
					else {
						ParseElement(def->term()->isStruct(), fn);
						NewTemplateMission.Element = MissionElementArray;
					}
				}

				else if (def->name()->value() == "event") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: event struct missing in '%s'"), *FString(fn));
					}
					else {
						ParseEvent(def->term()->isStruct(), fn);
						NewTemplateMission.Event = MissionEventArray;
					}
				}
			}     // def
		}        // term
	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
	TemplateMissionArray.Add(NewTemplateMission);
}
// +--------------------------------------------------------------------+
void
AGameDataLoader::ParseAlias(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseAlias()"));

	Text  AliasName;
	Text  Design;
	Text  Code;
	Text  ElementName;
	Text  Mission;
	int   iff = -1;
	int   player = 0;
	RLoc* rloc = 0;
	bool  UseLocation = false;
	Vec3  Location;

	MissionRLocArray.Empty();
	MissionNavpointArray.Empty();
	MissionObjectiveArray.Empty();

	FS_MissionAlias NewMissionAlias;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {

			if (pdef->name()->value() == "name") {
				GetDefText(AliasName, pdef, fn);
				NewMissionAlias.AliasName = FString(AliasName);
			}
			else if (pdef->name()->value() == "elem") {
				GetDefText(ElementName, pdef, fn);
				NewMissionAlias.ElementName = FString(ElementName);
			}
			else if (pdef->name()->value() == "code") {
				GetDefText(Code, pdef, fn);
				NewMissionAlias.Code = FString(Code);
			}
			else if (pdef->name()->value() == "design") {
				GetDefText(Design, pdef, fn);
				NewMissionAlias.Design = FString(Design);
			}
			else if (pdef->name()->value() == "mission") {
				GetDefText(Mission, pdef, fn);
				NewMissionAlias.Mission = FString(Mission);
			}
			else if (pdef->name()->value() == "iff") {
				GetDefNumber(iff, pdef, fn);
				NewMissionAlias.Iff = iff;
			}
			else if (pdef->name()->value() == "loc") {
				GetDefVec(Location, pdef, fn);
				UseLocation = true;
				NewMissionAlias.Location.X = Location.x;
				NewMissionAlias.Location.Y = Location.y;
				NewMissionAlias.Location.Z = Location.z;
				NewMissionAlias.UseLocation = UseLocation;
			}

			else if (pdef->name()->value() == "rloc") {
				if (pdef->term()->isStruct()) {
					ParseRLoc(pdef->term()->isStruct(), fn);
					NewMissionAlias.RLoc = MissionRLocArray;
				}
			}

			else if (pdef->name()->value() == "player") {
				GetDefNumber(player, pdef, fn);
				if (player && !Code.length()) {
					Code = "player";
					NewMissionAlias.Code = FString(Code);
					NewMissionAlias.Player = player;
				}
			}
			else if (pdef->name()->value() == "navpt") {
				if (pdef->term()->isStruct()) {
					ParseNavpoint(pdef->term()->isStruct(), fn);
					NewMissionAlias.Navpoint = MissionNavpointArray;
				}
			}
			else if (pdef->name()->value() == "objective") {
				if (pdef->term()->isStruct()) {
					ParseObjective(pdef->term()->isStruct(), fn);
					NewMissionAlias.Objective = MissionObjectiveArray;
				}
			}
		}
	}
	MissionAliasArray.Add(NewMissionAlias);
}

void
AGameDataLoader::ParseRLoc(TermStruct* rval, const char* fn)
{
	Vec3     BaseLocation;
	Text     Reference;

	double   dex = 0;
	double   dex_var = 5e3;
	double   az = 0;
	double   az_var = 3.1415;
	double   el = 0;
	double   el_var = 0.1;
	FS_RLoc NewRLocElement;

	for (int index = 0; index < rval->elements()->size(); index++) {
		TermDef* rdef = rval->elements()->at(index)->isDef();
		if (rdef) {
			if (rdef->name()->value() == "dex") {
				GetDefNumber(dex, rdef, fn);
				NewRLocElement.Dex = dex;

			}
			else if (rdef->name()->value() == "dex_var") {
				GetDefNumber(dex_var, rdef, fn);
				NewRLocElement.DexVar = dex_var;

			}
			else if (rdef->name()->value() == "az") {
				GetDefNumber(az, rdef, fn);
				NewRLocElement.Azimuth = az;

			}
			else if (rdef->name()->value() == "az_var") {
				GetDefNumber(az_var, rdef, fn);
				NewRLocElement.AzimuthVar = az_var;

			}
			else if (rdef->name()->value() == "el") {
				GetDefNumber(el, rdef, fn);
				NewRLocElement.Elevation = el;

			}
			else if (rdef->name()->value() == "el_var") {
				GetDefNumber(el_var, rdef, fn);
				NewRLocElement.ElevationVar = el_var;

			}
			else if (rdef->name()->value() == "loc") {
				GetDefVec(BaseLocation, rdef, fn);
				NewRLocElement.BaseLocation.X = BaseLocation.x;
				NewRLocElement.BaseLocation.Y = BaseLocation.y;
				NewRLocElement.BaseLocation.Z = BaseLocation.z;
			}

			else if (rdef->name()->value() == "ref") {
				GetDefText(Reference, rdef, fn);
				NewRLocElement.Reference = FString(Reference);
			}
		}
	}
	MissionRLocArray.Add(NewRLocElement);
}

// +--------------------------------------------------------------------+

void
AGameDataLoader::ParseCallsign(TermStruct* val, const char* fn)
{
	Text  CallsignName;
	int   Iff = -1;

	FS_MissionCallsign NewMissionCallsign;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name") {
				GetDefText(CallsignName, pdef, fn);
				NewMissionCallsign.Callsign = FString(CallsignName);
			}

			else if (pdef->name()->value() == "iff") {
				GetDefNumber(Iff, pdef, fn);
				NewMissionCallsign.Iff = Iff;
			}
		}
	}
	MissionCallsignArray.Add(NewMissionCallsign);
}

// +--------------------------------------------------------------------+

void
AGameDataLoader::ParseOptional(TermStruct* val, const char* fn)
{
	int   min = 0;
	int   max = 1000;
	int   total = val->elements()->size();

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "min") {
				GetDefNumber(min, pdef, fn);
			}

			else if (pdef->name()->value() == "max") {
				GetDefNumber(max, pdef, fn);
			}

			else if (pdef->name()->value() == "element") {
				//ParseElement(pdef->term()->isStruct(), fn);

			}
		}
	}
}

// +--------------------------------------------------------------------+

void
AGameDataLoader::LoadGalaxyMap()
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadGalaxyMap()"));

	FString ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Galaxy/"));
	FString FileName = ProjectPath;
	FileName.Append("Galaxy.def");
	const char* fn = TCHAR_TO_ANSI(*FileName);

	SSWInstance->loader->GetLoader();

	FString FileString;
	BYTE* block = 0;

	SSWInstance->loader->LoadBuffer(fn, block, true);

	UE_LOG(LogTemp, Log, TEXT("Loading Galaxy: %s"), *FileName);

	if (FFileHelper::LoadFileToString(FileString, *FileName, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);
	}

	Parser parser(new BlockReader((const char*)block));

	Term* term = parser.ParseTerm();

	if (!term) {
		UE_LOG(LogTemp, Log, TEXT("WARNING: could not parse '%s'"), *FileName);
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "GALAXY") {
			UE_LOG(LogTemp, Log, TEXT("WARNING: invalid galaxy file '%s'"), *FileName);
			return;
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("Galaxy file '%s'"), *FileName);
		}
	}

	FS_Galaxy NewGalaxyData;

	// parse the galaxy:
	do {
		delete term;
		term = parser.ParseTerm();
		FVector fv;

		double Radius;
		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "radius") {
					GetDefNumber(Radius, def, fn);
				}

				else if (def->name()->value() == "system") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: system struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();

						UE_LOG(LogTemp, Log, TEXT("%s"), *FString(def->name()->value()));
						Text  SystemName;
						Text  ClassName;
						Vec3  SystemLocation;
						int   SystemIff = 0;
						int   StarClass = (int8)ESPECTRAL_CLASS::G;

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "name") {
									GetDefText(SystemName, pdef, fn);
									NewGalaxyData.Name = FString(SystemName);
								}
								else if (pdef->name()->value() == "loc") {

									GetDefVec(SystemLocation, pdef, fn);
									fv = FVector(SystemLocation.x, SystemLocation.y, SystemLocation.z);
									NewGalaxyData.Location = fv;
								}
								else if (pdef->name()->value() == "iff") {
									GetDefNumber(SystemIff, pdef, fn);
									NewGalaxyData.Iff = SystemIff;
								}
								else if (pdef->name()->value() == "class") {
									GetDefText(ClassName, pdef, fn);

									switch (ClassName[0]) {
									case 'B':
										StarClass = (int8)ESPECTRAL_CLASS::B;
										break;
									case 'A':
										StarClass = (int8)ESPECTRAL_CLASS::A;
										break;
									case 'F':
										StarClass = (int8)ESPECTRAL_CLASS::F;
										break;
									case 'G':
										StarClass = (int8)ESPECTRAL_CLASS::G;
										break;
									case 'K':
										StarClass = (int8)ESPECTRAL_CLASS::K;
										break;
									case 'M':
										StarClass = (int8)ESPECTRAL_CLASS::M;
										break;
									case 'R':
										StarClass = (int8)ESPECTRAL_CLASS::RED_GIANT;
										break;
									case 'W':
										StarClass = (int8)ESPECTRAL_CLASS::WHITE_DWARF;
										break;
									case 'Z':
										StarClass = (int8)ESPECTRAL_CLASS::BLACK_HOLE;
										break;
									}
									NewGalaxyData.Class = StarClass;
								}
							}
						}

						// define our data table struct
			
						NewGalaxyData.Empire = GetEmpireName(SystemIff);
						FName RowName = FName(FString(SystemName));

						// call AddRow to insert the record
						GalaxyDataTable->AddRow(RowName, NewGalaxyData);

						GalaxyData = NewGalaxyData;
					}
				}

				else if (def->name()->value() == "star") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: star struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						UE_LOG(LogTemp, Log, TEXT("%s"), *FString(def->name()->value()));
						char  star_name[32];
						char  classname[32];
						Vec3  star_loc;
						int   star_class = (int8)ESPECTRAL_CLASS::G;

						star_name[0] = 0;

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "name")
									GetDefText(star_name, pdef, fn);

								else if (pdef->name()->value() == "loc")
									GetDefVec(star_loc, pdef, fn);

								else if (pdef->name()->value() == "class") {
									GetDefText(classname, pdef, fn);

									switch (classname[0]) {
									case 'O':
										star_class = (int8)ESPECTRAL_CLASS::O;
										break;
									case 'B':
										star_class = (int8)ESPECTRAL_CLASS::B;
										break;
									case 'A':
										star_class = (int8)ESPECTRAL_CLASS::A;
										break;
									case 'F':
										star_class = (int8)ESPECTRAL_CLASS::F;
										break;
									case 'G':
										star_class = (int8)ESPECTRAL_CLASS::G;
										break;
									case 'K':
										star_class = (int8)ESPECTRAL_CLASS::K;
										break;
									case 'M':
										star_class = (int8)ESPECTRAL_CLASS::M;
										break;
									case 'R':
										star_class = (int8)ESPECTRAL_CLASS::RED_GIANT;
										break;
									case 'W':
										star_class = (int8)ESPECTRAL_CLASS::WHITE_DWARF;
										break;
									case 'Z':
										star_class = (int8)ESPECTRAL_CLASS::BLACK_HOLE;
										break;
									}
								}
							}
						}
					}
				}
			}
			UE_LOG(LogTemp, Log, TEXT("------------------------------------------------------------"));
		}
	} while (term);
	SSWInstance->loader->ReleaseBuffer(block);
}

void
AGameDataLoader::ParseStar(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseStar()"));

	Text  StarName = "";
	Text  ImgName = "";
	Text  MapName = "";
	double Light = 0.0;
	double Radius = 0.0;
	double Rot = 0.0;
	double Mass = 0.0;
	double Orbit = 0.0;
	double Tscale = 1.0;
	bool   Retro = false;

	FS_Star NewStarData;
	PlanetDataArray.Empty();

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name") {
				GetDefText(StarName, pdef, fn);
				NewStarData.Name = FString(StarName);
			}
			else if (pdef->name()->value() == "map" || pdef->name()->value() == "icon") {
				GetDefText(MapName, pdef, fn);
				NewStarData.Map = FString(MapName);
			}
			else if (pdef->name()->value() == "image") {
				GetDefText(ImgName, pdef, fn);
				NewStarData.Image = FString(ImgName);
			}
			else if (pdef->name()->value() == "mass") {
				GetDefNumber(Mass, pdef, fn);
				NewStarData.Mass = Mass;
			}
			else if (pdef->name()->value() == "orbit") {
				GetDefNumber(Orbit, pdef, fn);
				NewStarData.Orbit = Orbit;
			}
			else if (pdef->name()->value() == "radius") {
				GetDefNumber(Radius, pdef, fn);
				NewStarData.Radius = Radius;
			}
			else if (pdef->name()->value() == "rotation") {
				GetDefNumber(Rot, pdef, fn);
				NewStarData.Rot = Rot;
			}
			else if (pdef->name()->value() == "tscale") {
				GetDefNumber(Tscale, pdef, fn);
				NewStarData.Tscale = Tscale;
			}
			else if (pdef->name()->value() == "light") {
				GetDefNumber(Light, pdef, fn);
				NewStarData.Light = Light;
			}
			else if (pdef->name()->value() == "retro") {
				GetDefBool(Retro, pdef, fn);
				NewStarData.Retro = Retro;
			}
			else if (pdef->name()->value() == "color") {
				Vec3 a;
				GetDefVec(a, pdef, fn);
				NewStarData.Color = FColor(a.x, a.y, a.z, 1);
			}

			else if (pdef->name()->value() == "back" || pdef->name()->value() == "back_color") {
				Vec3 a;
				GetDefVec(a, pdef, fn);
				NewStarData.Back = FColor(a.x, a.y, a.z, 1);
			}
			else if (pdef->name()->value() == "planet") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: planet struct missing in '%s'"), *FString(fn));
				}
				else {
					ParsePlanet(pdef->term()->isStruct(), fn);
					NewStarData.Planet = PlanetDataArray;
				}
			}
		}	
	}
	StarDataArray.Add(NewStarData);
}

void AGameDataLoader::ParsePlanet(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParsePlanet()"));

	Text   PlanetName = "";
	Text   ImgName = "";
	Text   MapName = "";
	Text   HiName = "";
	Text   ImgRing= "";
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
	FColor AtmosColor = FColor::Black;
	
	FS_Planet NewPlanetData;
	MoonDataArray.Empty();

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name") {
				GetDefText(PlanetName, pdef, fn);
				NewPlanetData.Name = FString(PlanetName);
			}
			else if (pdef->name()->value() == "map" || pdef->name()->value() == "icon") {
				GetDefText(MapName, pdef, fn);
				NewPlanetData.Map = FString(MapName);
			}
			else if (pdef->name()->value() == "image" || pdef->name()->value() == "image_west" || pdef->name()->value() == "image_east") {
				GetDefText(ImgName, pdef, fn);
				NewPlanetData.Image = FString(ImgName);
			}
			else if (pdef->name()->value() == "glow") {
				GetDefText(GloName, pdef, fn);
				NewPlanetData.Glow = FString(GloName);
			}
			else if (pdef->name()->value() == "gloss") {
				GetDefText(GlossName, pdef, fn);
				NewPlanetData.Gloss = FString(GlossName);
			}
			else if (pdef->name()->value() == "high_res" || pdef->name()->value() == "high_res_west" || pdef->name()->value() == "high_res_east") {
				GetDefText(HiName, pdef, fn);
				NewPlanetData.High = FString(HiName);
			}
			else if (pdef->name()->value() == "glow_high_res") {
				GetDefText(GloHiName, pdef, fn);
				NewPlanetData.GlowHigh = FString(GloHiName);
			}
			else if (pdef->name()->value() == "mass") {
				GetDefNumber(Mass, pdef, fn);
				NewPlanetData.Mass = Mass;
			}
			else if (pdef->name()->value() == "orbit") {
				GetDefNumber(Orbit, pdef, fn);
				NewPlanetData.Orbit = Orbit;
			}
			else if (pdef->name()->value() == "retro") {
				GetDefBool(Retro, pdef, fn);
				NewPlanetData.Retro = Retro;
			}
			else if (pdef->name()->value() == "luminous") {
				GetDefBool(Lumin, pdef, fn);
				NewPlanetData.Lumin = Lumin;
			}
			else if (pdef->name()->value() == "rotation") {
				GetDefNumber(Rot, pdef, fn);
				NewPlanetData.Rot = Rot;
			}
			else if (pdef->name()->value() == "radius") {
				GetDefNumber(Radius, pdef, fn);
				NewPlanetData.Radius = Radius;
			}	
			else if (pdef->name()->value() == "ring") {
				GetDefText(ImgRing, pdef, fn);
				NewPlanetData.Rings = FString(ImgRing);
			}
			else if (pdef->name()->value() == "minrad") {
				GetDefNumber(Minrad, pdef, fn);
				NewPlanetData.Minrad = Minrad;
			}
			else if (pdef->name()->value() == "maxrad") {
				GetDefNumber(Maxrad, pdef, fn);
				NewPlanetData.Maxrad = Maxrad;
			}
			else if (pdef->name()->value() == "tscale") {
				GetDefNumber(Tscale, pdef, fn);
				NewPlanetData.Tscale = Tscale;
			}
			else if (pdef->name()->value() == "tilt") {
				GetDefNumber(Tilt, pdef, fn);
				NewPlanetData.Tilt = Tilt;
			}
			else if (pdef->name()->value() == "atmosphere") {
				Vec3 a;
				GetDefVec(a, pdef, fn);
				AtmosColor = FColor(a.x, a.y, a.z, 1);
				NewPlanetData.Atmos = AtmosColor;
			}
			else if (pdef->name()->value() == "moon") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: moon struct missing in '%s'"), *FString(fn));
				}
				else {
					ParseMoon(pdef->term()->isStruct(), fn);
					NewPlanetData.Moon = MoonDataArray;
				}
			}
		}
	}
	PlanetDataArray.Add(NewPlanetData);
}

void AGameDataLoader::ParseMoon(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseMoon()"));

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
	FColor AtmosColor = FColor::Black;

	FS_Moon NewMoonData;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name") {
				GetDefText(MoonName, pdef, fn);
				NewMoonData.Name = FString(MoonName);
			}
			else if (pdef->name()->value() == "map" || pdef->name()->value() == "icon") {
				GetDefText(MapName, pdef, fn);
				NewMoonData.Map = FString(MapName);
			}
			else if (pdef->name()->value() == "image") {
				GetDefText(ImgName, pdef, fn);
				NewMoonData.Image = FString(ImgName);
			}
			else if (pdef->name()->value() == "glow") {
				GetDefText(GloName, pdef, fn);
				NewMoonData.Glow = FString(GloName);
			}
			else if (pdef->name()->value() == "high_res") {
				GetDefText(HiName, pdef, fn);
				NewMoonData.High = FString(HiName);
			}
			else if (pdef->name()->value() == "glow_high_res") {
				GetDefText(GloHiName, pdef, fn);
				NewMoonData.GlowHigh = FString(GloHiName);
			}
			else if (pdef->name()->value() == "gloss") {
				GetDefText(GlossName, pdef, fn);
				NewMoonData.Gloss = FString(GlossName);
			}
			else if (pdef->name()->value() == "mass") {
				GetDefNumber(Mass, pdef, fn);
				NewMoonData.Mass = Mass;
			}
			else if (pdef->name()->value() == "orbit") {
				GetDefNumber(Orbit, pdef, fn);
				NewMoonData.Orbit = Orbit;
			}
			else if (pdef->name()->value() == "rotation") {
				GetDefNumber(Rot, pdef, fn);
				NewMoonData.Rot = Rot;
			}
			else if (pdef->name()->value() == "retro") {
				GetDefBool(Retro, pdef, fn);
				NewMoonData.Retro = Retro;
			}
			else if (pdef->name()->value() == "radius") {
				GetDefNumber(Radius, pdef, fn);
				NewMoonData.Radius = Radius;
			}
			else if (pdef->name()->value() == "tscale") {
				GetDefNumber(Tscale, pdef, fn);
				NewMoonData.Tscale = Tscale;
			}
			else if (pdef->name()->value() == "inclination") {
				GetDefNumber(Tilt, pdef, fn);
				NewMoonData.Tilt = Tilt;
			}
			else if (pdef->name()->value() == "atmosphere") {
				Vec3 a;
				GetDefVec(a, pdef, fn);
				AtmosColor = FColor(a.x, a.y, a.z, 1);
				NewMoonData.Atmos = AtmosColor;
			}
		}
	}
	MoonDataArray.Add(NewMoonData);
}

// +-------------------------------------------------------------------+

void AGameDataLoader::LoadStarsystems()
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadStarsystems()"));
	FString ProjectPath = FPaths::ProjectDir();
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

void AGameDataLoader::ParseStarSystem(const char* fn)
{
	SSWInstance->loader->GetLoader();

	FString fs = FString(ANSI_TO_TCHAR(fn));
	FString FileString;
	BYTE* block = 0;

	if (FFileHelper::LoadFileToString(FileString, *fs, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);
	}

	SSWInstance->loader->LoadBuffer(fn, block, true);

	if (!block) {
		UE_LOG(LogTemp, Log, TEXT("ERROR: invalid star system file '%s'"), *FString(fn));
		return;
	}

	Parser parser(new BlockReader((const char*)block));
	Term* term = parser.ParseTerm();

	if (!term) {
		UE_LOG(LogTemp, Log, TEXT("ERROR: could not parse '%s'"), *FString(fn));
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "STARSYSTEM") {
			UE_LOG(LogTemp, Log, TEXT("ERROR: invalid star system file '%s'"), *FString(fn));
			return;
		}
	}

	Text  SystemName = "";
	Text SkyPolyStars = "";
	Text SkyNebula = "";
	Text SkyHaze = "";

	int SkyStars = 0;
	int SkyDust = 0;

	FColor AmbientColor = FColor::Black;
	FS_StarSystem NewStarSystem;
	StarDataArray.Empty();
	MoonDataArray.Empty();

	// parse the system:
	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "name") {
					GetDefText(SystemName, def, fn);
					NewStarSystem.SystemName = FString(SystemName);
				}

				else if (def->name()->value() == "sky") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: sky struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();
						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "poly_stars") {
									GetDefText(SkyPolyStars, pdef, fn);
									NewStarSystem.StarSky.SkyPolyStars = FString(SkyPolyStars);
								}
								else if (pdef->name()->value() == "nebula") {
									GetDefText(SkyNebula, pdef, fn);
									NewStarSystem.StarSky.SkyNebula = FString(SkyNebula);
								}	
								else if (pdef->name()->value() == "haze") {
									GetDefText(SkyHaze, pdef, fn);
									NewStarSystem.StarSky.SkyHaze = FString(SkyHaze);
								}
							}
						}
					}
				}

				else if (def->name()->value() == "stars") {
					GetDefNumber(SkyStars, def, fn);
					NewStarSystem.SkyStars = SkyStars;
				}

				else if (def->name()->value() == "ambient") {
					Vec3 a;
					GetDefVec(a, def, fn);
					AmbientColor = FColor((BYTE)a.x, (BYTE)a.y, (BYTE)a.z, 1);
					NewStarSystem.AmbientColor = AmbientColor;
				}

				else if (def->name()->value() == "dust") {
					GetDefNumber(SkyDust, def, fn);
					NewStarSystem.SkyDust = SkyDust;
				}

				else if (def->name()->value() == "star") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: star struct missing in '%s'\n", fn);
					}
					else {
						ParseStar(def->term()->isStruct(), fn);
						NewStarSystem.Star = StarDataArray;
					}
				}
				
				else if (def->name()->value() == "region") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: region struct missing in '%s'\n", fn);
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseRegion(val);
					}
				}

				else if (def->name()->value() == "terrain") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: terrain struct missing in '%s'\n", fn);
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseTerrain(val);
					}
				}
				FName RowName = FName(FString(SystemName));

				// call AddRow to insert the record
				StarSystemDataTable->AddRow(RowName, NewStarSystem);
			}
		}
	} while (term);
	// define our data table struct
	
	SSWInstance->loader->ReleaseBuffer(block);
}
// +--------------------------------------------------------------------+

EEMPIRE_NAME
AGameDataLoader::GetEmpireName(int32 emp)
{
	EEMPIRE_NAME empire_name;

	switch (emp)
	{
	case 0:
		empire_name = EEMPIRE_NAME::Terellian_Alliance;
		break;
	case 1:
		empire_name = EEMPIRE_NAME::Marakan_Hegemony;
		break;
	case 2:
		empire_name = EEMPIRE_NAME::Dantari_Separatists;
		break;
	case 3:
		empire_name = EEMPIRE_NAME::Other;
		break;
	case 4:
		empire_name = EEMPIRE_NAME::INDEPENDENT_SYSTEMS;
		break;
	default:
		empire_name = EEMPIRE_NAME::Other;
		break;
	}
	return empire_name;
}

CombatGroup*
AGameDataLoader::CloneOver(CombatGroup* force, CombatGroup* clone, CombatGroup* group)
{
	CombatGroup* orig_parent = group->GetParent();

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
	}
}

// +--------------------------------------------------------------------+

void
AGameDataLoader::Unload()
{
	SetStatus(CAMPAIGN_INIT);

	Game::ResetGameTime();
	AStarSystem::SetBaseTime(0);

	startTime = Stardate();
	loadTime = startTime;
	lockout = 0;

	//for (int i = 0; i < NUM_IMAGES; i++)
	//	image[i].ClearImage();

	Clear();

	//zones.destroy();
}

// +--------------------------------------------------------------------+

void
AGameDataLoader::SetStatus(int s)
{
	status = s;

	// record the win in player profile:
	if (status == CAMPAIGN_SUCCESS) {
		PlayerData* player = PlayerData::GetCurrentPlayer();

		if (player)
			player->SetCampaignComplete(campaign_id);
	}

	if (status > CAMPAIGN_ACTIVE) {
		::Print("Campaign::SetStatus() destroying mission list at campaign end\n");
		//missions.destroy();
	}
}

double
AGameDataLoader::Stardate()
{
	return AStarSystem::GetStardate();
}

void
AGameDataLoader::Clear()
{
	//missions.destroy();
	//planners.destroy();
	//combatants.destroy();
	//events.destroy();
	//actions.destroy();

	player_group = 0;
	player_unit = 0;

	updateTime = time;
}

Combatant*
AGameDataLoader::GetCombatant(const char* cname)
{
	ListIter<Combatant> iter = combatants;
	while (++iter) {
		Combatant* c = iter.value();
		if (!strcmp(c->Name(), cname))
			return c;
	}

	return 0;
}
