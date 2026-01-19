/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Game
	FILE:         AwardInfoLoader.cpp
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Loader and Parser class for initial generation of the Award Info Data Table
	Will notxbe used after Data Table is Generated.
*/


#include "AwardInfoLoader.h"
#include "Ship.h"




// Sets default values
AAwardInfoLoader::AAwardInfoLoader()
{
 	
	UE_LOG(LogTemp, Log, TEXT("AAwardInfoLoader::AAwardInfoLoader()"));
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UDataTable> AwardDataTableObject(TEXT("DataTable'/Game/Game/DT_AwardInfo.DT_AwardInfo'"));

	if (AwardDataTableObject.Succeeded())
	{
		AwardsDataTable = AwardDataTableObject.Object;
		//GalaxyDataTable->EmptyTable();
	}

}

// Called when the game starts or when spawned
void AAwardInfoLoader::BeginPlay()
{
	Super::BeginPlay();
	GetSSWInstance();
	LoadAwardTables();
	
}

// Called every frame
void AAwardInfoLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// +-------------------------------------------------------------------+

void AAwardInfoLoader::GetSSWInstance()
{
	SSWInstance = (USSWGameInstance*)GetGameInstance();
}

void
AAwardInfoLoader::LoadAwardTables()
{
	UE_LOG(LogTemp, Log, TEXT("AAwardInfoLoader::LoadAwardTables()")); 
	FString ProjectPath = FPaths::ProjectDir();
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
									if(AwardType = "rank")
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

