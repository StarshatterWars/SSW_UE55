/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         GameDataLoader.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Master Game Data Loader
*/



#include "GameDataLoader.h"
#include "../System/SSWGameInstance.h"
#include "../System/Game.h"
#include "../Space/Starsystem.h"
#include "CombatGroup.h"
#include "CombatRoster.h"
#include "CombatAction.h"
#include "CombatActionReq.h"
#include "CombatEvent.h"
#include "Combatant.h"
#include "Mission.h"
//#include "Intel.h"
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

}

// Called when the game starts or when spawned
void AGameDataLoader::BeginPlay()
{
	Super::BeginPlay();
	GetSSWInstance();
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
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::LoadCampaignData"));

	FString ProjectPath = FPaths::ProjectDir();
	ProjectPath.Append(TEXT("GameData/Campaigns/"));
	FString FileName = ProjectPath;

	for (int i = 1; i < 6; i++) {		
		FileName = ProjectPath; 
		FileName.Append("0");
		FileName.Append(FString::FormatAsNumber(i));
		FileName.Append("/");
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

	TArray<FS_Combatant> CombatantArray;
	CombatantArray.Empty();

	CampaignActionArray.Empty();

	do {
		delete term; 
		term = parser.ParseTerm();

		if (term) {
			TermDef* def = term->isDef();
			if (def) {


				UE_LOG(LogTemp, Log, TEXT("%s"), *FString(def->name()->value()));

				if (def->name()->value() == "name") {
					if (!def->term() || !def->term()->isText()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: name missing in '%s'"), *FString(FileName));
					}
					else {
						GetDefText(name, def, filename);
					}
				}
				else if (def->name()->value() == "desc") {
					if (!def->term() || !def->term()->isText()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: description missing in '%s'"), *FString(FileName));
					}
					else {
						GetDefText(description, def, filename);
					}
				}
				else if (def->name()->value() == "situation") {
					if (!def->term() || !def->term()->isText()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: situation missing in '%s'"), *FString(FileName)); 
					}
					else {
						GetDefText(situation, def, filename);
					}
				}
				else if (def->name()->value() == "orders") {
					if (!def->term() || !def->term()->isText()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: orders missing in '%s'"), *FString(FileName));
					}
					else {
						GetDefText(orders, def, filename);
						OrdersArray.Add(FString(orders));
					}
				}
				else if (def->name()->value() == "scripted") {
					if (def->term() && def->term()->isBool()) {
						scripted = def->term()->isBool()->value();
					}
				}
				else if (def->name()->value() == "sequential") {
					if (def->term() && def->term()->isBool()) {
						sequential = def->term()->isBool()->value();
					}
				}


				else if (full && def->name()->value() == "combatant") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: combatant struct missing in '%s'"), *FString(FileName));
					}
					else {	

						TermStruct* val = def->term()->isStruct();

						char           action_name[64];
						int            iff = 0;
						CombatGroup* force = 0;
						CombatGroup* clone = 0;

						for (int i = 0; i < val->elements()->size(); i++) {
							
							FS_Combatant CombatantEntry;

							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "name") {
									GetDefText(action_name, pdef, filename);
									UE_LOG(LogTemp, Log, TEXT("Combatant name is '%s'"), *FString(action_name));
									
									//force = CombatRoster::GetInstance()->GetForce(name);

									//if (force)
									//	clone = force->Clone(false); // shallow copy
								}

								//else if (pdef->name()->value() == "group") {
								//	ParseGroup(pdef->term()->isStruct(), force, clone, filename);
								//}
			
							}
						}

						SSWInstance->loader->SetDataPath(FileName);
						Combatant* c = new Combatant(name, clone);
						if (c) {
							combatants.append(c);
						}
						else {
							Unload();
							return;
						}

					}
				}
				else if (full && def->name()->value() == "action") {
					if (!def->term() || !def->term()->isStruct()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: action struct missing in '%s'"), *FString(FileName));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseAction(val, FileName);
					}
				}

				// define our data table struct
				
				NewCampaignData.Name = FString(name);
				NewCampaignData.Description = FString(description);
				NewCampaignData.Situation = FString(situation);
				NewCampaignData.Orders = OrdersArray;
			
				NewCampaignData.Scripted = scripted;
				NewCampaignData.Sequential = sequential;
				NewCampaignData.Combatant = CombatantArray;
				NewCampaignData.Action = CampaignActionArray;
				FName RowName = FName(FString(name));

				// call AddRow to insert the record
				CampaignDataTable->AddRow(RowName, NewCampaignData);

				CampaignData = NewCampaignData;

			}
		}
	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
}

// +--------------------------------------------------------------------+

void
AGameDataLoader::ParseGroup(TermStruct* val,
	CombatGroup* force,
	CombatGroup* clone,
	const char* grp_fn)
{
	if (!val) {
		::Print("invalid combat group in campaign %s\n", name.data());
		return;
	}

	int   type = 0;
	int   id = 0;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "type") {
				char type_name[64];
				GetDefText(type_name, pdef, grp_fn);
				type = CombatGroup::TypeFromName(type_name);
			}

			else if (pdef->name()->value() == "id") {
				GetDefNumber(id, pdef, grp_fn);
			}
		}
	}

	if (type && id) {
		CombatGroup* g = force->FindGroup(type, id);

		// found original group, now clone it over
		if (g && g->GetParent()) {
			CombatGroup* parent = CloneOver(force, clone, g->GetParent());
			parent->AddComponent(g->Clone());
		}
	}
}

// +--------------------------------------------------------------------+

void
AGameDataLoader::ParseAction(TermStruct* val, const char* fn)
{
	if (!val) {
		::Print("invalid action in campaign %s\n", name.data());
		return;
	}

	int   id = 0;
	int   type = 0;
	int   subtype = 0;
	int   opp_type = -1;
	int   team = 0;
	int   source = 0;
	Vec3  loc(0.0f, 0.0f, 0.0f);
	Text  system;
	Text  region;
	Text  file;
	Text  image;
	Text  scene;
	Text  text;
	int   count = 1;
	int   start_before = Game::TIME_NEVER;
	int   start_after = 0;
	int   min_rank = 0;
	int   max_rank = 100;
	int   delay = 0;
	int   probability = 100;

	int   asset_type = 0;
	int   asset_id = 0;
	int   target_type = 0;
	int   target_id = 0;
	int   target_iff = 0;

	CombatAction* action = 0;


	for (int i = 0; i < val->elements()->size(); i++) {
		
		FS_CampaignAction NewCampaignAction;

		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "id") {
				GetDefNumber(id, pdef, fn);
				NewCampaignAction.Id = id;
			}
			else if (pdef->name()->value() == "type") {
				char txt[64];
				GetDefText(txt, pdef, fn);
				type = CombatAction::TypeFromName(txt);
				NewCampaignAction.Type = FString(txt);
			}
			else if (pdef->name()->value() == "subtype") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(subtype, pdef, fn);
					NewCampaignAction.Subtype = subtype;
				}

				else if (pdef->term()->isText()) {
					char txt[64];
					GetDefText(txt, pdef, fn);

					if (type == CombatAction::MISSION_TEMPLATE) {
						subtype = Mission::TypeFromName(txt);
					}
					else if (type == CombatAction::COMBAT_EVENT) {
						subtype = CombatEvent::TypeFromName(txt);
					}
					else if (type == CombatAction::INTEL_EVENT) {
						//subtype = Intel::IntelFromName(txt);
					}
					NewCampaignAction.Subtype = subtype;
				}
				
			}
			else if (pdef->name()->value() == "opp_type") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(opp_type, pdef, fn);
				}

				else if (pdef->term()->isText()) {
					char txt[64];
					GetDefText(txt, pdef, fn);

					if (type == CombatAction::MISSION_TEMPLATE) {
						opp_type = Mission::TypeFromName(txt);
					}
				}
			}
			else if (pdef->name()->value() == "source") {
				char txt[64];
				GetDefText(txt, pdef, fn);
				source = CombatEvent::SourceFromName(txt);
			}
			else if (pdef->name()->value() == "team") {
				GetDefNumber(team, pdef, fn);
			}
			else if (pdef->name()->value() == "iff") {
				GetDefNumber(team, pdef, fn);
			}
			else if (pdef->name()->value() == "count") {
				GetDefNumber(count, pdef, fn);
			}
			else if (pdef->name()->value().contains("before")) {
				if (pdef->term()->isNumber()) {
					GetDefNumber(start_before, pdef, fn);
				}
				else {
					GetDefTime(start_before, pdef, fn);
					start_before -= Game::ONE_DAY;
				}
			}
			else if (pdef->name()->value().contains("after")) {
				if (pdef->term()->isNumber()) {
					GetDefNumber(start_after, pdef, fn);
				}
				else {
					GetDefTime(start_after, pdef, fn);
					start_after -= Game::ONE_DAY;
				}
			}
			else if (pdef->name()->value() == "min_rank") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(min_rank, pdef, fn);
				}
				else {
					char rank_name[64];
					GetDefText(rank_name, pdef, fn);
					min_rank = PlayerData::RankFromName(rank_name);
				}
			}
			else if (pdef->name()->value() == "max_rank") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(max_rank, pdef, fn);
				}
				else {
					char rank_name[64];
					GetDefText(rank_name, pdef, fn);
					max_rank = PlayerData::RankFromName(rank_name);
				}
			}
			else if (pdef->name()->value() == "delay") {
				GetDefNumber(delay, pdef, fn);
			}
			else if (pdef->name()->value() == "probability") {
				GetDefNumber(probability, pdef, fn);
			}
			else if (pdef->name()->value() == "asset_type") {
				char type_name[64];
				GetDefText(type_name, pdef, fn);
				asset_type = CombatGroup::TypeFromName(type_name);
			}
			else if (pdef->name()->value() == "target_type") {
				char type_name[64];
				GetDefText(type_name, pdef, fn);
				target_type = CombatGroup::TypeFromName(type_name);
			}
			else if (pdef->name()->value() == "location" ||
				pdef->name()->value() == "loc") {
				GetDefVec(loc, pdef, fn);
			}
			else if (pdef->name()->value() == "system" ||
				pdef->name()->value() == "sys") {
				GetDefText(system, pdef, fn);
			}
			else if (pdef->name()->value() == "region" ||
				pdef->name()->value() == "rgn" ||
				pdef->name()->value() == "zone") {
				GetDefText(region, pdef, fn);
			}
			else if (pdef->name()->value() == "file") {
				GetDefText(file, pdef, fn);
			}
			else if (pdef->name()->value() == "image") {
				GetDefText(image, pdef, fn);
			}
			else if (pdef->name()->value() == "scene") {
				GetDefText(scene, pdef, fn);
			}
			else if (pdef->name()->value() == "text") {
				GetDefText(text, pdef, fn);
				text = Game::GetText(text);
			}
			else if (pdef->name()->value() == "asset_id") {
				GetDefNumber(asset_id, pdef, fn);
			}
			else if (pdef->name()->value() == "target_id") {
				GetDefNumber(target_id, pdef, fn);
			}
			else if (pdef->name()->value() == "target_iff") {
				GetDefNumber(target_iff, pdef, fn);
			}

			else if (pdef->name()->value() == "asset_kill") {
				if (!action)
					action = new CombatAction(id, type, subtype, team);

				if (action) {
					char txt[64];
					GetDefText(txt, pdef, fn);
					action->AssetKills().append(new Text(txt));
				}
			}

			else if (pdef->name()->value() == "target_kill") {
				if (!action)
					action = new CombatAction(id, type, subtype, team);

				if (action) {
					char txt[64];
					GetDefText(txt, pdef, fn);
					action->TargetKills().append(new Text(txt));
				}
			}

			else if (pdef->name()->value() == "req") {
				if (!action)
					action = new CombatAction(id, type, subtype, team);

				if (!pdef->term() || !pdef->term()->isStruct()) {
					UE_LOG(LogTemp, Log, TEXT("WARNING: action req struct missing in '%s'"), *FString(fn)); 

				}
				else if (action) {
					TermStruct* val2 = pdef->term()->isStruct();

					int  act = 0;
					int  stat = CombatAction::COMPLETE;
					bool not_action = false;

					Combatant* c1 = 0;
					Combatant* c2 = 0;
					int         comp = 0;
					int         score = 0;
					int         intel = 0;
					int         gtype = 0;
					int         gid = 0;

					for (int index = 0; index < val2->elements()->size(); index++) {
						TermDef* pdef2 = val2->elements()->at(i)->isDef();
						if (pdef2) {
							if (pdef2->name()->value() == "action") {
								GetDefNumber(act, pdef2, filename);
							}
							else if (pdef2->name()->value() == "status") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								stat = CombatAction::StatusFromName(txt);
							}
							else if (pdef2->name()->value() == "not") {
								GetDefBool(not_action, pdef2, filename);
							}

							else if (pdef2->name()->value() == "c1") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								c1 = GetCombatant(txt);
							}
							else if (pdef2->name()->value() == "c2") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								c2 = GetCombatant(txt);
							}
							else if (pdef2->name()->value() == "comp") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								comp = CombatActionReq::CompFromName(txt);
							}
							else if (pdef2->name()->value() == "score") {
								GetDefNumber(score, pdef2, filename);
							}
							else if (pdef2->name()->value() == "intel") {
								if (pdef2->term()->isNumber()) {
									GetDefNumber(intel, pdef2, filename);
								}
								else if (pdef2->term()->isText()) {
									char txt[64];
									GetDefText(txt, pdef2, filename);
									//intel = Intel::IntelFromName(txt);
								}
							}
							else if (pdef2->name()->value() == "group_type") {
								char type_name[64];
								GetDefText(type_name, pdef2, filename);
								gtype = CombatGroup::TypeFromName(type_name);
							}
							else if (pdef2->name()->value() == "group_id") {
								GetDefNumber(gid, pdef2, filename);
							}
						}
					}

					if (act)
						action->AddRequirement(act, stat, not_action);

					else if (gtype)
						action->AddRequirement(c1, gtype, gid, comp, score, intel);

					else
						action->AddRequirement(c1, c2, comp, score);
				}
			}
		}
		CampaignActionArray.Add(NewCampaignAction);
	}

	if (!action)
		action = new CombatAction(id, type, subtype, team);

	if (action) {
		action->SetSource(source);
		action->SetOpposingType(opp_type);
		action->SetLocation(loc);
		action->SetSystem(system);
		action->SetRegion(region);
		action->SetFilename(file);
		action->SetImageFile(image);
		action->SetSceneFile(scene);
		action->SetCount(count);
		action->SetStartAfter(start_after);
		action->SetStartBefore(start_before);
		action->SetMinRank(min_rank);
		action->SetMaxRank(max_rank);
		action->SetDelay(delay);
		action->SetProbability(probability);

		action->SetAssetId(asset_id);
		action->SetAssetType(asset_type);
		action->SetTargetId(target_id);
		action->SetTargetIFF(target_iff);
		action->SetTargetType(target_type);
		action->SetText(text);

		actions.append(action);
	}
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
