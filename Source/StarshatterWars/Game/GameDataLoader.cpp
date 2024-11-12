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
#include "CombatGroup.h"
#include "CombatRoster.h"
#include "Combatant.h"


// Sets default values
AGameDataLoader::AGameDataLoader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	UE_LOG(LogTemp, Log, TEXT("AGameDataLoader::AGameDataLoader()"));
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
		LoadCampaignData(TCHAR_TO_ANSI(*FileName));
	}
}

void AGameDataLoader::LoadCampaignData(const char* FileName)
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
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "CAMPAIGN") {
			return;
		}
	}

	do {
		delete term; term = 0;
		term = parser.ParseTerm();

		/*if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "name") {
					if (!def->term() || !def->term()->isText()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: name missing in '%s'"), *FString(FileName));
					}
					else {
						name = def->term()->isText()->value();
						name = Game::GetText(name);
					}
				}
				else if (def->name()->value() == "desc") {
					if (!def->term() || !def->term()->isText()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: description missing in '%s'"), *FString(FileName));
					}
					else {
						description = def->term()->isText()->value();
						description = Game::GetText(description);
					}
				}
				else if (def->name()->value() == "situation") {
					if (!def->term() || !def->term()->isText()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: situation missing in '%s'"), *FString(FileName)); 
					}
					else {
						situation = def->term()->isText()->value();
						situation = Game::GetText(situation);
					}
				}
				else if (def->name()->value() == "orders") {
					if (!def->term() || !def->term()->isText()) {
						UE_LOG(LogTemp, Log, TEXT("WARNING: orders missing in '%s'"), *FString(FileName));
					}
					else {
						orders = def->term()->isText()->value();
						orders = Game::GetText(orders);
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

						char           camp_name[64];
						int            iff = 0;
						CombatGroup* force = 0;
						CombatGroup* clone = 0;

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "name") {
									GetDefText(camp_name, pdef, filename);

									force = CombatRoster::GetInstance()->GetForce(name);

									if (force)
										clone = force->Clone(false); // shallow copy
								}

								else if (pdef->name()->value() == "group") {
									ParseGroup(pdef->term()->isStruct(), force, clone, filename);
								}
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
						UE_LOG(LogTemp, Log, TEXT("WARNING: action struct missing in '%s '%s'"), *FString(FileName));
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseAction(val, filename);
					}
				}
			}
		}*/
	} while (term);

	SSWInstance->loader->ReleaseBuffer(block);
}

