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
#include "Ship.h"
#include "ShipDesign.h"
#include "PlayerData.h"
#include "SystemDesign.h"
#include "ComponentDesign.h"
#include "../Foundation/GameContent.h"
#include "Engine/TimerHandle.h"
#include "TimerManager.h"

const char* ShipDesignClassName[32] = {
	"Drone",          "Fighter",
	"Attack",         "LCA",
	"Courier",        "Cargo",
	"Corvette",       "Freighter",

	"Frigate",        "Destroyer",
	"Cruiser",        "Battleship",
	"Carrier",        "Dreadnaught",

	"Station",        "Farcaster",

	"Mine",           "DEFSAT",
	"COMSAT",         "SWACS",

	"Building",       "Factory",
	"SAM",            "EWR",
	"C3I",            "Starbase",

	"0x04000000",     "0x08000000",
	"0x10000000",     "0x20000000",
	"0x40000000",     "0x80000000"
};

//List<SystemDesign> SystemDesign::catalog;

// Sets default values
AGameDataLoader::AGameDataLoader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::AGameDataLoader()"));

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

	static ConstructorHelpers::FObjectFinder<UDataTable> ShipDesignDataTableObject(TEXT("DataTable'/Game/Game/DT_ShipDesign.DT_ShipDesign'"));

	if (ShipDesignDataTableObject.Succeeded())
	{
		ShipDesignDataTable = ShipDesignDataTableObject.Object;
		//GalaxyDataTable->EmptyTable();
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> SystemDesignDataTableObject(TEXT("DataTable'/Game/Game/DT_SystemDesign.DT_SystemDesign'"));

	if (SystemDesignDataTableObject.Succeeded())
	{
		SystemDesignDataTable = SystemDesignDataTableObject.Object;
		//GalaxyDataTable->EmptyTable();
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> FormDefDataTableObject(TEXT("DataTable'/Game/Game/DT_FormDef.DT_FormDef'"));

	if (FormDefDataTableObject.Succeeded())
	{
		FormDefDataTable = FormDefDataTableObject.Object;
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

	static ConstructorHelpers::FObjectFinder<UDataTable> CombatGroupDataTableObject(TEXT("DataTable'/Game/Game/DT_CombatGroup.DT_CombatGroup'"));

	if (CombatGroupDataTableObject.Succeeded())
	{
		CombatGroupDataTable = CombatGroupDataTableObject.Object;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Failed to get Combat Group Data Table"));
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> AwardDataTableObject(TEXT("DataTable'/Game/Game/DT_AwardInfo.DT_AwardInfo'"));

	if (AwardDataTableObject.Succeeded())
	{
		AwardsDataTable = AwardDataTableObject.Object;
	}

	PrimaryActorTick.bCanEverTick = true;

	
}



// Called when the game starts or when spawned
void AGameDataLoader::BeginPlay()
{
	Super::BeginPlay();
	GetSSWInstance();
	LoadContentBundle();
	LoadForms();
	LoadGalaxyMap();
	LoadSystemDesigns();
	LoadShipDesigns();
	LoadCombatRoster();
	LoadStarsystems();
	LoadAwardTables();
	
	if(SSWInstance->bClearTables) {
		InitializeCampaignData();
	}
	LoadSystemDesignsFromDT();
	SSWInstance->StartGameTimers();
	

	//USystemDesign::Initialize(SystemDesignTable);
}

// Called every frame
void AGameDataLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// +--------------------------------------------------------------------+

void AGameDataLoader::GetSSWInstance()
{
	SSWInstance = (USSWGameInstance*)GetGameInstance();
}

// +--------------------------------------------------------------------+

void AGameDataLoader::InitializeCampaignData() {
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::InitializeCampaignData()"));

	FString ProjectPath = FPaths::ProjectContentDir();
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
						ActionLocation = Vec3(0.0f, 0.0f, 0.0f);
						
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
	NewCampaignData.bAvailable  = true;
	NewCampaignData.bCompleted = false;

	// define our data table struct
	FName RowName = FName(FString(name));
	SSWInstance->CampaignDataTable->AddRow(RowName, NewCampaignData);
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

// +--------------------------------------------------------------------+

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

					for (int i = 0; i < val->elements()->size(); i++) {
						TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef->name()->value() == "id") {
							GetDefNumber(MissionId, pdef, fn);
							NewMissionList.Id = MissionId;
						}
						else if (pdef->name()->value() == "name") {
							GetDefText(MLName, pdef, fn);
							NewMissionList.Name = FString(MLName);
						}
						else if (pdef->name()->value() == "desc") {
							GetDefText(Desc, pdef, fn);
							NewMissionList.Description = FString(Desc);
						}
						else if (pdef->name()->value() == "start") {
							GetDefText(Start, pdef, fn);
							NewMissionList.Start = FString(Start);
						}
						else if (pdef->name()->value() == "system") {
							GetDefText(System, pdef, fn);
							NewMissionList.System = FString(System);
						}
						else if (pdef->name()->value() == "region") {
							GetDefText(Region, pdef, fn);
							NewMissionList.Region = FString(Region);
						}
						else if (pdef->name()->value() == "objective") {
							GetDefText(Objective, pdef, fn);
							NewMissionList.Objective = FString(Objective);
						}
						else if (pdef->name()->value() == "image") {
							GetDefText(MissionImage, pdef, fn);
							NewMissionList.MissionImage = FString(MissionImage);
						}
						else if (pdef->name()->value() == "audio") {
							GetDefText(MissionAudio, pdef, fn);
							NewMissionList.MissionAudio = FString(MissionAudio);
						}
						else if (pdef->name()->value() == "sitrep") {
							GetDefText(Sitrep, pdef, fn);
							FString SitrepText = FString(Sitrep);
							SitrepText = SitrepText.Replace(TEXT("\n"), TEXT("\\n"));
							NewMissionList.Sitrep = FString(SitrepText);
						}
						else if (pdef->name()->value() == "script") {
							GetDefText(Script, pdef, fn);
							NewMissionList.Script = FString(Script);
						}
						else if (pdef->name()->value() == "type") {
							GetDefText(TypeName, pdef, fn);
							//Type = Mission::TypeFromName(typestr);
							NewMissionList.TypeName = FString(TypeName);
						}
						
						NewMissionList.Available = true;
						NewMissionList.Complete = false;
						NewMissionList.Status = EMISSIONSTATUS::Available;

					}
					MissionListArray.Add(NewMissionList);
				}
			}
		}
	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
}

// +--------------------------------------------------------------------+

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

					for (int i = 0; i < val->elements()->size(); i++) {
					TermDef* pdef = val->elements()->at(i)->isDef();
						if (pdef->name()->value() == "id") {
							GetDefNumber(id, pdef, fn);
							NewTemplateList.Id = id;
						}
						else if (pdef->name()->value() == "name") {
							GetDefText(TLName, pdef, fn);
							NewTemplateList.Name = FString(TLName);
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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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
	Text  ElementIntel = "";

	Vec3  ElementLoc(0.0f, 0.0f, 0.0f);

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
				NewMissionElement.Intel = FString(ElementIntel);
			}

			else if (pdef->name()->value() == "loc") {
				GetDefVec(ElementLoc, pdef, fn);
				NewMissionElement.Location.X = ElementLoc.x;
				NewMissionElement.Location.Y = ElementLoc.y;
				NewMissionElement.Location.Z = ElementLoc.z;
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

// +--------------------------------------------------------------------+

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
	int   CallsignIff = -1;

	FS_MissionCallsign NewMissionCallsign;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name") {
				GetDefText(CallsignName, pdef, fn);
				NewMissionCallsign.Callsign = FString(CallsignName);
			}

			else if (pdef->name()->value() == "iff") {
				GetDefNumber(CallsignIff, pdef, fn);
				NewMissionCallsign.Iff = CallsignIff;
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

	FString ProjectPath = FPaths::ProjectContentDir();

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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

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


void AGameDataLoader::ParseRegion(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseRegion()"));
	
	Text  RegionName = "";
	Text  RegionParent = "";
	Text  LinkName = "";
	Text  ParentType = "";

	double Size = 1.0e6;
	double Orbit = 0.0;
	double Grid = 25000;
	double Inclination = 0.0;
	int    Asteroids = 0;

	EOrbitalType parent_class = EOrbitalType::NOTHING;
	TArray<FString> LinksName;
	List<Text> links;

	FS_Region NewRegionData;

	for (int i = 0; i < val->elements()->size(); i++) {

		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "name") {
				GetDefText(RegionName, pdef, fn);
				NewRegionData.Name = FString(RegionName);
			}

			else if (pdef->name()->value() == "parent") {
				GetDefText(RegionParent, pdef, fn);
				NewRegionData.Parent = FString(RegionParent);
			}

			else if (pdef->name()->value() == "link") {
				GetDefText(LinkName, pdef, fn);
				if (LinkName.length() > 0) {
					links.append(new Text(LinkName));
					LinksName.Add(FString(LinkName));
				}
				NewRegionData.Link = LinksName;
			}

			else if (pdef->name()->value() == "orbit") {
				GetDefNumber(Orbit, pdef, fn);
			}
			else if (pdef->name()->value() == "size" || pdef->name()->value() == "radius") {
				GetDefNumber(Size, pdef, fn);
				NewRegionData.Size = Size;
			}
			else if (pdef->name()->value() == "grid") {
				GetDefNumber(Grid, pdef, fn);
				NewRegionData.Grid = Grid;
			}
			else if (pdef->name()->value() == "inclination") {
				GetDefNumber(Inclination, pdef, fn);
				NewRegionData.Inclination = Inclination;
			}
			else if (pdef->name()->value() == "asteroids") {
				GetDefNumber(Asteroids, pdef, fn);
				NewRegionData.Asteroids = Asteroids;
			}
			else if (pdef->name()->value() == "type") {
				GetDefText(ParentType, pdef, fn);

				switch (ParentType[0]) {
				case 'S':
					parent_class = EOrbitalType::STAR;
					break;
				case 'P':
					parent_class = EOrbitalType::PLANET;
					break;
				case 'M':
					parent_class = EOrbitalType::MOON;
					break;
				default:
					parent_class = EOrbitalType::NOTHING;
					break;
				}
				NewRegionData.Type = parent_class;
			}
		}

	}
	RegionDataArray.Add(NewRegionData);
}

void AGameDataLoader::ParseTerrain(TermStruct* val, const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::ParseTerrain()"));

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
					//	region = new(__FILE__, __LINE__) TerrainRegion(this, rgn_name, size, primary);

					//TermStruct* val = pdef->term()->isStruct();
					//ParseLayer(region, val);
				}
			}
		}
	}
}
// +-------------------------------------------------------------------+

void AGameDataLoader::LoadStarsystems()
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadStarsystems()"));
	FString ProjectPath = FPaths::ProjectContentDir();
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
	RegionDataArray.Empty();

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
						ParseRegion(def->term()->isStruct(), fn);
						NewStarSystem.Region = RegionDataArray;
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

// +-------------------------------------------------------------------+

void AGameDataLoader::LoadCombatRoster()
{
	UE_LOG(LogTemp, Log, TEXT("ACombatGroupLoader::LoadCombatRoster()"));
	FString ProjectPath = FPaths::ProjectContentDir();
	ProjectPath.Append(TEXT("GameData/Campaigns/"));
	FString PathName = ProjectPath;

	TArray<FString> output;
	output.Empty();

	FString Path = PathName + "*.def";
	FFileManagerGeneric::Get().FindFiles(output, *Path, true, false);

	for (int i = 0; i < output.Num(); i++) {

		FString FileName = ProjectPath;
		FileName.Append(output[i]);

		char* fn = TCHAR_TO_ANSI(*FileName);

		LoadOrderOfBattle(fn, -1);
	}
}

void AGameDataLoader::LoadShipDesigns()
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadShipDesigns()"));
	FString ProjectPath = FPaths::ProjectContentDir();
	ProjectPath.Append(TEXT("GameData/Ships/"));
	FString PathName = ProjectPath;

	TArray<FString> output;
	output.Empty();

	FString Path = PathName + "*.def";
	FFileManagerGeneric::Get().FindFiles(output, *Path, true, false);

	for (int i = 0; i < output.Num(); i++) {

		FString FileName = ProjectPath;
		FileName.Append(output[i]);

		char* fn = TCHAR_TO_ANSI(*FileName);

		LoadShipDesign(fn);
	}
}

void AGameDataLoader::LoadSystemDesignsFromDT() {
	
	TArray<FName> RowNames = SystemDesignDataTable->GetRowNames();

	FS_SystemDesign* NewSystemDesign;

	for (int index = 0; index < RowNames.Num(); index++) {
		
		SystemDesign* Design = new SystemDesign();

		NewSystemDesign = SystemDesignDataTable->FindRow<FS_SystemDesign>(FName(RowNames[index]), "");
		Design->name = TCHAR_TO_ANSI(*NewSystemDesign->Name);
		UE_LOG(LogTemp, Log, TEXT("System Design: %s"), *FString(Design->name));
		SystemDesignTable.Add(NewSystemDesign);

		ComponentDesign* comp_design = new ComponentDesign();
		for (int comp_index = 0; comp_index < NewSystemDesign->Component.Num(); comp_index++) {
			comp_design->name = TCHAR_TO_ANSI(*NewSystemDesign->Component[comp_index].Name);
			comp_design->abrv = TCHAR_TO_ANSI(*NewSystemDesign->Component[comp_index].Abrv);
			comp_design->repair_time = NewSystemDesign->Component[comp_index].RepairTime;
			comp_design->replace_time = NewSystemDesign->Component[comp_index].ReplaceTime;
			comp_design->spares = NewSystemDesign->Component[comp_index].Spares;
			comp_design->affects = NewSystemDesign->Component[comp_index].Affects;
			UE_LOG(LogTemp, Log, TEXT("Component Design: %s"), *FString(comp_design->name));
			Design->components.append(comp_design);
		}		
		SystemDesign::catalog.append(Design);
	}
}

void AGameDataLoader::LoadSystemDesigns()
{
	SystemDesignTable.Empty();
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadSystemDesigns()"));
	FString ProjectPath = FPaths::ProjectContentDir();
	ProjectPath.Append(TEXT("GameData/Systems/sys.def"));

	char* fn = TCHAR_TO_ANSI(*ProjectPath);
	LoadSystemDesign(fn);
}

void AGameDataLoader::LoadOrderOfBattle(const char* fn, int team)
{
	UE_LOG(LogTemp, Log, TEXT("Loading Order of Battle Data: %s"), *FString(fn));

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
		if (!file_type || file_type->value() != "ORDER_OF_BATTLE")
		{
			UE_LOG(LogTemp, Log, TEXT("Invalid Order of Battle File: %s"), *FString(fn));
			return;
		}
	}

	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "group") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: group struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();

						FS_CombatGroup NewCombatGroup;
						NewCombatUnitArray.Empty();

						Text Name = "";
						Text Type = "";
						Text Intel = "KNOWN";
						Text Region = "";
						Text System = "";
						Text ParentType = "";
						int UnitIndex = 0;
						int ParentId = 0;
						int Id = 0;
						int Iff = -1;
						Vec3 Loc = Vec3(1.0e9f, 0.0f, 0.0f);

						for (int i = 0; i < val->elements()->size(); i++)
						{
							NewCombatGroup.UnitIndex = 0;
							TermDef* pdef = val->elements()->at(i)->isDef();

							if (pdef->name()->value() == ("name"))
							{
								GetDefText(Name, pdef, fn);
								NewCombatGroup.Name = FString(Name);
							}
							else if (pdef->name()->value() == ("type"))
							{
								GetDefText(Type, pdef, fn);
								NewCombatGroup.Type = FString(Type);
							}
							else if (pdef->name()->value() == ("intel"))
							{
								GetDefText(Intel, pdef, fn);
								NewCombatGroup.Intel = FString(Intel);
							}
							else if (pdef->name()->value() == ("region"))
							{
								GetDefText(Region, pdef, fn);
								NewCombatGroup.Region = FString(Region);
							}
							else if (pdef->name()->value() == ("system"))
							{
								GetDefText(System, pdef, fn);
								NewCombatGroup.System = FString(System);
							}
							else if (pdef->name()->value() == ("loc"))
							{
								GetDefVec(Loc, pdef, fn);

								NewCombatGroup.Location.X = Loc.x;
								NewCombatGroup.Location.Y = Loc.y;
								NewCombatGroup.Location.Z = Loc.z;
							}
							else if (pdef->name()->value() == ("parent_type"))
							{
								GetDefText(ParentType, pdef, fn);
								NewCombatGroup.ParentType = FString(ParentType);
							}
							else if (pdef->name()->value() == ("parent_id"))
							{
								GetDefNumber(ParentId, pdef, fn);
								NewCombatGroup.ParentId = ParentId;
							}
							else if (pdef->name()->value() == ("iff"))
							{
								GetDefNumber(Iff, pdef, fn);
								NewCombatGroup.Iff = Iff;
							}
							else if (pdef->name()->value() == ("id"))
							{
								GetDefNumber(Id, pdef, fn);
								NewCombatGroup.Id = Id;
							}
							else if (pdef->name()->value() == ("unit_index"))
							{
								GetDefNumber(UnitIndex, pdef, fn);
								NewCombatGroup.UnitIndex = UnitIndex;
							}

							else if (pdef->name()->value() == ("unit"))
							{
								TermStruct* UnitTerm = pdef->term()->isStruct();

								NewCombatGroup.UnitIndex = UnitTerm->elements()->size();

								if (NewCombatGroup.UnitIndex > 0)
								{
									// Add Unit Stuff Here

									FS_CombatGroupUnit NewCombatGroupUnit;

									UnitName = "";
									UnitRegnum = "";
									UnitRegion = "";
									UnitClass = "";
									UnitDesign = "";
									UnitSkin = "";

									UnitCount = 1;
									UnitDamage = 0;
									UnitDead = 0;
									UnitHeading = 0;
									UnitLoc = Vec3(1.0e9f, 0.0f, 0.0f);

									for (int UnitIdx = 0; UnitIdx < NewCombatGroup.UnitIndex; UnitIdx++)
									{
										pdef = UnitTerm->elements()->at(UnitIdx)->isDef();


										if (pdef->name()->value() == "name") {
											GetDefText(UnitName, pdef, fn);
											UE_LOG(LogTemp, Log, TEXT("unit name '%s'"), *FString(UnitName));
											NewCombatGroupUnit.UnitName = FString(UnitName);
										}
										else if (pdef->name()->value() == "regnum") {
											GetDefText(UnitRegnum, pdef, fn);
										}
										else if (pdef->name()->value() == "region") {
											GetDefText(UnitRegion, pdef, fn);
										}
										else if (pdef->name()->value() == "loc") {
											GetDefVec(UnitLoc, pdef, fn);
										}
										else if (pdef->name()->value() == "type") {
											//char typestr[32];
											GetDefText(UnitClass, pdef, fn);
											//UnitClass = ShipDesign::ClassForName(typestr);
										}
										else if (pdef->name()->value() == "design") {
											GetDefText(UnitDesign, pdef, fn);
										}
										else if (pdef->name()->value() == "skin") {
											GetDefText(UnitSkin, pdef, fn);
										}
										else if (pdef->name()->value() == "count") {
											GetDefNumber(UnitCount, pdef, fn);
										}
										else if (pdef->name()->value() == "dead_count") {
											GetDefNumber(UnitDead, pdef, fn);
										}
										else if (pdef->name()->value() == "damage") {
											GetDefNumber(UnitDamage, pdef, fn);
										}
										else if (pdef->name()->value() == "heading") {
											GetDefNumber(UnitHeading, pdef, fn);
										}


										NewCombatGroupUnit.UnitRegnum = FString(UnitRegnum);
										NewCombatGroupUnit.UnitRegion = FString(UnitRegion);
										NewCombatGroupUnit.UnitLoc.X = UnitLoc.x;
										NewCombatGroupUnit.UnitLoc.Y = UnitLoc.y;
										NewCombatGroupUnit.UnitLoc.Z = UnitLoc.z;
										NewCombatGroupUnit.UnitClass = FString(UnitClass);
										NewCombatGroupUnit.UnitDesign = FString(UnitDesign);
										NewCombatGroupUnit.UnitSkin = FString(UnitSkin);
										NewCombatGroupUnit.UnitCount = UnitCount;
										NewCombatGroupUnit.UnitDead = UnitDead;
										NewCombatGroupUnit.UnitDamage = UnitDamage;
										NewCombatGroupUnit.UnitHeading = UnitHeading;
									}

									NewCombatUnitArray.Add(NewCombatGroupUnit);
								}
								NewCombatGroup.Unit = NewCombatUnitArray;
							}

							FName RowName = FName(GetOrdinal(Id) + " " + FString(Name) + " " + +" " + FString(GetNameFromType(FString(Type))));
							// call AddRow to insert the record

							if (Iff > 0) {
								CombatGroupDataTable->AddRow(RowName, NewCombatGroup);
							}
							CombatGroupData = NewCombatGroup;

						}  /// iff == team?
					}    // group-struct
				}          // group
			}           // def
		}             // term
	} while (term);
	SSWInstance->loader->ReleaseBuffer(block);
}

void
AGameDataLoader::LoadShipDesign(const char* fn)
{
	UE_LOG(LogTemp, Log, TEXT("Loading Ship Design Data: %s"), *FString(filename));

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
		if (!file_type || file_type->value() != "SHIP")
		{
			UE_LOG(LogTemp, Log, TEXT("Invalid Ship File: %s"), *FString(fn));
			return;
		}
	}

	Text ShipName = "";
	Text DisplayName = "";
	Text Description = "";
	Text Abrv = "";

	Text	DetailName0 = "";
	Text	DetailName1 = "";
	Text	DetailName2 = "";
	Text	DetailName3 = "";

	Text	ShipClass = "";
	Text	CockpitName = "";
	Text	BeautyName = "";
	Text    HudIconName = "";

	int pcs = 3.0f;
	int acs = 1.0f;
	int detet = 250.0e3f;
	float scale = 1.0f;
	float explosion_scale = 0.0f;
	float mass = 0;

	int ShipType = 0;

	float vlimit = 8e3f;
	float agility = 2e2f;
	float air_factor = 0.1f;
	float roll_rate = 0.0f;
	float pitch_rate = 0.0f;
	float yaw_rate = 0.0f;
	float trans_x = 0.0f;
	float trans_y = 0.0f;
	float trans_z = 0.0f;
	float turn_bank = (float)(PI / 8);

	float cockpit_scale = 1.0f;
	float auto_roll = 0;

	float CL = 0.0f;
	float CD = 0.0f;
	float stall = 0.0f;
	float drag = 2.5e-5f;

	float arcade_drag = 1.0f;
	float roll_drag = 5.0f;
	float pitch_drag = 5.0f;
	float yaw_drag = 5.0f;

	float prep_time = 30.0f;
	float avoid_time = 0.0f;
	float avoid_fighter = 0.0f;
	float avoid_strike = 0.0f;
	float avoid_target = 0.0f;
	float commit_range = 0.0f;

	float splash_radius = -1.0f;
	float scuttle = 5e3f;
	float repair_speed = 1.0f;

	int  repair_teams = 2;

	float feature_size[4];
	float e_factor[3];

	bool secret = false;
	bool repair_auto = true;
	bool repair_screen = true;
	bool wep_screen = true;
	bool degrees = false;

	e_factor[0] = 0.1f;
	e_factor[1] = 0.3f;
	e_factor[2] = 1.0f;

	Vec3    off_loc = Vec3(0.0, 0.0, 0.0);
	Vec3    spin = Vec3(0.0, 0.0, 0.0);
	Vec3    BeautyCam = Vec3(0.0, 0.0, 0.0);
	Vec3	chase_vec = Vec3(0, -100, 20);
	Vec3	bridge_vec = Vec3(0.0, 0.0, 0.0);

	FS_ShipDesign NewShipDesign;

	// parse the system:
	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "name") {
					GetDefText(ShipName, def, fn);	
					NewShipDesign.ShipName = FString(ShipName);	
				}
				else if (def->name()->value() == "display_name") {
					GetDefText(DisplayName, def, fn);
					NewShipDesign.DisplayName = FString(DisplayName);
				}
				else if (def->name()->value() == "class") {
					GetDefText(ShipClass, def, fn);
					NewShipDesign.ShipClass = FString(ShipClass);

					ShipType = ClassForName(ShipClass);

					if (ShipType <= (int)UShip::LCA) {
						repair_auto = false;
						repair_screen = false;
						wep_screen = false;
					}

					NewShipDesign.ShipType = ShipType;
					NewShipDesign.RepairAuto = repair_auto;
					NewShipDesign.RepairScreen = repair_screen;
					NewShipDesign.WepScreen = wep_screen;
				}

				else if (def->name()->value() == "description") {
					GetDefText(Description, def, fn);
					NewShipDesign.Description = FString(Description);
				}
				else if (def->name()->value() == "abrv") {
					GetDefText(Abrv, def, fn);
					NewShipDesign.Abrv = FString(Abrv);
				}

				else if (def->name()->value() == "pcs") {
					GetDefNumber(pcs, def, fn);
					NewShipDesign.PCS = pcs;
				}
				else if (def->name()->value() == "acs") {
					GetDefNumber(acs, def, fn);
					NewShipDesign.ACS = acs;

				}
				else if (def->name()->value() == "detec") {
					GetDefNumber(detet, def, fn);
					NewShipDesign.Detet = detet;
				}
				else if (def->name()->value() == "scale") {
					GetDefNumber(scale, def, fn);
					NewShipDesign.Scale = scale;
				}
				else if (def->name()->value() == "explosion_scale") {
					GetDefNumber(explosion_scale, def, fn);
					NewShipDesign.ExplosionScale = explosion_scale;
				}
				else if (def->name()->value() == "mass") {
					GetDefNumber(mass, def, fn);
					NewShipDesign.Mass = mass;
				}

				else if (def->name()->value() == "vlimit") {
					GetDefNumber(vlimit, def, fn);
					NewShipDesign.Vlimit = vlimit;
				}
				else if (def->name()->value() == "agility") {
					GetDefNumber(agility, def, fn);
					NewShipDesign.Agility = agility;
				}
				else if (def->name()->value() == "air_factor") {
					GetDefNumber(air_factor, def, fn);
					NewShipDesign.AirFactor = air_factor;
				}
				else if (def->name()->value() == "roll_rate") {
					GetDefNumber(roll_rate, def, fn);
					NewShipDesign.RollRate = roll_rate;
				}
				else if (def->name()->value() == "pitch_rate") {
					GetDefNumber(pitch_rate, def, fn);
					NewShipDesign.PitchRate = pitch_rate;

				}
				else if (def->name()->value() == "yaw_rate") {
					GetDefNumber(yaw_rate, def, fn);
					NewShipDesign.YawRate = yaw_rate;
				}
				else if (def->name()->value() == "trans_x") {
					GetDefNumber(trans_x, def, fn);
					NewShipDesign.Trans.X = trans_x;
				}
				else if (def->name()->value() == "trans_y") {
					GetDefNumber(trans_y, def, fn);
					NewShipDesign.Trans.Y = trans_y;
				}
				else if (def->name()->value() == "trans_z") {
					GetDefNumber(trans_z, def, fn);
					NewShipDesign.Trans.Z = trans_z;
				}
				else if (def->name()->value() == "turn_bank") {
					GetDefNumber(turn_bank, def, fn);
					NewShipDesign.TurnBank = turn_bank;
				}
				else if (def->name()->value() == "cockpit_scale") {
					GetDefNumber(cockpit_scale, def, fn);
					NewShipDesign.CockpitScale = cockpit_scale;
				}
				else if (def->name()->value() == "auto_roll") {
					GetDefNumber(auto_roll, def, fn);
					NewShipDesign.AutoRoll = auto_roll;
				}
				else if (def->name()->value() == "CL") {
					GetDefNumber(CL, def, fn);
					NewShipDesign.CL = CL;
				}
				else if (def->name()->value() == "CD") {
					GetDefNumber(CD, def, fn);
					NewShipDesign.CD = CD;
				}
				else if (def->name()->value() == "stall") {
					GetDefNumber(stall, def, fn);
					NewShipDesign.Stall = stall;
				}
				else if (def->name()->value() == "prep_time") {
					GetDefNumber(prep_time, def, fn);
					NewShipDesign.PrepTime = prep_time;
				}
				else if (def->name()->value() == "avoid_time") {
					GetDefNumber(avoid_time, def, fn);
					NewShipDesign.AvoidTime = avoid_time;
				}
				else if (def->name()->value() == "avoid_fighter") {
					GetDefNumber(avoid_fighter, def, fn);
					NewShipDesign.AvoidFighter = avoid_fighter;
				}
				else if (def->name()->value() == "avoid_strike") {
					GetDefNumber(avoid_strike, def, fn);
					NewShipDesign.AvoidStrike = avoid_strike;
				}
				else if (def->name()->value() == "avoid_target") {
					GetDefNumber(avoid_target, def, fn);
					NewShipDesign.AvoidTarget = avoid_target;
				}
				else if (def->name()->value() == "commit_range") {
					GetDefNumber(commit_range, def, fn);
					NewShipDesign.CommitRange = commit_range;
				}
				else if (def->name()->value() == "splash_radius") {
					GetDefNumber(splash_radius, def, fn);
					NewShipDesign.SplashRadius = splash_radius;
				}
				else if (def->name()->value() == "scuttle") {
					GetDefNumber(scuttle, def, fn);
					NewShipDesign.Scuttle = scuttle;
				}
				else if (def->name()->value() == "repair_speed") {
					GetDefNumber(repair_speed, def, fn);
					NewShipDesign.RepairSpeed = repair_speed;
				}
				else if (def->name()->value() == "repair_teams") {
					GetDefNumber(repair_teams, def, fn);
					NewShipDesign.RepairTeams = repair_teams;
				}
				else if (def->name()->value() == "cockpit_model") {
					GetDefText(CockpitName, def, fn);
					NewShipDesign.CockpitName = FString(CockpitName);
				}

				else if (def->name()->value() == "model" || def->name()->value() == "detail_0") {
					GetDefText(DetailName0, def, fn);
					NewShipDesign.DetailName0 = FString(DetailName0);
					//detail[0].append(new Text(detail_name));
				}

				else if (def->name()->value() == "detail_1") {
					GetDefText(DetailName1, def, fn);
					NewShipDesign.DetailName1 = FString(DetailName1);
					//detail[1].append(new Text(detail_name));
				}

				else if (def->name()->value() == "detail_2") {
					GetDefText(DetailName2, def, fn);
					NewShipDesign.DetailName2 = FString(DetailName2);
					//detail[2].append(new Text(detail_name));
				}

				else if (def->name()->value() == "detail_3") {
					GetDefText(DetailName3, def, fn);
					NewShipDesign.DetailName3 = FString(DetailName3);
					//detail[3].append(new Text(detail_name));
				}

				else if (def->name()->value() == "spin") {
					GetDefVec(spin, def, fn);
					NewShipDesign.Spin= FVector(spin.x, spin.y, spin.z);
					//spin_rates.append(new Point(spin));
				}

				else if (def->name()->value() == "offset_0") {
					GetDefVec(off_loc, def, fn);	
					NewShipDesign.Offset[0] = FVector(off_loc.x, off_loc.y, off_loc.z);
					//offset[0].append(new Point(off_loc));
				}

				else if (def->name()->value() == "offset_1") {
					GetDefVec(off_loc, def, fn);
					NewShipDesign.Offset[1] = FVector(off_loc.x, off_loc.y, off_loc.z);
					//offset[1].append(new Point(off_loc));
				}

				else if (def->name()->value() == "offset_2") {
					GetDefVec(off_loc, def, fn);
					NewShipDesign.Offset[2] = FVector(off_loc.x, off_loc.y, off_loc.z);
					//offset[2].append(new Point(off_loc));
				}

				else if (def->name()->value() == "offset_3") {
					GetDefVec(off_loc, def, fn);
					NewShipDesign.Offset[3] = FVector(off_loc.x, off_loc.y, off_loc.z);
					//offset[3].append(new Point(off_loc));
				}

				else if (def->name()->value() == "beauty") {
					if (def->term() && def->term()->isArray()) {
						GetDefVec(BeautyCam, def, fn);

						if (degrees) {
							BeautyCam.x *= (float)DEGREES;
							BeautyCam.y *= (float)DEGREES;
						}

						NewShipDesign.BeautyCam = FVector(BeautyCam.x, BeautyCam.y, BeautyCam.z);
					}

					else {
						if (!GetDefText(BeautyName, def, fn))
							Print("WARNING: invalid or missing beauty in '%s'\n", filename);

						//DataLoader* loader = DataLoader::GetLoader();
						//loader->LoadBitmap(beauty_name, beauty);
					}
				}

				else if (def->name()->value() == "hud_icon") {
					GetDefText(HudIconName, def, fn);
					NewShipDesign.HudIconName = FString(HudIconName);
					//DataLoader* loader = DataLoader::GetLoader();
					//loader->LoadBitmap(hud_icon_name, hud_icon);
				}

				else if (def->name()->value() == "feature_0") {
					GetDefNumber(feature_size[0], def, fn);
					NewShipDesign.FeatureSize[0] = feature_size[0];
				}

				else if (def->name()->value() == "feature_1") {
					GetDefNumber(feature_size[1], def, fn);
					NewShipDesign.FeatureSize[1] = feature_size[1];
	
				}

				else if (def->name()->value() == "feature_2") {
					GetDefNumber(feature_size[2], def, fn);
					NewShipDesign.FeatureSize[2] = feature_size[2];

				}

				else if (def->name()->value() == "feature_3") {
					GetDefNumber(feature_size[3], def, fn);
					NewShipDesign.FeatureSize[3] = feature_size[3];

				}
				else if (def->name()->value() == "class") {
					GetDefText(ShipClass, def, fn);
					NewShipDesign.ShipClass = FString(ShipClass);

					ShipType = ClassForName(ShipClass);

					if (ShipType <= (int) UShip::LCA) {
						repair_auto = false;
						repair_screen = false;
						wep_screen = false;
					}

					NewShipDesign.ShipType = ShipType;
					NewShipDesign.RepairAuto = repair_auto;
					NewShipDesign.RepairScreen = repair_screen;
					NewShipDesign.WepScreen = wep_screen;
				}

				else if (def->name()->value() == "emcon_1") {
					GetDefNumber(e_factor[0], def, fn);
					NewShipDesign.EFactor[0] = e_factor[0];
				}

				else if (def->name()->value() == "emcon_2") {
					GetDefNumber(e_factor[1], def, fn);
					NewShipDesign.EFactor[1] = e_factor[1];
				}

				else if (def->name()->value() == "emcon_3") {
					GetDefNumber(e_factor[2], def, fn);
					NewShipDesign.EFactor[2] = e_factor[2];
				}

				else if (def->name()->value() == "chase") {
					GetDefVec(chase_vec, def, fn);
					chase_vec *= (float)scale;
					NewShipDesign.ChaseVec = FVector(chase_vec.x, chase_vec.y, chase_vec.z);
				}

				else if (def->name()->value() == "bridge") {
					GetDefVec(bridge_vec, def, fn);

					bridge_vec *= (float)scale;
					NewShipDesign.BridgeVec = FVector(bridge_vec.x, bridge_vec.y, bridge_vec.z);
				}

				else if (def->name()->value() == "power") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: power source struct missing in '%s'"), *FString(fn));

					}
					else {
						ParsePower(def->term()->isStruct(), fn);
					}
				}

				else if (def->name()->value() == "main_drive" || def->name()->value() == "drive") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: main drive struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseDrive(val);
					}
				}

				else if (def->name()->value() == "quantum" || def->name()->value() == "quantum_drive") {
					if (!def->term() || !def->term()->isStruct()) {	
						UE_LOG(LogTemp, Log, TEXT("WARNING: quantum_drive struct missing in '%s'"), *FString(fn)); 
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseQuantumDrive(val);
					}
				}

				else if (def->name()->value() == "sender" || def->name()->value() == "farcaster") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: farcaster struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseFarcaster(val);
					}
				}

				else if (def->name()->value() == "thruster") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: thruster struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseThruster(val);
					}
				}

				else if (def->name()->value() == "navlight") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: navlight struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseNavlight(val);
					}
				}

				else if (def->name()->value() == "flightdeck") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: flightdeck struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseFlightDeck(val);
					}
				}

				else if (def->name()->value() == "gear") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: landing gear struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseLandingGear(val);
					}
				}

				else if (def->name()->value() == "weapon") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: weapon struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseWeapon(val);
					}
				}

				else if (def->name()->value() == "hardpoint") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: hardpoint struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseHardPoint(val);
					}
				}

				else if (def->name()->value() == "loadout") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: loadout struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseLoadout(val);
					}
				}

				else if (def->name()->value() == "decoy") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: decoy struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseWeapon(val);
					}
				}

				else if (def->name()->value() == "probe") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: probe struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseWeapon(val);
					}
				}

				else if (def->name()->value() == "sensor") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: sensor struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseSensor(val);
					}
				}

				else if (def->name()->value() == "nav") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: nav struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseNavsys(val);
					}
				}

				else if (def->name()->value() == "computer") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: computer struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseComputer(val);
					}
				}

				else if (def->name()->value() == "shield") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: shield struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseShield(val);
					}
				}

				else if (def->name()->value() == "death_spiral") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: death spiral struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseDeathSpiral(val);
					}
				}

				else if (def->name()->value() == "map") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: map struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseMap(val);
					}
				}

				else if (def->name()->value() == "squadron") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: squadron struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseSquadron(val);
					}
				}

				else if (def->name()->value() == "skin") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: skin struct missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						//ParseSkin(val);
					}
				}

				else {
					UE_LOG(LogTemp, Log, TEXT("WARNING: unknown parameter '%s'"), *FString(fn));
				}
				FName RowName = FName(FString(ShipName));

				// call AddRow to insert the record
				ShipDesignDataTable->AddRow(RowName, NewShipDesign);
			}
		}

	} while (term);
	// define our data table struct

	SSWInstance->loader->ReleaseBuffer(block);
}

// +--------------------------------------------------------------------+

void
AGameDataLoader::ParsePower(TermStruct* val, const char* fn)
{
	int   stype = 0;
	float output = 1000.0f;
	float fuel = 0.0f;
	Vec3  loc(0.0f, 0.0f, 0.0f);
	float size = 0.0f;
	float hull = 0.5f;
	Text  design_name;
	Text  pname;
	Text  pabrv;
	int   etype = 0;
	int   emcon_1 = -1;
	int   emcon_2 = -1;
	int   emcon_3 = -1;

	FS_ShipPower NewShipPower;

	/*for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "type") {
				TermText* tname = pdef->term()->isText();
				if (tname) {
					if (tname->value()[0] == 'B') stype = PowerSource::BATTERY;
					else if (tname->value()[0] == 'A') stype = PowerSource::AUX;
					else if (tname->value()[0] == 'F') stype = PowerSource::FUSION;
					else Print("WARNING: unknown power source type '%s' in '%s'\n", tname->value().data(), filename);
				}
				NewShipPower.SType = 
			}

			else if (defname == "name") {
				GetDefText(pname, pdef, filename);
			}

			else if (defname == "abrv") {
				GetDefText(pabrv, pdef, filename);
			}

			else if (defname == "design") {
				GetDefText(design_name, pdef, filename);
			}

			else if (defname == "max_output") {
				GetDefNumber(output, pdef, filename);
			}
			else if (defname == "fuel_range") {
				GetDefNumber(fuel, pdef, filename);
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
	}*/
}

// +--------------------------------------------------------------------+

/*void
AGameDataLoader::ParseDrive(TermStruct* val, const char* fn)
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
					drive = new(__FILE__, __LINE__) Drive((Drive::SUBTYPE)dtype, dthrust, daug, trail);

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

	QuantumDrive* drive = new(__FILE__, __LINE__) QuantumDrive((QuantumDrive::SUBTYPE)subtype, capacity, consumption);
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

	Farcaster* caster = new(__FILE__, __LINE__) Farcaster(capacity, consumption);
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
					drive = new(__FILE__, __LINE__) Thruster(dtype, thrust, tscale);

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
		drive = new(__FILE__, __LINE__) Thruster(dtype, thrust, tscale);
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

	NavLight* nav = new(__FILE__, __LINE__) NavLight(period, dscale);
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

	FlightDeck* deck = new(__FILE__, __LINE__) FlightDeck();
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
	Model* model[LandingGear::MAX_GEAR];

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
						Model* m = new(__FILE__, __LINE__) Model;
						if (!m->Load(mod_name, scale)) {
							Print("WARNING: Could not load landing gear model '%s'\n", mod_name);
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

	gear = new(__FILE__, __LINE__) LandingGear();
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

		Weapon* gun = new(__FILE__, __LINE__) Weapon(meta, nmuz, muzzles, az, el);
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

	HardPoint* hp = new(__FILE__, __LINE__) HardPoint(muzzle, az, el);
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
	ShipLoad* load = new(__FILE__, __LINE__) ShipLoad;

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
		sensor = new(__FILE__, __LINE__) Sensor();

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
		navsys = new(__FILE__, __LINE__) NavSystem;

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

	Computer* comp = new(__FILE__, __LINE__) Computer(comp_type, comp_name);
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
			shield = new(__FILE__, __LINE__) Shield((Shield::SUBTYPE)shield_type);
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
				shield_model = new(__FILE__, __LINE__) Model;
				if (!shield_model->Load(model_name, scale)) {
					Print("ERROR: Could not load shield model '%s'\n", model_name.data());
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
			DWORD       SOUND_FLAGS = Sound::LOCALIZED | Sound::LOC_3D;

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
					Model* model = new(__FILE__, __LINE__) Model;
					if (!model->Load(model_name, scale)) {
						Print("Could not load debris model '%s'\n", model_name.data());
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
				Model* model = new(__FILE__, __LINE__) Model;
				if (!model->Load(model_name, scale)) {
					Print("Could not load debris model '%s'\n", model_name);
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

				Bitmap* sprite = new(__FILE__, __LINE__) Bitmap();
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

	ShipSquadron* s = new(__FILE__, __LINE__) ShipSquadron;
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

				skin = new(__FILE__, __LINE__) Skin(name);
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
	Material* mtl = new(__FILE__, __LINE__) Material;
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
AGameDataLoader::LoadSystemDesign(const char* fn)
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

int
AGameDataLoader::ClassForName(const char* cls)
{
	if (!cls || !cls[0])
		return 0;

	for (int i = 0; i < 32; i++) {
		if (!_stricmp(name, ShipDesignClassName[i])) {
			return 1 << i;
		}
	}

	return 0;
}

const char*
AGameDataLoader::ClassName(int type)
{
	if (type != 0) {
		int index = 0;

		while (!(type & 1)) {
			type >>= 1;
			index++;
		}

		if (index >= 0 && index < 32) {
			return ShipDesignClassName[index];
		}
	}

	return "Unknown";
}

FString
AGameDataLoader::GetOrdinal(int id)
{
	FString ordinal;

	int last_two_digits = id % 100;

	if (last_two_digits > 10 && last_two_digits < 20) {
		ordinal = FString::FormatAsNumber(id) + "th";
	}
	else {
		int last_digit = last_two_digits % 10;

		if (last_digit == 1)
			ordinal = FString::FormatAsNumber(id) + "st";
		else if (last_digit == 2)
			ordinal = FString::FormatAsNumber(id) + "nd";
		else if (last_digit == 3)
			ordinal = FString::FormatAsNumber(id) + "rd";
		else
			ordinal = FString::FormatAsNumber(id) + "th";
	}

	return ordinal;
}

FString AGameDataLoader::GetNameFromType(FString nt)
{
	FString TypeName;

	if (nt == "force") {
		TypeName = "Force";
	}
	else if (nt == "wing") {
		TypeName = "Wing";
	}
	else if (nt == "intercept_squadron") {
		TypeName = "Intercept Squadron";
	}
	else if (nt == "fighter_squadron") {
		TypeName = "Fighter Squadron";
	}
	else if (nt == "attack_squadron") {
		TypeName = "Attack Squadron";
	}
	else if (nt == "lca_squadron") {
		TypeName = "LCA Squadron";
	}
	else if (nt == "fleet") {
		TypeName = "Fleet";
	}
	else if (nt == "destroyer_squadron") {
		TypeName = "DESRON";
	}
	else if (nt == "battle_group") {
		TypeName = "Battle Group";
	}
	else if (nt == "carrier_group") {
		TypeName = "CVBG";
	}
	else
	{
		TypeName = nt;
	}
	return TypeName;
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

// +--------------------------------------------------------------------+

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

// +--------------------------------------------------------------------+

Text
AGameDataLoader::GetContentBundleText(const char* key) const
{
	return ContentValues.Find(key, Text(key));
}

// +--------------------------------------------------------------------+

void
AGameDataLoader::LoadContentBundle() {
	FString ProjectPath = FPaths::ProjectContentDir();
	ProjectPath.Append(TEXT("GameData/Content/content.txt"));

	char* fn = TCHAR_TO_ANSI(*ProjectPath);
	
	SSWInstance->loader->GetLoader(); 
	if (SSWInstance->loader->GetLoader()) {
		BYTE* buffer = 0;
		BYTE* block = 0;
		SSWInstance->loader->LoadBuffer(fn, buffer, true, true);
		if (buffer && *buffer) {

			Text key;
			Text val;

			char* p = (char*)buffer;
			int   s = 0;

			key = "";
			val = "";

			while (*p) {
				if (*p == '=') {
					s = 1;
				}
				else if (*p == '\n' || *p == '\r') {
					if (key != ""  && val != "") {
						ContentValues.Insert(Text(key).trim(), Text(val).trim());
						UE_LOG(LogTemp, Log, TEXT("Inserted- %s: %s"), *FString(Text(key).trim()), *FString(Text(val).trim()));
					}
					s = 0;
					
					key = "";
					val = "";
				}
				else if (s == 0) {
					if (!key[0]) {
						if (*p == '#') {
							s = -1; // comment
						}
						else if (!isspace(*p)) {
							key.append(*p);
						}
					}
					else {
						key.append(*p);
					}
				}
				else if (s == 1) {
					if (!isspace(*p)) {
						s = 2;
						val.append(*p);
					}
				}
				else if (s == 2) {
					val.append(*p);
				}

				p++;
				
			}
			SSWInstance->loader->ReleaseBuffer(buffer);
		}
	}
}

// +--------------------------------------------------------------------+

void AGameDataLoader::LoadForms()
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadForms()"));
	FString ProjectPath = FPaths::ProjectContentDir();
	ProjectPath.Append(TEXT("GameData/Screens/"));
	FString PathName = ProjectPath;

	TArray<FString> output;
	output.Empty();

	FString Path = PathName + "*.frm";
	FFileManagerGeneric::Get().FindFiles(output, *Path, true, false);

	for (int i = 0; i < output.Num(); i++) {

		FString FileName = ProjectPath;
		FileName.Append(output[i]);

		char* fn = TCHAR_TO_ANSI(*FileName);

		LoadForm(fn);
	}
}

void AGameDataLoader::ParseCtrlDef(TermStruct* val, const char* fn)
{
	Text buf;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "text" ||
				pdef->name()->value() == "caption") {
				GetDefText(buf, pdef, fn);
			}

			else if (pdef->name()->value() == "id") {
				DWORD id;
				GetDefNumber(id, pdef, fn);
			}

			else if (pdef->name()->value() == "pid") {
				DWORD id;
				GetDefNumber(id, pdef, fn);
			}

			else if (pdef->name()->value() == "alt") {
				GetDefText(buf, pdef, fn);
			}

			else if (pdef->name()->value() == "type") {
				DWORD type = (int8)EControlType::WINDEF_LABEL;

				GetDefText(buf, pdef, fn);
				Text type_name(buf);

				if (type_name == "button")
					type = (int8)EControlType::WINDEF_BUTTON;

				else if (type_name == "combo")
					type = (int8)EControlType::WINDEF_COMBO;

				else if (type_name == "edit")
					type = (int8)EControlType::WINDEF_EDIT;

				else if (type_name == "image")
					type = (int8)EControlType::WINDEF_IMAGE;

				else if (type_name == "slider")
					type = (int8)EControlType::WINDEF_SLIDER; 

				else if (type_name == "list")
					type = (int8)EControlType::WINDEF_LIST;

				else if (type_name == "rich" || type_name == "text" || type_name == "rich_text")
					type = (int8)EControlType::WINDEF_RICH;
			}

			else if (pdef->name()->value() == "rect") {
				Rect r;
				GetDefRect(r, pdef, fn);
			}

			else if (pdef->name()->value() == "font") {
				GetDefText(buf, pdef, fn);
			}

			else if (pdef->name()->value() == "active_color") {
				Color c;
				GetDefColor(c, pdef, fn);
			}

			else if (pdef->name()->value() == "back_color") {
				Color c;
				GetDefColor(c, pdef, fn);
			}

			else if (pdef->name()->value() == "base_color") {
				Color c;
				GetDefColor(c, pdef, fn);
			}

			else if (pdef->name()->value() == "border_color") {
				Color c;
				GetDefColor(c, pdef, fn);
			}

			else if (pdef->name()->value() == "fore_color") {
				Color c;
				GetDefColor(c, pdef, fn);
			}

			else if (pdef->name()->value() == "texture") {
				GetDefText(buf, pdef, fn);

				if (buf.length() > 0 && !buf.contains('.'))
					buf.append(".pcx");
			}

			else if (pdef->name()->value() == "margins") {
				Insets margins;
				GetDefInsets(margins, pdef, fn);
			}

			else if (pdef->name()->value() == "text_insets") {
				Insets text_insets;
				GetDefInsets(text_insets, pdef, fn);
			}

			else if (pdef->name()->value() == "cell_insets") {
				Insets cell_insets;
				GetDefInsets(cell_insets, pdef, fn);
			}

			else if (pdef->name()->value() == "cells") {
				Rect cells;
				GetDefRect(cells, pdef, fn);
			}

			else if (pdef->name()->value() == "fixed_width") {
				int fixed_width;
				GetDefNumber(fixed_width, pdef, fn);
			}

			else if (pdef->name()->value() == "fixed_height") {
				int fixed_height;
				GetDefNumber(fixed_height, pdef, fn);
			}

			else if (pdef->name()->value() == "standard_image") {
				GetDefText(buf, pdef, fn);

				if (buf.length() > 0 && !buf.contains('.'))
					buf.append(".pcx");
			}

			else if (pdef->name()->value() == "activated_image") {
				GetDefText(buf, pdef, fn);

				if (buf.length() > 0 && !buf.contains('.'))
					buf.append(".pcx");
			}

			else if (pdef->name()->value() == "transition_image") {
				GetDefText(buf, pdef, fn);

				if (buf.length() > 0 && !buf.contains('.'))
					buf.append(".pcx");
			}

			else if (pdef->name()->value() == "picture") {
				GetDefText(buf, pdef, fn);

				if (buf.length() > 0 && !buf.contains('.'))
					buf.append(".pcx");

			}

			else if (pdef->name()->value() == "enabled") {
				bool e;
				GetDefBool(e, pdef, fn);
			}

			else if (pdef->name()->value() == "item") {
				GetDefText(buf, pdef, fn);
			}

			else if (pdef->name()->value() == "tab") {
				int tab = 0;
				GetDefNumber(tab, pdef, fn);
			}

			else if (pdef->name()->value() == "column") {

				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: column structure missing in '%s'"), *FString(fn));
				}
				else {
					TermStruct* val = pdef->term()->isStruct();
				}
			}

			else if (pdef->name()->value() == "orientation") {
				int n;
				GetDefNumber(n, pdef, fn);
			}

			else if (pdef->name()->value() == "leading") {
				int n;
				GetDefNumber(n, pdef, fn);
			}

			else if (pdef->name()->value() == "line_height") {
				int n;
				GetDefNumber(n, pdef, fn);
			}

			else if (pdef->name()->value() == "multiselect") {
				int n;
				GetDefNumber(n, pdef, fn);
			}

			else if (pdef->name()->value() == "dragdrop") {
				int n;
				GetDefNumber(n, pdef, fn);
			}

			else if (pdef->name()->value() == "scroll_bar") {
				int n;
				GetDefNumber(n, pdef, fn);
			}

			else if (pdef->name()->value() == "smooth_scroll") {
				bool b;
				GetDefBool(b, pdef, fn);
			}

			else if (pdef->name()->value() == "picture_loc") {
				int n;
				GetDefNumber(n, pdef, fn);
			}

			else if (pdef->name()->value() == "picture_type") {
				int n;
				GetDefNumber(n, pdef, fn);
			}

			else if (pdef->name()->value() == "style") {
				DWORD s;
				GetDefNumber(s, pdef, fn);
			}

			else if (pdef->name()->value() == "align" ||
				pdef->name()->value() == "text_align") {
				DWORD a = DT_LEFT;

				if (GetDefText(buf, pdef, fn)) {
					if (!_stricmp(buf, "left"))
						a = DT_LEFT;
					else if (!_stricmp(buf, "right"))
						a = DT_RIGHT;
					else if (!_stricmp(buf, "center"))
						a = DT_CENTER;
				}

				else {
					GetDefNumber(a, pdef, fn);
				}
			}

			else if (pdef->name()->value() == "single_line") {
				bool single = false;
				GetDefBool(single, pdef, fn);
			}

			else if (pdef->name()->value() == "bevel_width") {
				DWORD s;
				GetDefNumber(s, pdef, fn);
			}

			else if (pdef->name()->value() == "active") {
				bool b;
				GetDefBool(b, pdef, fn);
			}

			else if (pdef->name()->value() == "animated") {
				bool b;
				GetDefBool(b, pdef, fn);
			}

			else if (pdef->name()->value() == "border") {
				bool b;
				GetDefBool(b, pdef, fn);
			}

			else if (pdef->name()->value() == "drop_shadow") {
				bool b;
				GetDefBool(b, pdef, fn);
			}

			else if (pdef->name()->value() == "show_headings") {
				bool b;
				GetDefBool(b, pdef, fn);
			}

			else if (pdef->name()->value() == "sticky") {
				bool b;
				GetDefBool(b, pdef, fn);
			}

			else if (pdef->name()->value() == "transparent") {
				bool b;
				GetDefBool(b, pdef, fn);
			}

			else if (pdef->name()->value() == "hide_partial") {
				bool b;
				GetDefBool(b, pdef, fn);
			}

			else if (pdef->name()->value() == "num_leds") {
				int n;
				GetDefNumber(n, pdef, fn);
			}

			else if (pdef->name()->value() == "item_style") {
				int n;
				GetDefNumber(n, pdef, fn);
			}

			else if (pdef->name()->value() == "selected_style") {
				int n;
				GetDefNumber(n, pdef, fn);
			}

			else if (pdef->name()->value() == "password") {
				Text password;
				GetDefText(password, pdef, fn);
			}

			// layout constraints:

			else if (pdef->name()->value() == "layout") {

				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: layout structure missing in '%s'"), *FString(fn));
				}
				else {
					ParseLayoutDef(pdef->term()->isStruct(), fn);
				}
			}
		}
	}
}

void AGameDataLoader::ParseLayoutDef(TermStruct* val, const char* fn)
{
	std::vector<DWORD>   x_mins;
	std::vector<DWORD>   y_mins;
	std::vector<float>   x_weights;
	std::vector<float>   y_weights;
	FS_LayoutDef NewLayoutDef;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "x_mins" ||
				pdef->name()->value() == "cols") {
				GetDefArray(x_mins, pdef, fn);
				NewLayoutDef.XMin.SetNum(x_mins.size());
				for (int index = 0; index < x_mins.size(); index++) {
					NewLayoutDef.XMin[index] = x_mins[index];
				}
			}

			else if (pdef->name()->value() == "y_mins" ||
				pdef->name()->value() == "rows") {
				GetDefArray(y_mins, pdef, fn);
				NewLayoutDef.YMin.SetNum(y_mins.size());
				for (int index = 0; index < y_mins.size(); index++) {
					NewLayoutDef.YMin[index] = y_mins[index];
				}
			}

			else if (pdef->name()->value() == "x_weights" ||
				pdef->name()->value() == "col_wts") {
				GetDefArray(x_weights, pdef, fn);
				NewLayoutDef.XWeight.SetNum(x_weights.size());
				for (int index = 0; index < x_weights.size(); index++) {
					NewLayoutDef.XWeight[index] = x_weights[index];
				}
			}

			else if (pdef->name()->value() == "y_weights" ||
				pdef->name()->value() == "row_wts") {
				GetDefArray(y_weights, pdef, fn);
				NewLayoutDef.YWeight.SetNum(y_weights.size());
				for (int index = 0; index < y_weights.size(); index++) {
					NewLayoutDef.YWeight[index] = y_weights[index];
				}
			}
		}
	}
	LayoutDef = NewLayoutDef;
}

void
AGameDataLoader::LoadForm(const char* fn)
{
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
		if (!file_type || file_type->value() != "FORM") {
			UE_LOG(LogTemp, Log, TEXT("Invalid Form File: %s"), *FString(fn));
			return;
		}
	}

	FS_FormDesign NewForm;
	FString FormName = FString(fn);
	FormName.RemoveFromStart(FPaths::ProjectContentDir() + "GameData/Screens/");
	FormName.RemoveFromEnd(".frm");
	NewForm.Name = FormName;

	do {
		delete term;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "form") {

					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: form structure missing in '%s'"), *FString(fn));
					}
					else {
						TermStruct* val = def->term()->isStruct();

						for (int i = 0; i < val->elements()->size(); i++) {
							Text buf;

							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "text" ||
									pdef->name()->value() == "caption") {

									GetDefText(buf, pdef, fn);
									NewForm.Caption = FString(buf);
								}

								else if (pdef->name()->value() == "id") {
									DWORD id;
									GetDefNumber(id, pdef, fn);
									NewForm.Id = id;
								}

								else if (pdef->name()->value() == "pid") {
									DWORD id;
									GetDefNumber(id, pdef, fn);
									NewForm.PId = id;
								}

								else if (pdef->name()->value() == "rect") {
									Rect r;
									GetDefRect(r, pdef, fn);
									NewForm.Rect.X = r.x;
									NewForm.Rect.Y = r.y; 
									NewForm.Rect.Z = r.h; 
									NewForm.Rect.W = r.w;
								}

								else if (pdef->name()->value() == "font") {
									GetDefText(buf, pdef, fn);
									NewForm.Font = FString(buf);
									
								}

								else if (pdef->name()->value() == "back_color") {
									Vec3 c;
									GetDefVec(c, pdef, fn);
									NewForm.BackColor = FColor(c.x, c.y, c.z, 1);
								}

								else if (pdef->name()->value() == "base_color") {
									Vec3 c;
									GetDefVec(c, pdef, fn);
									NewForm.BaseColor = FColor(c.x, c.y, c.z, 1);
								}

								else if (pdef->name()->value() == "fore_color") {
									Vec3 c;
									GetDefVec(c, pdef, fn);
									NewForm.ForeColor = FColor(c.x, c.y, c.z, 1);
								}

								else if (pdef->name()->value() == "margins") {
									Insets m;
									GetDefInsets(m, pdef, fn);
									NewForm.Insets.X = m.left;
									NewForm.Insets.Y = m.right;
									NewForm.Insets.Z = m.top;
									NewForm.Insets.W = m.bottom;
								}

								else if (pdef->name()->value() == "text_insets") {
									Insets t;
									GetDefInsets(t, pdef, fn);
									NewForm.TextInsets.X = t.left;
									NewForm.TextInsets.Y = t.right;
									NewForm.TextInsets.Z = t.top;
									NewForm.TextInsets.W = t.bottom;
								}

								else if (pdef->name()->value() == "cell_insets") {
									Insets c;
									GetDefInsets(c, pdef, fn);
									NewForm.CellInsets.X = c.left;
									NewForm.CellInsets.Y = c.right;
									NewForm.CellInsets.Z = c.top;
									NewForm.CellInsets.W = c.bottom;
								}

								else if (pdef->name()->value() == "cells") {
									Rect c;
									GetDefRect(c, pdef, fn);
									NewForm.Cells.X = c.x;
									NewForm.Cells.Y = c.y;
									NewForm.Cells.Z = c.h;
									NewForm.Cells.W = c.w;
								}

								else if (pdef->name()->value() == "texture") {
									GetDefText(buf, pdef, fn);
									NewForm.Texture = FString(buf);
								}

								else if (pdef->name()->value() == "transparent") {
									bool b;
									GetDefBool(b, pdef, fn);
									NewForm.Transparent = b;	
								}

								else if (pdef->name()->value() == "style") {
									DWORD s;
									GetDefNumber(s, pdef, fn);
									NewForm.Style = s;
								}

								else if (pdef->name()->value() == "align" ||
									pdef->name()->value() == "text_align") {
									DWORD a = DT_LEFT;

									if (GetDefText(buf, pdef, fn)) {
										if (!_stricmp(buf, "left"))
											a = DT_LEFT;
										else if (!_stricmp(buf, "right"))
											a = DT_RIGHT;
										else if (!_stricmp(buf, "center"))
											a = DT_CENTER;
										NewForm.Align = a;
									}

									else {
										GetDefNumber(a, pdef, fn);
										NewForm.Align = a;
									}

								}

								// layout constraints:

								else if (pdef->name()->value() == "layout") {

									if (!pdef->term() || !pdef->term()->isStruct()) {
										UE_LOG(LogTemp, Log, TEXT("WARNING: layout structure missing in '%s'"), *FString(fn));
									}
									else {
										ParseLayoutDef(pdef->term()->isStruct(), fn);
										NewForm.LayoutDef = LayoutDef;
									}
								}

								// controls:

								else if (pdef->name()->value() == "defctrl") {

									if (!pdef->term() || !pdef->term()->isStruct()) {
										UE_LOG(LogTemp, Log, TEXT("WARNING: defctrl structure missing in '%s'"), *FString(fn));
									}
									else {
										TermStruct* dval = pdef->term()->isStruct();
										ParseCtrlDef(pdef->term()->isStruct(), fn);
									}
								}

								else if (pdef->name()->value() == "ctrl") {

									if (!pdef->term() || !pdef->term()->isStruct()) {
										UE_LOG(LogTemp, Log, TEXT("WARNING: ctrl structure missing in '%s'"), *FString(fn));
									}
									else {
										TermStruct* cval = pdef->term()->isStruct();

										//form->AddCtrl(ctrl);
										//*ctrl = form->defctrl;  // copy default params

										//ParseCtrlDef(ctrl, val);
									}
								}

								// end of controls.
							}
						}     // end form params
					}        // end form struct
				}           // end form

				// call AddRow to insert the record
				FormDefDataTable->AddRow(FName(FormName), NewForm);
			}
		}
	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
	//SSWInstance->loader->SetDataPath(0);
}

void
AGameDataLoader::LoadAwardTables()
{
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadAwardTables()"));
	FString ProjectPath = FPaths::ProjectContentDir();
	ProjectPath.Append(TEXT("GameData/Awards/"));
	FString FileName = ProjectPath;

	FileName.Append("awards.def");

	SSWInstance->loader->GetLoader();
	SSWInstance->loader->SetDataPath(FileName);

	//if (!SSWInstance->loader) return;

	//FString fs = FString(ANSI_TO_TCHAR(FileName));
	FString FileString;
	BYTE* block = 0;
	char* fs = TCHAR_TO_ANSI(*FileName);

	SSWInstance->loader->LoadBuffer(fs, block, true);
	UE_LOG(LogTemp, Log, TEXT("Loading Award Info Data: %s"), *FileName);

	if (FFileHelper::LoadFileToString(FileString, *FileName, FFileHelper::EHashOptions::None))
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FileString);

		const char* result = TCHAR_TO_ANSI(*FileString);
	}

	Parser parser(new BlockReader((const char*)block));

	Term* term = parser.ParseTerm();

	if (!term) {
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "AWARDS") {
			return;
		}
	}

	//rank_table.destroy();
	//medal_table.destroy();

	UE_LOG(LogTemp, Log, TEXT("Loading Ranks and Medals"));

	do {
		delete term; term = 0;
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "award") {

					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: award structure missing in '%s'"), *FileName);
					}
					else {
						TermStruct* val = def->term()->isStruct();

						FS_AwardInfo NewAwardData;

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

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == ("name")) {
									GetDefText(AwardName, pdef, filename);
									NewAwardData.AwardName = FString(AwardName);
								}

								else if (pdef->name()->value() == ("desc")) {
									GetDefText(AwardDesc, pdef, filename);
								}

								else if (pdef->name()->value() == ("award")) {
									GetDefText(AwardText, pdef, filename);
								}

								else if (pdef->name()->value() == ("desc_sound"))
									GetDefText(DescSound, pdef, filename);

								else if (pdef->name()->value() == ("award_sound"))
									GetDefText(GrantSound, pdef, filename);

								else if (pdef->name()->value().indexOf("large") == 0) {

									GetDefText(LargeImage, pdef, filename);
									//txt.setSensitive(false);

									//if (!txt.contains(".pcx"))
									//	txt.append(".pcx");

									//loader->CacheBitmap(txt, award->large_insignia);
								}

								else if (pdef->name()->value().indexOf("small") == 0) {
									//	Text txt;
									GetDefText(SmallImage, pdef, filename);
									//txt.setSensitive(false);

									//if (!txt.contains(".pcx"))
									//	txt.append(".pcx");

									//loader->CacheBitmap(txt, award->small_insignia);

									//if (award->small_insignia)
									//	award->small_insignia->AutoMask();
								}

								else if (pdef->name()->value() == ("type")) {

									Text txt;
									GetDefText(txt, pdef, filename);
									txt.setSensitive(false);

									if (txt == "rank")
										AwardType = "rank";

									else if (txt == "medal")
										AwardType = "medal";
								}

								else if (pdef->name()->value() == ("abrv")) {

									Text txt;
									GetDefText(txt, pdef, filename);
									if (AwardType = "rank")
										AwardAbrv = txt;
									else
										AwardAbrv = "";
								}
								else if (pdef->name()->value() == ("id"))
									GetDefNumber(AwardId, pdef, filename);

								else if (pdef->name()->value() == ("total_points"))
									GetDefNumber(TotalPoints, pdef, filename);

								else if (pdef->name()->value() == ("mission_points"))
									GetDefNumber(MissionPoints, pdef, filename);

								else if (pdef->name()->value() == ("total_missions"))
									GetDefNumber(TotalMissions, pdef, filename);

								else if (pdef->name()->value() == ("kills"))
									GetDefNumber(Kills, pdef, filename);

								else if (pdef->name()->value() == ("lost"))
									GetDefNumber(Lost, pdef, filename);

								else if (pdef->name()->value() == ("collision"))
									GetDefNumber(Collision, pdef, filename);

								else if (pdef->name()->value() == ("campaign_id"))
									GetDefNumber(CampaignId, pdef, filename);

								else if (pdef->name()->value() == ("campaign_complete"))
									GetDefBool(CampaignComplete, pdef, filename);

								else if (pdef->name()->value() == ("dynamic_campaign"))
									GetDefBool(DynamicCampaign, pdef, filename);

								else if (pdef->name()->value() == ("ceremony"))
									GetDefBool(Ceremony, pdef, filename);

								else if (pdef->name()->value() == ("required_awards"))
									GetDefNumber(RequiredAwards, pdef, filename);

								else if (pdef->name()->value() == ("lottery"))
									GetDefNumber(Lottery, pdef, filename);

								else if (pdef->name()->value() == ("min_rank"))
									GetDefNumber(MinRank, pdef, filename);

								else if (pdef->name()->value() == ("max_rank"))
									GetDefNumber(MaxRank, pdef, filename);

								else if (pdef->name()->value() == ("min_ship_class")) {
									Text classname;
									GetDefText(classname, pdef, filename);
									MinShipClass = UShip::ClassForName(classname);
								}

								else if (pdef->name()->value() == ("max_ship_class")) {
									Text classname;
									GetDefText(classname, pdef, filename);
									MaxShipClass = UShip::ClassForName(classname);
								}

								else if (pdef->name()->value().indexOf("grant") == 0)
									GetDefNumber(GrantedShipClasses, pdef, filename);
							}

							// define our data table struct

							NewAwardData.AwardId = AwardId;
							NewAwardData.AwardType = FString(AwardType);

							NewAwardData.AwardAbrv = FString(AwardAbrv);
							NewAwardData.AwardDesc = FString(AwardDesc);
							NewAwardData.AwardText = FString(AwardText);
							NewAwardData.DescSound = FString(DescSound);
							NewAwardData.GrantSound = FString(GrantSound);
							NewAwardData.LargeImage = FString(LargeImage);
							NewAwardData.SmallImage = FString(SmallImage);
							NewAwardData.AwardGrant = AwardGrant;
							NewAwardData.RequiredAwards = RequiredAwards;
							NewAwardData.Lottery = Lottery;
							NewAwardData.MinRank = MinRank;
							NewAwardData.MaxRank = MaxRank;
							NewAwardData.MinShipClass = MinShipClass;
							NewAwardData.MaxShipClass = MaxShipClass;
							NewAwardData.GrantedShipClasses = GrantedShipClasses;

							NewAwardData.TotalPoints = TotalPoints;
							NewAwardData.MissionPoints = MissionPoints;
							NewAwardData.TotalMissions = TotalMissions;

							NewAwardData.Kills = Kills;
							NewAwardData.Lost = Lost;
							NewAwardData.Collision = Collision;
							NewAwardData.CampaignId = CampaignId;

							NewAwardData.CampaignComplete = CampaignComplete;
							NewAwardData.DynamicCampaign = DynamicCampaign;
							NewAwardData.Ceremony = Ceremony;

							FName RowName = FName(FString(AwardName));
							// call AddRow to insert the record
							AwardsDataTable->AddRow(RowName, NewAwardData);

							AwardData = NewAwardData;

						}
					}
				}
				else {
					Print("WARNING: unknown label '%s' in '%s'\n",
						def->name()->value().data(), filename);
				}
			}
			else {
				Print("WARNING: term ignored in '%s'\n", filename);

			}

		}


	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
}



